
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MapEditorWindow.cpp
 * Description: MapEditorWindow class, it's a map editor window.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WxStuff.h"
#include "MapEditorWindow.h"
#include "MainApp.h"
#include "ConsolePanel.h"
#include "BaseResourceArchivesPanel.h"
#include "PreferencesDialog.h"
#include "ArchiveManager.h"
#include "MapObjectPropsPanel.h"
#include "MainWindow.h"
#include "SToolBar.h"
#include "WadArchive.h"
#include "SFileDialog.h"
#include "NodeBuilders.h"
#include <wx/aui/aui.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
MapEditorWindow* MapEditorWindow::instance = NULL;
string map_window_layout = "";
CVAR(Int, mew_width, 1024, CVAR_SAVE);
CVAR(Int, mew_height, 768, CVAR_SAVE);
CVAR(Int, mew_left, -1, CVAR_SAVE);
CVAR(Int, mew_top, -1, CVAR_SAVE);
CVAR(Bool, mew_maximized, true, CVAR_SAVE);
CVAR(String, nodebuilder_id, "zdbsp", CVAR_SAVE);
CVAR(String, nodebuilder_options, "", CVAR_SAVE);


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(Int, flat_drawtype);


/*******************************************************************
 * MAPEDITORWINDOW CLASS FUNCTIONS
 *******************************************************************/

/* MapEditorWindow::MapEditorWindow
 * MapEditorWindow class constructor
 *******************************************************************/
MapEditorWindow::MapEditorWindow()
: STopWindow("SLADE", mew_left, mew_top, mew_width, mew_height) {
	if (mew_maximized) Maximize();
	setupLayout();
	Show(false);
	custom_menus_begin = 2;

	// Set icon
	string icon_filename = appPath("slade.ico", DIR_TEMP);
	theArchiveManager->programResourceArchive()->getEntry("slade.ico")->exportFile(icon_filename);
	SetIcon(wxIcon(icon_filename, wxBITMAP_TYPE_ICO));
	wxRemoveFile(icon_filename);

	// Bind events
	Bind(wxEVT_CLOSE_WINDOW, &MapEditorWindow::onClose, this);
	Bind(wxEVT_MOVE, &MapEditorWindow::onMove, this);
	Bind(wxEVT_SIZE, &MapEditorWindow::onSize, this);
}

/* MapEditorWindow::~MapEditorWindow
 * MapEditorWindow class destructor
 *******************************************************************/
MapEditorWindow::~MapEditorWindow() {
}

/* MapEditorWindow::setupLayout
 * Sets up the basic map editor window layout
 *******************************************************************/
