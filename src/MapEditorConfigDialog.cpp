
#include "Main.h"
#include "WxStuff.h"
#include "MapEditorConfigDialog.h"
#include "BaseResourceChooser.h"
#include "GameConfiguration.h"
#include "ArchiveManager.h"
#include "WadArchive.h"
#include "SFileDialog.h"
#include "SplashWindow.h"
#include "ColourConfiguration.h"
#include "Icons.h"
#include <wx/statline.h>

MapEditorConfigDialog::MapEditorConfigDialog(wxWindow* parent, Archive* archive, bool show_maplist) : wxDialog(parent, -1, "Launch Map Editor") {
	// Init variables
	this->archive = archive;
	canvas_preview = NULL;

	// Setup main sizer
	wxBoxSizer* mainsizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(mainsizer);

	// Left side sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	mainsizer->Add(sizer, 0, wxEXPAND);

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

	wxStaticBox* frame;
	wxStaticBoxSizer* framesizer;

	// Setup image list
	img_list = new wxImageList(16, 16);
	img_list->Add(getIcon("i_tick"));
	img_list->Add(getIcon("t_close"));

	// Map section
	if (show_maplist) {
		frame = new wxStaticBox(this, -1, "Maps");
		framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
		sizer->Add(framesizer, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

		// Map list
		list_maps = new ListView(this, -1, wxLC_SINGLE_SEL|wxLC_LIST);
		list_maps->SetImageList(img_list, wxIMAGE_LIST_SMALL);
		framesizer->Add(list_maps, 1, wxEXPAND|wxALL, 4);

		// New map button
		//btn_new_map = new wxButton(this, -1, "New Map");
		//framesizer->Add(btn_new_map, 0, wxLEFT|wxRIGHT|wxBOTTOM, 4);
	}
	else {
		list_maps = NULL;
		btn_new_map = NULL;
	}

	// Resources section
	frame = new wxStaticBox(this, -1, "Resources");
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Base resource dropdown
	hbox = new wxBoxSizer(wxHORIZONTAL);
	framesizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(this, -1, "Base Resource:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	choice_base_resource = new BaseResourceChooser(this);
	hbox->Add(choice_base_resource, 1, wxEXPAND, 0);

	// Resource archive list
	list_resources = new wxCheckListBox(this, -1);
	framesizer->Add(list_resources, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	list_resources->SetInitialSize(wxSize(350, 100));

	// Populate resource archive list
	int index = 0;
	for (int a = 0; a < theArchiveManager->numArchives(); a++) {
		Archive* arch = theArchiveManager->getArchive(a);
		if (arch != archive) {
			list_resources->Append(arch->getFilename(false));
			if (theArchiveManager->archiveIsResource(arch))
				list_resources->Check(index);
			index++;
		}
	}

	// 'Open Resource' button
	hbox = new wxBoxSizer(wxHORIZONTAL);
	framesizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	btn_open_resource = new wxButton(this, -1, "Open Archive");
	hbox->Add(btn_open_resource, 0, wxEXPAND|wxRIGHT, 4);

	// 'Open Recent' button
	btn_recent = new wxButton(this, -1, "Open Recent");
	hbox->Add(btn_recent, 0, wxEXPAND, 0);


	// Right side (map preview)
	if (show_maplist) {
		frame = new wxStaticBox(this, -1, "Preview");
		framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
		mainsizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

		// Add map preview
		canvas_preview = new MapPreviewCanvas(this);
		framesizer->Add(canvas_preview->toPanel(this), 1, wxEXPAND|wxALL, 4);
		canvas_preview->SetInitialSize(wxSize(400, 400));
	}


	// Dialog buttons
	sizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxBOTTOM, 4);

	// Populate map list
	populateMapList();

	// Bind events
	choice_game_config->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &MapEditorConfigDialog::onChoiceGameConfigChanged, this);
	if (show_maplist) {
		list_maps->Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &MapEditorConfigDialog::onMapActivated, this);
		list_maps->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &MapEditorConfigDialog::onMapSelected, this);
		//btn_new_map->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MapEditorConfigDialog::onBtnNewMap, this);
	}
	btn_open_resource->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MapEditorConfigDialog::onBtnOpenResource, this);
	btn_recent->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MapEditorConfigDialog::onBtnRecent, this);

	Layout();
	mainsizer->Fit(this);
	CenterOnParent();

	// Select first map
	if (show_maplist)
		list_maps->selectItem(0);
}

MapEditorConfigDialog::~MapEditorConfigDialog() {
	delete img_list;
}

