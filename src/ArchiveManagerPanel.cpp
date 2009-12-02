
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ArchiveManagerPanel.cpp
 * Description: ArchiveManagerPanel class. Basically the UI for the
 *              ArchiveManager class, has a tree of all open
 *              archives and basic file management controls.
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
#include "ArchiveManagerPanel.h"
#include "ArchiveManager.h"
#include "ArchivePanel.h"
#include "MapEditorWindow.h"
#include "ZipArchivePanel.h"
#include "ZipArchive.h"


/*******************************************************************
 * WMFILEBROWSER CLASS FUNCTIONS
 *******************************************************************/

/* WMFileBrowser::WMFileBrowser
 * WMFileBrowser class constructor
 *******************************************************************/
WMFileBrowser::WMFileBrowser(wxWindow* parent, ArchiveManagerPanel* wm, int id)
: wxGenericDirCtrl(parent, id, wxDirDialogDefaultFolderStr, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_SHOW_FILTERS,
_T("Any Supported Archive File (*.wad; *.zip; *.pk3)|*.wad;*.zip;*.pk3|Doom Wad files (*.wad)|*.wad|Zip files (*.zip)|*.zip|Pk3 (zip) files (*.pk3)|*.pk3|All Files (*.*)|*.*")) {
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
 * Event called when an item in the tree is activated. Opens a file
 * if one is selected.
 *******************************************************************/
void WMFileBrowser::onItemActivated(wxTreeEvent &e) {
	// Get related objects
	wxTreeCtrl* tree = (wxTreeCtrl*) e.GetEventObject();
	WMFileBrowser* browser = (WMFileBrowser*) tree->GetParent();

	// If the selected item has no children (ie it's a file),
	// open it in the archive manager
	if (!tree->ItemHasChildren(e.GetItem()))
		browser->parent->openFile(browser->GetPath());

	e.Skip();
}


/*******************************************************************
 * ARCHIVEMANAGERPANEL CLASS FUNCTIONS
 *******************************************************************/

/* ArchiveManagerPanel::ArchiveManagerPanel
 * ArchiveManagerPanel class constructor
 *******************************************************************/
ArchiveManagerPanel::ArchiveManagerPanel(wxWindow *parent, wxAuiNotebook* nb_archives)
: wxPanel(parent, -1) {
	notebook_archives = nb_archives;

	// Create main sizer
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	// Create/setup tabs
	notebook_tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS);
	vbox->Add(notebook_tabs, 1, wxEXPAND | wxALL, 4);

	// Open archives & maps list
	wxPanel *panel_am = new wxPanel(notebook_tabs);
	notebook_tabs->AddPage(panel_am, _T("Archives"), true);

	// Create/setup archive list
	wxBoxSizer *box_am = new wxBoxSizer(wxVERTICAL);
	panel_am->SetSizer(box_am);
	box_am->Add(new wxStaticText(panel_am, -1, _T("Open Archives:")), 0, wxEXPAND | wxALL, 4);
	list_archives = new ListView(panel_am, -1);
	box_am->Add(list_archives, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);
	refreshArchiveList();

	// Create/setup map list
	box_am->Add(new wxStaticText(panel_am, -1, _T("Maps:")), 0, wxEXPAND | wxALL, 4);
	list_maps = new wxListCtrl(panel_am, -1, wxDefaultPosition, wxSize(-1, 128), wxLC_LIST | wxLC_SINGLE_SEL);
	box_am->Add(list_maps, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

	// Create/setup file browser
	file_browser = new WMFileBrowser(notebook_tabs, this, -1);
	notebook_tabs->AddPage(file_browser, _("File Browser"));

	// Create/setup Archive context menu
	menu_context = new wxMenu();
	menu_context->Append(MENU_SAVE, _("Save"), _("Save the selected Archive(s)"));
	menu_context->Append(MENU_SAVEAS, _("Save As"), _("Save the selected Archive(s) to a new file(s)"));
	menu_context->Append(MENU_CLOSE, _T("Close"), _("Close the selected Archive(s)"));

	// Bind events
	list_archives->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &ArchiveManagerPanel::onListArchivesChanged, this);
	list_archives->Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &ArchiveManagerPanel::onListArchivesActivated, this);
	list_archives->Bind(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, &ArchiveManagerPanel::onListArchivesRightClick, this);
	list_maps->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &ArchiveManagerPanel::onListMapsChanged, this);
	list_maps->Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &ArchiveManagerPanel::onListMapsActivated, this);
	notebook_archives->Bind(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, &ArchiveManagerPanel::onTabChanged, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ArchiveManagerPanel::onMenu, this, MENU_SAVE, MENU_END);

	// Listen to the ArchiveManager
	listenTo(theArchiveManager);
}

