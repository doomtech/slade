
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
#include <wx/aui/aui.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
MapEditorWindow* MapEditorWindow::instance = NULL;


/*******************************************************************
 * MAPEDITORWINDOW CLASS FUNCTIONS
 *******************************************************************/

/* MapEditorWindow::MapEditorWindow
 * MapEditorWindow class constructor
 *******************************************************************/
MapEditorWindow::MapEditorWindow()
: wxFrame((wxFrame *) NULL, -1, "SLADE", wxPoint(0, 0), wxSize(1024, 768)) {
	setupLayout();
	Show();
	Maximize();

	// Bind events
	Bind(wxEVT_CLOSE_WINDOW, &MapEditorWindow::onClose, this);
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
	if (ok)
		this->Show(true);

	return ok;
}

void MapEditorWindow::onClose(wxCloseEvent& e) {
	this->Show(false);
	editor.clearMap();
}
