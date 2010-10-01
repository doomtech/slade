
#include "Main.h"
#include "WxStuff.h"
#include "BrowserCanvas.h"

BrowserCanvas::BrowserCanvas(wxWindow* parent) : OGLCanvas(parent, -1) {
	// Init variables
	yoff = 0;
	item_size = 64;
	item_border = 4;
	scrollbar = NULL;

	// Bind events
	Bind(wxEVT_SIZE, &BrowserCanvas::onSize, this);
	Bind(wxEVT_MOUSEWHEEL, &BrowserCanvas::onMouseEvent, this);
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
	for (unsigned a = 0; a < items.size(); a++) {
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

		glPushMatrix();

		// Go to position
		glTranslated(x, y - yoff, 0);

		// Draw item
		items[a]->draw(item_size);

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



void BrowserCanvas::onSize(wxSizeEvent& e) {
	// Do nothing special if no scrollbar present
	if (!scrollbar)
		return;

	// Determine total height of all items
	int items_row = GetSize().x / fullItemSize();
	int rows = items.size() / items_row;
	int total_height = rows * fullItemSize();

	// Setup scrollbar
	scrollbar->SetScrollbar(scrollbar->GetThumbPosition(), GetSize().y, total_height, GetSize().y);
	yoff = scrollbar->GetThumbPosition();

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
		int scroll_amount = fullItemSize() * -scroll_mult;

		// Do scroll
		scrollbar->SetThumbPosition(yoff + scroll_amount);

		// Update y-offset and refresh
		yoff = scrollbar->GetThumbPosition();
		Refresh();
	}
}