/* ArchiveManagerPanel::~ArchiveManagerPanel
 * ArchiveManagerPanel class destructor
 *******************************************************************/
ArchiveManagerPanel::~ArchiveManagerPanel() {
}

/* ArchiveManagerPanel::refreshArchiveList
 * Clears and rebuilds the open archives list
 *******************************************************************/
void ArchiveManagerPanel::refreshArchiveList() {
	// Clear the list
	list_archives->ClearAll();

	// Add columns
	list_archives->InsertColumn(0, _T("Path"));

	// Add each archive that is opened in the ArchiveManager
	list_archives->enableSizeUpdate(false);
	for (int a = 0; a < theArchiveManager->numArchives(); a++) {
		list_archives->addItem(a, wxEmptyString);
		updateListItem(a);
	}

	// Update size
	list_archives->enableSizeUpdate(true);
	list_archives->updateSize();
}

/* ArchiveManagerPanel::populateMapList
 * Adds all maps in <archive> to the maps list widget
 *******************************************************************/
void ArchiveManagerPanel::populateMapList(Archive* archive) {
	// Clear current maps list
	list_maps->ClearAll();

	// Do nothing if no archive was given
	if (!archive)
		return;

	// Get the list of maps in the selected archive
	vector<Archive::mapdesc_t> maps = archive->detectMaps();

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
}

/* ArchiveManagerPanel::updateListItem
 * Updates the archive list item at <index>
 *******************************************************************/
void ArchiveManagerPanel::updateListItem(int index) {
	Archive* archive = theArchiveManager->getArchive(index);

	if (!archive)
		return;

	// Set item name
	list_archives->setItemText(index, 0, archive->getFileName());

	// Set item status colour
	if (archive->isOnDisk()) {
		if (archive->isModified())
			list_archives->setItemStatus(index, LV_STATUS_MODIFIED);
		else
			list_archives->setItemStatus(index, LV_STATUS_NORMAL);
	}
	else
		list_archives->setItemStatus(index, LV_STATUS_NEW);
}

/* ArchiveManagerPanel::isArchivePanel
 * Checks if the currently selected tab is an ArchivePanel
 * Returns true if it is, false if not
 *******************************************************************/
bool ArchiveManagerPanel::isArchivePanel(int tab_index) {
	// Check that tab index is in range
	if (tab_index < 0 || tab_index >= notebook_archives->GetPageCount())
		return false;

	// Check that it isn't the start page
	if (!(notebook_archives->GetPageText(tab_index).compare(_T("Start Page"))))
		return false;

	// Currently can't be anything other than the start page or an archive panel,
	// so it's valid
	return true;
}

/* ArchiveManagerPanel::openTab
 * Opens a new tab for the archive at <archive_index> in the archive
 * manager
 *******************************************************************/
void ArchiveManagerPanel::openTab(int archive_index) {
	Archive* archive = theArchiveManager->getArchive(archive_index);

	if (archive) {
		// Go through all tabs
		for (size_t a = 0; a < notebook_archives->GetPageCount(); a++) {
			// Check for a match
			if (((ArchivePanel*) notebook_archives->GetPage(a))->getArchive() == archive) {
				// Selected archive is already open in a tab, so switch to this tab
				notebook_archives->SetSelection(a);
				return;
			}
		}

		// If tab isn't already open, open a new one
		ArchivePanel *wp = NULL;
		if (archive->getType() == ARCHIVE_WAD)
			wp = new ArchivePanel(notebook_archives, archive);
		else if (archive->getType() == ARCHIVE_ZIP)
			wp = new ZipArchivePanel(notebook_archives, archive);

		notebook_archives->AddPage(wp, archive->getFileName(false), true);
		wp->Show(true);
		wp->init();
	}
}

/* ArchiveManagerPanel::openFile
 * Opens an archive and initialises the UI for it
 *******************************************************************/
void ArchiveManagerPanel::openFile(string filename) {
	// Open the file in the archive manager
	Archive* new_archive = theArchiveManager->openArchive(filename);

	// Check that the archive opened ok
	if (!new_archive) {
		// If archive didn't open ok, show error message
		wxMessageBox(s_fmt(_T("Error opening %s:\n%s"), filename.c_str(), Global::error.c_str()), _T("Error"), wxICON_ERROR);
	}
}

