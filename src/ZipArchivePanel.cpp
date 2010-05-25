
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ZipArchivePanel.cpp
 * Description: ZipArchivePanel class. An extension of ArchivePanel
 *              to deal with zip archives that have a directory tree
 *              rather than a linear list of entries
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
#include "ZipArchivePanel.h"
#include "ZipArchiveEntryList.h"
#include "ZipArchive.h"
#include "Clipboard.h"
#include <wx/aui/auibook.h>
#include <wx/filename.h>


/*******************************************************************
 * CLASSES
 *******************************************************************/
// A simple dialog that prompts the user for an entry type (entry or folder) and name
// Used when creating new entries for zip format archives
class NewEntryDialog : public wxDialog {
private:
	wxTextCtrl* text_entryname;
	wxRadioBox*	rb_entrytype;

public:
	NewEntryDialog() : wxDialog(NULL, -1, _T("New Entry"), wxDefaultPosition) {
		// Setup main sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		wxSizerFlags sizer_flags;
		sizer_flags.DoubleBorder();
		sizer_flags.Expand();

		// Add entry type radio options
		string types[] = { _T("Entry"), _T("Folder") };
		rb_entrytype = new wxRadioBox(this, -1, _T("Entry Type:"), wxDefaultPosition, wxDefaultSize, 2, types, 2);
		sizer->Add(rb_entrytype, sizer_flags);

		// Add entry name textbox
		sizer->Add(CreateTextSizer(_T("Entry Name:")), wxSizerFlags().Expand().DoubleBorder(wxLEFT|wxRIGHT));
		text_entryname = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(300, -1));
		sizer->Add(text_entryname, sizer_flags);

		// Add OK/Cancel buttons
		sizer->Add(CreateSeparatedButtonSizer(wxOK|wxCANCEL), sizer_flags);

		// Layout dialog
		SetAutoLayout(true);
		SetSizer(sizer);
		sizer->SetSizeHints(this);
		sizer->Fit(this);

		// Set focus to the text entry
		text_entryname->SetFocus();
	}

	~NewEntryDialog() {}

	string getName() {
		if (text_entryname)
			return text_entryname->GetValue();
		else
			return _T("");
	}

	int getType() {
		if (rb_entrytype)
			return rb_entrytype->GetSelection();
		else
			return 0;
	}
};


/*******************************************************************
 * ZIPARCHIVEPANEL CLASS FUNCTIONS
 *******************************************************************/

/* ZipArchivePanel::ZipArchivePanel
 * ZipArchivePanel class constructor
 *******************************************************************/
ZipArchivePanel::ZipArchivePanel(wxWindow* parent, Archive* archive)
: ArchivePanel(parent, archive) {
}

/* ArchivePanel::~ArchivePanel
 * ArchivePanel class destructor
 *******************************************************************/
ZipArchivePanel::~ZipArchivePanel() {
}

/* ZipArchivePanel::init
 * Initialises the panel layout (has to be called separately from the
 * constructor, otherwise it won't create a ZipEntryListPanel for
 * whatever reason)
 *******************************************************************/
void ZipArchivePanel::init() {
	// Create & set sizer
	wxBoxSizer *m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Entry list panel

	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, "Entries");
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_hbox->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// Create entry list panel
	entry_list = new ZipArchiveEntryList(this);
	entry_list->setArchive(archive);
	framesizer->Add(entry_list, 1, wxEXPAND | wxALL, 4);

	// Add default entry panel
	cur_area = entry_area;
	m_hbox->Add(cur_area, 1, wxEXPAND | wxALL, 4);
	cur_area->Show(true);

	// Setup events
	entry_list->Bind(wxEVT_KEY_DOWN, &ArchivePanel::onEntryListKeyDown, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_FOCUSED, &ArchivePanel::onEntryListFocusChange, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &ArchivePanel::onEntryListFocusChange, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, &ArchivePanel::onEntryListRightClick, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &ArchivePanel::onEntryListActivated, this);

	// Update size+layout
	entry_list->updateWidth();
	Layout();
}

