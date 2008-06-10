
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
#include "WadPanel.h"


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
extern WadManager wad_manager;


/* WadManagerPanel::WadManagerPanel
 * WadManagerPanel class constructor
 *******************************************************************/
WadManagerPanel::WadManagerPanel(wxWindow *parent, wxNotebook* nb_wads)
: wxPanel(parent, -1)
{
	notebook_wads = nb_wads;

	// Create main sizer
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	// Create/setup tabs
	notebook_tabs = new wxNotebook(this, -1, wxDefaultPosition, wxDefaultSize);
	vbox->Add(notebook_tabs, 1, wxEXPAND|wxALL, 4);

	// Open wads & maps list
	wxPanel *panel_wm = new wxPanel(notebook_tabs);
	notebook_tabs->AddPage(panel_wm, _T("Wads"), true);

	// Create/setup wad list
	wxBoxSizer *box_wm = new wxBoxSizer(wxVERTICAL);
	panel_wm->SetSizer(box_wm);
	box_wm->Add(new wxStaticText(panel_wm, -1, _T("Open Wads:")), 0, wxEXPAND|wxALL, 4);
	list_openwads = new wxListBox(panel_wm, LIST_OPENWADS, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE);
	box_wm->Add(list_openwads, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Create/setup map list
	box_wm->Add(new wxStaticText(panel_wm, -1, _T("Maps:")), 0, wxEXPAND|wxALL, 4);
	list_maps = new wxListCtrl(panel_wm, LIST_MAPS, wxDefaultPosition, wxSize(-1, 128), wxLC_LIST|wxLC_SINGLE_SEL);
	box_wm->Add(list_maps, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Create/setup file browser
	//file_browser = new wxGenericDirCtrl(notebook_tabs, -1, wxDirDialogDefaultFolderStr, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_SHOW_FILTERS,
	//									_T("Any Supported Wad File (*.wad; *.zip; *.pk3)|*.wad;*.zip;*.pk3|Doom Wad files (*.wad)|*.wad|Zip files (*.zip)|*.zip|Pk3 (zip) files (*.pk3)|*.pk3|All Files (*.*)|*.*"));
	//file_browser->GetTreeCtrl()->SetId(TREE_BROWSER);
	//notebook_tabs->AddPage(file_browser, _("File Browser"));
}

/* WadManagerPanel::~WadManagerPanel
 * WadManagerPanel class destructor
 *******************************************************************/
WadManagerPanel::~WadManagerPanel()
{
}

/* WadManagerPanel::openFile
 * Opens a wad file and initialises the UI for it
 *******************************************************************/
void WadManagerPanel::openFile(string filename)
{
	// Open the file in the wad manager
	string error;
	Wad* new_wad = wad_manager.openWad(filename, error);

	// Check that the wad opened ok
	if (new_wad)
	{
		// Add wad to the list
		list_openwads->Append(filename);

		// Open a new wad panel tab
		WadPanel *wp = new WadPanel(notebook_wads, new_wad);
		notebook_wads->AddPage(wp, new_wad->getFileName(false), true);
	}
	else // If wad didn't open ok, show error message
		wxMessageBox(s_fmt(_T("Error opening %s:\n%s"), filename.c_str(), error.c_str()), _T("Error"));
}

/* WadManagerPanel::openFiles
 * Opens each file in the supplied array of filenames
 *******************************************************************/
void WadManagerPanel::openFiles(wxArrayString& files)
{
	// Go through each filename in the array
	for (int a = 0; a < (int)files.size(); a++)
	{
		// Open the wadfile
		openFile(files[a]);
	}
}

/*******************************************************************
 * WXWIDGETS EVENTS & HANDLERS
 *******************************************************************/
BEGIN_EVENT_TABLE(WadManagerPanel, wxPanel)
	EVT_LISTBOX(LIST_OPENWADS, WadManagerPanel::onListWadsChanged)
	EVT_TREE_ITEM_ACTIVATED(TREE_BROWSER, WadManagerPanel::onBrowserItemActivated)
END_EVENT_TABLE()

/* WadManagerPanel::onListWadsChanged
 * Event handler for when the user selects a wad file in the open
 * wads list. Updates the maps list with any maps found within the
 * selected wadfile.
 *******************************************************************/
void WadManagerPanel::onListWadsChanged(wxCommandEvent &e)
{
	// Clear current maps list
	list_maps->ClearAll();

	// Get a list of maps in the selected wad (if any)
	int selection = list_openwads->GetSelection();
	vector<Wad::mapdesc_t> maps = wad_manager.getWad(selection)->detectMaps();

	// Go through the list and add maps
	for (int a = 0; a < (int)maps.size(); a++)
	{
		// Setup map name string
		string name;

		// Add map format to name string
		if (maps[a].format == 0)
			name = _T("(D) ");
		if (maps[a].format == 1)
			name = _T("(H) ");
		if (maps[a].format == 2)
			name = _T("(U) ");

		// Add map name to string
		name += maps[a].name;
		
		// Add the list item
		wxListItem li;
		li.SetText(name);
		li.SetId(a);
		list_maps->InsertItem(li);
	}

	// Set minimum column size
	list_maps->SetColumnWidth(0, wxLIST_AUTOSIZE);
}

/* WadManagerPanel::onListWadsChanged
 * Event handler for when the user selects a wad file in the open
 * wads list. Updates the maps list with any maps found within the
 * selected wadfile.
 *******************************************************************/
void WadManagerPanel::onListWadsActivated(wxCommandEvent &e)
{
}

/* WadManagerPanel::onListMapsChanged
 * Event handler for when the user selects a map in the maps list.
 *******************************************************************/
void WadManagerPanel::onListMapsChanged(wxListEvent &e)
{
}

/* WadManagerPanel::onListMapsActivated
 * Event handler for when the user activates a map in the maps list.
 * Opens the map in a new map editor window
 *******************************************************************/
void WadManagerPanel::onListMapsActivated(wxListEvent &e)
{
}

/* WadManagerPanel::onBrowserItemActivated
 * Event handler for when the user activates an item on the file
 * browser list. This makes it act funny, but I can't find another
 * way to get this event to work :/
 *******************************************************************/
void WadManagerPanel::onBrowserItemActivated(wxTreeEvent &e)
{
	// Get the tree control
	wxTreeCtrl* tree = file_browser->GetTreeCtrl();

	// If the selected item has children, expand it,
	// otherwise attempt to open the file selected
	if (tree->ItemHasChildren(e.GetItem()))
		file_browser->OnExpandItem(e);
	else
		openFile(file_browser->GetPath());
}
