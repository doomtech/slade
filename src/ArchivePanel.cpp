
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ArchivePanel.cpp
 * Description: ArchivePanel class. The base wxWidgets panel for
 *              archive content editing. One of these is opened in
 *              a tab for each open archive.
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
#include "Misc.h"
#include "WxStuff.h"
#include "ArchivePanel.h"
#include "WadArchive.h"
#include "TextEntryPanel.h"
#include "DefaultEntryPanel.h"
#include "GfxEntryPanel.h"
#include "PaletteEntryPanel.h"
#include "AnimatedEntryPanel.h"
#include "SwitchesEntryPanel.h"
#include "HexEntryPanel.h"
#include "ANSIEntryPanel.h"
#include "MapEntryPanel.h"
#include "AudioEntryPanel.h"
#include "GfxConvDialog.h"
#include "ModifyOffsetsDialog.h"
#include "EntryOperations.h"
#include "Clipboard.h"
#include "ArchiveManager.h"
#include "MainWindow.h"
#include "SplashWindow.h"
#include "ArchiveOperations.h"
#include "Icons.h"
#include "Conversions.h"
#include "MainWindow.h"
#include "TranslationEditorDialog.h"
#include "SFileDialog.h"
#include "MapEditorWindow.h"
#include "KeyBind.h"
#include <wx/aui/auibook.h>
#include <wx/aui/auibar.h>
#include <wx/filename.h>
#include <wx/gbsizer.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Int, autosave_entry_changes, 2, CVAR_SAVE)	// 0=no, 1=yes, 2=ask
CVAR(Bool, confirm_entry_delete, true, CVAR_SAVE)
CVAR(Bool, context_submenus, true, CVAR_SAVE)
wxMenu* menu_archive = NULL;
wxMenu* menu_entry = NULL;
wxAuiToolBar* tb_archive = NULL;
wxAuiToolBar* tb_entry = NULL;


/*******************************************************************
 * APENTRYLISTDROPTARGET CLASS
 *******************************************************************
 Handles drag'n'drop of files on to the entry list
*/
class APEntryListDropTarget : public wxFileDropTarget {
private:
	ArchivePanel* parent;
	ArchiveEntryList* list;

public:
	APEntryListDropTarget(ArchivePanel* parent, ArchiveEntryList* list) { this->parent = parent; this->list = list; }
	~APEntryListDropTarget(){}

	bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) {
		// Determine what item the files were dragged onto
		int flags;
		long index = list->HitTest(wxPoint(x, y), flags) - list->entriesBegin();

		// Add to end if no item was hit
		if (index < 0)
			index = list->GetItemCount() - list->entriesBegin();

		// Import all dragged files, inserting after the item they were dragged onto
		for (int a = filenames.size()-1; a >= 0; a--) {
			wxFileName fn(filenames[a]);

			// Create new entry
			ArchiveEntry* entry = parent->getArchive()->addNewEntry(fn.GetFullName(), index, list->getCurrentDir());

			// Import the file to it
			entry->importFile(filenames[a]);
			EntryType::detectEntryType(entry);
		}

		return true;
	}
};


/*******************************************************************
 * ARCHIVEPANEL CLASS FUNCTIONS
 *******************************************************************/

/* ArchivePanel::ArchivePanel
 * ArchivePanel class constructor
 *******************************************************************/
ArchivePanel::ArchivePanel(wxWindow* parent, Archive* archive)
: wxPanel(parent, -1) {
	// Set archive
	this->archive = archive;
	listenTo(archive);

	// Create entry panels
	entry_area = new EntryPanel(this, "nil");
	default_area = new DefaultEntryPanel(this);
	text_area = new TextEntryPanel(this);
	gfx_area = new GfxEntryPanel(this);
	pal_area = new PaletteEntryPanel(this);
	animated_area = new AnimatedEntryPanel(this);
	switches_area = new SwitchesEntryPanel(this);
	hex_area = new HexEntryPanel(this);
	ansi_area = new ANSIEntryPanel(this);
	map_area = new MapEntryPanel(this);
	audio_area = new AudioEntryPanel(this);


	// --- Setup Layout ---

	// Create sizer
	wxBoxSizer *m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Entry list panel

	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, "Entries");
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_hbox->Add(framesizer, 0, wxEXPAND|wxALL, 4);


	// Create path display
	sizer_path_controls = new wxBoxSizer(wxHORIZONTAL);
	framesizer->Add(sizer_path_controls, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 4);
	framesizer->AddSpacer(2);

	// Label
	label_path = new wxStaticText(this, -1, "Path:", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_START);
	sizer_path_controls->Add(label_path, 1, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);

	// 'Up' button
	btn_updir = new wxBitmapButton(this, -1, getIcon("e_upfolder"));
	btn_updir->Enable(false);
	sizer_path_controls->Add(btn_updir, 0, wxEXPAND);


	// Create entry list panel
	entry_list = new ArchiveEntryList(this);
	entry_list->setArchive(archive);
	entry_list->SetDropTarget(new APEntryListDropTarget(this, entry_list));
	framesizer->Add(entry_list, 1, wxEXPAND | wxLEFT|wxRIGHT|wxBOTTOM, 4);


	wxGridBagSizer* gb_sizer = new wxGridBagSizer(4, 4);
	framesizer->Add(gb_sizer, 0, wxEXPAND|wxALL, 4);

	// Create category selector
	choice_category = new wxChoice(this, -1);
	choice_category->Append("All");
	vector<string> cats = EntryType::allCategories();
	for (unsigned a = 0; a < cats.size(); a++)
		choice_category->Append(cats[a]);
	choice_category->SetSelection(0);
	gb_sizer->Add(new wxStaticText(this, -1, "Show:"), wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(choice_category, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);
	gb_sizer->AddGrowableCol(1, 1);

	// Create filter
	text_filter = new wxTextCtrl(this, -1);
	gb_sizer->Add(new wxStaticText(this, -1, "Filter:"), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(text_filter, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);


	// Add default entry panel
	cur_area = entry_area;
	m_hbox->Add(cur_area, 1, wxEXPAND);
	cur_area->Show(true);

	// Bind events
	entry_list->Bind(EVT_VLV_SELECTION_CHANGED, &ArchivePanel::onEntryListSelectionChange, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_FOCUSED, &ArchivePanel::onEntryListFocusChange, this);
	entry_list->Bind(wxEVT_KEY_DOWN, &ArchivePanel::onEntryListKeyDown, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, &ArchivePanel::onEntryListRightClick, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &ArchivePanel::onEntryListActivated, this);
	text_filter->Bind(wxEVT_COMMAND_TEXT_UPDATED, &ArchivePanel::onTextFilterChanged, this);
	choice_category->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ArchivePanel::onChoiceCategoryChanged, this);
	Bind(EVT_AEL_DIR_CHANGED, &ArchivePanel::onDirChanged, this);
	btn_updir->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ArchivePanel::onBtnUpDir, this);
	((DefaultEntryPanel*)default_area)->getEditTextButton()->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ArchivePanel::onDEPEditAsText, this);
	((DefaultEntryPanel*)default_area)->getViewHexButton()->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ArchivePanel::onDEPViewAsHex, this);

	// Do a quick check to see if we need the path display
	if (archive->getRoot()->nChildren() == 0)
		sizer_path_controls->Show(false);

	// Update size+layout
	entry_list->updateWidth();
	Layout();
}

/* ArchivePanel::~ArchivePanel
 * ArchivePanel class destructor
 *******************************************************************/
ArchivePanel::~ArchivePanel() {
	removeMenus();
}

/* ArchivePanel::saveEntryChanges
 * Saves any changes made to the currently open entry
 *******************************************************************/
bool ArchivePanel::saveEntryChanges() {
	// Ignore if no changes have been made (or no entry is open)
	if (!cur_area->isModified() || !cur_area->getEntry())
		return true;

	// Don't save if autosave is off
	if (autosave_entry_changes == 0)
		return false;

	// Ask if needed
	if (autosave_entry_changes > 1) {
		int result = wxMessageBox(S_FMT("Save changes to entry \"%s\"?", cur_area->getEntry()->getName().c_str()),
									"Unsaved Changes", wxYES_NO|wxICON_QUESTION);

		// Stop if user clicked no
		if (result == wxNO)
			return false;
	}

	// Save entry changes
	return cur_area->saveEntry();
}

/* ArchivePanel::addMenus
 * Adds the 'Archive' and 'Entry' menus to the main window menubar
 *******************************************************************/
