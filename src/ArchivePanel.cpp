
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
#include "MultiEntryPanel.h"
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
#include <wx/aui/auibook.h>
#include <wx/filename.h>
#include <wx/gbsizer.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Int, autosave_entry_changes, 2, CVAR_SAVE)	// 0=no, 1=yes, 2=ask

// Temporary
const int MENU_GFX_CONVERT = 10001;
const int MENU_GFX_MODIFY_OFFSETS = 10002;
const int MENU_BAS_CONVERT = 10003;
const int MENU_GFX_ADD_PATCH_TABLE = 10004;
const int MENU_GFX_ADD_TEXTUREX = 10005;
const int MENU_VIEW_TEXT = 10006;
const int MENU_VIEW_HEX = 10007;
const int MENU_CONV_WAV_DSND = 10008;
const int MENU_CONV_DSND_WAV = 10009;
const int MENU_CONV_MUS_MIDI = 10010;
const int MENU_TEMP_END = 10100;


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
	m_hbox->Add(cur_area, 1, wxEXPAND | wxALL, 4);
	cur_area->Show(true);

	// Bind events
	entry_list->Bind(EVT_VLV_SELECTION_CHANGED, &ArchivePanel::onEntryListSelectionChange, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_FOCUSED, &ArchivePanel::onEntryListFocusChange, this);
	entry_list->Bind(wxEVT_KEY_DOWN, &ArchivePanel::onEntryListKeyDown, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, &ArchivePanel::onEntryListRightClick, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &ArchivePanel::onEntryListActivated, this);
	text_filter->Bind(wxEVT_COMMAND_TEXT_UPDATED, &ArchivePanel::onTextFilterChanged, this);
	choice_category->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ArchivePanel::onChoiceCategoryChanged, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ArchivePanel::onEntryMenuClick, this, MENU_GFX_CONVERT, MENU_TEMP_END);
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
		int result = wxMessageBox(s_fmt("Save changes to entry \"%s\"?", cur_area->getEntry()->getName().c_str()),
									"Unsaved Changes", wxYES_NO|wxICON_QUESTION);

		// Stop if user clicked no
		if (result == wxNO)
			return false;
	}

	// Save entry changes
	return cur_area->saveEntry();
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
		wxMessageBox(s_fmt("Error:\n%s", Global::error.c_str()), "Error", wxICON_ERROR);
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

	// Setup file filters (temporary, should go through all archive types somehow)
	string formats = archive->getFileExtensionString();
	string filename = wxFileSelector("Save Archive " + archive->getFilename(false) + " As", "", "", wxEmptyString, formats, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	// Check a filename was selected
	if (!filename.empty()) {
		// Save the archive
		if (!archive->save(filename)) {
			// If there was an error pop up a message box
			wxMessageBox(s_fmt("Error:\n%s", Global::error.c_str()), "Error", wxICON_ERROR);
			return false;
		}
	}

	// Refresh entry list
	entry_list->updateList();

	// Add as recent file
	theArchiveManager->addRecentFile(filename);

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

	// Remove any path from the name, if any (for now)
	wxFileName fn(name);
	name = fn.GetFullName();

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

	// Show path controls (if they aren't already)
	if (!GetSizer()->IsShown(sizer_path_controls)) {
		sizer_path_controls->Show(true);
		Layout();
	}

	// Return whether the directory was created ok
	return !!dir;
}

/* ArchivePanel::importFiles
 * Opens a file selection dialog and imports any selected files to
 * the current directory, using the filenames as entry names
 *******************************************************************/