void MapEditorConfigDialog::populateMapList() {
	// Do nothing if map list isn't active
	if (!list_maps)
		return;

	// Get current selection (if any)
	int selection = -1;
	if (list_maps->GetSelectedItemCount() > 0)
		selection = list_maps->selectedItems()[0];

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
		// Setup format string
		string fmt = "D";
		if (maps[a].format == MAP_DOOM64)		fmt = "64";
		else if (maps[a].format == MAP_HEXEN)	fmt = "H";
		else if (maps[a].format == MAP_UDMF)	fmt = "U";
		else if (maps[a].format == MAP_UNKNOWN)	fmt = "?";

		// Create list item
		wxListItem li;
		li.SetId(index++);
		li.SetText(S_FMT("(%s) %s", CHR(fmt), CHR(maps[a].name)));
		if (maps[a].format != theGameConfiguration->getMapFormat())
			li.SetImage(1);
		else
			li.SetImage(0);

		// Add to list
		list_maps->InsertItem(li);
		list_maps->SetItemImage(index, 0);
	}

	// Restore selection
	list_maps->selectItem(selection);
}

Archive::mapdesc_t MapEditorConfigDialog::selectedMap() {
	// Get selected map
	int selection = -1;
	wxArrayInt sel = list_maps->selectedItems();
	if (sel.size() > 0)
		selection = sel[0];

	// Return it if valid
	if ((unsigned)selection >= maps.size())
		return Archive::mapdesc_t();
	else
		return maps[selection];
}

bool MapEditorConfigDialog::configMatchesMap(Archive::mapdesc_t map) {
	if (theGameConfiguration->getMapFormat() == map.format)
		return true;
	else
		return false;
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

void MapEditorConfigDialog::onBtnNewMap(wxCommandEvent& e) {
	// Create simple dialog to select map name
	wxDialog dlg(this, -1, "Enter Map Name");

	// Setup dialog
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	dlg.SetSizer(sizer);

	// Check if the game configuration allows any map name
	int flags = 0;
	if (!theGameConfiguration->anyMapName())
		flags = wxCB_READONLY;

	// Create map name combo box
	wxComboBox* cbo_mapname  = new wxComboBox(&dlg, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, flags);
	sizer->Add(cbo_mapname, 1, wxEXPAND|wxALL, 4);

	// Add possible map names to the combo box
	for (unsigned a = 0; a < theGameConfiguration->nMapNames(); a++)
		cbo_mapname->Append(theGameConfiguration->mapName(a));

	// Set inital map name selection
	if (theGameConfiguration->nMapNames() > 0)
		cbo_mapname->SetValue(theGameConfiguration->mapName(0));

	// Add dialog buttons
	sizer->Add(dlg.CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Show it
	dlg.SetInitialSize(wxSize(250, -1));
	dlg.CenterOnParent();
	if (dlg.ShowModal() == wxID_OK) {
		string mapname = cbo_mapname->GetValue();
		if (mapname.IsEmpty())
			return;

		// Check the map name isn't already taken
		for (unsigned a = 0; a < maps.size(); a++) {
			if (S_CMPNOCASE(maps[a].name, mapname)) {
				wxMessageBox("Map " + mapname + " already exists", "Error");
				return;
			}
		}
	}
}

void MapEditorConfigDialog::onBtnOpenResource(wxCommandEvent& e) {
	SFileDialog::fd_info_t info;
	if (SFileDialog::openFile(info, "Open Resource Archive", theArchiveManager->getArchiveExtensionsString(), this)) {
		theSplashWindow->show("Opening Resource Archive", true);
		Archive* na = theArchiveManager->openArchive(info.filenames[0], true, true);
		theSplashWindow->hide();
		if (na) {
			list_resources->Append(na->getFilename(false));
			list_resources->Check(list_resources->GetCount()-1);
		}
	}
}

void MapEditorConfigDialog::onBtnRecent(wxCommandEvent& e) {
	// Build list of recent wad filename strings
	wxArrayString recent;
	for (unsigned a = 0; a < theArchiveManager->numRecentFiles(); a++)
		recent.Add(theArchiveManager->recentFile(a));

	// Show dialog
	wxSingleChoiceDialog dlg(this, "Select a recent Archive to open", "Open Recent", recent);
	if (dlg.ShowModal() == wxID_OK) {
		Archive* na = theArchiveManager->openArchive(theArchiveManager->recentFile(dlg.GetSelection()), true, true);
		if (na) {
			list_resources->Append(na->getFilename(false));
			list_resources->Check(list_resources->GetCount()-1);
		}
	}
}

void MapEditorConfigDialog::onMapSelected(wxListEvent& e) {
	if (!canvas_preview)
		return;

	canvas_preview->clearMap();
	canvas_preview->openMap(selectedMap());
}