void ArchivePanel::addMenus() {
	// Create menus if needed
	if (!menu_archive) {
		// Archive menu
		wxMenu* menu_new = new wxMenu("");
		theApp->getAction("arch_newentry")->addToMenu(menu_new, "&Entry");
		theApp->getAction("arch_newdir")->addToMenu(menu_new, "&Directory");
		menu_archive = new wxMenu();
		menu_archive->AppendSubMenu(menu_new, "&New");
		theApp->getAction("arch_importfiles")->addToMenu(menu_archive);
		menu_archive->AppendSeparator();
		theApp->getAction("arch_texeditor")->addToMenu(menu_archive);
		wxMenu* menu_clean = new wxMenu("");
		theApp->getAction("arch_clean_patches")->addToMenu(menu_clean);
		theApp->getAction("arch_clean_textures")->addToMenu(menu_clean);
		theApp->getAction("arch_clean_flats")->addToMenu(menu_clean);
		theApp->getAction("arch_check_duplicates")->addToMenu(menu_clean);
		menu_archive->AppendSubMenu(menu_clean, "&Maintenance");
	}
	if (!menu_entry) {
		// Entry menu
		menu_entry = new wxMenu();
		theApp->getAction("arch_entry_rename")->addToMenu(menu_entry);
		theApp->getAction("arch_entry_delete")->addToMenu(menu_entry);
		theApp->getAction("arch_entry_revert")->addToMenu(menu_entry);
		menu_entry->AppendSeparator();
		theApp->getAction("arch_entry_cut")->addToMenu(menu_entry);
		theApp->getAction("arch_entry_copy")->addToMenu(menu_entry);
		theApp->getAction("arch_entry_paste")->addToMenu(menu_entry);
		menu_entry->AppendSeparator();
		theApp->getAction("arch_entry_moveup")->addToMenu(menu_entry);
		theApp->getAction("arch_entry_movedown")->addToMenu(menu_entry);
		menu_entry->AppendSeparator();
		theApp->getAction("arch_entry_import")->addToMenu(menu_entry);
		theApp->getAction("arch_entry_export")->addToMenu(menu_entry);
		menu_entry->AppendSeparator();
		theApp->getAction("arch_entry_bookmark")->addToMenu(menu_entry);
	}

	// Add them to the main window menubar
	theMainWindow->addCustomMenu(menu_archive, "&Archive");
	theMainWindow->addCustomMenu(menu_entry, "&Entry");
	cur_area->addCustomMenu();
}

/* ArchivePanel::removeMenus
 * Removes the 'Archive' and 'Entry' menus from the main window
 * menubar
 *******************************************************************/
void ArchivePanel::removeMenus() {
	// Remove ArchivePanel menus from the main window menubar
	theMainWindow->removeCustomMenu(menu_archive);
	theMainWindow->removeCustomMenu(menu_entry);
	cur_area->removeCustomMenu();
}

/* ArchivePanel::save
 * Saves the archive
 *******************************************************************/
bool ArchivePanel::save() {
	// Check the archive exists
	if (!archive)
		return false;

	// Save any changes in the current entry panel
	saveEntryChanges();

	// Check the archive has been previously saved
	if (!archive->canSave())
		return saveAs();

	// Save the archive
	if (!archive->save()) {
		// If there was an error pop up a message box
		wxMessageBox(S_FMT("Error:\n%s", Global::error.c_str()), "Error", wxICON_ERROR);
		return false;
	}

	// Refresh entry list
	entry_list->updateList();

	return true;
}

/* ArchivePanel::saveAs
 * Saves the archive to a new file
 *******************************************************************/
bool ArchivePanel::saveAs() {
	// Check the archive exists
	if (!archive)
		return false;

	// Do save dialog
	SFileDialog::fd_info_t info;
	if (SFileDialog::saveFile(info, "Save Archive " + archive->getFilename(false) + " As", archive->getFileExtensionString(), this)) {
		// Save the archive
		if (!archive->save(info.filenames[0])) {
			// If there was an error pop up a message box
			wxMessageBox(S_FMT("Error:\n%s", Global::error.c_str()), "Error", wxICON_ERROR);
			return false;
		}
	}

	// Refresh entry list
	entry_list->updateList();

	// Add as recent file
	theArchiveManager->addRecentFile(info.filenames[0]);

	return true;
}

/* ArchivePanel::newEntry
 * Adds a new entry to the archive after the last selected entry in
 * the list. If nothing is selected it is added at the end of the
 * list. Asks the user for a name for the new entry, and doesn't add
 * one if no name is entered. Returns true if the entry was created,
 * false otherwise.
 *******************************************************************/
bool ArchivePanel::newEntry() {
	// Prompt for new entry name
	string name = wxGetTextFromUser("Enter new entry name:", "New Entry");

	// Check if any name was entered
	if (name == "")
		return false;

	// Check for \ character (e.g., from Arch-Viles graphics). They have to be kept.
	if (archive->getType() == ARCHIVE_WAD && name.length() <= 8
		&& (name.find('\\') != wxNOT_FOUND) || (name.find('/') != wxNOT_FOUND)) {
	} // Don't process as a file name

	// Remove any path from the name, if any (for now)
	else {
		wxFileName fn(name);
		name = fn.GetFullName();
	}

	// Get the entry index of the last selected list item
	int index = archive->entryIndex(entry_list->getLastSelectedEntry(), entry_list->getCurrentDir());

	// If something was selected, add 1 to the index so we add the new entry after the last selected
	if (index >= 0)
		index++;
	else
		index = -1;	// If not add to the end of the list

	// Add the entry to the archive
	ArchiveEntry* new_entry = archive->addNewEntry(name, index, entry_list->getCurrentDir());

	// Return whether the entry was created ok
	return !!new_entry;
}

/* ArchivePanel::newDirectory
 * Adds a new subdirectory to the current directory, but only if the
 * archive supports them
 *******************************************************************/
bool ArchivePanel::newDirectory() {
	// Check archive supports directories
	if (archive->getType() != ARCHIVE_ZIP) {
		wxMessageBox("This Archive format does not support directories", "Can't create new directory", wxICON_ERROR);
		return false;
	}

	// Prompt for new directory name
	string name = wxGetTextFromUser("Enter new directory name:", "New Directory");

	// Check if any name was entered
	if (name == "")
		return false;

	// Remove any path from the name, if any (for now)
	wxFileName fn(name);
	name = fn.GetFullName();

	// Add the directory to the archive
	ArchiveTreeNode* dir = archive->createDir(name, entry_list->getCurrentDir());

	// Return whether the directory was created ok
	return !!dir;
}

/* ArchivePanel::importFiles
 * Opens a file selection dialog and imports any selected files to
 * the current directory, using the filenames as entry names
 *******************************************************************/
bool ArchivePanel::importFiles() {
	// Run open files dialog
	SFileDialog::fd_info_t info;
	if (SFileDialog::openFiles(info, "Choose files to import", "Any File (*.*)|*.*", this)) {
		// Get the entry index of the last selected list item
		int index = archive->entryIndex(entry_list->getLastSelectedEntry(), entry_list->getCurrentDir());

		// If something was selected, add 1 to the index so we add the new entry after the last selected
		if (index >= 0)
			index++;
		else
			index = -1;	// If not add to the end of the list

		// Go through the list of files
		bool ok = false;
		entry_list->Show(false);
		theSplashWindow->show("Importing Files...", true);
		for (size_t a = 0; a < info.filenames.size(); a++) {
			// Get filename
			string name = wxFileName(info.filenames[a]).GetFullName();

			// Update splash window
			theSplashWindow->setProgress(float(a) / float(info.filenames.size()));
			theSplashWindow->setProgressMessage(name);

			// Add the entry to the archive
			ArchiveEntry* new_entry = archive->addNewEntry(name, index, entry_list->getCurrentDir());

			// If the entry was created ok, load the file into it
			if (new_entry) {
				new_entry->importFile(info.filenames[a]);		// Import file to entry
				EntryType::detectEntryType(new_entry);	// Detect entry type
				ok = true;
			}

			if (index > 0) index++;
		}
		theSplashWindow->hide();
		entry_list->Show(true);

		return ok;
	}
	else	// User cancelled, return false
		return false;
}

/* ArchivePanel::convertArchiveTo
 * Not implemented
 *******************************************************************/
bool ArchivePanel::convertArchiveTo() {
	wxMessageBox("Not Implemented");
	return false;
}

/* ArchivePanel::cleanupArchive
 * Not implemented
 *******************************************************************/
bool ArchivePanel::cleanupArchive() {
	wxMessageBox("Not Implemented");
	return false;
}

/* ArchivePanel::renameEntry
 * Opens a dialog to rename the selected entries. If multiple entries
 * are selected, a mass-rename is performed
 *******************************************************************/