/* ArchiveManagerPanel::openFiles
 * Opens each file in a supplied array of filenames
 *******************************************************************/
void ArchiveManagerPanel::openFiles(wxArrayString& files) {
	// Go through each filename in the array
	for (int a = 0; a < (int) files.size(); a++) {
		// Open the archive
		openFile(files[a]);
	}
}

/* ArchiveManagerPanel::closeAll
 * Closes all currently open archives
 *******************************************************************/
void ArchiveManagerPanel::closeAll() {
	theArchiveManager->closeAll();
}

/* ArchiveManagerPanel::saveAll
 * Saves all currently open archives
 *******************************************************************/
void ArchiveManagerPanel::saveAll() {
	// Go through all open archives
	for (int a = 0; a < theArchiveManager->numArchives(); a++) {
		// Get the archive to be saved
		Archive* archive = theArchiveManager->getArchive(a);

		if (archive->isOnDisk()) {
			// Save the archive if possible
			if (!archive->save()) {
				// If there was an error pop up a message box
				wxMessageBox(s_fmt(_T("Error: %s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
			}
		}
		else {
			// If the archive is newly created, do Save As instead

			// Popup file save dialog
			string formats = archive->getFileExtensionString();
			string filename = wxFileSelector(_T("Save Archive ") + archive->getFileName(false) + _T(" As"), _T(""), _T(""), wxEmptyString, formats, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

			// Check a filename was selected
			if (!filename.empty()) {
				// Save the archive
				if (!archive->save(filename)) {
					// If there was an error pop up a message box
					wxMessageBox(s_fmt(_T("Error: %s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
				}
			}
		}
	}
}

/* ArchiveManagerPanel::createNewArchive
 * Creates a new archive of the given type and opens it in a tab
 *******************************************************************/
void ArchiveManagerPanel::createNewArchive(uint8_t type) {
	Archive* new_archive = theArchiveManager->newArchive(type);

	if (new_archive) {
		openTab(theArchiveManager->archiveIndex(new_archive));
	}
}

/* ArchiveManagerPanel::getSelectedArchives
 * Gets a list of indices of all selected archive list items
 *******************************************************************/
vector<int> ArchiveManagerPanel::getSelectedArchives() {
	vector<int> ret;

	// Go through all wad list items
	long item = -1;
	while (true) {
		// Get the next item in the list that is selected
		item = list_archives->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// If -1 then none were selected
		if (item == -1)
			break;

		// Otherwise add the selected item to the vector
		ret.push_back(item);
	}

	return ret;
}

/* ArchiveManagerPanel::onAnnouncement
 * Called when an announcement is recieved from the Archive Manager
 *******************************************************************/
void ArchiveManagerPanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Reset event data for reading
	event_data.seek(0, SEEK_SET);

	// If an archive was closed
	if (event_name == _T("archive_closed")) {
		int32_t index = -1;
		event_data.read(&index, 4);
		list_archives->DeleteItem(index);
		populateMapList(NULL);
	}

	// If an archive was added
	if (event_name == _T("archive_added")) {
		int index = theArchiveManager->numArchives();
		list_archives->addItem(index, theArchiveManager->getArchive(theArchiveManager->numArchives()-1)->getFileName(true));
		openTab(theArchiveManager->numArchives()-1);
	}

	// If an archive was saved
	if (event_name == _T("archive_saved")) {
		int32_t index = -1;
		event_data.read(&index, 4);
		updateListItem(index);
	}

	// If an archive was modified
	if (event_name == _T("archive_modified")) {
		int32_t index = -1;
		event_data.read(&index, 4);
		updateListItem(index);
	}
}

/* ArchiveManagerPanel::saveSelection
 * Saves the currently selected archive(s) in the list
 *******************************************************************/
void ArchiveManagerPanel::saveSelection() {
	// Get the list of selected archives
	vector<int> selection = getSelectedArchives();

	// Don't continue if there are no selected items
	if (selection.size() == 0)
		return;

	// Go through the selection
	for (size_t a = 0; a < selection.size(); a++) {
		// Get the archive to be saved
		Archive* archive = theArchiveManager->getArchive(selection[a]);

		if (archive->isOnDisk()) {
			// Save the archive if possible
			if (!theArchiveManager->getArchive(selection[a])->save()) {
				// If there was an error pop up a message box
				wxMessageBox(s_fmt(_T("Error: %s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
			}
		}
		else {
			// If the archive is newly created, do Save As instead

			// Popup file save dialog
			string formats = archive->getFileExtensionString();
			string filename = wxFileSelector(_T("Save Archive ") + archive->getFileName(false) + _T(" As"), _T(""), _T(""), wxEmptyString, formats, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

			// Check a filename was selected
			if (!filename.empty()) {
				// Save the archive
				if (!archive->save(filename)) {
					// If there was an error pop up a message box
					wxMessageBox(s_fmt(_T("Error: %s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
				}
			}
		}
	}
}

/* ArchiveManagerPanel::saveSelectionAs
 * Saves the currently selected archive(s) in the list as new file(s)
 *******************************************************************/
void ArchiveManagerPanel::saveSelectionAs() {
	// Get the list of selected archives
	vector<int> selection = getSelectedArchives();

	// Don't continue if there are no selected items
	if (selection.size() == 0)
		return;

	// Setup file filters (temporary, should go through all archive types somehow)
	//string formats = _T("Doom Wad File (*.wad)|*.wad");
	//string deftype = _T("*.wad");

	// Go through the selection
	for (size_t a = 0; a < selection.size(); a++) {
		// Get the archive
		Archive* archive = theArchiveManager->getArchive(selection[a]);

		// Popup file save dialog
		string formats = archive->getFileExtensionString();
		string filename = wxFileSelector(_T("Save Archive ") + archive->getFileName(false) + _T(" As"), _T(""), _T(""), wxEmptyString, formats, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		// Check a filename was selected
		if (!filename.empty()) {
			// Save the archive
			if (!archive->save(filename)) {
				// If there was an error pop up a message box
				wxMessageBox(s_fmt(_T("Error: %s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
			}
		}
	}

	refreshArchiveList();
}

/* ArchiveManagerPanel::closeSelection
 * Closes the currently selected archive(s) in the list
 *******************************************************************/
void ArchiveManagerPanel::closeSelection() {
	// Get the list of selected list items
	vector<int> selection = getSelectedArchives();

	// Don't continue if there are no selected items
	if (selection.size() == 0)
		return;

	// Get the list of selected archives
	vector<Archive*> selected_archives;
	for (size_t a = 0; a < selection.size(); a++)
		selected_archives.push_back(theArchiveManager->getArchive(selection[a]));

	// Close all selected archives
	for (size_t a = 0; a < selected_archives.size(); a++)
		theArchiveManager->closeArchive(selected_archives[a]);
}

/* ArchiveManagerPanel::saveCurrent
 * Saves the currently opened archive (the currently opened tab)
 *******************************************************************/
void ArchiveManagerPanel::saveCurrent() {
	// Send to current archive panel
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->save();

	refreshArchiveList();
}

/* ArchiveManagerPanel::saveCurrentAs
 * Saves the currently opened archive as a new file (the currently
 * opened tab)
 *******************************************************************/
void ArchiveManagerPanel::saveCurrentAs() {
	// Send to current archive panel
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->saveAs();

	refreshArchiveList();
}

/* ArchiveManagerPanel::closeCurrent
 * Closes the currently opened archive (the currently opened tab)
 *******************************************************************/
void ArchiveManagerPanel::closeCurrent() {
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection)) {
		Archive* archive = ((ArchivePanel*) notebook_archives->GetPage(selection))->getArchive();
		theArchiveManager->closeArchive(archive);
	}
}

/* ArchiveManagerPanel::newEntry
 * Signals the currently opened archive panel tab to create a new
 * entry
 *******************************************************************/
void ArchiveManagerPanel::newEntry() {
	// Send to current archive panel
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->newEntry();
}

/* ArchiveManagerPanel::newEntryFromFile
 * Signals the currently opened archive panel tab to create a new
 * entry from a file
 *******************************************************************/
void ArchiveManagerPanel::newEntryFromFile() {
	// Send to current archive panel
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->newEntryFromFile();
}

/* ArchiveManagerPanel::renameEntry
 * Signals the currently opened archive panel tab to rename any
 * selected entries
 *******************************************************************/
void ArchiveManagerPanel::renameEntry() {
	// Send to current archive panel
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->renameEntry();
}

/* ArchiveManagerPanel::deleteEntry
 * Signals the currently opened archive panel tab to delete any
 * selected entries
 *******************************************************************/
void ArchiveManagerPanel::deleteEntry() {
	// Send to current archive panel
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->deleteEntry();
}

/* ArchiveManagerPanel::deleteEntry
 * Signals the currently opened archive panel tab to import files to
 * any selected entries
 *******************************************************************/
void ArchiveManagerPanel::importEntry() {
	// Send to current archive panel
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->importEntry();
}

/* ArchiveManagerPanel::deleteEntry
 * Signals the currently opened archive panel tab to export any
 * selected entries to files
 *******************************************************************/
void ArchiveManagerPanel::exportEntry() {
	// Send to current archive panel
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->exportEntry();
}

/* ArchiveManagerPanel::deleteEntry
 * Signals the currently opened archive panel tab to export any
 * selected entries to a new wad archive
 *******************************************************************/
void ArchiveManagerPanel::exportEntryWad() {
	// Send to current archive panel
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->exportEntryWad();
}

/* ArchiveManagerPanel::moveUp
 * Signals the currently opened archive panel tab to move selected
 * entries up
 *******************************************************************/
void ArchiveManagerPanel::moveUp() {
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->moveUp();
}

/* ArchiveManagerPanel::moveDown
 * Signals the currently opened archive panel tab to move selected
 * entries down
 *******************************************************************/
void ArchiveManagerPanel::moveDown() {
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection))
		((ArchivePanel*) notebook_archives->GetPage(selection))->moveDown();
}


/*******************************************************************
 * ARCHIVEMANAGERPANEL EVENTS
 *******************************************************************/

/* ArchiveManagerPanel::onListArchivesChanged
 * Called when the user selects an archive in the open files list.
 * Updates the maps list with any maps found within the selected
 * archive
 *******************************************************************/
void ArchiveManagerPanel::onListArchivesChanged(wxListEvent& e) {
	// Get the selected archive
	Archive* selected_archive = theArchiveManager->getArchive(e.GetIndex());

	// Return if selection doesn't exist
	if (!selected_archive)
		return;

	populateMapList(selected_archive);
}

/* ArchiveManagerPanel::onListArchivesActivated
 * Called when the user activates an archive in the list.
 * Opens the archive in a new tab, if it isn't already open.
 *******************************************************************/
void ArchiveManagerPanel::onListArchivesActivated(wxListEvent& e) {
	// Open the archive tab, or create a new tab if it isn't already
	openTab(e.GetIndex());
}

/* ArchiveManagerPanel::onListMapsChanged
 * Called when the user selects a map in the maps list.
 *******************************************************************/
void ArchiveManagerPanel::onListMapsChanged(wxCommandEvent& e) {
}

/* ArchiveManagerPanel::onListMapsActivated
 * Called when the user activates a map in the maps list.
 * Opens the map in a new map editor window
 *******************************************************************/
void ArchiveManagerPanel::onListMapsActivated(wxListEvent& e) {
	//new MapEditorWindow();
	wxMessageBox(_T("Map Editor is not implemented yet."));
}

/* ArchiveManagerPanel::onListArchivesRightClick
 * Called when the user right clicks an item on the archive list,
 * pops up a context menu
 *******************************************************************/
void ArchiveManagerPanel::onListArchivesRightClick(wxListEvent& e) {
	PopupMenu(menu_context);
}

/* ArchiveManagerPanel::onMenu
 * Called when an item on the archive list context menu is selected
 *******************************************************************/
void ArchiveManagerPanel::onMenu(wxCommandEvent& e) {
	// Save
	if (e.GetId() == MENU_SAVE)
		saveSelection();

	// Save As
	else if (e.GetId() == MENU_SAVEAS)
		saveSelectionAs();

	// Close
	else if (e.GetId() == MENU_CLOSE)
		closeSelection();
}

/* ArchiveManagerPanel::onTabChanged
 * Called when the user switches between archive tabs
 *******************************************************************/
void ArchiveManagerPanel::onTabChanged(wxAuiNotebookEvent& e) {
	// If an archive tab is selected, set the frame title accordingly
	int selection = notebook_archives->GetSelection();
	if (isArchivePanel(selection)) {
		Archive* archive = ((ArchivePanel*)notebook_archives->GetPage(selection))->getArchive();
		((wxFrame*)GetParent())->SetTitle(s_fmt(_T("SLADE - %s"), archive->getFileName(false)));
	}
	else
		((wxFrame*)GetParent())->SetTitle(_T("SLADE"));
}
