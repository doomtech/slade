
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
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


/*******************************************************************
 * MAPEDITORWINDOW CLASS FUNCTIONS
 *******************************************************************/

/* MapEditorWindow::MapEditorWindow
 * MapEditorWindow class constructor
 *******************************************************************/
MapEditorWindow::MapEditorWindow()
: wxFrame((wxFrame *) NULL, -1, "SLADE", wxPoint(mew_left, mew_top), wxSize(mew_width, mew_height)) {
	if (mew_maximized) Maximize();
	setupLayout();
	Show();

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
	theApp->getAction("mapw_rename")->addToMenu(menu_map);
	menu->Append(menu_map, "&Map");

	// Mode menu
	wxMenu* menu_mode = new wxMenu("");
	theApp->getAction("mapw_mode_vertices")->addToMenu(menu_mode);
	theApp->getAction("mapw_mode_lines")->addToMenu(menu_mode);
	theApp->getAction("mapw_mode_sectors")->addToMenu(menu_mode);
	theApp->getAction("mapw_mode_things")->addToMenu(menu_mode);
	menu->Append(menu_mode, "Mode");

	// View menu
	wxMenu* menu_view = new wxMenu("");
	theApp->getAction("mapw_showconsole")->addToMenu(menu_view);
	menu->Append(menu_view, "View");

	SetMenuBar(menu);


	// --- Toolbars ---

	// Map toolbar
	wxAuiToolBar* tb_map = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	theApp->getAction("mapw_save")->addToToolbar(tb_map);
	theApp->getAction("mapw_rename")->addToToolbar(tb_map);
	tb_map->Realize();

	// Mode toolbar
	wxAuiToolBar* tb_mode = new wxAuiToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	theApp->getAction("mapw_mode_vertices")->addToToolbar(tb_mode);
	theApp->getAction("mapw_mode_lines")->addToToolbar(tb_mode);
	theApp->getAction("mapw_mode_sectors")->addToToolbar(tb_mode);
	theApp->getAction("mapw_mode_things")->addToToolbar(tb_mode);
	tb_mode->Realize();

	// Add toolbar panels
	m_mgr->AddPane(tb_map, wxAuiPaneInfo().ToolbarPane().Top().Name("tb_map").CloseButton(false));		// Map toolbar
	m_mgr->AddPane(tb_mode, wxAuiPaneInfo().ToolbarPane().Top().Name("tb_mode").CloseButton(false));	// Mode toolbar

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
	// Set texture manager archive
	tex_man.setArchive(map.head->getParent());

	// Clear current map
	editor.clearMap();

	// Attempt to open map
	bool ok = editor.openMap(map);

	// Show window if opened ok
	if (ok) {
		this->Show(true);
		map_canvas->viewFitToMap();
	}

	return ok;
}

/* MapEditorWindow::handleAction
 * Handles the action [id]. Returns true if the action was handled,
 * false otherwise
 *******************************************************************/
bool MapEditorWindow::handleAction(string id) {
	// Don't handle actions if hidden
	if (!IsShown())
		return false;

	// Only interested in mapw_ actions
	if (!id.StartsWith("mapw_"))
		return false;

	// View->Console
	if (id == "mapw_showconsole") {
		wxAuiManager *m_mgr = wxAuiManager::GetManager(this);
		wxAuiPaneInfo& p_inf = m_mgr->GetPane("console");
		p_inf.Show(!p_inf.IsShown());
		p_inf.MinSize(200, 128);
		m_mgr->Update();
		return true;
	}

	return false;
}

void MapEditorWindow::onClose(wxCloseEvent& e) {
	// Save current layout
	wxAuiManager *m_mgr = wxAuiManager::GetManager(this);
	map_window_layout = m_mgr->SavePerspective();
	mew_maximized = IsMaximized();

	this->Show(false);
	editor.clearMap();
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
