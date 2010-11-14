
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
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
 * BROWSERCANVAS CLASS FUNCTIONS
 *******************************************************************/

/* BrowserCanvas::BrowserCanvas
 * BrowserCanvas class constructor
 *******************************************************************/
BrowserCanvas::BrowserCanvas(wxWindow* parent) : OGLCanvas(parent, -1) {
	// Init variables
	yoff = 0;
	item_size = 96;
	item_border = 8;
	scrollbar = NULL;
	item_selected = 0;

	// Bind events
	Bind(wxEVT_SIZE, &BrowserCanvas::onSize, this);
	Bind(wxEVT_MOUSEWHEEL, &BrowserCanvas::onMouseEvent, this);
	Bind(wxEVT_LEFT_DOWN, &BrowserCanvas::onMouseEvent, this);
}

/* BrowserCanvas::~BrowserCanvas
 * BrowserCanvas class destructor
 *******************************************************************/
BrowserCanvas::~BrowserCanvas() {
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

	// Translate to middle of pixel (otherwise inaccuracies can occur on certain gl implemenataions)
	glTranslatef(0.375f, 0.375f, 0);

	// Init for texture drawing
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw items
	int x = item_border;
	int y = item_border;
	top_index = -1;
	for (unsigned a = 0; a < items_filter.size(); a++) {
		// If we're not yet into the viewable area, skip
		if (y < yoff - fullItemSize()) {
			x += fullItemSize();
			if (x - item_border > GetSize().x - fullItemSize()) {
				x = item_border;
				y += fullItemSize();

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
		glPushMatrix();
		glTranslated(x, y - yoff, 0);
		items[items_filter[a]]->draw(item_size);

		// Draw selection box if selected
		if (item_selected == a) {
			// Setup
			glDisable(GL_TEXTURE_2D);
			glColor4f(0.3f, 0.5f, 1.0f, 0.3f);
			glPushMatrix();
			glTranslated(-item_border, -item_border, 0);

			// Selection background
			glBegin(GL_QUADS);
			glVertex2i(2, 2);
			glVertex2i(2, fullItemSize()-3);
			glVertex2i(fullItemSize()-3, fullItemSize()-3);
			glVertex2i(fullItemSize()-3, 2);
			glEnd();

			// Selection border
			glColor4f(0.6f, 0.8f, 1.0f, 1.0f);
			glBegin(GL_LINE_LOOP);
			glVertex2i(2, 2);
			glVertex2i(2, fullItemSize()-3);
			glVertex2i(fullItemSize()-3, fullItemSize()-3);
			glVertex2i(fullItemSize()-3, 2);
			glEnd();

			// Finish
			glPopMatrix();
			glEnable(GL_TEXTURE_2D);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}

		glPopMatrix();

		// Move over for next item
		x += fullItemSize();
		if (x - item_border > GetSize().x - fullItemSize()) {
			x = item_border;
			y += fullItemSize();

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
	int items_row = GetSize().x / fullItemSize();
	int rows = (double)items_filter.size() / (double)items_row + 0.9999;
	int total_height = rows * fullItemSize();

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
	scrollbar->SetThumbPosition(yoff - fullItemSize());

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
	scrollbar->SetThumbPosition(yoff + fullItemSize());

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
		int scroll_amount = (fullItemSize()) * -scroll_mult;

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
		int col = e.GetPosition().x / fullItemSize();
		int num_cols = GetSize().x / fullItemSize();

		// Get row clicked
		int row = (e.GetPosition().y - top_y) / (fullItemSize());

		// Select item
		item_selected = top_index + (row * num_cols) + col;
		Refresh();
	}

	e.Skip();
}