bool ArchivePanel::importFiles() {
	// Create open file dialog
	wxFileDialog dialog_open(this, "Choose files to import", wxEmptyString, wxEmptyString,
			"Any File (*.*)|*.*", wxFD_OPEN|wxFD_MULTIPLE|wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open.ShowModal() == wxID_OK) {
		// Get file selection
		wxArrayString files;
		dialog_open.GetPaths(files);

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
		for (size_t a = 0; a < files.size(); a++) {
			// If only 1 file was selected, prompt for a name
			string name = wxFileName(files[a]).GetFullName();

			// Update splash window
			theSplashWindow->setProgress(float(a) / float(files.size()));
			theSplashWindow->setProgressMessage(name);

			// Add the entry to the archive
			ArchiveEntry* new_entry = archive->addNewEntry(name, index, entry_list->getCurrentDir());

			// If the entry was created ok, load the file into it
			if (new_entry) {
				new_entry->importFile(files[a]);		// Import file to entry
				EntryType::detectEntryType(new_entry);	// Detect entry type
				ok = true;
			}

			index++;
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
bool ArchivePanel::renameEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Check any are selected
	if (selection.size() == 1) {
		// If only one entry is selected, just do basic rename

		// Prompt for a new name
		string new_name = wxGetTextFromUser("Enter new entry name: (* = unchanged)", "Rename", selection[0]->getName());

		// Rename entry (if needed)
		if (!new_name.IsEmpty() && selection[0]->getName() != new_name)
			archive->renameEntry(selection[0], new_name);
	}
	else if (selection.size() > 1) {
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
		string new_name = wxGetTextFromUser("Enter new directory name:", s_fmt("Rename Directory %s", old_name.c_str()), old_name);

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
bool ArchivePanel::deleteEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selected_entries = entry_list->getSelectedEntries();

	// Get a list of selected directories
	vector<ArchiveTreeNode*> selected_dirs = entry_list->getSelectedDirectories();

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
	for (size_t a = 0; a < selection.size(); a++) {
		// Get the entries to swap
		ArchiveEntry* entry = entry_list->getEntry(selection[a]);
		ArchiveEntry* above = entry_list->getEntry(selection[a]-1);

		// Swap them in the archive
		archive->swapEntries(entry, above);

		// Swap the selection
		entry_list->selectItem(selection[a], false);
		entry_list->selectItem(selection[a] - 1, true);
	}

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
	if (selection.back() == entry_list->GetItemCount()-1)
		return false;

	// Move each one down by swapping it with the entry below it
	for (int a = selection.size()-1; a >= 0; a--) {
		// Get the entries to swap
		ArchiveEntry* entry = entry_list->getEntry(selection[a]);
		ArchiveEntry* below = entry_list->getEntry(selection[a]+1);

		// Swap them in the archive
		archive->swapEntries(entry, below);

		// Swap the selection
		entry_list->selectItem(selection[a], false);
		entry_list->selectItem(selection[a] + 1, true);
	}

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
		// Create open file dialog
		wxFileDialog *dialog_open = new wxFileDialog(this, s_fmt("Import Entry \"%s\"", selection[a]->getName().c_str()),
											wxEmptyString, wxEmptyString, "Any File (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST, wxDefaultPosition);

		// Run the dialog & check that the user didn't cancel
		if (dialog_open->ShowModal() == wxID_OK) {
			// If a file was selected, import it
			selection[a]->importFile(dialog_open->GetPath());

			// Re-detect entry type
			EntryType::detectEntryType(selection[a]);

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

		// Create save file dialog
		wxFileDialog *dialog_save = new wxFileDialog(this, s_fmt("Export Entry \"%s\"", selection[0]->getName().c_str()),
											wxEmptyString, fn.GetFullName(), "Any File (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

		// Run the dialog & check that the user didn't cancel
		if (dialog_save->ShowModal() == wxID_OK) {
			// If a filename was selected, export it
			selection[0]->exportFile(dialog_save->GetPath());
		}

		return true;
	}
	else {
		// Open dialog to select folder to export to
		wxDirDialog dd(this, "Select a Directory to Export Entries to");

		if (dd.ShowModal() == wxID_OK) {
			// Go through the selection
			for (size_t a = 0; a < selection.size(); a++) {
				// Setup entry filename
				wxFileName fn(selection[a]->getName());
				fn.SetPath(dd.GetPath());

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

	// If something is selected, clear the clipboard
	if (entries.size() > 0 || dirs.size() > 0)
		theClipboard->clear();
	else
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
		return deleteEntry();
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
		archive->paste(clip->getTree(), index, entry_list->getCurrentDir());
		//entry_list->getCurrentDir()->merge(clip->getTree(), index);
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

	// Send entries to the gcd
	gcd.openEntries(entry_list->getSelectedEntries());

	// Run the gcd
	gcd.ShowModal();

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
		"ANIMDEFS" : "animdefs.txt"), index);

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

	// Load entry data into the text editor
	//DefaultEntryPanel * meh = (DefaultEntryPanel *)default_area;
	//meh->openTextEntry(animdef);

	return true;
}

/* ArchivePanel::palConvert
 * Unused (converts 6-bit palette to 8-bit)
 *******************************************************************/
bool ArchivePanel::palConvert() {
	// Get the entry index of the last selected list item
	ArchiveEntry* pal6bit = currentEntry();
	const uint8_t * mehmeh = pal6bit->getData(true);
	uint8_t * meh = new uint8_t[pal6bit->getSize()];
	memcpy(meh, mehmeh, pal6bit->getSize());
	for (size_t i = 0; i < pal6bit->getSize(); ++i)
	{
		meh[i] = ((meh[i] << 2) | (meh[i] >> 4));
	}
	pal6bit->importMem(meh, pal6bit->getSize());
	delete[] meh;
	return true;
}

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
				wxLogMessage("Error: Unable to convert entry %s: %s", chr(selection[a]->getName()), chr(Global::error));
				continue;
			}
			selection[a]->importMemChunk(dsnd);							// Load doom sound data
			EntryType::detectEntryType(selection[a]);					// Update entry type
			selection[a]->setExtensionByType();							// Update extension if necessary
		}
	}

	return true;
}

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
		// Or Doom 64 SFX format
		else if (selection[a]->getType()->getFormat() == "snd_doom64")
			worked = Conversions::d64SfxToWav(selection[a]->getMCData(), wav);
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
			wxLogMessage("Error: Unable to convert entry %s: %s", chr(selection[a]->getName()), chr(Global::error));
			continue;
		}
	}

	return true;
}