void MapEditorWindow::setupLayout() {
	// Create the wxAUI manager & related things
	wxAuiManager* m_mgr = new wxAuiManager(this);
	wxAuiPaneInfo p_inf;

	// Map canvas
	map_canvas = new MapCanvas(this, -1, &editor);
	p_inf.CenterPane();
	m_mgr->AddPane(map_canvas, p_inf);

	// --- Menus ---
	wxMenuBar *menu = new wxMenuBar();

	// Map menu
	wxMenu* menu_map = new wxMenu("");
	theApp->getAction("mapw_save")->addToMenu(menu_map);
	theApp->getAction("mapw_saveas")->addToMenu(menu_map);
	theApp->getAction("mapw_rename")->addToMenu(menu_map);
	menu->Append(menu_map, "&Map");

	// Editor menu
	wxMenu* menu_editor = new wxMenu("");
	theApp->getAction("mapw_preferences")->addToMenu(menu_editor);
	theApp->getAction("mapw_setbra")->addToMenu(menu_editor);
	menu->Append(menu_editor, "&Editor");

	// Mode menu
	/*
	wxMenu* menu_mode = new wxMenu("");
	theApp->getAction("mapw_mode_vertices")->addToMenu(menu_mode);
	theApp->getAction("mapw_mode_lines")->addToMenu(menu_mode);
	theApp->getAction("mapw_mode_sectors")->addToMenu(menu_mode);
	theApp->getAction("mapw_mode_things")->addToMenu(menu_mode);
	menu->Append(menu_mode, "Mode");
	*/

	// View menu
	wxMenu* menu_view = new wxMenu("");
	theApp->getAction("mapw_showconsole")->addToMenu(menu_view);
	theApp->getAction("mapw_showproperties")->addToMenu(menu_view);
	menu->Append(menu_view, "View");

	SetMenuBar(menu);


	// --- Toolbars ---
	toolbar = new SToolBar(this);

	// Map toolbar
	SToolBarGroup* tbg_map = new SToolBarGroup(toolbar, "_Map");
	tbg_map->addActionButton("mapw_save");
	tbg_map->addActionButton("mapw_saveas");
	tbg_map->addActionButton("mapw_rename");
	toolbar->addGroup(tbg_map);

	// Mode toolbar
	SToolBarGroup* tbg_mode = new SToolBarGroup(toolbar, "_Mode");
	tbg_mode->addActionButton("mapw_mode_vertices");
	tbg_mode->addActionButton("mapw_mode_lines");
	tbg_mode->addActionButton("mapw_mode_sectors");
	tbg_mode->addActionButton("mapw_mode_things");
	theApp->toggleAction("mapw_mode_lines");	// Lines mode by default
	toolbar->addGroup(tbg_mode);

	// Flat type toolbar
	SToolBarGroup* tbg_flats = new SToolBarGroup(toolbar, "_Flats Type");
	tbg_flats->addActionButton("mapw_flat_none");
	tbg_flats->addActionButton("mapw_flat_untextured");
	tbg_flats->addActionButton("mapw_flat_textured");
	toolbar->addGroup(tbg_flats);

	// Toggle current flat type
	if (flat_drawtype == 0) theApp->toggleAction("mapw_flat_none");
	else if (flat_drawtype == 1) theApp->toggleAction("mapw_flat_untextured");
	else theApp->toggleAction("mapw_flat_textured");

	// Add toolbar
	m_mgr->AddPane(toolbar, wxAuiPaneInfo().Top().CaptionVisible(false).MinSize(-1, 30).Resizable(false).PaneBorder(false).Name("toolbar"));


	// Status bar
	CreateStatusBar();

	// -- Console Panel --
	ConsolePanel *panel_console = new ConsolePanel(this, -1);

	// Setup panel info & add panel
	p_inf.DefaultPane();
	p_inf.Bottom();
	p_inf.Dock();
	p_inf.BestSize(480, 192);
	p_inf.FloatingSize(600, 400);
	p_inf.FloatingPosition(100, 100);
	p_inf.MinSize(-1, 192);
	p_inf.Show(false);
	p_inf.Caption("Console");
	p_inf.Name("console");
	m_mgr->AddPane(panel_console, p_inf);


	// -- Map Object Properties Panel --
	panel_obj_props = new MapObjectPropsPanel(this);

	// Setup panel info & add panel
	p_inf.Right();
	p_inf.BestSize(256, 256);
	p_inf.FloatingSize(400, 600);
	p_inf.FloatingPosition(120, 120);
	p_inf.MinSize(256, 256);
	p_inf.Show(true);
	p_inf.Caption("Item Properties");
	p_inf.Name("item_props");
	m_mgr->AddPane(panel_obj_props, p_inf);


	// Load previously saved perspective string
	// Doesn't play nice for whatever reason (god I hate wxAUI, it's terrible)
	//long vers = 0;
	//map_window_layout.Left(3).ToLong(&vers);
	//if (vers == MEW_LAYOUT_VERS)
	//	m_mgr->LoadPerspective(map_window_layout.Right(map_window_layout.Length() - 3));

	m_mgr->Update();
	Layout();
}

bool MapEditorWindow::openMap(Archive::mapdesc_t map) {
	// Get map parent archive
	Archive* archive = map.head->getParent();

	// Set texture manager archive
	tex_man.setArchive(archive);

	// Clear current map
	closeMap();

	// Attempt to open map
	bool ok = editor.openMap(map);

	// Show window if opened ok
	if (ok) {
		// Lock map entries
		mdesc_current = map;
		ArchiveEntry* entry = map.head;
		while (entry && entry != map.end->nextEntry()) {
			entry->lock();
			entry = entry->nextEntry();
		}

		this->Show(true);
		map_canvas->viewFitToMap();
		map_canvas->Refresh();

		// Set window title
		SetTitle(S_FMT("SLADE - %s of %s", CHR(map.name), CHR(archive->getFilename(false))));
	}

	return ok;
}

