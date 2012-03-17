
#include "Main.h"
#include "WxStuff.h"
#include "SToolBar.h"
#include "SToolBarButton.h"
#include "Drawing.h"
#include <wx/wrapsizer.h>

CVAR(Bool, show_toolbar_names, false, CVAR_SAVE)
DEFINE_EVENT_TYPE(wxEVT_STOOLBAR_LAYOUT_UPDATED)

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
		wxColour col_background = Drawing::getMenuBarBGColour();
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

class SToolBarVLine : public wxControl {
	public:
	SToolBarVLine(wxWindow* parent) : wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) {
		// Set size
		SetMaxSize(wxSize(-1, 2));
		SetMinSize(wxSize(-1, 2));

		// Set window name
		SetName("tb_vline");

		// Bind events
		Bind(wxEVT_PAINT, &SToolBarVLine::onPaint, this);
	}

	void onPaint(wxPaintEvent& e) {
		wxPaintDC dc(this);

		// Get system colours needed
		wxColour col_background = Drawing::getMenuBarBGColour();
		wxColour col_light = Drawing::lightColour(col_background, 1.5f);
		wxColour col_dark = Drawing::darkColour(col_background, 1.5f);

		// Draw lines
		dc.SetPen(wxPen(col_dark));
		dc.DrawLine(wxPoint(0, 0), wxPoint(GetSize().x+1, 0));
		dc.SetPen(wxPen(col_light));
		dc.DrawLine(wxPoint(0, 1), wxPoint(GetSize().x+1, 1));
	}
};


SToolBarGroup::SToolBarGroup(wxWindow* parent, string name, bool force_name) : wxPanel(parent, -1) {
	// Init variables
	this->name = name;

	// Set colours
	SetBackgroundColour(Drawing::getMenuBarBGColour());

	// Create sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);

	// Create group label if necessary
	if (show_toolbar_names || force_name) {
		string showname = name;
		if (name.StartsWith("_"))
			showname.Remove(0, 1);

		wxStaticText* label = new wxStaticText(this, -1, S_FMT("%s:", CHR(showname)));
		label->SetForegroundColour(Drawing::getMenuTextColour());
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
	// Set background colour
	SetBackgroundColour(Drawing::getMenuBarBGColour());

	// Create sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Bind events
	Bind(wxEVT_SIZE, &SToolBar::onSize, this);
	Bind(wxEVT_PAINT, &SToolBar::onPaint, this);
	Bind(wxEVT_KILL_FOCUS, &SToolBar::onFocus, this);
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

void SToolBar::deleteGroup(string name) {
	// Do nothing if no name specified
	if (name.IsEmpty())
		return;

	// Find group to delete
	for (unsigned a = 0; a < groups.size(); a++) {
		if (S_CMPNOCASE(groups[a]->getName(), name)) {
			delete groups[a];
			groups.erase(groups.begin() + a);
			break;
		}
	}

	// Update layout
	updateLayout();
}

void SToolBar::addActionGroup(string name, wxArrayString actions) {
	// Do nothing if no actions were given
	if (actions.size() == 0)
		return;

	// Create new toolbar group
	SToolBarGroup* group = new SToolBarGroup(this, name);
	groups.push_back(group);

	// Add actions to the group
	for (unsigned a = 0; a < actions.size(); a++)
		group->addActionButton(actions[a]);

	// Update layout
	updateLayout();
}

void SToolBar::updateLayout(bool generate_event) {
	// Clear main sizer
	wxSizer* sizer = GetSizer();
	sizer->Clear();

	// Delete previous separators
	for (unsigned a = 0; a < separators.size(); a++)
		delete separators[a];
	separators.clear();

	// Delete previous vlines
	for (unsigned a = 0; a < vlines.size(); a++)
		delete vlines[a];
	vlines.clear();

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
			SToolBarVLine* vline = new SToolBarVLine(this);
			sizer->Add(vline, 0, wxEXPAND);
			vlines.push_back(vline);

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

	// Generate layout update event
	if (generate_event) {
		wxNotifyEvent e(wxEVT_STOOLBAR_LAYOUT_UPDATED, GetId());
		e.SetEventObject(this);
		GetEventHandler()->ProcessEvent(e);
	}
}

void SToolBar::enableGroup(string name, bool enable) {
	// Go through toolbar groups
	for (unsigned a = 0; a < groups.size(); a++) {
		if (S_CMPNOCASE(groups[a]->getName(), name))
			groups[a]->Enable(enable);
	}

	// Redraw
	Update();
	Refresh();
}


void SToolBar::onSize(wxSizeEvent& e) {
	// Update layout
	updateLayout(false);

	e.Skip();
}

void SToolBar::onPaint(wxPaintEvent& e) {
	wxPaintDC dc(this);

	// Get system colours needed
	wxColour col_background = Drawing::getMenuBarBGColour();
	wxColour col_light = Drawing::lightColour(col_background, 1.5f);
	wxColour col_dark = Drawing::darkColour(col_background, 1.5f);

	// Draw background
	dc.SetBackground(wxBrush(col_background));
	dc.Clear();

	// Draw top
	dc.SetPen(wxPen(col_light));
	dc.DrawLine(wxPoint(0, 0), wxPoint(GetSize().x+1, 0));

	// Draw bottom
	dc.SetPen(wxPen(col_dark));
	dc.DrawLine(wxPoint(0, GetSize().y-1), wxPoint(GetSize().x+1, GetSize().y-1));
}

void SToolBar::onFocus(wxFocusEvent& e) {
	// Redraw
	Update();
	Refresh();

	e.Skip();
}