/* ZipArchivePanel::save
 * Override of ArchivePanel::save to also update any directories in
 * the entry list
 *******************************************************************/
void ZipArchivePanel::save() {
	// Do default save
	ArchivePanel::save();

	// Update all directories
	//for (int a = 0; a < ((ZipEntryListPanel*)entry_list)->entriesBegin(); a++)
	//	((ZipEntryListPanel*)entry_list)->updateDirectoryEntry(a);
}

/* ZipArchivePanel::saveAs
 * Override of ArchivePanel::saveAs to also update any directories in
 * the entry list
 *******************************************************************/
void ZipArchivePanel::saveAs() {
	// Do default save as
	ArchivePanel::saveAs();

	// Update all directories
	//for (int a = 0; a < ((ZipEntryListPanel*)entry_list)->entriesBegin(); a++)
	//	((ZipEntryListPanel*)entry_list)->updateDirectoryEntry(a);
}

/* ZipArchivePanel::newEntry
 * The same as ArchivePanel::newEntry, but more advanced to deal
 * with directories. If the given name is a relative or absolute
 * directory, the entry is created with the appropriate directory
 * value.
 *******************************************************************/
bool ZipArchivePanel::newEntry() {
	// Let's call him ned
	NewEntryDialog ned;

	if (ned.ShowModal() == wxID_OK) {
		// Get the entry name that was entered
		string name = ned.getName();

		// Get the current directory
		zipdir_t* dir = ((ZipArchiveEntryList*)entry_list)->getCurrentDir();

		// If an absolute path wasn't given, add the current directory before the name
		if (!name.StartsWith(_T("/")))
			name = dir->getFullPath() + name;

		// Check if the user selected to create an entry
		if (ned.getType() == 0) {
			// Get the entry index of the last selected list item
			int index = archive->entryIndex(entry_list->getLastSelectedEntry());

			// If something was selected, add 1 to the index so we add the new entry after the last selected
			if (index >= 0)
				index++;
			else
				index = -1; // If not add to the end of the list

			// Add the entry to the archive
			return !!(archive->addNewEntry(name, index));
		}

		// Otherwise if the user selected to create a folder
		else if (ned.getType() == 1) {
			// Add an ending / if one wasn't entered
			if (!name.EndsWith(_T("/")))
				name += _T("/");

			// Add the directory to the archive
			((ZipArchive*)archive)->addDirectory(name);

			// Added ok so return true
			return true;
		}
	}

	// If user didn't click ok, return false
	return false;
}

/* ZipArchivePanel::newEntryFromFile
 * Same as newEntry, but imports a file into the created entry
 *******************************************************************/