bool nb_warned = false;
bool MapEditorWindow::saveMap() {
	// Get map data entries
	vector<ArchiveEntry*> map_data;
	if (mdesc_current.format == MAP_DOOM)
		editor.getMap().writeDoomMap(map_data);
	else if (mdesc_current.format == MAP_HEXEN)
		editor.getMap().writeHexenMap(map_data);
	else // TODO: other formats
		return false;

	// Add map data to temporary wad
	Archive* wad = new WadArchive();
	wad->addNewEntry("MAP01");
	for (unsigned a = 0; a < map_data.size(); a++)
		wad->addEntry(map_data[a]);
	if (theGameConfiguration->getMapFormat() == MAP_UDMF)
		wad->addNewEntry("ENDMAP");

	// Check for map archive
	Archive* tempwad = NULL;
	Archive::mapdesc_t map = mdesc_current;
	if (mdesc_current.archive && mdesc_current.head) {
		tempwad = new WadArchive();
		tempwad->open(mdesc_current.head);
		vector<Archive::mapdesc_t> amaps = tempwad->detectMaps();
		if (amaps.size() > 0)
			map = amaps[0];
		else
			return false;
	}
	
	// --- Build nodes ---

	// Save wad to disk
	string filename = appPath("sladetemp.wad", DIR_TEMP);
	wad->save(filename);

	// Get current nodebuilder
	NodeBuilders::builder_t& builder = NodeBuilders::getBuilder(nodebuilder_id);
	string command = builder.command;

	// Check for undefined path
	if (!wxFileExists(builder.path) && !nb_warned) {
		// Open nodebuilder preferences
		PreferencesDialog pd(this);
		pd.showPage("Node Builders");
		if (pd.ShowModal() == wxID_OK)
			pd.applyPreferences();
		theMainWindow->getArchiveManagerPanel()->refreshAllTabs();

		// Get new builder if one was selected
		builder = NodeBuilders::getBuilder(nodebuilder_id);
		string command = builder.command;

		// Check again
		if (!wxFileExists(builder.path)) {
			wxMessageBox("No valid Node Builder is currently configured, nodes will not be built!", "Warning", wxICON_WARNING);
			nb_warned = true;
		}
	}

	// Build command line
	command.Replace("$f", S_FMT("\"%s\"", CHR(filename)));
	command.Replace("$o", CHR(wxString(nodebuilder_options)));

	// Run nodebuilder
	if (wxFileExists(builder.path)) {
		wxArrayString out;
		wxExecute(S_FMT("\"%s\" %s", CHR(builder.path), CHR(command)), out, wxEXEC_HIDE_CONSOLE);
		Raise();

		// Re-load wad
		wad->close();
		wad->open(filename);
	}
	

	// Delete current map entries
	ArchiveEntry* entry = map.end;
	Archive* archive = map.head->getParent();
	while (entry && entry != map.head) {
		ArchiveEntry* prev = entry->prevEntry();
		entry->unlock();
		archive->removeEntry(entry);
		entry = prev;
	}

	// Add new map entries
	for (unsigned a = 1; a < wad->numEntries(); a++) {
		entry = archive->addEntry(wad->getEntry(a), archive->entryIndex(map.head) + a, NULL, true);
		entry->lock();
	}

	// Clean up
	delete wad;
	wxRemoveFile(filename);
	if (tempwad) {
		tempwad->save();
		delete tempwad;
	}
	else {
		// Update map description
		mdesc_current.end = entry;
	}

	return true;
}

bool MapEditorWindow::saveMapAs() {
	// Show dialog
	SFileDialog::fd_info_t info;
	if (!SFileDialog::saveFile(info, "Save Map As", "Wad Archives (*.wad)|*.wad", this))
		return false;

	// Unlock current map entries
	ArchiveEntry* entry = mdesc_current.head;
	while (entry && entry != mdesc_current.end->nextEntry()) {
		entry->unlock();
		entry = entry->nextEntry();
	}
	mdesc_current.head = NULL;

	// Create new, empty wad
	WadArchive wad;
	ArchiveEntry* head = wad.addNewEntry(mdesc_current.name);
	ArchiveEntry* end = NULL;
	if (theGameConfiguration->getMapFormat() == MAP_UDMF) {
		// TODO: UDMF
		return false;
	}
	else {
		wad.addNewEntry("THINGS");
		wad.addNewEntry("LINEDEFS");
		wad.addNewEntry("SIDEDEFS");
		wad.addNewEntry("VERTEXES");
		end = wad.addNewEntry("SECTORS");
	}

	// Update current map description
	mdesc_current.head = head;
	mdesc_current.archive = false;
	mdesc_current.end = end;
	mdesc_current.format = theGameConfiguration->getMapFormat();

	// Save map data
	saveMap();

	// Write wad to file
	wad.save(info.filenames[0]);
	theArchiveManager->openArchive(info.filenames[0], true, true);

	// Set window title
	SetTitle(S_FMT("SLADE - %s of %s", CHR(mdesc_current.name), CHR(wad.getFilename(false))));

	return true;
}

