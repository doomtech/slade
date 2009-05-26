
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
#include "WxStuff.h"
#include "ArchivePanel.h"
#include "EntryListPanel.h"
#include "WadArchive.h"
#include <wx/aui/auibook.h>
#include <wx/filename.h>


/* ArchivePanel::ArchivePanel
 * ArchivePanel class constructor
 *******************************************************************/
ArchivePanel::ArchivePanel(wxWindow* parent, Archive* archive)
: wxPanel(parent, -1) {
	this->archive = archive;
	listenTo(archive);

	// Create & set sizer
	wxBoxSizer *m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Entry list panel (depends on archive type)
	if (archive->getType() == ARCHIVE_WAD)
		entry_list = new EntryListPanel(this, ENTRY_LIST_PANEL, archive);
	else
		entry_list = new ZipEntryListPanel(this, ENTRY_LIST_PANEL, archive);
	m_hbox->Add(entry_list, 0, wxEXPAND | wxALL, 4);

	entry_list->populateEntryList();

	// Create lump areas
	entry_area = new EntryPanel(this);
	text_area = new TextEntryPanel(this);

	// Add default lump area to the panel
	cur_area = entry_area;
	m_hbox->Add(cur_area, 1, wxEXPAND | wxALL, 4);
	cur_area->Show(true);

	Layout();
}

/* ArchivePanel::~ArchivePanel
 * ArchivePanel class destructor
 *******************************************************************/
ArchivePanel::~ArchivePanel() {
}

/* ArchivePanel::save
 * Saves the archive
 *******************************************************************/