bool ArchivePanel::musMidiConvert() {
	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		// Convert MUS -> MIDI if the entry is Doom Sound format
		if (selection[a]->getType()->getFormat() == "mus") {
			MemChunk midi;
			Conversions::musToMidi(selection[a]->getMCData(), midi);	// Convert
			selection[a]->importMemChunk(midi);							// Load wav data
			EntryType::detectEntryType(selection[a]);					// Update entry type
			selection[a]->setExtensionByType();							// Update extension if necessary
		}
	}

	return true;
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
			wxLogMessage("Error: Trying to open nonexistant directory %s", chr(name));
			return false;
		}
		entry_list->setDir(dir);
	} else {
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
			wxMessageBox(s_fmt("Error loading entry:\n%s", Global::error.c_str()), "Error", wxOK|wxICON_ERROR);
		}
	}
	return true;
}

bool ArchivePanel::openEntryAsText(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Show the text entry panel
	if (!showEntryPanel(text_area))
		return false;

	// Load the current entry into the panel
	if (!cur_area->openEntry(entry)) {
		wxMessageBox(s_fmt("Error loading entry:\n%s", Global::error.c_str()), "Error", wxOK|wxICON_ERROR);
	}

	return true;
}

bool ArchivePanel::openEntryAsHex(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Show the text entry panel
	if (!showEntryPanel(hex_area))
		return false;

	// Load the current entry into the panel
	if (!cur_area->openEntry(entry)) {
		wxMessageBox(s_fmt("Error loading entry:\n%s", Global::error.c_str()), "Error", wxOK|wxICON_ERROR);
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
		sizer->Replace(cur_area, new_area);	// Swap the panels
		cur_area = new_area;				// Set the new panel to current
		cur_area->Show(true);				// Show current

		// Update panel layout
		Layout();
	}

	return true;
}

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
 * Handles a menu action from the main window
 *******************************************************************/