bool ArchivePanel::renameEntry(bool each) {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Check any are selected
	if (each || selection.size() == 1) {
		// If only one entry is selected, or "rename each" mode is desired, just do basic rename
		for (unsigned a = 0; a < selection.size(); a++) {

			// Prompt for a new name
			string new_name = wxGetTextFromUser("Enter new entry name: (* = unchanged)", "Rename", selection[a]->getName());

			// Rename entry (if needed)
			if (!new_name.IsEmpty() && selection[a]->getName() != new_name)
				archive->renameEntry(selection[a], new_name);
		}
	} else if (selection.size() > 1) {
		// Get a list of entry names
		wxArrayString names;
		for (unsigned a = 0; a < selection.size(); a++)
			names.push_back(selection[a]->getName(true));

		// Get filter string
		string filter = Misc::massRenameFilter(names);

		// Prompt for a new name
		string new_name = wxGetTextFromUser("Enter new entry name: (* = unchanged)", "Rename", filter);

		// Apply mass rename to list of names
		if (!new_name.IsEmpty()) {
			Misc::doMassRename(names, new_name);

			// Go through the list
			for (size_t a = 0; a < selection.size(); a++) {
				ArchiveEntry* entry = selection[a];

				// If the entry is a folder then skip it
				if (entry->getType() == EntryType::folderType())
					continue;

				// Get current name as wxFileName for processing
				wxFileName fn(entry->getName());

				// Rename the entry (if needed)
				if (fn.GetName() != names[a]) {
					fn.SetName(names[a]);							// Change name
					archive->renameEntry(entry, fn.GetFullName());	// Rename in archive
				}
			}
		}
	}


	// Get a list of selected directories
	vector<ArchiveTreeNode*> selected_dirs = entry_list->getSelectedDirectories();

	// Go through the list
	for (size_t a = 0; a < selected_dirs.size(); a++) {
		// Get the current directory's name
		string old_name = selected_dirs[a]->getName();

		// Prompt for a new name
		string new_name = wxGetTextFromUser("Enter new directory name:", S_FMT("Rename Directory %s", old_name.c_str()), old_name);

		// Do nothing if no name was entered
		if (new_name.IsEmpty())
			continue;

		// Discard any given path (for now)
		wxFileName fn(new_name);
		new_name = fn.GetName();

		// Rename the directory if the new entered name is different from the original
		if (new_name != old_name)
			archive->renameDir(selected_dirs[a], new_name);
	}

	return true;
}

/* ArchivePanel::deleteEntry
 * Deletes any selected entries from the archive
 *******************************************************************/
bool ArchivePanel::deleteEntry(bool confirm) {
	// Get a list of selected entries
	vector<ArchiveEntry*> selected_entries = entry_list->getSelectedEntries();

	// Get a list of selected directories
	vector<ArchiveTreeNode*> selected_dirs = entry_list->getSelectedDirectories();

	// Confirmation dialog
	if (confirm_entry_delete && confirm) {
		string item;
		int num = selected_entries.size() + selected_dirs.size();
		if (num == 1) {
			if (selected_entries.size() == 1)
				item = selected_entries[0]->getName();
			else
				item = selected_dirs[0]->getName();
		}
		else if (num > 0)
			item = S_FMT("these %d items", num);

		if (wxMessageBox(S_FMT("Are you sure you want to delete %s?", CHR(item)), "Delete Confirmation", wxYES_NO|wxICON_QUESTION) == wxNO)
			return false;
	}

	// Clear the selection
	entry_list->clearSelection();

	// Go through the selected entries
	for (int a = selected_entries.size() - 1; a >= 0; a--) {
		// Remove the current selected entry if it isn't a directory
		if (selected_entries[a]->getType() != EntryType::folderType())
			archive->removeEntry(selected_entries[a]);
	}

	// Go through the selected directories
	for (int a = selected_dirs.size() - 1; a >= 0; a--) {
		// Remove the selected directory from the archive
		archive->removeDir(selected_dirs[a]->getName(), entry_list->getCurrentDir());
	}

	// Switch to blank entry panel
	wxSizer* sizer = GetSizer();
	cur_area->Show(false);
	cur_area->nullEntry();
	sizer->Replace(cur_area, entry_area);
	cur_area = entry_area;
	cur_area->Show(true);
	Layout();

	return true;
}

/* ArchivePanel::revertEntry
 * Reverts any selected entries
 *******************************************************************/
bool ArchivePanel::revertEntry() {
	// Get selected entries
	vector<ArchiveEntry*> selected_entries = entry_list->getSelectedEntries();

	// Go through selection
	for (unsigned a = 0; a < selected_entries.size(); a++)
		archive->revertEntry(selected_entries[a]);

	// If the entries reverted were the only modified entries in the
	// archive, the archive is no longer modified.
	archive->findModifiedEntries();

	// If there was only one selected entry, chances are its content
	// were displayed, so this should be updated
	if (theActivePanel)
		theActivePanel->callRefresh();

	return true;
}

/* ArchivePanel::moveUp
 * Moves any selected entries up in the list
 *******************************************************************/
bool ArchivePanel::moveUp() {
	// Get selection
	vector<long> selection = entry_list->getSelection();

	// If nothing is selected, do nothing
	if (selection.size() == 0)
		return false;

	// If the first selected item is at the top of the list
	// or before entries start then don't move anything up
	if (selection[0] <= entry_list->entriesBegin())
		return false;

	// Move each one up by swapping it with the entry above it
	for (size_t a = 0; a < selection.size(); a++)
		archive->swapEntries(entry_list->getEntryIndex(selection[a]), entry_list->getEntryIndex(selection[a]-1), entry_list->getCurrentDir());

	// Update selection
	entry_list->clearSelection();
	for (unsigned a = 0; a < selection.size(); a++)
		entry_list->selectItem(selection[a] - 1);

	// Return success
	return true;
}

/* ArchivePanel::moveDown
 * Moves any selected entries down in the list
 *******************************************************************/
bool ArchivePanel::moveDown() {
	// Get selection
	vector<long> selection = entry_list->getSelection();

	// If nothing is selected, do nothing
	if (selection.size() == 0)
		return false;

	// If the last selected item is at the end of the list
	// then don't move anything down
	if (selection.back() == entry_list->GetItemCount()-1 || selection.back() < entry_list->entriesBegin())
		return false;

	// Move each one down by swapping it with the entry below it
	for (int a = selection.size()-1; a >= 0; a--)
		archive->swapEntries(entry_list->getEntryIndex(selection[a]), entry_list->getEntryIndex(selection[a]+1), entry_list->getCurrentDir());

	// Update selection
	entry_list->clearSelection();
	for (unsigned a = 0; a < selection.size(); a++)
		entry_list->selectItem(selection[a] + 1);

	// Return success
	return true;
}

/* ArchivePanel::bookmark
 * Adds the currently focused archive entry to the list of bookmarks
 *******************************************************************/
bool ArchivePanel::bookmark() {
	ArchiveEntry* entry = entry_list->getFocusedEntry();

	if (entry) {
		theArchiveManager->addBookmark(entry_list->getFocusedEntry());
		return true;
	}
	else
		return false;
}

/* ArchivePanel::openTab
 * Opens currently selected entries in separate tabs
 *******************************************************************/
bool ArchivePanel::openTab() {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Open each in its own tab
	for (unsigned a = 0; a < selection.size(); a++)
		theMainWindow->openEntry(selection[a]);

	return true;
}

/* ArchivePanel::crc32
 * Computes the CRC-32 checksums of the selected entries
 *******************************************************************/
bool ArchivePanel::crc32() {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Compute CRC-32 checksums for each
	string checksums = "\nCRC-32:\n";
	for (unsigned a = 0; a < selection.size(); a++) {
		uint32_t crc = selection[a]->getMCData().crc();
		checksums += S_FMT("%s:\t%x\n", CHR(selection[a]->getName()), crc);
	}
	wxLogMessage(checksums);
	wxMessageBox(checksums);

	return true;
}

/* ArchivePanel::convertEntryTo
 * Not implemented
 *******************************************************************/
bool ArchivePanel::convertEntryTo() {
	wxMessageBox("Not Implemented");
	return false;
}

/* ArchivePanel::importEntry
 * For each selected entry, opens a file selection dialog. The
 * selected file will be imported into the entry
 *******************************************************************/
bool ArchivePanel::importEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through the list
	for (size_t a = 0; a < selection.size(); a++) {
		// Run open file dialog
		SFileDialog::fd_info_t info;
		if (SFileDialog::openFile(info, "Import Entry \"" + selection[a]->getName() + "\"", "Any File (*.*)|*.*", this)) {

			// Preserve gfx offset if needed
			point2_t offset;
			if (!selection[a]->getType()->getEditor().Cmp("gfx")) {
				// We have an image
				SImage si;
				si.open(selection[a]->getMCData());
				offset = si.offset();
			}

			// If a file was selected, import it
			selection[a]->importFile(info.filenames[0]);

			// Re-detect entry type
			EntryType::detectEntryType(selection[a]);

			// Restore offsets if needed
			if (!selection[a]->getType()->getEditor().Cmp("gfx")) {
				SImage si;
				si.open(selection[a]->getMCData());

				point2_t noffset = si.offset();
				bool ok = true;
				// Don't bother if the same offsets are reimported
				if (offset == noffset) ok = false;
				// Ask for confirmation if there actually are offsets but they are different
				else if (noffset.x | noffset.y) {
					wxMessageDialog md(this,
						S_FMT("Image %s had offset [%d, %d], imported file has offset [%d, %d]. "
						"Do you want to keep the old offset and override the new?",
						CHR(selection[a]->getName()), offset.x, offset.y, noffset.x, noffset.y),
						"Conflicting Offsets", wxYES_NO);
					int result = md.ShowModal();
					if (result != wxID_YES)
						ok = false;
				}
				// Warn if the offsets couldn't be written
				if (ok && !si.getFormat()->writeOffset(si, selection[a], offset))
					wxLogMessage("Old offset information [%d, %d] couldn't be "
					"preserved in the new image format for image %s.",
					offset.x, offset.y, CHR(selection[a]->getName()));
			}

			// Set extension by type
			selection[a]->setExtensionByType();

			// If the entry is currently open, refresh the entry panel
			if (cur_area->getEntry() == selection[a])
				openEntry(selection[a], true);
		}
	}

	return true;
}