void ArchivePanel::save() {
	// Check the archive exists
	if (!archive)
		return;

	// Check the archive has been previously saved
	if (!archive->getFileName().Cmp(_T("")))
		saveAs();

	// Save the archive
	if (!archive->save()) {
		// If there was an error pop up a message box
		wxMessageBox(s_fmt(_T("Error:\n%s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
	}
}

/* ArchivePanel::saveAs
 * Saves the archive to a new file
 *******************************************************************/
void ArchivePanel::saveAs() {
	// Check the archive exists
	if (!archive)
		return;

	// Setup file filters (temporary, should go through all archive types somehow)
	string formats = archive->getFileExtensionString();
	//string deftype = _T("*.wad");
	string filename = wxFileSelector(_T("Save Archive ") + archive->getFileName(false) + _T(" As"), _T(""), _T(""), wxEmptyString, formats, wxSAVE | wxOVERWRITE_PROMPT);

	// Check a filename was selected
	if (!filename.empty()) {
		// Save the archive
		if (!archive->save(filename)) {
			// If there was an error pop up a message box
			wxMessageBox(s_fmt(_T("Error:\n%s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
		}
	}
}

/* ArchivePanel::newEntry
 * Creates a new entry, asks for a name and adds it before the
 * currently focused entry
 *******************************************************************/
bool ArchivePanel::newEntry() {
	// Get the currently focused entry index
	int index = archive->entryIndex(entry_list->getFocusedEntry());

	// Prompt for new entry name
	string name = wxGetTextFromUser(_T("Enter new entry name:"), _T("New Entry"));

	// Add the entry to the archive
	ArchiveEntry* new_entry = archive->addNewEntry(name, index);

	// Return success if entry was created
	if (new_entry)
		return true;
	else
		return false;
}

/* ArchivePanel::newEntryFromFile
 * Creates a new entry, imports a file into it and adds it before the
 * currently focused entry
 *******************************************************************/
bool ArchivePanel::newEntryFromFile() {
	// Get the currently focused entry index
	int index = archive->entryIndex(entry_list->getFocusedEntry());

	// Create open file dialog
	wxFileDialog *dialog_open = new wxFileDialog(this, _T("Choose file to open"), wxEmptyString, wxEmptyString,
			_T("Any File (*.*)|*.*"), wxFD_OPEN | /*wxFD_MULTIPLE |*/ wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open->ShowModal() == wxID_OK) {
		wxFileName fn(dialog_open->GetPath());
		string filename = fn.GetFullName();

		// Prompt for new entry name
		string name = wxGetTextFromUser(_T("Enter new entry name:"), _T("New Entry"), filename);

		// Add the entry to the archive
		ArchiveEntry* new_entry = archive->addNewEntry(name, index);

		// Load entry data from selected file
		new_entry->importFile(fn.GetFullPath());

		// Return success if entry was created
		if (new_entry)
			return true;
		else
			return false;
	}
	else // If user canceled return false
		return false;
}

/* ArchivePanel::renameEntry
 * Renames the selected entries
 *******************************************************************/
bool ArchivePanel::renameEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through the list
	for (size_t a = 0; a < selection.size(); a++) {
		// Get the current entry's name
		ArchiveEntry* entry = selection[a];
		string old_name = entry->getName();

		// Prompt for a new name
		string new_name = wxGetTextFromUser(_T("Enter new entry name:"), _T("New Entry"), old_name);

		// Rename the entry if a different name was specified
		if (new_name.Cmp(wxEmptyString) && new_name.Cmp(old_name))
			archive->renameEntry(entry, new_name);
	}
}

/* ArchivePanel::deleteEntry
 * Deletes the selected entries
 *******************************************************************/
bool ArchivePanel::deleteEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through the list
	for (size_t a = 0; a < selection.size(); a++) {
		// Remove the current selected entry
		archive->removeEntry(selection[a]);
	}
}

/* ArchivePanel::importEntry
 * Imports files to the selected entries
 *******************************************************************/
bool ArchivePanel::importEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through the list
	for (size_t a = 0; a < selection.size(); a++) {
		// Create open file dialog
		wxFileDialog *dialog_open = new wxFileDialog(this, s_fmt(_T("Import Entry \"%s\""), selection[a]->getName().c_str()),
											wxEmptyString, wxEmptyString, _T("Any File (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST, wxDefaultPosition);

		// Run the dialog & check that the user didn't cancel
		if (dialog_open->ShowModal() == wxID_OK) {
			// If a file was selected, import it
			selection[a]->importFile(dialog_open->GetPath());

			// If the entry is currently open, refresh the entry panel
			if (cur_area->getEntry() == selection[a])
				cur_area->loadEntry(selection[a]);
		}
	}
}

/* ArchivePanel::exportEntry
 * Exports the selected entries to files
 *******************************************************************/
bool ArchivePanel::exportEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Go through the list
	for (size_t a = 0; a < selection.size(); a++) {
		// Setup filename
		wxFileName fn(selection[a]->getName());

		// Add .lmp extension if needed
		fn.SetExt(_T("lmp"));

		// Create save file dialog
		wxFileDialog *dialog_save = new wxFileDialog(this, s_fmt(_T("Export Entry \"%s\""), selection[a]->getName().c_str()),
											wxEmptyString, fn.GetFullName(), _T("Any File (*.*)|*.*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

		// Run the dialog & check that the user didn't cancel
		if (dialog_save->ShowModal() == wxID_OK) {
			// If a filename was selected, export it
			selection[a]->exportFile(dialog_save->GetPath());
		}
	}
}

bool ArchivePanel::exportEntryWad() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Init wad archive to save
	WadArchive* wad = new WadArchive();

	// Go through the list
	for (size_t a = 0; a < selection.size(); a++) {
		// Add each entry to the wad archive
		wad->addExistingEntry(selection[a], selection.size(), true);
	}

	// Create save file dialog
	wxFileDialog *dialog_save = new wxFileDialog(this, _T(""), wxEmptyString, wxEmptyString,
										_T("Doom Wad File (*.wad)|*.wad"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_save->ShowModal() == wxID_OK) {
		// If a filename was selected, save the wad archive
		wad->save(dialog_save->GetPath());
	}

	// Delete the wad archive
	delete wad;
}

/* ArchivePanel::moveUp
 * Moves selected entries up
 *******************************************************************/
bool ArchivePanel::moveUp() {
	// Get the entry list class to handle it
	return entry_list->moveUp();
}

/* ArchivePanel::moveDown
 * Moves selected entries down
 *******************************************************************/
bool ArchivePanel::moveDown() {
	// Get the entry list class to handle it
	return entry_list->moveDown();
}

/* ArchivePanel::onAnnouncement
 * Called when an announcement is recieved from the archive that
 * this ArchivePanel is managing
 *******************************************************************/
void ArchivePanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Reset event data for reading
	event_data.seek(0, SEEK_SET);

	// If the archive was closed
	if (announcer == archive && event_name == _T("close")) {
		// Remove the archive panel from the parent notebook
		wxAuiNotebook* parent = (wxAuiNotebook*)GetParent();
		parent->RemovePage(parent->GetPageIndex(this));

		// Destroy the archive panel
		this->Destroy();
	}

	// Archive entries were swapped
	if (announcer == archive && event_name == _T("entries_swapped")) {
		int e1, e2;
		if (event_data.read(&e1, sizeof(int)) && event_data.read(&e2, sizeof(int)))
			entry_list->swapItems(e1, e2);
	}

	// An entry was added to the archive
	if (announcer == archive && event_name == _T("entry_added")) {
		DWORD index = 0;
		if (event_data.read(&index, sizeof(DWORD)))
			entry_list->addEntry(index);
	}

	// An entry in the archive was modified
	if (announcer == archive && event_name == _T("entry_modified")) {
		DWORD index = 0;
		if (event_data.read(&index, sizeof(DWORD)))
			entry_list->updateEntry(index);
	}

	// An entry in the archive was removed
	if (announcer == archive && event_name == _T("entry_removed")) {
		DWORD index = 0;
		if (event_data.read(&index, sizeof(DWORD)))
			entry_list->removeEntry(index);
	}
}

BEGIN_EVENT_TABLE(ArchivePanel, wxPanel)
	EVT_LIST_ITEM_FOCUSED(ArchivePanel::ENTRY_LIST_PANEL, ArchivePanel::onEntryListChange)
END_EVENT_TABLE()

/* ArchivePanel::onEntryListChange
 * Called when the current focus on the list control in the
 * entry list panel is changed (ie when the user selects an entry
 * in the list)
 *******************************************************************/
void ArchivePanel::onEntryListChange(wxListEvent& event) {
	// Get the panel sizer
	wxSizer* sizer = GetSizer();

	// Hide the current lump area, replace it with the new
	// lump area, and show that
	cur_area->Show(false);
	sizer->Replace(cur_area, text_area);
	cur_area = text_area;
	if (!cur_area->loadEntry(entry_list->getFocusedEntry())) {
		wxMessageBox(s_fmt(_T("Error loading entry:\n%s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
	}
	cur_area->Show(true);

	Layout();
}
