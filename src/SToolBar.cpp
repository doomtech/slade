
#include "Main.h"
#include "WxStuff.h"
#include "SToolBar.h"
#include "SToolBarButton.h"

CVAR(Bool, show_toolbar_names, false, CVAR_SAVE)
//wxSystemColour stb_bg = wxSYS_COLOUR_WINDOW;
wxSystemColour stb_bg = wxSYS_COLOUR_MENUBAR;

class SToolBarSeparator : public wxControl {
public:
	SToolBarSeparator(wxWindow* parent) : wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) {
		// Set size (10 x height)
		int size = 22;
		SetSizeHints(4, size, 4, size);
		SetMinSize(wxSize(4, size));
		SetSize(4, size);

		// Bind events
		Bind(wxEVT_PAINT, &SToolBarSeparator::onPaint, this);
	}

	void onPaint(wxPaintEvent& e) {
		wxPaintDC dc(this);

		// Get system colours needed
		wxColour col_background = wxSystemSettings::GetColour(stb_bg);
		rgba_t bg(col_background.Red(), col_background.Green(), col_background.Blue());
		wxColour col_light = WXCOL(bg.ampf(1.7f, 1.7f, 1.7f, 1.0f));
		wxColour col_dark = WXCOL(bg.ampf(0.7f, 0.7f, 0.7f, 1.0f));

		// Draw background
		dc.SetBackground(wxBrush(col_background));
		dc.Clear();

		// Draw separator lines
		dc.SetPen(wxPen(col_light));
		dc.DrawLine(1, 2, 1, 20);
		dc.SetPen(wxPen(col_dark));
		dc.DrawLine(2, 2, 2, 20);
	}
};


SToolBarGroup::SToolBarGroup(wxWindow* parent, string name, bool force_name) : wxPanel(parent, -1) {
	// Set colours
	SetBackgroundColour(wxSystemSettings::GetColour(stb_bg));

	// Create sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);

	// Create group label if necessary
	if (show_toolbar_names || force_name) {
		wxStaticText* label = new wxStaticText(this, -1, S_FMT("%s:", CHR(name)));
		label->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT));
		sizer->AddSpacer(4);
		sizer->Add(label, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 4);
		sizer->AddSpacer(2);
	}
}

SToolBarGroup::~SToolBarGroup() {
}

void SToolBarGroup::addActionButton(string action, string icon) {
	// Get sizer
	wxSizer* sizer = GetSizer();

	// Create button
	SToolBarButton* button = new SToolBarButton(this, action, icon);

	// Add it to the group
	sizer->AddSpacer(1);
	sizer->Add(button, 0, wxTOP|wxBOTTOM, 2);
	sizer->AddSpacer(1);
}

void SToolBarGroup::addCustomControl(wxWindow* control) {
	// Set the control's parent to this panel
	control->SetParent(this);

	// Add it to the group
	GetSizer()->Add(control, 0, wxALL|wxEXPAND, 2);
}


SToolBar::SToolBar(wxWindow* parent) : wxPanel(parent, -1) {
	// Set background colour
	SetBackgroundColour(wxSystemSettings::GetColour(stb_bg));

	// Create sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);
	sizer->AddSpacer(4);
}

SToolBar::~SToolBar() {
}

void SToolBar::addGroup(SToolBarGroup* group) {
	// Set the group's parent
	group->SetParent(this);

	// Add it to the list of groups
	groups.push_back(group);

	// Add a spacer if there is a previous group
	if (groups.size() > 1)
		GetSizer()->Add(new SToolBarSeparator(this), 0, wxALL, 4);

	// Add it to the sizer
	GetSizer()->Add(group, 0, wxALL, 2);

	// Update layout
	Layout();
}