void ArchivePanel::handleAction(int menu_id) {
	// *************************************************************
	// FILE MENU
	// *************************************************************

	// File->Save
	if (menu_id == MainWindow::MENU_FILE_SAVE)
		save();

	// File->Save As
	else if (menu_id == MainWindow::MENU_FILE_SAVEAS)
		saveAs();

	// File->Close
	else if (menu_id == MainWindow::MENU_FILE_CLOSE)
		theArchiveManager->closeArchive(archive);


	// *************************************************************
	// ARCHIVE MENU
	// *************************************************************

	// Archive->New->Entry
	else if (menu_id == MainWindow::MENU_ARCHIVE_NEWENTRY)
		newEntry();

	// Archive->New->Directory
	else if (menu_id == MainWindow::MENU_ARCHIVE_NEWDIRECTORY)
		newDirectory();

	// Archive->Import Files
	else if (menu_id == MainWindow::MENU_ARCHIVE_IMPORTFILES)
		importFiles();

	// Archive->Texture Editor
	else if (menu_id == MainWindow::MENU_ARCHIVE_TEXEDITOR)
		theMainWindow->openTextureEditor(archive);

	// Archive->Convert To...
	else if (menu_id == MainWindow::MENU_ARCHIVE_CONVERTTO)
		convertArchiveTo();

	// Archive->Clean Up
	else if (menu_id == MainWindow::MENU_ARCHIVE_CLEAN_PATCHES)
		ArchiveOperations::removeUnusedPatches(archive);


	// *************************************************************
	// ENTRY MENU
	// *************************************************************

	// Entry->Rename
	else if (menu_id == MainWindow::MENU_ENTRY_RENAME)
		renameEntry();

	// Entry->Delete
	else if (menu_id == MainWindow::MENU_ENTRY_DELETE)
		deleteEntry();

	else if (menu_id == MainWindow::MENU_ENTRY_REVERT)
		revertEntry();

	// Entry->Cut
	else if (menu_id == MainWindow::MENU_ENTRY_CUT)
		cutEntry();

	// Entry->Copy
	else if (menu_id == MainWindow::MENU_ENTRY_COPY)
		copyEntry();

	// Entry->Paste
	else if (menu_id == MainWindow::MENU_ENTRY_PASTE)
		pasteEntry();

	// Entry->Move Up
	else if (menu_id == MainWindow::MENU_ENTRY_MOVEUP)
		moveUp();

	// Entry->Move Down
	else if (menu_id == MainWindow::MENU_ENTRY_MOVEDOWN)
		moveDown();

	// Entry->Bookmark
	else if (menu_id == MainWindow::MENU_ENTRY_BOOKMARK)
		bookmark();

	// Entry->Convert To...
	else if (menu_id == MainWindow::MENU_ENTRY_CONVERTTO)
		convertEntryTo();

	// Entry->Import
	else if (menu_id == MainWindow::MENU_ENTRY_IMPORT)
		importEntry();

	// Entry->Export
	else if (menu_id == MainWindow::MENU_ENTRY_EXPORT)
		exportEntry();

	// Entry->Export As...
	else if (menu_id == MainWindow::MENU_ENTRY_EXPORTAS)
		exportEntryAs();

	// Entry->Bookmark
	else if (menu_id == MainWindow::MENU_ENTRY_BOOKMARK)
		bookmark();



	// Temporary ones
	else if (menu_id == MENU_GFX_CONVERT)
		gfxConvert();
	else if (menu_id == MENU_GFX_MODIFY_OFFSETS)
		gfxModifyOffsets();
	else if (menu_id == MENU_BAS_CONVERT)
		basConvert();
	else if (menu_id == MENU_GFX_ADD_PATCH_TABLE)
		EntryOperations::addToPatchTable(entry_list->getSelectedEntries());
	else if (menu_id == MENU_GFX_ADD_TEXTUREX)
		EntryOperations::createTexture(entry_list->getSelectedEntries());
	else if (menu_id == MENU_VIEW_TEXT)
		openEntryAsText(entry_list->getFocusedEntry());
	else if (menu_id == MENU_VIEW_HEX)
		openEntryAsHex(entry_list->getFocusedEntry());
	else if (menu_id == MENU_CONV_DSND_WAV)
		dSndWavConvert();
	else if (menu_id == MENU_CONV_WAV_DSND)
		wavDSndConvert();
	else if (menu_id == MENU_CONV_MUS_MIDI)
		musMidiConvert();
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
}


