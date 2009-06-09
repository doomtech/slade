
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
#include "ZipEntryListPanel.h"
#include "ZipArchive.h"
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
	entry_list = new ZipEntryListPanel(this, ENTRY_LIST_PANEL, archive);
	m_hbox->Add(entry_list, 0, wxEXPAND | wxALL, 4);
	entry_list->populateEntryList();

	// Add default entry panel to the panel
	cur_area = entry_area;
	m_hbox->Add(cur_area, 1, wxEXPAND | wxALL, 4);
	cur_area->Show(true);

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
	for (int a = 0; a < ((ZipEntryListPanel*)entry_list)->entriesBegin(); a++)
		((ZipEntryListPanel*)entry_list)->updateDirectoryEntry(a);
}

/* ZipArchivePanel::saveAs
 * Override of ArchivePanel::saveAs to also update any directories in
 * the entry list
 *******************************************************************/
void ZipArchivePanel::saveAs() {
	// Do default save as
	ArchivePanel::saveAs();

	// Update all directories
	for (int a = 0; a < ((ZipEntryListPanel*)entry_list)->entriesBegin(); a++)
		((ZipEntryListPanel*)entry_list)->updateDirectoryEntry(a);
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
		zipdir_t* dir = ((ZipEntryListPanel*)entry_list)->getCurrentDir();

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
				index = entry_list->getListSize(); // If not add to the end of the list

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
			_T("Any File (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open.ShowModal() == wxID_OK) {
		wxFileName fn(dialog_open.GetPath());
		string filename = fn.GetFullName();

		// Prompt for new entry name
		string name = wxGetTextFromUser(_T("Enter new entry name:"), _T("New Entry"), filename);

		// Get the current directory
		zipdir_t* dir = ((ZipEntryListPanel*)entry_list)->getCurrentDir();

		// If an absolute path wasn't given, add the current directory before the name
		if (!name.StartsWith(_T("/")))
			name = dir->getFullPath() + name;

		// Get the entry index of the last selected list item
		int index = archive->entryIndex(entry_list->getLastSelectedEntry());

		// If something was selected, add 1 to the index so we add the new entry after the last selected
		if (index >= 0)
			index++;
		else
			index = entry_list->getListSize(); // If not add to the end of the list

		// Add the entry to the archive
		ArchiveEntry* new_entry = archive->addNewEntry(name, index);

		// If the entry was created ok load the file into it and return true
		// otherwise, return false
		if (new_entry) {
			new_entry->importFile(dialog_open.GetPath());
			return true;
		}
		else
			return false;
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
		if (entry->getType() == ETYPE_FOLDER)
			continue;

		// Prompt for a new name
		string new_name = wxGetTextFromUser(_T("Enter new entry name:"), s_fmt(_T("Rename Entry %s"), old_name.c_str()), old_name);

		// Rename the entry if a different name was specified
		if (new_name.Cmp(wxEmptyString) && new_name.Cmp(old_name))
			archive->renameEntry(entry, new_name);
	}

	// Get a list of selected directories
	vector<zipdir_t*> selected_dirs = ((ZipEntryListPanel*)entry_list)->getSelectedDirectories();

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
}

/* ZipArchivePanel::deleteEntry
 * Deletes the selected entries/directories
 *******************************************************************/
bool ZipArchivePanel::deleteEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through the list
	for (size_t a = 0; a < selection.size(); a++) {
		// Remove the current selected entry if it isn't a directory
		if (selection[a]->getType() != ETYPE_FOLDER)
			archive->removeEntry(selection[a]);
	}

	// Get a list of selected directories
	vector<zipdir_t*> selected_dirs = ((ZipEntryListPanel*)entry_list)->getSelectedDirectories();

	// Go through the list
	for (size_t a = 0; a < selected_dirs.size(); a++) {
		// Remove the selected directory from the archive
		((ZipArchive*)archive)->deleteDirectory(selected_dirs[a]);
	}
}

/* ZipArchivePanel::onAnnouncement
 * Called when an announcement is recieved from the archive that
 * this ZipArchivePanel is managing
 *******************************************************************/
void ZipArchivePanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Do default announcement handling
	ArchivePanel::onAnnouncement(announcer, event_name, event_data);

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
}