/* ArchivePanel::exportEntry
 * Exports any selected entries to files. If multiple entries are
 * selected, a directory selection dialog is shown, and any selected
 * entries will be exported to that directory
 *******************************************************************/
bool ArchivePanel::exportEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// If we're just exporting 1 entry
	if (selection.size() == 1) {
		// Setup filename
		wxFileName fn(selection[0]->getName());

		// Add appropriate extension if needed
		if (fn.GetExt().Len() == 0) fn.SetExt(selection[0]->getType()->getExtension());

		// Run save file dialog
		SFileDialog::fd_info_t info;
		if (SFileDialog::saveFile(info, "Export Entry \"" + selection[0]->getName() + "\"", "Any File (*.*)|*.*", this, fn.GetFullName()))
			selection[0]->exportFile(info.filenames[0]);	// Export entry if ok was clicked
	}
	else {
		// Run save files dialog
		SFileDialog::fd_info_t info;
		if (SFileDialog::saveFiles(info, "Export Multiple Entries (Filename is ignored)", "Any File (*.*)|*.*", this)) {
			// Go through the selection
			for (size_t a = 0; a < selection.size(); a++) {
				// Setup entry filename
				wxFileName fn(selection[a]->getName());
				fn.SetPath(info.path);

				// Add file extension if it doesn't exist
				if (!fn.HasExt())
					fn.SetExt(selection[a]->getType()->getExtension());

				// Do export
				selection[a]->exportFile(fn.GetFullPath());
			}
		}
	}

	return true;
}

/* ArchivePanel::exportEntryAs
 * Not implemented
 *******************************************************************/
bool ArchivePanel::exportEntryAs() {
	wxMessageBox("Not Implemented");
	return false;
}

/* ArchivePanel::copyEntry
 * Copies selected entries+directories to the clipboard
 *******************************************************************/
bool ArchivePanel::copyEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> entries = entry_list->getSelectedEntries();

	// Get a list of selected directories
	vector<ArchiveTreeNode*> dirs = entry_list->getSelectedDirectories();

	// Do nothing if nothing selected
	if (entries.size() == 0 && dirs.size() == 0)
		return false;

	// Create clipboard item from selection
	theClipboard->addItem(new EntryTreeClipboardItem(entries, dirs));

	return true;
}

/* ArchivePanel::cutEntry
 * Copies selected entries+diretories to the clipboard, and deletes
 * them from the archive
 *******************************************************************/
bool ArchivePanel::cutEntry() {
	if (copyEntry())
		return deleteEntry(false);
	else
		return false;
}

/* ArchivePanel::pasteEntry
 * Pastes any entries and directories on the clipboard into the
 * current directory. Entries will be pasted after the last selected
 * entry, whereas directories will be pasted after any
 * subdirectories. Pasting a directory will also paste any entries
 * and subdirectories within it.
 *******************************************************************/
bool ArchivePanel::pasteEntry() {
	// Do nothing if there is nothing in the clipboard
	if (theClipboard->nItems() == 0)
		return false;

	// Get the entry index of the last selected list item
	int index = archive->entryIndex(entry_list->getLastSelectedEntry(), entry_list->getCurrentDir());

	// If something was selected, add 1 to the index so we add the new entry after the last selected
	if (index >= 0)
		index++;
	else
		index = -1;	// If not add to the end of the list

	// Go through all clipboard items
	bool pasted = false;
	for (unsigned a = 0; a < theClipboard->nItems(); a++) {
		// Check item type
		if (theClipboard->getItem(a)->getType() != CLIPBOARD_ENTRY_TREE)
			continue;

		// Get clipboard item
		EntryTreeClipboardItem* clip = (EntryTreeClipboardItem*)theClipboard->getItem(a);

		// Merge it in
		if (archive->paste(clip->getTree(), index, entry_list->getCurrentDir()))
			pasted = true;
	}

	if (pasted) {
		// Update archive
		archive->setModified(true);

		return true;
	}
	else
		return false;
}

/* ArchivePanel::gfxConvert
 * Opens the Gfx Conversion dialog and sends selected entries to it
 *******************************************************************/
bool ArchivePanel::gfxConvert() {
	// Create gfx conversion dialog
	GfxConvDialog gcd;

	// Send selection to the gcd
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();
	gcd.openEntries(selection);

	// Run the gcd
	gcd.ShowModal();

	// Show splash window
	theSplashWindow->show("Writing converted image data...", true);

	// Write any changes
	for (unsigned a = 0; a < selection.size(); a++) {
		// Update splash window
		theSplashWindow->setProgressMessage(selection[a]->getName());
		theSplashWindow->setProgress((float)a / (float)selection.size());

		// Skip if the image wasn't converted
		if (!gcd.itemModified(a))
			continue;

		// Get image and conversion info
		SImage* image = gcd.getItemImage(a);
		SIFormat* format = gcd.getItemFormat(a);

		// Write converted image back to entry
		MemChunk mc;
		format->saveImage(*image, mc, gcd.getItemPalette(a));
		selection[a]->importMemChunk(mc);
		EntryType::detectEntryType(selection[a]);
		selection[a]->setExtensionByType();
	}

	// Hide splash window
	theSplashWindow->hide();

	return true;
}

/* ArchivePanel::gfxModifyOffsets
 * Opens the Translation editor dialog to remap colours on selected
 * gfx entries
 *******************************************************************/
bool ArchivePanel::gfxRemap() {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Create preview image (just use first selected entry)
	SImage image(PALMASK);
	Misc::loadImageFromEntry(&image, selection[0]);

	// Create translation editor dialog
	Palette8bit* pal = theMainWindow->getPaletteChooser()->getSelectedPalette();
	TranslationEditorDialog ted(this, pal, "Colour Remap", &image);
	ted.openTranslation(((GfxEntryPanel*)gfx_area)->prevTranslation());

	// Run dialog
	if (ted.ShowModal() == wxID_OK) {
		// Apply translation to all entry images
		SImage temp;
		MemChunk mc;
		for (unsigned a = 0; a < selection.size(); a++) {
			ArchiveEntry* entry = selection[a];
			if (Misc::loadImageFromEntry(&temp, entry)) {
				// Apply translation
				temp.applyTranslation(&ted.getTranslation(), pal);

				// Write modified image data
				if (!temp.getFormat()->saveImage(temp, mc, pal))
					wxLogMessage("Error: Could not write image data to entry %s, unsupported format for writing", CHR(entry->getName()));
				else
					entry->importMemChunk(mc);
			}
		}

		// Update variables
		((GfxEntryPanel*)gfx_area)->prevTranslation().copy(ted.getTranslation());
	}

	return true;
}

/* ArchivePanel::gfxModifyOffsets
 * Opens the Modify Offsets dialog to mass-modify offsets of any
 * selected, offset-compatible gfx entries
 *******************************************************************/
bool ArchivePanel::gfxModifyOffsets() {
	// Create modify offsets dialog
	ModifyOffsetsDialog mod;

	// Run the dialog
	if (mod.ShowModal() == wxID_CANCEL)
		return false;

	// Go through selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();
	for (uint32_t a = 0; a < selection.size(); a++)
		EntryOperations::modifyGfxOffsets(selection[a], mod.getAlignType(), mod.getOffset(), mod.xOffChange(), mod.yOffChange(), mod.relativeOffset());

	return true;
}

/* ArchivePanel::gfxExportPNG
 * Exports any selected gfx entries as png format images
 *******************************************************************/
bool ArchivePanel::gfxExportPNG() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// If we're just exporting 1 entry
	if (selection.size() == 1) {
		// Setup filename
		wxFileName fn(selection[0]->getName());

		// Set extension
		fn.SetExt("png");

		// Run save file dialog
		SFileDialog::fd_info_t info;
		if (SFileDialog::saveFile(info, "Export Entry \"" + selection[0]->getName() + "\" as PNG", "PNG Files (*.png)|*.png", this, fn.GetFullName())) {
			// If a filename was selected, export it
			if (!EntryOperations::exportAsPNG(selection[0], info.filenames[0])) {
				wxMessageBox(S_FMT("Error: %s", CHR(Global::error)), "Error", wxOK|wxICON_ERROR);
				return false;
			}
		}

		return true;
	}
	else {
		// Run save files dialog
		SFileDialog::fd_info_t info;
		if (SFileDialog::saveFiles(info, "Export Entries as PNG (Filename will be ignored)", "PNG Files (*.png)|*.png", this)) {
			// Go through the selection
			for (size_t a = 0; a < selection.size(); a++) {
				// Setup entry filename
				wxFileName fn(selection[a]->getName());
				fn.SetPath(info.path);
				fn.SetExt("png");

				// Do export
				EntryOperations::exportAsPNG(selection[a], fn.GetFullPath());
			}
		}
	}

	return true;
}

