
#include "Main.h"
#include "WxStuff.h"
#include "MapEditorConfigDialog.h"
#include "BaseResourceChooser.h"
#include "GameConfiguration.h"
#include "ArchiveManager.h"
#include <wx/statline.h>

MapEditorConfigDialog::MapEditorConfigDialog(wxWindow* parent, Archive* archive) : wxDialog(parent, -1, "Map Editor Configuration") {
	// Init variables
	this->archive = archive;

	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Get list of game configuration titles
	wxArrayString games;
	int game_current = 0;
	for (unsigned a = 0; a < theGameConfiguration->nConfigs(); a++) {
		games.Add(theGameConfiguration->configTitle(a));
		if (games.back() == theGameConfiguration->getName())
			game_current = a;
	}

	// Game configuration dropdown
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(this, -1, "Game Configuration:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	choice_game_config = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, games);
	choice_game_config->SetSelection(game_current);
	hbox->Add(choice_game_config, 1, wxEXPAND);

	// Base resource dropdown
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	hbox->Add(new wxStaticText(this, -1, "Base Resource:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	choice_base_resource = new BaseResourceChooser(this);
	hbox->Add(choice_base_resource, 1, wxEXPAND, 0);

	// Separator
	sizer->Add(new wxStaticLine(this, -1), 0, wxEXPAND|wxALL, 4);

	// Map list
	list_maps = new ListView(this, -1, wxLC_LIST|wxLC_SINGLE_SEL);
	sizer->Add(new wxStaticText(this, -1, "Select Map:"), 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
	sizer->AddSpacer(2);
	sizer->Add(list_maps, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Dialog buttons
	sizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxBOTTOM, 4);

	// Populate map list
	populateMapList();

	// Bind events
	choice_game_config->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &MapEditorConfigDialog::onChoiceGameConfigChanged, this);
	list_maps->Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &MapEditorConfigDialog::onMapActivated, this);

	SetSize(400, 500);
	CenterOnParent();
	Layout();
}

MapEditorConfigDialog::~MapEditorConfigDialog() {
}

void MapEditorConfigDialog::populateMapList() {
	// Clear list
	list_maps->ClearAll();
	maps.clear();

	// Check if an archive is open
	if (!archive)
		return;

	// Get all archive maps
	maps = archive->detectMaps();

	// Add maps matching the current game configuration
	int index = 0;
	for (unsigned a = 0; a < maps.size(); a++) {
		if (maps[a].format == theGameConfiguration->getMapFormat()) {
			wxListItem li;
			li.SetId(index++);
			li.SetText(maps[a].name);
			list_maps->InsertItem(li);
		}
	}
}

Archive::mapdesc_t MapEditorConfigDialog::selectedMap() {
	// Get selected map
	int selection = -1;
	wxArrayInt sel = list_maps->selectedItems();
	if (sel.size() > 0)
		selection = sel[0];

	// Check if a map is selected
	if (selection < 0 || selection >= choice_game_config->GetCount())
		return Archive::mapdesc_t();
	else
		return maps[selection];
}



void MapEditorConfigDialog::onChoiceGameConfigChanged(wxCommandEvent& e) {
	// Load selected configuration
	int selection = choice_game_config->GetSelection();
	string name = theGameConfiguration->configName(selection);
	theGameConfiguration->openConfig(name);

	// Refresh map list
	populateMapList();
}

void MapEditorConfigDialog::onMapActivated(wxListEvent& e) {
	EndModal(wxID_OK);
}
