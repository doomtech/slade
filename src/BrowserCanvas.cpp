
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    BrowserCanvas.cpp
 * Description: The OpenGL canvas for displaying browser items. Also
 *              keeps track of a vertical scrollbar to scroll through
 *              the items it contains
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WxStuff.h"
#include "BrowserCanvas.h"


/*******************************************************************
 * INCLUDES
 *******************************************************************/
CVAR(Bool, browser_bg_black, false, CVAR_SAVE)
CVAR(Int, browser_item_size, 96, CVAR_SAVE)


/*******************************************************************
 * BROWSERCANVAS CLASS FUNCTIONS
 *******************************************************************/

/* BrowserCanvas::BrowserCanvas
 * BrowserCanvas class constructor
 *******************************************************************/
BrowserCanvas::BrowserCanvas(wxWindow* parent) : OGLCanvas(parent, -1) {
	// Init variables
	yoff = 0;
	item_border = 8;
	scrollbar = NULL;
	item_selected = 0;

	// Bind events
	Bind(wxEVT_SIZE, &BrowserCanvas::onSize, this);
	Bind(wxEVT_MOUSEWHEEL, &BrowserCanvas::onMouseEvent, this);
	Bind(wxEVT_LEFT_DOWN, &BrowserCanvas::onMouseEvent, this);
	Bind(wxEVT_KEY_DOWN, &BrowserCanvas::onKeyDown, this);
	Bind(wxEVT_CHAR, &BrowserCanvas::onKeyChar, this);
}

/* BrowserCanvas::~BrowserCanvas
 * BrowserCanvas class destructor
 *******************************************************************/
BrowserCanvas::~BrowserCanvas() {
}

/* BrowserCanvas::fullItemSizeX
 * Returns the 'full' (including border) width of each item
 *******************************************************************/
int BrowserCanvas::fullItemSizeX() {
	return browser_item_size + (item_border*2);
}

/* BrowserCanvas::fullItemSizeY
 * Returns the 'full' (including border and row gap) height of each
 * item
 *******************************************************************/
int BrowserCanvas::fullItemSizeY() {
	return browser_item_size + (item_border*2) + 16;
}

/* BrowserCanvas::draw
 * Handles drawing of the canvas content
 *******************************************************************/
void BrowserCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Translate to inside of pixel (otherwise inaccuracies can occur on certain gl implementations)
	glTranslatef(0.375f, 0.375f, 0);

	// Draw background if required
	if (!browser_bg_black)
		drawCheckeredBackground();

	// Init for texture drawing
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw items
	int x = item_border;
	int y = item_border;
	top_index = -1;
	for (unsigned a = 0; a < items_filter.size(); a++) {
		// If we're not yet into the viewable area, skip
		if (y < yoff - fullItemSizeY()) {
			x += fullItemSizeX();
			if (x - item_border > GetSize().x - fullItemSizeX()) {
				x = item_border;
				y += fullItemSizeY();

				// Canvas is filled, stop drawing
				if (y > yoff + GetSize().y)
					break;
			}
			continue;
		}

		// If we're drawing the first non-hidden item, save it
		if (top_index < 0) {
			top_index = a;
			top_y = y - yoff;
		}

		// Draw item
		items[items_filter[a]]->draw(browser_item_size, x, y - yoff);

		// Draw selection box if selected
		if (item_selected == a) {
			// Setup
			glDisable(GL_TEXTURE_2D);
			glColor4f(0.3f, 0.5f, 1.0f, 0.3f);
			glPushMatrix();
			glTranslated(x, y - yoff, 0);
			glTranslated(-item_border, -item_border, 0);

			// Selection background
			glBegin(GL_QUADS);
			glVertex2i(2, 2);
			glVertex2i(2, fullItemSizeY()-3);
			glVertex2i(fullItemSizeX()-3, fullItemSizeY()-3);
			glVertex2i(fullItemSizeX()-3, 2);
			glEnd();

			// Selection border
			glColor4f(0.6f, 0.8f, 1.0f, 1.0f);
			glBegin(GL_LINE_LOOP);
			glVertex2i(2, 2);
			glVertex2i(2, fullItemSizeY()-3);
			glVertex2i(fullItemSizeX()-3, fullItemSizeY()-3);
			glVertex2i(fullItemSizeX()-3, 2);
			glEnd();

			// Finish
			glPopMatrix();
			glEnable(GL_TEXTURE_2D);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// Move over for next item
		x += fullItemSizeX();
		if (x - item_border > GetSize().x - fullItemSizeX()) {
			x = item_border;
			y += fullItemSizeY();

			// Canvas is filled, stop drawing
			if (y > yoff + GetSize().y)
				break;
		}
	}

	// Swap Buffers
	SwapBuffers();
}