/* ArchivePanel::currentEntry
 * Returns the entry currently open for editing
 *******************************************************************/
ArchiveEntry * ArchivePanel::currentEntry() {
	if (entry_list->GetSelectedItemCount() == 1)
		return cur_area->getEntry();
	else
		return NULL;
}

/* ArchivePanel::currentEntry
 * Returns a vector of all selected entries
 *******************************************************************/
vector<ArchiveEntry*> ArchivePanel::currentEntries() {
	vector<ArchiveEntry*> selection;
	if (entry_list)
		selection = entry_list->getSelectedEntries();
	return selection;
}

/* ArchivePanel::basConvert
 * Converts any selected SWITCHES or ANIMATED entries to a newly
 * created ANIMDEFS entry
 *******************************************************************/
bool ArchivePanel::basConvert() {
	// Get the entry index of the last selected list item
	int index = archive->entryIndex(currentEntry(), entry_list->getCurrentDir());

	// If something was selected, add 1 to the index so we add the new entry after the last selected
	if (index >= 0)
		index++;
	else
		return false; // If not there's a problem

	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Create new entry
	ArchiveEntry * animdef = archive->addNewEntry((archive->getType() == ARCHIVE_WAD ?
		"ANIMDEFS" : "animdefs.txt"), index, entry_list->getCurrentDir());

	if (animdef) {
		// Create the memory buffer
		MemChunk animdata(38);
		animdata.seek(0, SEEK_SET);
		animdata.write("// ANIMDEFS lump generated by SLADE3\n\n", 38);

		// Check each selected entry for possible conversion
		for (size_t a = 0; a < selection.size(); a++) {
			if (selection[a]->getType()->getFormat() == "animated")
				AnimatedList::convertAnimated(selection[a], &animdata);
			else if (selection[a]->getType()->getFormat() == "switches")
				SwitchesList::convertSwitches(selection[a], &animdata);
		}
		animdef->importMemChunk(animdata);

		// Identify the new lump as what it is
		EntryType::detectEntryType(animdef);
		// Failsafe is detectEntryType doesn't accept to work, grumble
		if (animdef->getType() == EntryType::unknownType())
			animdef->setType(EntryType::getType("animdefs"));
	}

	// Force entrylist width update
	Layout();

	return true;
}

/* ArchivePanel::palConvert
 * Unused (converts 6-bit palette to 8-bit)
 *******************************************************************/
bool ArchivePanel::palConvert() {
	// Get the entry index of the last selected list item
	ArchiveEntry* pal6bit = currentEntry();
	const uint8_t * source = pal6bit->getData(true);
	uint8_t * dest = new uint8_t[pal6bit->getSize()];
	memcpy(dest, source, pal6bit->getSize());
	for (size_t i = 0; i < pal6bit->getSize(); ++i)
	{
		dest[i] = ((dest[i] << 2) | (dest[i] >> 4));
	}
	pal6bit->importMem(dest, pal6bit->getSize());
	delete[] dest;
	return true;
}

/* ArchivePanel::wavDSndConvert
 * Converts selected wav format entries to doom sound format
 *******************************************************************/
bool ArchivePanel::wavDSndConvert() {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		// Convert WAV -> Doom Sound if the entry is WAV format
		if (selection[a]->getType()->getFormat() == "snd_wav") {
			MemChunk dsnd;
			// Attempt conversion
			if (!Conversions::wavToDoomSnd(selection[a]->getMCData(), dsnd)) {
				wxLogMessage("Error: Unable to convert entry %s: %s", CHR(selection[a]->getName()), CHR(Global::error));
				continue;
			}
			selection[a]->importMemChunk(dsnd);							// Load doom sound data
			EntryType::detectEntryType(selection[a]);					// Update entry type
			selection[a]->setExtensionByType();							// Update extension if necessary
		}
	}

	return true;
}

/* ArchivePanel::dSndWavConvert
 * Converts selected doom sound format entries to wav format
 *******************************************************************/
bool ArchivePanel::dSndWavConvert() {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		bool worked = false;
		MemChunk wav;
		// Convert Doom Sound -> WAV if the entry is Doom Sound format
		if (selection[a]->getType()->getFormat() == "snd_doom")
			worked = Conversions::doomSndToWav(selection[a]->getMCData(), wav);
		// Or Creative Voice File format
		else if (selection[a]->getType()->getFormat() == "snd_voc")
			worked = Conversions::vocToWav(selection[a]->getMCData(), wav);
		// Or Blood SFX format (this one needs to be given the entry, not just the mem chunk)
		else if (selection[a]->getType()->getFormat() == "snd_bloodsfx")
			worked = Conversions::bloodToWav(selection[a], wav);
		// If successfully converted, update the entry
		if (worked) {
			selection[a]->importMemChunk(wav);			// Load wav data
			EntryType::detectEntryType(selection[a]);	// Update entry type
			selection[a]->setExtensionByType();			// Update extension if necessary
		} else {
			wxLogMessage("Error: Unable to convert entry %s: %s", CHR(selection[a]->getName()), CHR(Global::error));
			continue;
		}
	}

	return true;
}

/* ArchivePanel::musMidiConvert
 * Converts selected mus format entries to midi format
 *******************************************************************/
bool ArchivePanel::musMidiConvert() {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		// Convert MUS -> MIDI if the entry is Doom MUS format
		if (selection[a]->getType()->getFormat() == "mus") {
			MemChunk midi;
			Conversions::musToMidi(selection[a]->getMCData(), midi);	// Convert
			selection[a]->importMemChunk(midi);							// Load midi data
			EntryType::detectEntryType(selection[a]);					// Update entry type
			selection[a]->setExtensionByType();							// Update extension if necessary
		}
	}

	return true;
}

/* ArchivePanel::compileACS
 * Compiles any selected text entries as ACS scripts
 *******************************************************************/
bool ArchivePanel::compileACS(bool hexen) {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		// Compile ACS script
		EntryOperations::compileACS(selection[a], hexen);
	}

	return true;
}

/* ArchivePanel::optimizePNG
 * Compiles any selected text entries as ACS scripts
 *******************************************************************/
bool ArchivePanel::optimizePNG() {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	theSplashWindow->show("Running external programs, please wait...", true);

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		theSplashWindow->Raise();
		theSplashWindow->setProgressMessage(selection[a]->getName(true));
		theSplashWindow->setProgress(float(a) / float(selection.size()));
		if (selection[a]->getType()->getFormat() == "img_png")
			EntryOperations::optimizePNG(selection[a]);
	}
	theSplashWindow->hide();
	theMainWindow->Raise();

	return true;
}

/* ArchivePanel::convertTextures
 * Converts any selected TEXTUREx entries to a ZDoom TEXTURES entry
 *******************************************************************/
bool ArchivePanel::convertTextures() {
	// Get selected entries
	long index = entry_list->getSelection()[0];
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Do conversion
	if (EntryOperations::convertTextures(selection)) {
		// Select new TEXTURES entry
		entry_list->clearSelection();
		entry_list->selectItem(index);

		return true;
	}

	return false;
}

/* ArchivePanel::mapOpenDb2
 * Opens the currently selected entry in Doom Builder 2 if it is a
 * valid map entry (either a map header or archive in maps/)
 *******************************************************************/
bool ArchivePanel::mapOpenDb2() {
	// Get first selected entry
	ArchiveEntry* entry = entry_list->getEntry(entry_list->getFirstSelected());

	// Do open in db2
	return EntryOperations::openMapDB2(entry);
}


/* ArchivePanel::openEntry
 * Shows the appropriate entry area and sends the given entry to it.
 * If [force] is true, the entry is opened even if it is already open
 *******************************************************************/
