
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
#include <wx/aui/aui.h>


/*******************************************************************
 * MAPEDITORWINDOW CLASS FUNCTIONS
 *******************************************************************/

/* MapEditorWindow::MapEditorWindow
 * MapEditorWindow class constructor
 *******************************************************************/
MapEditorWindow::MapEditorWindow()
: wxFrame((wxFrame *) NULL, -1, _T("SLADE"), wxPoint(0, 0), wxSize(800, 600)) {
	map = new SLADEMap();
	setupLayout();
	Show();
	Maximize();
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
	wxAuiManager *m_mgr = new wxAuiManager(this);
	wxAuiPaneInfo p_inf;

	CreateToolBar();

	// Map canvas
	map_canvas = new MapCanvas(this, -1, map);
	p_inf.CenterPane();
	//m_mgr->AddPane(new wxTextCtrl(this, -1), p_inf);
	m_mgr->AddPane(map_canvas, p_inf);

	CreateStatusBar();

	m_mgr->Update();
	Layout();
}

bool MapEditorWindow::openMap(Archive* map_data, uint8_t format) {
	return map->readMap(map_data, format);
}