bool ZipArchivePanel::newEntryFromFile() {
	// Create open file dialog
	wxFileDialog dialog_open(this, _T("Choose file to open"), wxEmptyString, wxEmptyString,
			_T("Any File (*.*)|*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open.ShowModal() == wxID_OK) {
		// Get list of selected files
		wxArrayString files;
		dialog_open.GetPaths(files);

		// Get the entry index of the last selected list item
		int index = archive->entryIndex(entry_list->getLastSelectedEntry());

		// If something was selected, add 1 to the index so we add the new entry after the last selected
		if (index >= 0)
			index++;
		else
			index = -1; // If not add to the end of the list

		// Go through the list of files
		bool ok = false;
		for (size_t a = 0; a < files.size(); a++) {
			string name = wxFileName(files[a]).GetFullName();

			// If only 1 file was selected, prompt for an entry name
			if (files.size() == 1)
				name = wxGetTextFromUser(_T("Enter new entry name:"), _T("New Entry"), name);

			// Get the current directory
			zipdir_t* dir = ((ZipArchiveEntryList*)entry_list)->getCurrentDir();

			// If an absolute path wasn't given, add the current directory before the name
			if (!name.StartsWith(_T("/")))
				name = dir->getFullPath() + name;

			// Add the entry to the archive
			ArchiveEntry* new_entry = archive->addNewEntry(name, index);

			// If the entry was created ok, load the file into it
			if (new_entry) {
				new_entry->importFile(files[a]);
				ok = true;
			}

			index++;
		}

		return ok;
	}
	else // If user canceled return false
		return false;
}

/* ZipArchivePanel::renameEntry
 * Renames the selected entries/directories
 *******************************************************************/
bool ZipArchivePanel::renameEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through the list
	for (size_t a = 0; a < selection.size(); a++) {
		// Get the current entry's name
		ArchiveEntry* entry = selection[a];
		string old_name = entry->getName();

		// If the entry is a folder then skip it
		if (entry->getType() == EntryType::folderType())
			continue;

		// Prompt for a new name
		string new_name = wxGetTextFromUser(_T("Enter new entry name:"), s_fmt(_T("Rename Entry %s"), old_name.c_str()), old_name);

		// Rename the entry if a different name was specified
		if (new_name.Cmp(wxEmptyString) && new_name.Cmp(old_name))
			archive->renameEntry(entry, new_name);
	}

	// Get a list of selected directories
	vector<zipdir_t*> selected_dirs = ((ZipArchiveEntryList*)entry_list)->getSelectedDirectories();

	// Go through the list
	for (size_t a = 0; a < selected_dirs.size(); a++) {
		// Get the current directory's name
		string old_name = selected_dirs[a]->getName();

		// Prompt for a new name
		string new_name = wxGetTextFromUser(_T("Enter new directory name:"), s_fmt(_T("Rename Directory %s"), old_name.c_str()), old_name);

		// Do nothing if no name was entered
		if (!new_name.Cmp(_T("")))
			continue;

		// Add trailing '/' to new name if one wasn't entered
		if (!new_name.EndsWith(_T("/")))
			new_name += _T("/");

		// If the entered name is a path with more than one directory, just use the first (for now)
		wxFileName fn(new_name);
		if (fn.GetDirCount() > 1)
			new_name = fn.GetDirs()[0] + _T("/");

		// Rename the directory if the new entered name is different from the original
		if (new_name.Cmp(old_name))
			((ZipArchive*)archive)->renameDirectory(selected_dirs[a], new_name);
	}

	return true;
}

/* ZipArchivePanel::deleteEntry
 * Deletes the selected entries/directories
 *******************************************************************/
bool ZipArchivePanel::deleteEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selected_entries = entry_list->getSelectedEntries();

	// Get a list of selected directories
	vector<zipdir_t*> selected_dirs = ((ZipArchiveEntryList*)entry_list)->getSelectedDirectories();

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
		((ZipArchive*)archive)->deleteDirectory(selected_dirs[a]);
	}

	return true;
}

/* ZipArchivePanel::copyEntry
 * Copies the selected entries and directories
 *******************************************************************/
bool ZipArchivePanel::copyEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// If something is selected, clear the clipboard
	if (selection.size() > 0)
		theClipboard->clear();
	else
		return false;

	// Go through the list
	for (size_t a = 0; a < selection.size(); a++) {
		// Copy the current selected entry if it isn't a directory
		if (selection[a]->getType() != EntryType::folderType())
			theClipboard->addItem(new EntryClipboardItem(selection[a]));
	}

	// Get a list of selected directories
	vector<zipdir_t*> selected_dirs = ((ZipArchiveEntryList*)entry_list)->getSelectedDirectories();

	// Go through the list
	for (size_t a = 0; a < selected_dirs.size(); a++) {
		// Get all entries within the selected dir and its subdirs
		vector<ArchiveEntry*> entries;
		selected_dirs[a]->addToList(entries);

		// Create clipboard item from non-directory entries
		ZipDirClipboardItem* item = new ZipDirClipboardItem();
		for (size_t a = 0; a < entries.size(); a++) {
			if (entries[a]->getType() != EntryType::folderType())
				item->addEntry(entries[a]);
		}

		// Add it to the clipboard
		theClipboard->addItem(item);
	}

	return true;
}

/* ZipArchivePanel::pasteEntry
 * Pastes any entries on the clipboard either after the last selected
 * entry or at the end of the entry list if nothing is selected. All
 * entries/directories are pasted to the current directory
 *******************************************************************/