bool ArchivePanel::openEntry(ArchiveEntry* entry, bool force) {
	// Null entry, do nothing
	if (!entry) {
		wxLogMessage("Warning: NULL entry focused in the list");
		return false;
	}

	// Do nothing if the entry is already open
	if (cur_area->getEntry() == entry && !force)
		return false;

	// Detect entry type if it hasn't been already
	if (entry->getType() == EntryType::unknownType())
		EntryType::detectEntryType(entry);

	// Are we trying to open a directory? This can happen from bookmarks.
	if (entry->getType() == EntryType::folderType()) {
		// Get directory to open
		ArchiveTreeNode* dir = NULL;

		// Removes starting / from path
		string name = entry->getPath(true);
		if (name.StartsWith("/"))
			name.Remove(0, 1);

		dir = archive->getDir(name, NULL);

		// Check it exists (really should)
		if (!dir) {
			wxLogMessage("Error: Trying to open nonexistant directory %s", CHR(name));
			return false;
		}
		entry_list->setDir(dir);
	} else {
		// Close the current entry
		cur_area->closeEntry();

		// Get the appropriate entry panel for the entry's type
		EntryPanel* new_area = default_area;
		if (entry->getType() == EntryType::mapMarkerType())
			new_area = map_area;
		else if (!entry->getType()->getEditor().Cmp("gfx"))
			new_area = gfx_area;
		else if (!entry->getType()->getEditor().Cmp("palette"))
			new_area = pal_area;
		else if (!entry->getType()->getEditor().Cmp("ansi"))
			new_area = ansi_area;
		else if (!entry->getType()->getEditor().Cmp("text"))
			new_area = text_area;
		else if (!entry->getType()->getEditor().Cmp("animated"))
			new_area = animated_area;
		else if (!entry->getType()->getEditor().Cmp("switches"))
			new_area = switches_area;
		else if (!entry->getType()->getEditor().Cmp("audio"))
			new_area = audio_area;
		else if (!entry->getType()->getEditor().Cmp("default"))
			new_area = default_area;
		else
			wxLogMessage("Entry editor %s does not exist, using default editor", entry->getType()->getEditor().c_str());

		// Show the new entry panel
		if (!showEntryPanel(new_area))
			return false;

		// Load the entry into the panel
		if (!cur_area->openEntry(entry)) {
			wxMessageBox(S_FMT("Error loading entry:\n%s", Global::error.c_str()), "Error", wxOK|wxICON_ERROR);
		}
	}
	return true;
}

/* ArchivePanel::openEntryAsText
 * Opens [entry] in the text editor panel
 *******************************************************************/
bool ArchivePanel::openEntryAsText(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Show the text entry panel
	if (!showEntryPanel(text_area))
		return false;

	// Load the current entry into the panel
	if (!cur_area->openEntry(entry)) {
		wxMessageBox(S_FMT("Error loading entry:\n%s", Global::error.c_str()), "Error", wxOK|wxICON_ERROR);
	}

	return true;
}

/* ArchivePanel::openEntryAsHex
 * Opens [entry] in the hex editor panel
 *******************************************************************/
bool ArchivePanel::openEntryAsHex(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Show the text entry panel
	if (!showEntryPanel(hex_area))
		return false;

	// Load the current entry into the panel
	if (!cur_area->openEntry(entry)) {
		wxMessageBox(S_FMT("Error loading entry:\n%s", Global::error.c_str()), "Error", wxOK|wxICON_ERROR);
	}

	return true;
}

/* ArchivePanel::reloadCurrentPanel
 * If only one entry is selected, force its reload
 *******************************************************************/
bool ArchivePanel::reloadCurrentPanel() {
	// Do nothing if there are multiple entries
	if (cur_area == default_area)
		return false;

	return openEntry(cur_area->getEntry(), true);
}

/* ArchivePanel::focusOnEntry
 * Makes sure the list view display shows the given entry
 *******************************************************************/
void ArchivePanel::focusOnEntry(ArchiveEntry* entry) {
	if (entry) {
		// Do we need to change directory?
		if (entry->getParentDir() != entry_list->getCurrentDir())
			entry_list->setDir(entry->getParentDir());

		// Now focus on the entry if it is listed
		for (long index = 0; index < entry_list->GetItemCount(); ++index) {
			if (entry == entry_list->getEntry(index)) {
				entry_list->focusOnIndex(index);
				return;
			}
		}
	}
}

/* ArchivePanel::showEntryPanel
 * Show an entry panel appropriate to the current entry
 *******************************************************************/
bool ArchivePanel::showEntryPanel(EntryPanel* new_area, bool ask_save) {
	// Save any changes if needed
	saveEntryChanges();

	// Get the panel sizer
	wxSizer* sizer = GetSizer();

	// If the new panel is different than the current, swap them
	if (new_area != cur_area) {
		cur_area->Show(false);				// Hide current
		cur_area->removeCustomMenu();		// Remove current custom menu (if any)
		sizer->Replace(cur_area, new_area);	// Swap the panels
		cur_area = new_area;				// Set the new panel to current
		cur_area->Show(true);				// Show current

		// Add the current panel's custom menu if needed
		cur_area->addCustomMenu();

		// Update panel layout
		Layout();
		theMainWindow->Update();
		theMainWindow->Refresh();
		theMainWindow->Update();
	}

	return true;
}

/* ArchivePanel::refreshPanel
 * Refreshes everything on the panel
 *******************************************************************/
void ArchivePanel::refreshPanel() {
	// Refresh entry list
	entry_list->applyFilter();

	// Refresh current entry panel
	cur_area->refreshPanel();

	// Refresh entire panel
	Update();
	Refresh();
}

/* ArchivePanel::handleAction
 * Handles the action [id]. Returns true if the action was handled,
 * false otherwise
 *******************************************************************/
bool ArchivePanel::handleAction(string id) {
	// Don't handle actions if hidden
	if (!IsShown())
		return false;

	// We're only interested in "arch_" actions
	if (!id.StartsWith("arch_"))
		return false;


	// *************************************************************
	// ARCHIVE MENU
	// *************************************************************

	// Archive->New->Entry
	else if (id == "arch_newentry")
		newEntry();

	// Archive->New->Directory
	else if (id == "arch_newdir")
		newDirectory();

	// Archive->Import Files
	else if (id == "arch_importfiles")
		importFiles();

	// Archive->Texture Editor
	else if (id == "arch_texeditor")
		theMainWindow->openTextureEditor(archive);

	// Archive->Convert To...
	else if (id == "arch_convert")
		convertArchiveTo();

	// Archive->Maintenance->Remove Unused Patches
	else if (id == "arch_clean_patches")
		ArchiveOperations::removeUnusedPatches(archive);

	// Archive->Maintenance->Remove Unused Textures
	else if (id == "arch_clean_textures")
		ArchiveOperations::removeUnusedTextures(archive);

	// Archive->Maintenance->Remove Unused Flats
	else if (id == "arch_clean_flats")
		ArchiveOperations::removeUnusedFlats(archive);

	// Archive->Maintenance->Check Duplicate Entry Names
	else if (id == "arch_check_duplicates")
		ArchiveOperations::checkDuplicateEntryNames(archive);


	// *************************************************************
	// ENTRY MENU
	// *************************************************************

	// Entry->Rename
	else if (id == "arch_entry_rename")
		renameEntry();

	// Entry->Rename Each
	else if (id == "arch_entry_rename_each")
		renameEntry(true);

	// Entry->Delete
	else if (id == "arch_entry_delete")
		deleteEntry();

	else if (id == "arch_entry_revert")
		revertEntry();

	// Entry->Cut
	else if (id == "arch_entry_cut")
		cutEntry();

	// Entry->Copy
	else if (id == "arch_entry_copy")
		copyEntry();

	// Entry->Paste
	else if (id == "arch_entry_paste")
		pasteEntry();

	// Entry->Move Up
	else if (id == "arch_entry_moveup")
		moveUp();

	// Entry->Move Down
	else if (id == "arch_entry_movedown")
		moveDown();

	// Entry->Bookmark
	else if (id == "arch_entry_bookmark")
		bookmark();

	// Open in Tab
	else if (id == "arch_entry_opentab")
		openTab();

	// CRC-32
	else if (id == "arch_entry_crc32")
		crc32();

	// Entry->Convert To...
	else if (id == "arch_entry_convert")
		convertEntryTo();

	// Entry->Import
	else if (id == "arch_entry_import")
		importEntry();

	// Entry->Export
	else if (id == "arch_entry_export")
		exportEntry();

	// Entry->Export As...
	else if (id == "arch_entry_exportas")
		exportEntryAs();



	// Context menu actions
	else if (id == "arch_bas_convert")
		basConvert();
	else if (id == "arch_gfx_convert")
		gfxConvert();
	else if (id == "arch_gfx_translate")
		gfxRemap();
	else if (id == "arch_gfx_offsets")
		gfxModifyOffsets();
	else if (id == "arch_gfx_addptable")
		EntryOperations::addToPatchTable(entry_list->getSelectedEntries());
	else if (id == "arch_gfx_addtexturex")
		EntryOperations::createTexture(entry_list->getSelectedEntries());
	else if (id == "arch_gfx_exportpng")
		gfxExportPNG();
	else if (id == "arch_gfx_pngopt")
		optimizePNG();
	else if (id == "arch_view_text")
		openEntryAsText(entry_list->getFocusedEntry());
	else if (id == "arch_view_hex")
		openEntryAsHex(entry_list->getFocusedEntry());
	else if (id == "arch_audio_convertdw")
		dSndWavConvert();
	else if (id == "arch_audio_convertwd")
		wavDSndConvert();
	else if (id == "arch_audio_convertmus")
		musMidiConvert();
	else if (id == "arch_scripts_compileacs")
		compileACS();
	else if (id == "arch_scripts_compilehacs")
		compileACS(true);
	else if (id == "arch_texturex_convertzd")
		convertTextures();
	else if (id == "arch_map_opendb2")
		mapOpenDb2();

	// Unknown action
	else
		return false;

	// Action handled, return true
	return true;
}

