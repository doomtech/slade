
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
#include "TextEntryPanel.h"
#include "DefaultEntryPanel.h"
#include "GfxEntryPanel.h"
#include "PaletteEntryPanel.h"
#include <wx/aui/auibook.h>
#include <wx/filename.h>


/* ArchivePanel::ArchivePanel
 * ArchivePanel class constructor
 *******************************************************************/
ArchivePanel::ArchivePanel(wxWindow* parent, Archive* archive)
: wxPanel(parent, -1) {
	this->archive = archive;
	listenTo(archive);

	// Create entry panels
	entry_area = new EntryPanel(this);
	default_area = new DefaultEntryPanel(this);
	text_area = new TextEntryPanel(this);
	gfx_area = new GfxEntryPanel(this);
	pal_area = new PaletteEntryPanel(this);

	// Create entry menu
	menu_entry = new wxMenu(_T(""));
	menu_entry->Append(MENU_ENTRY_RENAME, _T("Rename"));
	menu_entry->Append(MENU_ENTRY_DELETE, _T("Delete"));
	menu_entry->AppendSeparator();
	menu_entry->Append(MENU_ENTRY_IMPORT, _T("Import"));
	menu_entry->Append(MENU_ENTRY_EXPORT, _T("Export"));
	menu_entry->Append(MENU_ENTRY_EXPORTWAD, _T("Export as Wad"));
	menu_entry->AppendSeparator();
	menu_entry->Append(MENU_ENTRY_MOVEUP, _T("Move Up"));
	menu_entry->Append(MENU_ENTRY_MOVEDOWN, _T("Move Down"));
}

/* ArchivePanel::~ArchivePanel
 * ArchivePanel class destructor
 *******************************************************************/
ArchivePanel::~ArchivePanel() {
}

/* ArchivePanel::init
 * Initialises the panel layout (has to be called separately from the
 * constructor, otherwise ZipArchivePanel won't create a
 * ZipEntryListPanel for whatever reason)
 *******************************************************************/
void ArchivePanel::init() {
	// Create & set sizer
	wxBoxSizer *m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Entry list panel
	entry_list = new EntryListPanel(this, ENTRY_LIST_PANEL, archive);
	m_hbox->Add(entry_list, 0, wxEXPAND | wxALL, 4);

	// Add default entry panel to the panel
	cur_area = entry_area;
	m_hbox->Add(cur_area, 1, wxEXPAND | wxALL, 4);
	cur_area->Show(true);

	Layout();
	entry_list->populateEntryList();
}

/* ArchivePanel::save
 * Saves the archive
 *******************************************************************/
