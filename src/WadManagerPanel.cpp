
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
#include "MapEditorWindow.h"


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/

/* WMFileBrowser::WMFileBrowser
 * WMFileBrowser class constructor
 *******************************************************************/
WMFileBrowser::WMFileBrowser(wxWindow* parent, WadManagerPanel* wm, int id)
: wxGenericDirCtrl(parent, id, wxDirDialogDefaultFolderStr, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_SHOW_FILTERS,
					_T("Any Supported Wad File (*.wad; *.zip; *.pk3)|*.wad;*.zip;*.pk3|Doom Wad files (*.wad)|*.wad|Zip files (*.zip)|*.zip|Pk3 (zip) files (*.pk3)|*.pk3|All Files (*.*)|*.*")) {
	// Set the parent
	this->parent = wm;

	// Connect a custom event for when an item in the file tree is activated
	GetTreeCtrl()->Connect(GetTreeCtrl()->GetId(), wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(WMFileBrowser::onItemActivated));
}

/* WMFileBrowser::~WMFileBrowser
 * WMFileBrowser class destructor
 *******************************************************************/
WMFileBrowser::~WMFileBrowser() {
}

/* WMFileBrowser::onItemActivated
 * Event called when an item in the tree is activated. Opens a wadfile
 * if one is selected.
 *******************************************************************/
void WMFileBrowser::onItemActivated(wxTreeEvent &e) {
	// Get related objects
	wxTreeCtrl* tree = (wxTreeCtrl*) e.GetEventObject();
	WMFileBrowser* browser = (WMFileBrowser*) tree->GetParent();

	// If the selected item has no children (ie it's a file), open it
	// as a wad file in the wad manager
	if (!tree->ItemHasChildren(e.GetItem()))
		browser->parent->openFile(browser->GetPath());
}

/* WadManagerPanel::WadManagerPanel
 * WadManagerPanel class constructor
 *******************************************************************/