/* ArchivePanel::onAnnouncement
 * Called when an announcement is recieved from the archive that
 * this ArchivePanel is managing
 *******************************************************************/
void ArchivePanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Reset event data for reading
	event_data.seek(0, SEEK_SET);

	// If the archive was saved
	if (announcer == archive && event_name == "saved") {
		// Update this tab's name in the parent notebook (if filename was changed)
		wxAuiNotebook* parent = (wxAuiNotebook*)GetParent();
		parent->SetPageText(parent->GetPageIndex(this), archive->getFilename(false));
	}

	// If a directory was added
	if (announcer == archive && event_name == "directory_added") {
		// Show path controls (if they aren't already)
		if (!GetSizer()->IsShown(sizer_path_controls)) {
			sizer_path_controls->Show(true);
			Layout();
		}
	}
}


/*******************************************************************
 * ARCHIVEPANEL STATIC FUNCTIONS
 *******************************************************************/

/* ArchivePanel::createPanelForEntry
 * Creates the appropriate EntryPanel for [entry] and returns it
 *******************************************************************/
EntryPanel* ArchivePanel::createPanelForEntry(ArchiveEntry* entry, wxWindow* parent) {
	EntryPanel* entry_panel = NULL;

	if (entry->getType() == EntryType::mapMarkerType())
		entry_panel = new MapEntryPanel(parent);
	else if (!entry->getType()->getEditor().Cmp("gfx"))
		entry_panel = new GfxEntryPanel(parent);
	else if (!entry->getType()->getEditor().Cmp("palette"))
		entry_panel = new PaletteEntryPanel(parent);
	else if (!entry->getType()->getEditor().Cmp("ansi"))
		entry_panel = new ANSIEntryPanel(parent);
	else if (!entry->getType()->getEditor().Cmp("text"))
		entry_panel = new TextEntryPanel(parent);
	else if (!entry->getType()->getEditor().Cmp("animated"))
		entry_panel = new AnimatedEntryPanel(parent);
	else if (!entry->getType()->getEditor().Cmp("switches"))
		entry_panel = new SwitchesEntryPanel(parent);
	else if (!entry->getType()->getEditor().Cmp("audio"))
		entry_panel = new AudioEntryPanel(parent);
	else
		entry_panel = new DefaultEntryPanel(parent);

	return entry_panel;
}


/*******************************************************************
 * ARCHIVEPANEL EVENTS
 *******************************************************************/

/* ArchivePanel::onEntryListSelectionChange
 * Called when the selection on the entry list is changed
 *******************************************************************/
void ArchivePanel::onEntryListSelectionChange(wxCommandEvent& e) {
	// Do nothing if not shown
	if (!IsShown())
		return;

	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	if (selection.size() == 0)
		return;	// If no entries are selected do nothing
	else if (selection.size() == 1) {
		// If one entry is selected, open it in the entry area
		openEntry(selection[0]);
	}
	else {
		// If multiple entries are selected, show/update the multi entry area
		showEntryPanel(default_area);
		((DefaultEntryPanel*)default_area)->loadEntries(selection);
	}
}

/* ArchivePanel::onEntryListFocusChange
 * Called when the focused item on the entry list is changed
 *******************************************************************/
void ArchivePanel::onEntryListFocusChange(wxListEvent& e) {
	// Do nothing if not shown
	if (!IsShown())
		return;

	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	if (selection.size() == 0)
		return;	// If no entries are selected do nothing
	else if (selection.size() == 1) {
		// If one entry is selected, open it in the entry area
		openEntry(selection[0]);
	}
	else {
		// If multiple entries are selected, show/update the multi entry area
		showEntryPanel(default_area);
		((DefaultEntryPanel*)default_area)->loadEntries(selection);
	}
}

/* ArchivePanel::onEntryListRightClick
 * Called when the entry list is right clicked
 *******************************************************************/
void ArchivePanel::onEntryListRightClick(wxListEvent& e) {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Check what types exist in the selection
	// TODO: This stuff is absolutely terrible, nicer system needed
	bool gfx_selected = false;
	bool png_selected = false;
	bool bas_selected = false;
	bool wav_selected = false;
	bool dsnd_selected = false;
	bool mus_selected = false;
	bool text_selected = false;
	bool unknown_selected = false;
	bool texturex_selected = false;
	bool modified_selected = false;
	bool map_selected = false;
//	bool rle_selected = false;
	for (size_t a = 0; a < selection.size(); a++) {
		// Check for gfx entry
		if (!gfx_selected) {
			if (selection[a]->getType()->extraProps().propertyExists("image"))
				gfx_selected = true;
		}
		if (!png_selected) {
			if (selection[a]->getType()->getFormat() == "img_png")
				png_selected = true;
		}
		if (!bas_selected) {
			if (selection[a]->getType()->getFormat() == "animated" ||
				selection[a]->getType()->getFormat() == "switches")
				bas_selected = true;
		}
		if (!wav_selected) {
			if (selection[a]->getType()->getFormat() == "snd_wav")
				wav_selected = true;
		}
		if (!dsnd_selected) {
			if (selection[a]->getType()->getFormat() == "snd_doom" ||
				selection[a]->getType()->getFormat() == "snd_doom64" ||
				selection[a]->getType()->getFormat() == "snd_bloodsfx" ||
				selection[a]->getType()->getFormat() == "snd_voc")
				dsnd_selected = true;
		}
		if (!mus_selected) {
			if (selection[a]->getType()->getFormat() == "mus")
				mus_selected = true;
		}
		if (!text_selected) {
			if (selection[a]->getType()->getFormat() == "text")
				text_selected = true;
		}
		if (!unknown_selected) {
			if (selection[a]->getType() == EntryType::unknownType())
				unknown_selected = true;
		}
		if (!texturex_selected) {
			if (selection[a]->getType()->getFormat() == "texturex")
				texturex_selected = true;
		}
		if (!modified_selected) {
			if (selection[a]->getState() == 1)
				modified_selected = true;
		}
		if (!map_selected) {
			if (selection[a]->getType() == EntryType::mapMarkerType())
				map_selected = true;
			else if (selection[a]->getParentDir()->getName() == "maps")
				map_selected = true;
		}
#if 0
		if (!rle_selected) {
			if (selection[a]->getType()->getFormat() == "misc_rle0")
				rle_selected = true;
		}
#endif
	}

	// Generate context menu
	wxMenu context;
	theApp->getAction("arch_entry_rename")->addToMenu(&context);
	if (selection.size() > 1) theApp->getAction("arch_entry_rename_each")->addToMenu(&context);
	theApp->getAction("arch_entry_delete")->addToMenu(&context);
	if (modified_selected) theApp->getAction("arch_entry_revert")->addToMenu(&context);
	context.AppendSeparator();
	theApp->getAction("arch_entry_cut")->addToMenu(&context);
	theApp->getAction("arch_entry_copy")->addToMenu(&context);
	theApp->getAction("arch_entry_paste")->addToMenu(&context);
	context.AppendSeparator();
	theApp->getAction("arch_entry_import")->addToMenu(&context);
	theApp->getAction("arch_entry_export")->addToMenu(&context);
	context.AppendSeparator();
	theApp->getAction("arch_entry_moveup")->addToMenu(&context);
	theApp->getAction("arch_entry_movedown")->addToMenu(&context);
	context.AppendSeparator();
	theApp->getAction("arch_entry_bookmark")->addToMenu(&context);
	theApp->getAction("arch_entry_opentab")->addToMenu(&context);
	theApp->getAction("arch_entry_crc32")->addToMenu(&context);

	// Add custom menu items
	wxMenu* custom;
	if (context_submenus) {
		custom = new wxMenu();
	} else custom = &context;
	bool ok = cur_area->fillCustomMenu(custom);
	if (context_submenus) {
		if (ok) context.AppendSubMenu(custom, cur_area->getCustomMenuName());
		else delete custom;
	}

	// Add Boom Animations/Switches related menu items if they are selected
	if (bas_selected)
		theApp->getAction("arch_bas_convert")->addToMenu(&context);

	// Add texturex related menu items if needed
	if (texturex_selected)
		theApp->getAction("arch_texturex_convertzd")->addToMenu(&context);

	// 'View As' menu
	if (context_submenus) {
		wxMenu* viewas = new wxMenu();
		context.AppendSubMenu(viewas, "View As");
		theApp->getAction("arch_view_text")->addToMenu(viewas, "Text");
		theApp->getAction("arch_view_hex")->addToMenu(viewas, "Hex");
	}
	else {
		context.AppendSeparator();
		theApp->getAction("arch_view_text")->addToMenu(&context);
		theApp->getAction("arch_view_hex")->addToMenu(&context);
	}

	// Add gfx-related menu items if gfx are selected
	if (gfx_selected) {
		wxMenu* gfx;
		if (context_submenus) {
			gfx = new wxMenu();
			context.AppendSubMenu(gfx, "Gfx");
		}
		else {
			context.AppendSeparator();
			gfx = &context;
		}
		theApp->getAction("arch_gfx_convert")->addToMenu(gfx);
		theApp->getAction("arch_gfx_translate")->addToMenu(gfx);
		theApp->getAction("arch_gfx_offsets")->addToMenu(gfx);
		theApp->getAction("arch_gfx_addptable")->addToMenu(gfx);
		theApp->getAction("arch_gfx_addtexturex")->addToMenu(gfx);
		theApp->getAction("arch_gfx_exportpng")->addToMenu(gfx);
		if (png_selected)
			theApp->getAction("arch_gfx_pngopt")->addToMenu(gfx);
	}

	// Add Audio related menu items if needed
	if (wav_selected || dsnd_selected || mus_selected) {
		wxMenu* audio;
		if (context_submenus) {
			audio = new wxMenu();
			context.AppendSubMenu(audio, "Audio");
		}
		else {
			context.AppendSeparator();
			audio = &context;
		}
		if (wav_selected)
			theApp->getAction("arch_audio_convertwd")->addToMenu(audio);
		if (dsnd_selected)
			theApp->getAction("arch_audio_convertdw")->addToMenu(audio);
		if (mus_selected)
			theApp->getAction("arch_audio_convertmus")->addToMenu(audio);
	}

	// Add script related menu items if needed
	if (text_selected || unknown_selected) {
		wxMenu* scripts;
		if (context_submenus) {
			scripts = new wxMenu();
			context.AppendSubMenu(scripts, "Scripts");
		}
		else {
			context.AppendSeparator();
			scripts = &context;
		}
		theApp->getAction("arch_scripts_compileacs")->addToMenu(scripts);
		theApp->getAction("arch_scripts_compilehacs")->addToMenu(scripts);
	}

	// Add map related menu items if needed
	if (map_selected) {
		// 'Open in Doom Builder 2' (windows-only)
#ifdef __WXMSW__
		theApp->getAction("arch_map_opendb2")->addToMenu(&context);
#endif
	}

	// Popup the context menu
	PopupMenu(&context);
}

