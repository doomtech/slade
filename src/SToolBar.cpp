
#include "Main.h"
#include "WxStuff.h"
#include "SToolBar.h"
#include "SToolBarButton.h"
#include <wx/wrapsizer.h>

CVAR(Bool, show_toolbar_names, false, CVAR_SAVE)
CVAR(Bool, toolbar_background_grey, false, CVAR_SAVE)
wxColour stb_bg = wxColour(220, 220, 220);

class SToolBarSeparator : public wxControl {
public:
	SToolBarSeparator(wxWindow* parent) : wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) {
		// Set size
		int size = 22;
		SetSizeHints(4, size, 4, size);
		SetMinSize(wxSize(4, size));
		SetSize(4, size);

		// Set window name
		SetName("tb_sep");

		// Bind events
		Bind(wxEVT_PAINT, &SToolBarSeparator::onPaint, this);
	}

	void onPaint(wxPaintEvent& e) {
		wxPaintDC dc(this);

		// Get system colours needed
		wxColour col_background = stb_bg;
		rgba_t bg(col_background.Red(), col_background.Green(), col_background.Blue());
		wxColour col_light = WXCOL(bg.amp(50, 50, 50, 0));
		wxColour col_dark = WXCOL(bg.amp(-50, -50, -50, 0));

		// Draw background
		dc.SetBackground(wxBrush(col_background));
		dc.Clear();

		// Draw separator lines
		dc.GradientFillLinear(wxRect(1, 0, 1, 11), col_background, col_dark, wxSOUTH);
		dc.GradientFillLinear(wxRect(1, 11, 1, 11), col_background, col_dark, wxNORTH);
		dc.GradientFillLinear(wxRect(2, 0, 1, 11), col_background, col_light, wxSOUTH);
		dc.GradientFillLinear(wxRect(2, 11, 1, 11), col_background, col_light, wxNORTH);
	}
};


SToolBarGroup::SToolBarGroup(wxWindow* parent, string name, bool force_name) : wxPanel(parent, -1) {
	// Init variables
	this->name = name;

	// Set colours
	SetBackgroundColour(stb_bg);

	// Create sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);

	// Create group label if necessary
	if (show_toolbar_names || force_name) {
		wxStaticText* label = new wxStaticText(this, -1, S_FMT("%s:", CHR(name)));
		label->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT));
		sizer->AddSpacer(4);
		sizer->Add(label, 0, wxALIGN_CENTER_VERTICAL);
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
	sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
}

void SToolBarGroup::addCustomControl(wxWindow* control) {
	// Set the control's parent to this panel
	control->SetParent(this);

	// Add it to the group
	GetSizer()->Add(control, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 2);
}


class STBSizer : public wxWrapSizer {
public:
	STBSizer() : wxWrapSizer(wxHORIZONTAL, wxEXTEND_LAST_ON_EACH_LINE|wxREMOVE_LEADING_SPACES) {}
	~STBSizer() {}

protected:
	bool IsSpaceItem(wxSizerItem* item) const {
		if (!item->GetWindow())
			return true;

		if (item->GetWindow()->GetName() == "tb_sep")
			return true;

		return false;
	}
};


SToolBar::SToolBar(wxWindow* parent) : wxPanel(parent, -1) {
	// Setup background colour
	if (!toolbar_background_grey)
		stb_bg = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR);

	// Set background colour
	SetBackgroundColour(stb_bg);

	// Create sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Bind events
	Bind(wxEVT_SIZE, &SToolBar::onSize, this);
}

SToolBar::~SToolBar() {
}

void SToolBar::addGroup(SToolBarGroup* group) {
	// Set the group's parent
	group->SetParent(this);

	// Add it to the list of groups
	groups.push_back(group);

	// Update layout
	updateLayout();
}

void SToolBar::updateLayout() {
	// Clear main sizer
	wxSizer* sizer = GetSizer();
	sizer->Clear();

	// Delete previous separators
	for (unsigned a = 0; a < separators.size(); a++)
		delete separators[a];
	separators.clear();

	// Create horizontal sizer
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox);

	// Go through all groups
	int current_width = 0;
	int groups_line = 0;
	for (unsigned a = 0; a < groups.size(); a++) {
		// Check if the group will fit
		if (groups[a]->GetBestSize().x + current_width + 4 > GetSize().x && groups_line > 0) {
			// The group won't fit, begin a new line
			hbox = new wxBoxSizer(wxHORIZONTAL);
			sizer->Add(hbox, 0);
			groups_line = 0;
			current_width = 0;
		}

		// Add separator if needed
		if (groups_line > 0) {
			SToolBarSeparator* sep = new SToolBarSeparator(this);
			separators.push_back(sep);
			hbox->Add(sep, 0, wxALIGN_CENTER_VERTICAL);
			current_width += 4;
		}

		// Add the group
		hbox->Add(groups[a], 0, wxEXPAND|wxTOP|wxBOTTOM, 2);
		current_width += groups[a]->GetBestSize().x;

		groups_line++;
	}

	// Apply layout
	Layout();
}

void SToolBar::onSize(wxSizeEvent& e) {
	// Update layout
	updateLayout();

	e.Skip();
}