WadManagerPanel::WadManagerPanel(wxWindow *parent, wxAuiNotebook* nb_wads)
: wxPanel(parent, -1) {
	notebook_wads = nb_wads;

	// Create main sizer
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	// Create/setup tabs
	notebook_tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS);
	vbox->Add(notebook_tabs, 1, wxEXPAND | wxALL, 4);

	// Open wads & maps list
	wxPanel *panel_wm = new wxPanel(notebook_tabs);
	notebook_tabs->AddPage(panel_wm, _T("Wads"), true);

	// Create/setup wad list
	wxBoxSizer *box_wm = new wxBoxSizer(wxVERTICAL);
	panel_wm->SetSizer(box_wm);
	box_wm->Add(new wxStaticText(panel_wm, -1, _T("Open Wads:")), 0, wxEXPAND | wxALL, 4);
	list_openwads = new wxListCtrl(panel_wm, LIST_OPENWADS, wxDefaultPosition, wxDefaultSize, wxLC_LIST);
	box_wm->Add(list_openwads, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

	// Create/setup map list
	box_wm->Add(new wxStaticText(panel_wm, -1, _T("Maps:")), 0, wxEXPAND | wxALL, 4);
	list_maps = new wxListCtrl(panel_wm, LIST_MAPS, wxDefaultPosition, wxSize(-1, 128), wxLC_LIST | wxLC_SINGLE_SEL);
	box_wm->Add(list_maps, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

	// Create/setup file browser
	file_browser = new WMFileBrowser(notebook_tabs, this, TREE_BROWSER);
	notebook_tabs->AddPage(file_browser, _("File Browser"));

	// Create/setup Wad context menu
	menu_wadcontext = new wxMenu();
	menu_wadcontext->Append(MENU_SAVE,		_("Save"),		_("Save the selected Wad file(s)"));
	menu_wadcontext->Append(MENU_SAVEAS,	_("Save As"),	_("Save the selected Wad(s) to a new file(s)"));
	menu_wadcontext->Append(MENU_CLOSE,		_T("Close"),	_("Close the selected Wad(s)"));

	// Listen to the WadManager
	listenTo(&(WadManager::getInstance()));
}

/* WadManagerPanel::~WadManagerPanel
 * WadManagerPanel class destructor
 *******************************************************************/
WadManagerPanel::~WadManagerPanel() {
}

void WadManagerPanel::refreshWadList() {
	// Clear the list
	list_openwads->ClearAll();

	// Add each wad that is opened in the WadManager
	WadManager& wm = WadManager::getInstance();
	for (int a = 0; a < wm.numWads(); a++)
		list_openwads->InsertItem(list_openwads->GetItemCount(), wm.getWad(a)->getFileName(true));
}

/* WadManagerPanel::openFile
 * Opens a wad file and initialises the UI for it
 *******************************************************************/
void WadManagerPanel::openFile(string filename) {
	// Open the file in the wad manager
	Archive* new_wad = WadManager::getInstance().openWad(filename);

	// Check that the wad opened ok
	if (new_wad) {
		// Open a new wad panel tab
		WadPanel *wp = new WadPanel(notebook_wads, new_wad);
		notebook_wads->AddPage(wp, new_wad->getFileName(false), true);
	} else // If wad didn't open ok, show error message
		wxMessageBox(s_fmt(_T("Error opening %s:\n%s"), filename.c_str(), Global::error.c_str()), _T("Error"), wxICON_ERROR);
}

/* WadManagerPanel::openFiles
 * Opens each file in a supplied array of filenames
 *******************************************************************/
void WadManagerPanel::openFiles(wxArrayString& files) {
	// Go through each filename in the array
	for (int a = 0; a < (int) files.size(); a++) {
		// Open the wadfile
		openFile(files[a]);
	}
}

/* WadManagerPanel::getSelectedWads
 * Gets a list of indices of all selected wad list items
 *******************************************************************/
vector<int> WadManagerPanel::getSelectedWads() {
	vector<int> ret;

	// Go through all wad list items
	long item = -1;
	while (true) {
		// Get the next item in the list that is selected
		item = list_openwads->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// If -1 then none were selected
		if (item == -1)
			break;

		// Otherwise add the selected item to the vector
		//wxLogMessage("Item %ld is selected.", item);
		ret.push_back(item);
	}

	return ret;
}

/* WadManagerPanel::onAnnouncement
 * Called when an announcement is recieved from the Wad Manager
 *******************************************************************/
void WadManagerPanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// If a wad was opened
	if (event_name == _T("wad_opened"))
		refreshWadList();

	// If a wad was closed
	if (event_name == _T("wad_closed"))
		refreshWadList();
}

/* WadManagerPanel::saveSelection
 * Saves the currently selected wad file(s) in the list
 *******************************************************************/