/* ArchivePanel::onEntryListKeyDown
 * Called when a key is pressed on the entry list
 *******************************************************************/
void ArchivePanel::onEntryListKeyDown(wxKeyEvent& e) {
	// Check if keypress matches any keybinds
	wxArrayString binds = KeyBind::getBinds(KeyBind::asKeyPress(e.GetKeyCode(), e.GetModifiers()));

	// Go through matching binds
	for (unsigned a = 0; a < binds.size(); a++) {
		string name = binds[a];

		// --- General ---

		// Copy
		if (name == "copy") {
			copyEntry();
			return;
		}

		// Cut
		else if (name == "cut") {
			cutEntry();
			return;
		}

		// Paste
		else if (name == "paste") {
			pasteEntry();
			return;
		}

		// Select All
		else if (name == "select_all") {
			entry_list->selectAll();
			return;
		}


		// --- Entry list specific ---

		// New Entry
		else if (name == "el_new") {
			newEntry();
			return;
		}

		// Rename Entry
		else if (name == "el_rename") {
			renameEntry();
			return;
		}

		// Delete Entry
		else if (name == "el_delete") {
			deleteEntry();
			return;
		}

		// Move Entry up
		else if (name == "el_move_up") {
			moveUp();
			return;
		}

		// Move Entry down
		else if (name == "el_move_down") {
			moveDown();
			return;
		}

		// Import to Entry
		else if (name == "el_import") {
			importEntry();
			return;
		}

		// Import Files
		else if (name == "el_import_files") {
			importFiles();
			return;
		}

		// Export Entry
		else if (name == "el_export") {
			exportEntry();
			return;
		}

		// Up directory
		else if (name == "el_up_dir") {
			entry_list->goUpDir();
			return;
		}
	}

	// Not handled, send to parent
	e.Skip();
}

/* ArchivePanel::onEntryListActivated
 * Called when an item on the entry list is 'activated'
 * (via double-click or enter)
 *******************************************************************/
void ArchivePanel::onEntryListActivated(wxListEvent& e) {
	ArchiveEntry* entry = entry_list->getFocusedEntry();

	if (!entry)
		return;

	// Archive
	if (entry->getType()->getFormat().substr(0, 8) == "archive_")
		theArchiveManager->openArchive(entry);

	// Texture list
	else if (entry->getType()->getFormat() == "texturex" ||
			entry->getType() == EntryType::getType("zdtextures"))
		theMainWindow->openTextureEditor(archive);

	// Map
	else if (entry->getType() == EntryType::mapMarkerType()) {
		if (theMapEditor->openMap(archive->getMapInfo(entry)))
			theMapEditor->Show();
		else {
			theMapEditor->Hide();
			wxMessageBox(S_FMT("Unable to open map %s: %s", CHR(entry->getName()), CHR(Global::error)), "Invalid map error", wxICON_ERROR);
		}
	}

	// Other entry
	else if (entry->getType() != EntryType::folderType())
		theMainWindow->openEntry(entry);

	e.Skip();
}

/* ArchivePanel::onDEPEditAsText
 * Called when the 'Edit as Text' button is clicked on the default
 * entry panel - opens the entry in the text editor panel
 *******************************************************************/
void ArchivePanel::onDEPEditAsText(wxCommandEvent& e) {
	// Get entry to edit
	ArchiveEntry* entry = default_area->getEntry();

	// Open in text editor
	openEntryAsText(entry);
}

/* ArchivePanel::onDEPEViewAsHex
 * Called when the 'View as Hex' button is clicked on the default
 * entry panel - opens the entry in the hex editor panel
 *******************************************************************/
void ArchivePanel::onDEPViewAsHex(wxCommandEvent& e) {
	// Get entry to view
	ArchiveEntry* entry = default_area->getEntry();

	// Open in hex editor
	openEntryAsHex(entry);
}

/* ArchivePanel::onTextFilterChanged
 * Called when the filter text is changed
 *******************************************************************/
void ArchivePanel::onTextFilterChanged(wxCommandEvent& e) {
	// Get category string to filter by
	string category = "";
	if (choice_category->GetSelection() > 0)
		category = choice_category->GetStringSelection();

	// Filter the entry list
	entry_list->filterList(text_filter->GetValue(), category);

	e.Skip();
}

/* ArchivePanel::onChoiceCategoryChanged
 * Called when the 'Category' choice is changed
 *******************************************************************/
void ArchivePanel::onChoiceCategoryChanged(wxCommandEvent& e) {
	// Get category string to filter by
	string category = "";
	if (choice_category->GetSelection() > 0)
		category = choice_category->GetStringSelection();

	// Filter the entry list
	entry_list->filterList(text_filter->GetValue(), category);

	e.Skip();
}

/* ArchivePanel::onDirChanged
 * Called when the entry list directory is changed
 *******************************************************************/
void ArchivePanel::onDirChanged(wxCommandEvent& e) {
	// Get directory
	ArchiveTreeNode* dir = entry_list->getCurrentDir();

	if (!dir->getParent()) {
		// Root dir
		label_path->SetLabel("Path:");
		btn_updir->Enable(false);
	}
	else {
		// Setup path string
		string path = dir->getPath();
		path.Remove(0, 1);
		path.Prepend("Path: ");

		label_path->SetLabel(path);
		btn_updir->Enable(true);
	}
}

/* ArchivePanel::onBtnUpDir
 * Called when the 'Up Directory' button is clicked
 *******************************************************************/
void ArchivePanel::onBtnUpDir(wxCommandEvent& e) {
	// Go up a directory in the entry list
	entry_list->goUpDir();
}


/*******************************************************************
 * EXTRA CONSOLE COMMANDS
 *******************************************************************/

// I'd love to put them in their own file, but attempting to do so
// results in a circular include nightmare and nothing works anymore.
#include "ConsoleHelpers.h"
#include "Console.h"
#include "MainApp.h"
Archive * CH::getCurrentArchive() {
	if (theMainWindow)
	{
		if (theMainWindow->getArchiveManagerPanel())
		{
			return theMainWindow->getArchiveManagerPanel()->currentArchive();
		}
	}
	return NULL;
}

ArchivePanel * CH::getCurrentArchivePanel() {
	ArchiveManagerPanel * archie = theMainWindow->getArchiveManagerPanel();
	if (archie)
	{
		if (archie->isArchivePanel(archie->currentTabIndex()))
			return (ArchivePanel *)(archie->currentPanel());
	}
	return NULL;
}

CONSOLE_COMMAND(palconv, 0) {
	ArchivePanel * meep = CH::getCurrentArchivePanel();
	if (meep)
	{
		meep->palConvert();
		meep->reloadCurrentPanel();
	}
}