/* BrowserCanvas::setScrollBar
 * Sets this canvas' associated vertical scrollbar
 *******************************************************************/
void BrowserCanvas::setScrollBar(wxScrollBar* scrollbar) {
	// Set scrollbar
	this->scrollbar = scrollbar;

	// Bind events
	scrollbar->Bind(wxEVT_SCROLL_THUMBTRACK, &BrowserCanvas::onScrollThumbTrack, this);
	scrollbar->Bind(wxEVT_SCROLL_LINEUP, &BrowserCanvas::onScrollLineUp, this);
	scrollbar->Bind(wxEVT_SCROLL_LINEDOWN, &BrowserCanvas::onScrollLineDown, this);
	scrollbar->Bind(wxEVT_SCROLL_PAGEUP, &BrowserCanvas::onScrollPageUp, this);
	scrollbar->Bind(wxEVT_SCROLL_PAGEDOWN, &BrowserCanvas::onScrollPageDown, this);
}

/* BrowserCanvas::updateScrollBar
 * Updates the associated scrollbar's properties depending on the
 * number of items, the canvas size, etc.
 *******************************************************************/
void BrowserCanvas::updateScrollBar() {
	// Do nothing special if no scrollbar present
	if (!scrollbar)
		return;

	// Determine total height of all items
	int items_row = GetSize().x / fullItemSizeX();
	int rows = (double)items_filter.size() / (double)items_row + 0.9999;
	int total_height = rows * fullItemSizeY();

	// Setup scrollbar
	scrollbar->SetScrollbar(scrollbar->GetThumbPosition(), GetSize().y, total_height, GetSize().y);
	yoff = scrollbar->GetThumbPosition();
}

/* BrowserCanvas::getSelectedItem
 * Returns the currently selected BrowserItem, or NULL if nothing is
 * selected
 *******************************************************************/
BrowserItem* BrowserCanvas::getSelectedItem() {
	// Check selected index
	if (item_selected < 0 || item_selected >= (int)items_filter.size())
		return NULL;

	return items[items_filter[item_selected]];
}

/* BrowserCanvas::selectItem
 * Selects the item at [index]
 *******************************************************************/
void BrowserCanvas::selectItem(int index) {
	if (index < 0 || index >= (int)items_filter.size())
		return;

	item_selected = index;
}

/* BrowserCanvas::filterItems
 * Filters the visible items by [filter], by name
 *******************************************************************/
void BrowserCanvas::filterItems(string filter) {
	// Clear current filter list
	items_filter.clear();

	// If the filter is empty, just add all items to the filter
	if (filter.IsEmpty()) {
		for (unsigned a = 0; a < items.size(); a++)
			items_filter.push_back(a);
	}
	else {
		// Setup filter string
		filter.MakeLower();
		filter += "*";

		// Go through items
		for (unsigned a = 0; a < items.size(); a++) {
			// Add to filter list if name matches
			if (items[a]->getName().Lower().Matches(filter))
				items_filter.push_back(a);
		}
	}

	// Update scrollbar and refresh
	updateScrollBar();
	Refresh();
}

/* BrowserCanvas::showItem
 * Scrolls the view to show [item] if it is currently off-screen. If
 * [top] is true, the item will be shown on the top row, otherwise,
 * the item will be shown on the bottom row
 *******************************************************************/
