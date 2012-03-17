
#include "Main.h"
#include "WxStuff.h"
#include "MainApp.h"
#include "SToolBarButton.h"
#include "Icons.h"
#include "Drawing.h"
#include <wx/graphics.h>


SToolBarButton::SToolBarButton(wxWindow* parent, string action, string icon)
: wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, wxDefaultValidator, "stbutton") {
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
	wxColour col_background = Drawing::getMenuBarBGColour();
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
		int trans = 160;
		if (state == STATE_MOUSEDOWN)
			trans = 220;

		// Create semitransparent hilight colour
		wxColour col_trans(col_hilight.Red(), col_hilight.Green(), col_hilight.Blue(), trans);

		// Set brush/pen colours
		gc->SetBrush(col_trans);
		gc->SetPen(wxPen(Drawing::lightColour(col_hilight, 5.0f), 1));

		// Draw border
		gc->DrawRoundedRectangle(2, 2, 18, 18, 2);

		// Draw outer border
		gc->SetBrush(wxBrush(col_hilight, wxBRUSHSTYLE_TRANSPARENT));
		gc->SetPen(wxPen(Drawing::darkColour(col_hilight, 5.0f)));
		gc->DrawRoundedRectangle(1, 1, 20, 20, 2);
	}

	// Draw disabled icon if disabled
	if (!IsEnabled()) {
		// Determine toolbar background brightness
		uint8_t r,g,b;
		r = col_background.Red();
		g = col_background.Green();
		b = col_background.Blue();
		wxColor::MakeGrey(&r, &g, &b);

		// Draw disabled icon
		gc->DrawBitmap(icon.ConvertToDisabled(r), 3, 3, 16, 16);
	}

	// Otherwise draw normal icon
	else
		gc->DrawBitmap(icon, 3, 3, 16, 16);

	delete gc;
}

void SToolBarButton::onMouseEvent(wxMouseEvent& e) {
	// Do nothing if disabled
	//if (state == STATE_DISABLED) {
	//	e.Skip();
	//	return;
	//}

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

		//wxCommandEvent e(wxEVT_COMMAND_MENU_SELECTED, GetId());
		//e.SetId(theApp->getAction(action)->getWxId());
		//e.SetEventObject(this);
		//GetEventHandler()->ProcessEvent(e);
		theApp->doAction(action);
	}

	// Left button up
	else if (e.GetEventType() == wxEVT_LEFT_UP) {
		state = STATE_MOUSEOVER;
		Refresh();
	}

	else
		e.Skip();
}