bool ZipArchivePanel::pasteEntry() {
	// Get the entry index of the last selected list item
	int index = archive->entryIndex(entry_list->getLastSelectedEntry());

	// If something was selected, add 1 to the index so we add the new entry after the last selected
	if (index >= 0)
		index++;
	else
		index = -1; // If not add to the end of the list

	// Get the current directory
	zipdir_t* cur_dir = ((ZipArchiveEntryList*)entry_list)->getCurrentDir();

	// Go through all items on the clipboard
	for (uint32_t a = 0; a < theClipboard->nItems(); a++) {
		// If the item is an entry
		if (theClipboard->getItem(a)->getType() == CLIPBOARD_ENTRY) {
			EntryClipboardItem* item = (EntryClipboardItem*)theClipboard->getItem(a);
			ArchiveEntry* copied_entry = new ArchiveEntry(*(item->getEntry()));
			//copied_entry->setExProp(_T("Directory"), cur_dir->getFullPath());
			copied_entry->extraProp(_T("Directory")) = cur_dir->getFullPath();
			archive->addEntry(copied_entry, index);
			index++;
		}

		// If the item is a directory
		else if (theClipboard->getItem(a)->getType() == CLIPBOARD_ZIPDIR) {
			ZipDirClipboardItem* item = (ZipDirClipboardItem*)theClipboard->getItem(a);

			// Go through all entries in the directory
			for (uint32_t e = 0; e < item->nEntries(); e++) {
				ArchiveEntry* copied_entry = new ArchiveEntry(*(item->getEntry(e)));
				//string dir = copied_entry->getExProp(_T("Directory"));
				string dir = copied_entry->extraProp(_T("Directory"));
				//copied_entry->setExProp(_T("Directory"), cur_dir->getFullPath() + dir);
				copied_entry->extraProp(_T("Directory")) = cur_dir->getFullPath() + dir;
				archive->addEntry(copied_entry, index);
				index++;
			}
		}
	}

	return true;
}

bool ZipArchivePanel::moveUp() {
	// Get selection
	vector<int> selection = entry_list->getSelection();

	// If nothing is selected, do nothing
	if (selection.size() == 0)
		return false;

	// If the first selected item is at the top of the list
	// or before entries start then don't move anything up
	if (selection[0] <= ((ZipArchiveEntryList*)entry_list)->entriesBegin())
		return false;

	// Move each one up by swapping it with the entry above it
	for (size_t a = 0; a < selection.size(); a++) {
		// Get the entries to swap
		ArchiveEntry* entry = entry_list->getEntry(selection[a]);
		ArchiveEntry* above = entry_list->getEntry(selection[a]-1);

		// Swap them in the archive
		archive->swapEntries(entry, above);
	}

	// Return success
	return true;
}

bool ZipArchivePanel::moveDown() {
	// Get selection
	vector<int> selection = entry_list->getSelection();

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
	}

	// Return success
	return true;
}

/* ZipArchivePanel::onAnnouncement
 * Called when an announcement is recieved from the archive that
 * this ZipArchivePanel is managing
 *******************************************************************/
void ZipArchivePanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Do default announcement handling
	ArchivePanel::onAnnouncement(announcer, event_name, event_data);

	/*
	// If a directory was added to the archive
	if (announcer == archive && !event_name.Cmp(_T("directory_added"))) {
		wxUIntPtr ptr;
		if (event_data.read(&ptr, sizeof(wxUIntPtr)))
			((ZipEntryListPanel*)entry_list)->addDirectory(ptr);
	}

	// If a directory was removed from the archive
	if (announcer == archive && !event_name.Cmp(_T("directory_removed"))) {
		wxUIntPtr ptr;
		if (event_data.read(&ptr, sizeof(wxUIntPtr)))
			((ZipEntryListPanel*)entry_list)->removeDirectory(ptr);
	}

	// If a directory was renamed in the archive
	if (announcer == archive && !event_name.Cmp(_T("directory_modified"))) {
		wxUIntPtr ptr;
		if (event_data.read(&ptr, sizeof(wxUIntPtr)))
			((ZipEntryListPanel*)entry_list)->updateDirectory(ptr);
	}
	*/
}