void WadManagerPanel::saveSelection() {
	// Get the list of selected wads
	vector<int> selection = getSelectedWads();

	// Don't continue if there are no selected items
	if (selection.size() == 0)
		return;

	// Go through the selection
	for (size_t a = 0; a < selection.size(); a++) {
		// Save the wad
		if (!WadManager::getInstance().getWad(selection[a])->save()) {
			// If there was an error pop up a message box
			wxMessageBox(s_fmt(_T("Error: %s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
		}
	}
}

/* WadManagerPanel::saveSelectionAs
 * Saves the currently selected wad file(s) in the list as a new file
 *******************************************************************/
void WadManagerPanel::saveSelectionAs() {
	// Get the list of selected wads
	vector<int> selection = getSelectedWads();

	// Don't continue if there are no selected items
	if (selection.size() == 0)
		return;

	// Setup file filters (temporary, should go through all archive types somehow)
	string formats = _T("Doom Wad File (*.wad)|*.wad");
	string deftype = _T("*.wad");

	// Go through the selection
	for (size_t a = 0; a < selection.size(); a++) {
		// Get the wad
		Archive* wad = WadManager::getInstance().getWad(selection[a]);

		// Popup file save dialog
		string filename = wxFileSelector(_T("Save Wad ") + wad->getFileName(false) + _T(" As"), _T(""), _T(""), deftype, formats, wxSAVE | wxOVERWRITE_PROMPT);
		
		// Check a filename was selected
		if (!filename.empty()) {
			// Save the wad
			if (!wad->save(filename)) {
				// If there was an error pop up a message box
				wxMessageBox(s_fmt(_T("Error: %s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
			}
		}
	}
}

/* WadManagerPanel::closeSelection
 * Closes the currently selected wad file(s) in the list
 *******************************************************************/
void WadManagerPanel::closeSelection() {
	// Get the list of selected wads (indices)
	vector<int> selection = getSelectedWads();

	// Don't continue if there are no selected items
	if (selection.size() == 0)
		return;

	// Get the list of selected wads (Archive pointers)
	vector<Archive*> selected_wads;
	for (size_t a = 0; a < selection.size(); a++)
		selected_wads.push_back(WadManager::getInstance().getWad(selection[a]));

	// Close all selected wads
	for (size_t a = 0; a < selected_wads.size(); a++)
		WadManager::getInstance().closeWad(selected_wads[a]);
}

/* WadManagerPanel::saveCurrent
 * Saves the currently opened wad file (the currently opened tab)
 *******************************************************************/
void WadManagerPanel::saveCurrent() {
	int selection = notebook_wads->GetSelection();
	if (notebook_wads->GetPageText(selection).compare(_T("Start Page")))
		return;

	((WadPanel*) notebook_wads->GetPage(selection))->save();
}

/* WadManagerPanel::saveCurrentAs
 * Saves the currently opened wad file as a new file (the currently
 * opened tab)
 *******************************************************************/
void WadManagerPanel::saveCurrentAs() {
	int selection = notebook_wads->GetSelection();
	if (!notebook_wads->GetPageText(selection).compare(_T("Start Page")))
		return;

	((WadPanel*) notebook_wads->GetPage(notebook_wads->GetSelection()))->saveAs();
}

/* WadManagerPanel::closeCurrent
 * Closes the currently opened wad file (the currently opened tab)
 *******************************************************************/
void WadManagerPanel::closeCurrent() {
	int selection = notebook_wads->GetSelection();
	if (!notebook_wads->GetPageText(selection).compare(_T("Start Page")))
		return;

	Archive* wad = ((WadPanel*) notebook_wads->GetPage(notebook_wads->GetSelection()))->getWad();
	WadManager::getInstance().closeWad(wad);
}


/*******************************************************************
 * WXWIDGETS EVENTS & HANDLERS
 *******************************************************************/
BEGIN_EVENT_TABLE(WadManagerPanel, wxPanel)
	EVT_LIST_ITEM_SELECTED(LIST_OPENWADS, WadManagerPanel::onListWadsChanged)
	EVT_LIST_ITEM_ACTIVATED(LIST_OPENWADS, WadManagerPanel::onListWadsActivated)
	EVT_LISTBOX(LIST_MAPS, WadManagerPanel::onListMapsChanged)
	EVT_LIST_ITEM_ACTIVATED(LIST_MAPS, WadManagerPanel::onListMapsActivated)
	EVT_LIST_ITEM_RIGHT_CLICK(LIST_OPENWADS, WadManagerPanel::onListWadsRightClick)

	// Context Menu
	EVT_MENU(MENU_SAVE, WadManagerPanel::onMenuSave)
	EVT_MENU(MENU_SAVEAS, WadManagerPanel::onMenuSaveAs)
	EVT_MENU(MENU_CLOSE, WadManagerPanel::onMenuClose)
END_EVENT_TABLE()

/* WadManagerPanel::onListWadsChanged
 * Event handler for when the user selects a wad file in the open
 * wads list. Updates the maps list with any maps found within the
 * selected wadfile.
 *******************************************************************/
void WadManagerPanel::onListWadsChanged(wxListEvent &e) {
	// Clear current maps list
	list_maps->ClearAll();

	// Get the selected wad
	Archive* selected_wad = WadManager::getInstance().getWad(e.GetIndex());

	// Return if selection doesn't exist
	if (!selected_wad)
		return;

	// Get the list of maps in the selected wad
	vector<Archive::mapdesc_t> maps = selected_wad->detectMaps();

	// Go through the list and add maps
	for (int a = 0; a < (int) maps.size(); a++) {
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
	//list_maps->SetColumnWidth(0, wxLIST_AUTOSIZE);
}

/* WadManagerPanel::onListWadsActivated
 * Event handler for when the user activates a wad in the wad list.
 * Opens the wad in a new tab, if it isn't already open.
 *******************************************************************/
void WadManagerPanel::onListWadsActivated(wxListEvent &e) {
	// Get the selected wad
	Archive* selected_wad = WadManager::getInstance().getWad(e.GetIndex());

	// Check selected wad exists
	if (!selected_wad)
		return;

	// Go through all tabs
	for (size_t a = 0; a < notebook_wads->GetPageCount(); a++) {
		// Check for a match
		if (((WadPanel*) notebook_wads->GetPage(a))->getWad() == selected_wad) {
			// Selected wad is already open in a tab, so switch to this tab
			notebook_wads->SetSelection(a);
			return;
		}
	}

	// Open the selected wad in a new tab
	WadPanel *wp = new WadPanel(notebook_wads, selected_wad);
	notebook_wads->AddPage(wp, selected_wad->getFileName(false), true);
}

/* WadManagerPanel::onListMapsChanged
 * Event handler for when the user selects a map in the maps list.
 *******************************************************************/
void WadManagerPanel::onListMapsChanged(wxCommandEvent &e) {
}

/* WadManagerPanel::onListMapsActivated
 * Event handler for when the user activates a map in the maps list.
 * Opens the map in a new map editor window
 *******************************************************************/
void WadManagerPanel::onListMapsActivated(wxListEvent &e) {
	wxMessageBox(_T("heh"));
	new MapEditorWindow();
}

/* WadManagerPanel::onBrowserItemActivated
 * Event handler for when the user activates an item on the file
 * browser list. This makes it act funny, but I can't find another
 * way to get this event to work :/
 *******************************************************************/
void WadManagerPanel::onBrowserItemActivated(wxTreeEvent &e) {
	// Get the tree control
	wxTreeCtrl* tree = file_browser->GetTreeCtrl();

	// If the selected item has children, expand it,
	// otherwise attempt to open the file selected
	if (tree->ItemHasChildren(e.GetItem()))
		file_browser->OnExpandItem(e);
	else
		openFile(file_browser->GetPath());
}

/* WadManagerPanel::onListWadsRightClick
 * Event handler for when the user right clicks an item on the wad
 * list - pops up a context menu
 *******************************************************************/
void WadManagerPanel::onListWadsRightClick(wxListEvent& e) {
	PopupMenu(menu_wadcontext);
}

/* WadManagerPanel::onMenuSave
 * Event handler for when the 'Save' menu item is clicked on the
 * wad list context menu
 *******************************************************************/
void WadManagerPanel::onMenuSave(wxCommandEvent &e) {
	saveSelection();
}

/* WadManagerPanel::onMenuSaveAs
 * Event handler for when the 'Save As' menu item is clicked on the
 * wad list context menu
 *******************************************************************/
void WadManagerPanel::onMenuSaveAs(wxCommandEvent &e) {
	saveSelectionAs();
}

void WadManagerPanel::onMenuClose(wxCommandEvent& e) {
	closeSelection();
}
