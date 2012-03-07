
#include "Main.h"
#include "WxStuff.h"
#include "MainApp.h"
#include "SToolBarButton.h"
#include "Icons.h"
#include <wx/graphics.h>


extern wxSystemColour stb_bg;

SToolBarButton::SToolBarButton(wxWindow* parent, string action, string icon)
: wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) {
	// Init variables
	this->action = action;
	this->state = STATE_NORMAL;

	// Set size
	int size = 22;
	SetSizeHints(size, size, size, size);
	SetMinSize(wxSize(size, size));
	SetSize(size, size);

	// Load icon
	if (icon.IsEmpty())
		this->icon = getIcon(theApp->getAction(action)->getIconName());
	else
		this->icon = getIcon(icon);

	// Set tooltip
	SetToolTip(theApp->getAction(action)->getHelpText());

	// Bind events
	Bind(wxEVT_PAINT, &SToolBarButton::onPaint, this);
	Bind(wxEVT_ENTER_WINDOW, &SToolBarButton::onMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &SToolBarButton::onMouseEvent, this);
	Bind(wxEVT_LEFT_DOWN, &SToolBarButton::onMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &SToolBarButton::onMouseEvent, this);
}

SToolBarButton::~SToolBarButton() {
}

void SToolBarButton::onPaint(wxPaintEvent& e) {
	wxPaintDC dc(this);

	// Get system colours needed
	wxColour col_background = wxSystemSettings::GetColour(stb_bg);
	wxColour col_hilight_m = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUHILIGHT);
	wxColour col_hilight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);

	// Draw background
	dc.SetBackground(wxBrush(col_background));
	dc.Clear();

	// Create graphics context
	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
	if (!gc)
		return;

	// Draw border on mouseover
	if (state == STATE_MOUSEOVER || state == STATE_TOGGLED || state == STATE_MOUSEDOWN) {
		// Determine transparency level
		int trans = 128;
		if (state == STATE_MOUSEDOWN)
			trans = 220;

		// Create semitransparent hilight colour
		wxColour col_trans(col_hilight.Red(), col_hilight.Green(), col_hilight.Blue(), trans);

		// Set brush/pen colours
		gc->SetBrush(col_trans);
		gc->SetPen(wxPen(col_hilight, 1));

		// Draw border
		gc->DrawRoundedRectangle(1, 1, 20, 20, 2);
		//gc->DrawRectangle(1, 1, 20, 20);
	}

	// Draw icon
	gc->DrawBitmap(icon, 3, 3, 16, 16);

	delete gc;
}

void SToolBarButton::onMouseEvent(wxMouseEvent& e) {
	// Mouse enter
	if (e.GetEventType() == wxEVT_ENTER_WINDOW) {
		// Set state to mouseover
		if (state == STATE_NORMAL) {
			state = STATE_MOUSEOVER;
			Refresh();
		}
	}

	// Mouse leave
	else if (e.GetEventType() == wxEVT_LEAVE_WINDOW) {
		// Set state to normal
		if (state != STATE_TOGGLED) {
			state = STATE_NORMAL;
			Refresh();
		}
	}

	// Left button down
	else if (e.GetEventType() == wxEVT_LEFT_DOWN) {
		state = STATE_MOUSEDOWN;
		Refresh();
		theApp->doAction(action);
	}

	// Left button up
	else if (e.GetEventType() == wxEVT_LEFT_UP) {
		state = STATE_MOUSEOVER;
		Refresh();
	}

	e.Skip();
}