void BrowserCanvas::showItem(int item, bool top) {
	// Check item index
	if (item < 0 || item >= (int)items_filter.size())
		return;

	// Determine y-position of item
	int num_cols = GetSize().x / fullItemSizeX();
	int y_top = (item / num_cols) * fullItemSizeY();
	int y_bottom = y_top + fullItemSizeY();

	// Check if item is above current view
	if (y_top < yoff || y_bottom > yoff + GetSize().y) {
		if (top) {
			// Scroll view to show the item on the top row
			yoff = y_top;
			if (scrollbar) scrollbar->SetThumbPosition(yoff);
		}
		else {
			// Scroll view to show the item on the bottom row
			yoff = y_bottom - GetSize().y;
			if (scrollbar) scrollbar->SetThumbPosition(yoff);
		}
	}
}

/* BrowserCanvas::lookForSearchEntryFrom
 * Used by BrowserCanvas::onKeyChar, returns true if an item matching
 * [search] is found (starting from [from]), false otherwise
 *******************************************************************/
bool BrowserCanvas::searchItemFrom(int from) {
	int index = from;
	bool looped = false;
	bool gotmatch = false;
	while ((!looped && index < (int)items_filter.size()) || (looped && index < from)) {
		string name = items[items_filter[index]]->getName();
		if (name.Upper().StartsWith(search)) {
			// Matches, update selection
			item_selected = index;
			showItem(index);
			return true;
		}

		// No match, next item; look in the above entries
		// if no matches were found below.
		if (++index == items_filter.size() && !looped) {
			looped = true;
			index = 0;
		}
	}
	// Didn't get any match
	return false;
}


/*******************************************************************
 * BROWSERCANVAS CLASS EVENTS
 *******************************************************************/

/* BrowserCanvas::onSize
 * Called when the canvas is resized
 *******************************************************************/
void BrowserCanvas::onSize(wxSizeEvent& e) {
	updateScrollBar();

	// Do default stuff
	e.Skip();
}

/* BrowserCanvas::onScrollThumbTrack
 * Called when the scrollbar 'thumb' is moved
 *******************************************************************/
