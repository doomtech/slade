
#include "Main.h"
#include "WxStuff.h"
#include "BrowserCanvas.h"

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

BrowserCanvas::~BrowserCanvas() {
}

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
			if (x > GetSize().x - fullItemSize()) {
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
		if (x > GetSize().x - fullItemSize()) {
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

void BrowserCanvas::updateScrollBar() {
	// Do nothing special if no scrollbar present
	if (!scrollbar)
		return;

	// Determine total height of all items
	int items_row = GetSize().x / fullItemSize();
	int rows = ((double)items_filter.size() / (double)items_row) + 0.5;
	int total_height = rows * (fullItemSize());

	// Setup scrollbar
	scrollbar->SetScrollbar(scrollbar->GetThumbPosition(), GetSize().y, total_height, GetSize().y);
	yoff = scrollbar->GetThumbPosition();
}

BrowserItem* BrowserCanvas::getSelectedItem() {
	// Check selected index
	if (item_selected < 0 || item_selected >= (int)items_filter.size())
		return NULL;

	return items[items_filter[item_selected]];
}

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



void BrowserCanvas::onSize(wxSizeEvent& e) {
	updateScrollBar();

	// Do default stuff
	e.Skip();
}

void BrowserCanvas::onScrollThumbTrack(wxScrollEvent& e) {
	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

void BrowserCanvas::onScrollLineUp(wxScrollEvent& e) {
	// Scroll up by one row
	scrollbar->SetThumbPosition(yoff - fullItemSize());

	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

void BrowserCanvas::onScrollLineDown(wxScrollEvent& e) {
	// Scroll down by one row
	scrollbar->SetThumbPosition(yoff + fullItemSize());

	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

void BrowserCanvas::onScrollPageUp(wxScrollEvent& e) {
	// Scroll up by one screen
	scrollbar->SetThumbPosition(yoff - GetSize().y);

	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

void BrowserCanvas::onScrollPageDown(wxScrollEvent& e) {
	// Scroll down by one screen
	scrollbar->SetThumbPosition(yoff + GetSize().y);

	// Update y-offset and refresh
	yoff = scrollbar->GetThumbPosition();
	Refresh();
}

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