/*******************************************************************
 * ARCHIVEPANEL EVENTS
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
	// Generate context menu
	wxMenu* context = new wxMenu();
	context->Append(MainWindow::MENU_ENTRY_RENAME, "Rename");
	context->Append(MainWindow::MENU_ENTRY_DELETE, "Delete");
	context->Append(MainWindow::MENU_ENTRY_REVERT, "Revert");
	context->AppendSeparator();
	context->Append(MainWindow::MENU_ENTRY_CUT, "Cut");
	context->Append(MainWindow::MENU_ENTRY_COPY, "Copy");
	context->Append(MainWindow::MENU_ENTRY_PASTE, "Paste");
	context->AppendSeparator();
	context->Append(MainWindow::MENU_ENTRY_IMPORT, "Import");
	context->Append(MainWindow::MENU_ENTRY_EXPORT, "Export");
	context->AppendSeparator();
	context->Append(MainWindow::MENU_ENTRY_MOVEUP, "Move Up");
	context->Append(MainWindow::MENU_ENTRY_MOVEDOWN, "Move Down");
	context->AppendSeparator();
	context->Append(MainWindow::MENU_ENTRY_BOOKMARK, "Bookmark");

	// 'View As' menu
	wxMenu* viewas = new wxMenu();
	context->AppendSubMenu(viewas, "View As");
	viewas->Append(MENU_VIEW_TEXT, "Text", "Opens the selected entry in the text editor, regardless of type");
	viewas->Append(MENU_VIEW_HEX, "Hex", "Opens the selected entry in the hex editor, regardless of type");

	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Check what types exist in the selection
	bool gfx_selected = false;
	bool bas_selected = false;
	bool wav_selected = false;
	bool dsnd_selected = false;
	bool mus_selected = false;
//	bool rle_selected = false;
	for (size_t a = 0; a < selection.size(); a++) {
		// Check for gfx entry
		if (!gfx_selected) {
			if (selection[a]->getType()->extraProps().propertyExists("image"))
				gfx_selected = true;
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
#if 0
		if (!rle_selected) {
			if (selection[a]->getType()->getFormat() == "misc_rle0")
				rle_selected = true;
		}
#endif
	}

	// Add gfx-related menu items if gfx are selected
	if (gfx_selected) {
		wxMenu* gfx = new wxMenu();
		context->AppendSubMenu(gfx, "Gfx");
		gfx->Append(MENU_GFX_CONVERT, "Convert to...");
		gfx->Append(MENU_GFX_MODIFY_OFFSETS, "Modify Gfx Offsets");
		gfx->Append(MENU_GFX_ADD_PATCH_TABLE, "Add to Patch Table");
		gfx->Append(MENU_GFX_ADD_TEXTUREX, "Add to TEXTUREx");
	}

	// Add Boom Animations/Switches related menu items if they are selected
	if (bas_selected) {
		wxMenu* boom = new wxMenu();
		context->AppendSubMenu(boom, "Boom");
		boom->Append(MENU_BAS_CONVERT, "Convert to ANIMDEFS");
	}
	// This is not generally useful
	//context->Append(MENU_ENTRY_PAL_CONVERT, "Pal 6-bit to 8-bit");

	// Add Audio related menu items if needed
	if (wav_selected || dsnd_selected || mus_selected) {
		wxMenu* audio = new wxMenu();
		context->AppendSubMenu(audio, "Audio");
		if (wav_selected)
			audio->Append(MENU_CONV_WAV_DSND, "Convert WAV to Doom Sound");
		if (dsnd_selected)
			audio->Append(MENU_CONV_DSND_WAV, "Convert sounds to WAV");
		if (mus_selected)
			audio->Append(MENU_CONV_MUS_MIDI, "Convert MUS to MIDI");
	}

	// Popup the context menu
	PopupMenu(context);
}

/* ArchivePanel::onEntryMenuClick
 * Called when an entry menu item is selected
 *******************************************************************/
