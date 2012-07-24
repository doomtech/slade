
#include "Main.h"
#include "WxStuff.h"
#include "Map3DPrefsPanel.h"

Map3DPrefsPanel::Map3DPrefsPanel(wxWindow* parent) : PrefsPanelBase(parent) {
	// Create sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "Map Editor 3D Mode Preferences");
	wxStaticBoxSizer *fsizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(fsizer, 1, wxEXPAND|wxALL, 4);
}

Map3DPrefsPanel::~Map3DPrefsPanel() {
}

void Map3DPrefsPanel::applyPreferences() {
}
