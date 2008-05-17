
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    WadPanel.cpp
 * Description: WadManagerPanel class. Basically the UI for the
 *              WadManager stuff, has a tree of all open wads and
 *              basic wad management controls.
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
#include "WadManagerPanel.h"
#include "WadManager.h"
#include "Wad.h"
#include "Lump.h"


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
extern WadManager wad_manager;


/* WadManagerPanel::WadManagerPanel
 * WadManagerPanel class constructor
 *******************************************************************/
WadManagerPanel::WadManagerPanel(wxWindow *parent)
: wxPanel(parent, -1)
{
	// Create main sizer
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	// Create/setup tabs
	notebook_tabs = new wxNotebook(this, -1);
	vbox->Add(notebook_tabs, 1, wxEXPAND|wxALL, 4);

	// Create/setup wad tree
	tree_wadlist = new wxTreeCtrl(notebook_tabs, -1, wxDefaultPosition, wxDefaultSize);
	notebook_tabs->AddPage(tree_wadlist, _("Open Wads"), true);
	//vbox->Add(tree_wadlist, 1, wxEXPAND|wxALL, 4);

	// Create/setup file browser
	file_browser = new wxGenericDirCtrl(notebook_tabs, -1, wxDirDialogDefaultFolderStr, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_SHOW_FILTERS,
										_T("Any Supported Wad File (*.wad; *.zip; *.pk3)|*.wad;*.zip;*.pk3|Doom Wad files (*.wad)|*.wad|Zip files (*.zip)|*.zip|Pk3 (zip) files (*.pk3)|*.pk3|All Files (*.*)|*.*"));
	notebook_tabs->AddPage(file_browser, _("File Browser"));

	tree_wadlist->AddRoot(_T("Wads"));
}

/* WadManagerPanel::~WadManagerPanel
 * WadManagerPanel class destructor
 *******************************************************************/
WadManagerPanel::~WadManagerPanel()
{
}

/* WadManagerPanel::openFiles
 * Opens each file in the supplied array of filenames
 *******************************************************************/
void WadManagerPanel::openFiles(wxArrayString& files)
{
	// Go through each filename in the array
	for (int a = 0; a < files.size(); a++)
	{
		// Open the file in the wad manager
		Wad* new_wad = wad_manager.openWad(files[a]);

		// If it opened correctly, add it to the tree
		if (new_wad)
			tree_wadlist->AppendItem(tree_wadlist->GetRootItem(), files[a]);
	}
}