void ArchivePanel::onEntryMenuClick(wxCommandEvent& e) {
	handleAction(e.GetId());
}

/* ArchivePanel::onEntryListKeyDown
 * Called when a key is pressed on the entry list
 *******************************************************************/
void ArchivePanel::onEntryListKeyDown(wxKeyEvent& e) {
	// Rename entry (Ctrl+R or F2)
	if ((e.GetKeyCode() == 'R' && e.ControlDown()) || e.GetKeyCode() == WXK_F2)
		renameEntry();

	// Delete entry (Delete)
	else if (e.GetKeyCode() == WXK_DELETE)
		deleteEntry();

	// Copy entry (Ctrl+C)
	else if (e.GetKeyCode() == 'C' && e.ControlDown())
		copyEntry();

	// Cut entry (Ctrl+X)
	else if (e.GetKeyCode() == 'X' && e.ControlDown())
		cutEntry();

	// Paste entry (Ctrl+V)
	else if (e.GetKeyCode() == 'V' && e.ControlDown())
		pasteEntry();

	// Import to entry (Ctrl+I)
	else if (e.GetKeyCode() == 'I' && e.ControlDown())
		importEntry();

	// Export entry (Ctrl+E)
	else if (e.GetKeyCode() == 'E' && e.ControlDown() && !e.ShiftDown())
		exportEntry();

	// Move entry up (Ctrl+U or Ctrl+Up Arrow)
	else if (e.ControlDown() && (e.GetKeyCode() == 'U' || e.GetKeyCode() == WXK_UP))
		moveUp();

	// Move entry down (Ctrl+D or Ctrl+Down Arrow)
	else if (e.ControlDown() && (e.GetKeyCode() == 'D' || e.GetKeyCode() == WXK_DOWN))
		moveDown();

	// Select all entries (Ctrl+A)
	else if (e.GetKeyCode() == 'A' && e.ControlDown())
		entry_list->selectAll();

	// New entry (Ctrl+N)
	else if (e.GetKeyCode() == 'N' && e.ControlDown() && !e.ShiftDown())
		newEntry();

	// Up directory (backspace)
	else if (e.GetKeyCode() == WXK_BACK)
		entry_list->goUpDir();


	// Not handled here, send off to be handled by a parent window
	else
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

	if (entry->getType()->getFormat().substr(0, 8) == "archive_")
		theArchiveManager->openArchive(entry);

	if (entry->getType()->getFormat() == "texturex")
		theMainWindow->openTextureEditor(archive);

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

void ArchivePanel::onTextFilterChanged(wxCommandEvent& e) {
	// Get category string to filter by
	string category = "";
	if (choice_category->GetSelection() > 0)
		category = choice_category->GetStringSelection();

	// Filter the entry list
	entry_list->filterList(text_filter->GetValue(), category);

	e.Skip();
}

void ArchivePanel::onChoiceCategoryChanged(wxCommandEvent& e) {
	// Get category string to filter by
	string category = "";
	if (choice_category->GetSelection() > 0)
		category = choice_category->GetStringSelection();

	// Filter the entry list
	entry_list->filterList(text_filter->GetValue(), category);

	e.Skip();
}

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
	if (theApp->getMainWindow())
	{
		if (theApp->getMainWindow()->getArchiveManagerPanel())
		{
			return theApp->getMainWindow()->getArchiveManagerPanel()->currentArchive();
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