void BrowserCanvas::onScrollThumbTrack(wxScrollEvent& e) {
	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

/* BrowserCanvas::onScrollLineUp
 * Called when the scrollbar recieves a 'line up' command (ie when
 * the up arrow is clicked)
 *******************************************************************/
void BrowserCanvas::onScrollLineUp(wxScrollEvent& e) {
	// Scroll up by one row
	scrollbar->SetThumbPosition(yoff - fullItemSizeY());

	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

/* BrowserCanvas::onScrollLineDown
 * Called when the scrollbar recieves a 'line down' command (ie when
 * the down arrow is clicked)
 *******************************************************************/
void BrowserCanvas::onScrollLineDown(wxScrollEvent& e) {
	// Scroll down by one row
	scrollbar->SetThumbPosition(yoff + fullItemSizeY());

	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

/* BrowserCanvas::onScrollPageUp
 * Called when the scrollbar recieves a 'page up' command (ie when
 * the area above the 'thumb' is clicked)
 *******************************************************************/
void BrowserCanvas::onScrollPageUp(wxScrollEvent& e) {
	// Scroll up by one screen
	scrollbar->SetThumbPosition(yoff - GetSize().y);

	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

/* BrowserCanvas::onScrollPageDown
 * Called when the scrollbar recieves a 'page down' command (ie when
 * the area below the 'thumb' is clicked)
 *******************************************************************/
void BrowserCanvas::onScrollPageDown(wxScrollEvent& e) {
	// Scroll down by one screen
	scrollbar->SetThumbPosition(yoff + GetSize().y);

	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

/* BrowserCanvas::onMouseEvent
 * Called when any mouse event is generated (click, scroll, etc)
 *******************************************************************/
void BrowserCanvas::onMouseEvent(wxMouseEvent& e) {
	// --- Scroll wheel ---
	if (e.GetEventType() == wxEVT_MOUSEWHEEL) {
		// Detemine the scroll multiplier
		float scroll_mult = (float)e.GetWheelRotation() / (float)e.GetWheelDelta();

		// Scrolling by 1.0 means by 1 row
		int scroll_amount = (fullItemSizeY()) * -scroll_mult;

		// Do scroll
		scrollbar->SetThumbPosition(yoff + scroll_amount);

		// Update y-offset and refresh
		yoff = scrollbar->GetThumbPosition();
		Refresh();
	}

	// --- Left click ---
	else if (e.GetEventType() == wxEVT_LEFT_DOWN) {
		// Clear selection
		item_selected = -1;

		// Get column clicked & number of columns
		int col = e.GetPosition().x / fullItemSizeX();
		int num_cols = GetSize().x / fullItemSizeX();

		// Get row clicked
		int row = (e.GetPosition().y - top_y) / (fullItemSizeY());

		// Select item
		item_selected = top_index + (row * num_cols) + col;
		Refresh();
	}

	e.Skip();
}

/* BrowserCanvas::onMouseEvent
 * Called when a key is pressed within the canvas
 *******************************************************************/
void BrowserCanvas::onKeyDown(wxKeyEvent& e) {
	bool handled = true;
	int num_cols = GetSize().x / fullItemSizeX();

	// Down arrow
	if (e.GetKeyCode() == WXK_DOWN) {
		item_selected += num_cols;
		showItem(item_selected, false);
	}

	// Up arrow
	else if (e.GetKeyCode() == WXK_UP) {
		item_selected -= num_cols;
		showItem(item_selected);
	}

	// Left arrow
	else if (e.GetKeyCode() == WXK_LEFT) {
		item_selected--;
		showItem(item_selected);
	}

	// Right arrow
	else if (e.GetKeyCode() == WXK_RIGHT) {
		item_selected++;
		showItem(item_selected, false);
	}

	else {
		e.Skip();
		handled = false;
	}

	if (handled) {
		// Clamp selection
		if (item_selected >= (int)items_filter.size())
			item_selected = (int)items_filter.size() - 1;
		if (item_selected < 0)
			item_selected = 0;

		// Refresh canvas
		Refresh();
	}
}

/* BrowserCanvas::onKeyChar
 * Called when a 'character' key is pressed within the canvas
 *******************************************************************/
int bc_chars[] = {
	'.', ',', '_', '-', '+', '=', '`', '~',
	'!', '@', '#', '$', '(', ')', '[', ']',
	'{', '}', ':', ';', '/', '\\', '<', '>',
	'?', '^', '&', '\'', '\"',
};
int n_bc_chars = 30;
void BrowserCanvas::onKeyChar(wxKeyEvent& e) {
	// Check the key pressed is actually a character (a-z, 0-9 etc)
	bool isRealChar = false;
	if (e.GetKeyCode() >= 'a' && e.GetKeyCode() <= 'z')			// Lowercase
		isRealChar = true;
	else if (e.GetKeyCode() >= 'A' && e.GetKeyCode() <= 'Z')	// Uppercase
		isRealChar = true;
	else if (e.GetKeyCode() >= '0' && e.GetKeyCode() <= '9')	// Number
		isRealChar = true;
	else {
		for (int a = 0; a < n_bc_chars; a++) {
			if (e.GetKeyCode() == bc_chars[a]) {
				isRealChar = true;
				break;
			}
		}
	}

	if (isRealChar) {
		// Get currently selected item (or first if nothing is focused)
		int selected = item_selected;
		if (selected < 0) selected = 0;

		// Build search string
		search += e.GetKeyCode();
		search.MakeUpper();

		// Search for match from the current focus, and if failed
		// start a new search from after the current focus.
		if (!searchItemFrom(selected)) {
			search = S_FMT("%c", e.GetKeyCode());
			search.MakeUpper();
			searchItemFrom(selected+1);
		}

		// Refresh canvas
		Refresh();
	}
	else {
		search = "";
		e.Skip();
	}
}