void MapEditorWindow::closeMap() {
	// Close map in editor
	editor.clearMap();

	// Unlock map entries
	ArchiveEntry* entry = mdesc_current.head;
	while (entry && entry != mdesc_current.end->nextEntry()) {
		entry->unlock();
		entry = entry->nextEntry();
	}
	mdesc_current.head = NULL;
}

void MapEditorWindow::forceRefresh(bool renderer) {
	if (renderer) map_canvas->forceRefreshRenderer();
	map_canvas->Refresh();
}

void MapEditorWindow::refreshToolBar() {
	toolbar->Refresh();
}

/* MapEditorWindow::handleAction
 * Handles the action [id]. Returns true if the action was handled,
 * false otherwise
 *******************************************************************/
bool MapEditorWindow::handleAction(string id) {
	// Don't handle actions if hidden
	if (!IsShown())
		return false;

	// File->Save
	if (id == "mapw_save") {
		saveMap();
		return true;
	}

	// File->Save As
	if (id == "mapw_saveas") {
		saveMapAs();
		return true;
	}

	// Editor->Set Base Resource Archive
	if (id == "mapw_setbra") {
		wxDialog dialog_ebr(this, -1, "Edit Base Resource Archives", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
		BaseResourceArchivesPanel brap(&dialog_ebr);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(&brap, 1, wxEXPAND|wxALL, 4);

		sizer->Add(dialog_ebr.CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxDOWN, 4);

		dialog_ebr.SetSizer(sizer);
		dialog_ebr.Layout();
		dialog_ebr.SetInitialSize(wxSize(-1, 300));
		if (dialog_ebr.ShowModal() == wxID_OK)
			theArchiveManager->openBaseResource(brap.getSelectedPath());

		return true;
	}

	// Editor->Preferences
	if (id == "mapw_preferences") {
		PreferencesDialog pd(this);
		if (pd.ShowModal() == wxID_OK)
			pd.applyPreferences();
		theMainWindow->getArchiveManagerPanel()->refreshAllTabs();

		return true;
	}

	// View->Item Properties
	if (id == "mapw_showproperties") {
		wxAuiManager *m_mgr = wxAuiManager::GetManager(this);
		wxAuiPaneInfo& p_inf = m_mgr->GetPane("item_props");

		// Toggle window and focus
		p_inf.Show(!p_inf.IsShown());
		map_canvas->SetFocus();

		m_mgr->Update();
		return true;
	}

	// View->Console
	else if (id == "mapw_showconsole") {
		wxAuiManager *m_mgr = wxAuiManager::GetManager(this);
		wxAuiPaneInfo& p_inf = m_mgr->GetPane("console");

		// Toggle window and focus
		if (p_inf.IsShown()) {
			p_inf.Show(false);
			map_canvas->SetFocus();
		}
		else {
			p_inf.Show(true);
			p_inf.window->SetFocus();
		}

		p_inf.MinSize(200, 128);
		m_mgr->Update();
		return true;
	}

	return false;
}

/* MapEditorWindow::onClose
 * Called when the window is closed
 *******************************************************************/
void MapEditorWindow::onClose(wxCloseEvent& e) {
	// Save current layout
	mew_maximized = IsMaximized();
	wxAuiManager *m_mgr = wxAuiManager::GetManager(this);
	map_window_layout = m_mgr->SavePerspective();

	this->Show(false);
	closeMap();
}

/* MapEditorWindow::onSize
 * Called when the window is resized
 *******************************************************************/
void MapEditorWindow::onSize(wxSizeEvent& e) {
	// Update window size settings, but only if not maximized
	if (!IsMaximized()) {
		mew_width = GetSize().x;
		mew_height = GetSize().y;
	}

	// Update maximized cvar
	mew_maximized = IsMaximized();

	e.Skip();
}

/* MapEditorWindow::onMove
 * Called when the window moves
 *******************************************************************/
void MapEditorWindow::onMove(wxMoveEvent& e) {
	// Update window position settings, but only if not maximized
	if (!IsMaximized()) {
		mew_left = GetPosition().x;
		mew_top = GetPosition().y;
	}

	e.Skip();
}