void ArchivePanel::save() {
	// Check the archive exists
	if (!archive)
		return;

	// Check the archive has been previously saved
	if (!archive->isOnDisk()) {
		saveAs();
		return;
	}

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
	string filename = wxFileSelector(_T("Save Archive ") + archive->getFileName(false) + _T(" As"), _T(""), _T(""), wxEmptyString, formats, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

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
 * Adds a new entry to the archive after the last selected entry in
 * the list. If nothing is selected it is added at the end of the
 * list. Asks the user for a name for the new entry, and doesn't add
 * one if no name is entered. Returns true if the entry was created,
 * false otherwise.
 *******************************************************************/
bool ArchivePanel::newEntry() {
	// Prompt for new entry name
	string name = wxGetTextFromUser(_T("Enter new entry name:"), _T("New Entry"));

	// Check if any name was entered
	if (name == _T(""))
		return false;

	// Get the entry index of the last selected list item
	int index = archive->entryIndex(entry_list->getLastSelectedEntry());

	// If something was selected, add 1 to the index so we add the new entry after the last selected
	if (index >= 0)
		index++;
	else
		index = entry_list->getListSize(); // If not add to the end of the list

	// Add the entry to the archive
	ArchiveEntry* new_entry = archive->addNewEntry(name, index);

	// Return whether the entry was created ok
	return !!new_entry;
}

/* ArchivePanel::newEntryFromFile
 * Same as newEntry, but imports a file into the created entry
 *******************************************************************/
bool ArchivePanel::newEntryFromFile() {
	// Create open file dialog
	wxFileDialog dialog_open(this, _T("Choose file to open"), wxEmptyString, wxEmptyString,
			_T("Any File (*.*)|*.*"), wxFD_OPEN | /*wxFD_MULTIPLE |*/ wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open.ShowModal() == wxID_OK) {
		wxFileName fn(dialog_open.GetPath());
		string filename = fn.GetFullName();

		// Prompt for new entry name
		string name = wxGetTextFromUser(_T("Enter new entry name:"), _T("New Entry"), filename);

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

/* ArchivePanel::exportEntryWad
 * Exports the selected entries to a Wad archive
 *******************************************************************/
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

	// If the archive was saved
	if (announcer == archive && event_name == _T("saved")) {
		// Update this tab's name in the parent notebook (if filename was changed)
		wxAuiNotebook* parent = (wxAuiNotebook*)GetParent();
		parent->SetPageText(parent->GetPageIndex(this), archive->getFileName(false));
	}

	// Archive entries were swapped
	if (announcer == archive && event_name == _T("entries_swapped")) {
		int i1, i2;
		wxUIntPtr e1, e2;

		// Check all event data is there
		if (!event_data.read(&i1, sizeof(int)) || !event_data.read(&i2, sizeof(int)) ||
			!event_data.read(&e1, sizeof(wxUIntPtr)) || !event_data.read(&e2, sizeof(wxUIntPtr)))
			return;

		// Send to entry list to handle
		entry_list->swapItems(i1, i2, (ArchiveEntry*)wxUIntToPtr(e1), (ArchiveEntry*)wxUIntToPtr(e2));
	}

	// An entry was added to the archive
	if (announcer == archive && event_name == _T("entry_added")) {
		uint32_t index = 0;
		wxUIntPtr e = 0;

		// Check all event data is there
		if (!event_data.read(&index, sizeof(uint32_t)) || !event_data.read(&e, sizeof(wxUIntPtr)))
			return;

		// Send to entry list to handle
		entry_list->addEntry(index, (ArchiveEntry*)wxUIntToPtr(e));
	}

	// An entry in the archive was modified
	if (announcer == archive && event_name == _T("entry_modified")) {
		uint32_t index = 0;
		wxUIntPtr e = 0;

		// Check all event data is there
		if (!event_data.read(&index, sizeof(uint32_t)) || !event_data.read(&e, sizeof(wxUIntPtr)))
			return;

		// Send to entry list to handle
		entry_list->updateEntry(index, (ArchiveEntry*)wxUIntToPtr(e));
	}

	// An entry in the archive was removed
	if (announcer == archive && event_name == _T("entry_removed")) {
		uint32_t index = 0;
		wxUIntPtr e = 0;

		// Check all event data is there
		if (!event_data.read(&index, sizeof(uint32_t)) || !event_data.read(&e, sizeof(wxUIntPtr)))
			return;

		// Send to entry list to handle
		entry_list->removeEntry(index, (ArchiveEntry*)wxUIntToPtr(e));
	}
}

BEGIN_EVENT_TABLE(ArchivePanel, wxPanel)
	EVT_LIST_ITEM_FOCUSED(ArchivePanel::ENTRY_LIST_PANEL, ArchivePanel::onEntryListChange)
	EVT_LIST_ITEM_RIGHT_CLICK(ArchivePanel::ENTRY_LIST_PANEL, ArchivePanel::onEntryListRightClick)
	EVT_MENU_RANGE(MENU_ENTRY_RENAME, MENU_ENTRY_END, ArchivePanel::onEntryMenuClick)
END_EVENT_TABLE()

/* ArchivePanel::onEntryListChange
 * Called when the current focus on the list control in the
 * entry list panel is changed (ie when the user selects an entry
 * in the list)
 *******************************************************************/
void ArchivePanel::onEntryListChange(wxListEvent& event) {
	// Get the panel sizer
	wxSizer* sizer = GetSizer();

	// If nothing was selected, show blank entry area
	if (event.GetIndex() == -1) {
		cur_area->Show(false);
		sizer->Replace(cur_area, entry_area);
		cur_area = entry_area;
		cur_area->Show(true);
		return;
	}

	// Get the focused entry
	ArchiveEntry* entry = entry_list->getFocusedEntry();

	// Null entry, do nothing
	if (!entry) {
		wxLogMessage(_T("Warning: NULL entry focused in the list"));
		return;
	}

	// Get the appropriate entry panel for the entry's type
	EntryPanel* new_area = default_area;
	switch (entry->getType()) {
		case ETYPE_TEXT:
			new_area = text_area;
			break;
		case ETYPE_PATCH:
			new_area = gfx_area;
			break;
		case ETYPE_SPRITE:
			new_area = gfx_area;
			break;
		case ETYPE_FLAT:
			new_area = gfx_area;
			break;
		case ETYPE_GFX:
			new_area = gfx_area;
			break;
		case ETYPE_GFX2:
			new_area = gfx_area;
			break;
		case ETYPE_PNG:
			new_area = gfx_area;
			break;
		case ETYPE_IMAGE:
			new_area = gfx_area;
			break;
		case ETYPE_PLAYPAL:
			new_area = pal_area;
			break;
	}

	// If the new panel is different than the current, swap them
	if (new_area != cur_area) {
		cur_area->Show(false);				// Hide current
		sizer->Replace(cur_area, new_area);	// Swap the panels
		cur_area = new_area;				// Set the new panel to current
		cur_area->Show(true);				// Show current
	}

	// Load the entry into the panel
	if (!cur_area->loadEntry(entry)) {
		wxMessageBox(s_fmt(_T("Error loading entry:\n%s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
	}

	Layout();
}

void ArchivePanel::onEntryListRightClick(wxListEvent& event) {
	PopupMenu(menu_entry);
}

void ArchivePanel::onEntryMenuClick(wxCommandEvent& event) {
	switch (event.GetId()) {
		case MENU_ENTRY_RENAME:
			renameEntry();
			break;
		case MENU_ENTRY_DELETE:
			deleteEntry();
			break;
		case MENU_ENTRY_IMPORT:
			importEntry();
			break;
		case MENU_ENTRY_EXPORT:
			exportEntry();
			break;
		case MENU_ENTRY_EXPORTWAD:
			exportEntryWad();
			break;
		case MENU_ENTRY_MOVEUP:
			moveUp();
			break;
		case MENU_ENTRY_MOVEDOWN:
			moveDown();
			break;
	}
}
