
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
#include "EntryListPanel.h"
#include "WadArchive.h"
#include "TextEntryPanel.h"
#include "DefaultEntryPanel.h"
#include "GfxEntryPanel.h"
#include "PaletteEntryPanel.h"
#include "MultiEntryPanel.h"
#include "AnimatedEntryPanel.h"
#include "SwitchesEntryPanel.h"
#include "GfxConvDialog.h"
#include "ModifyOffsetsDialog.h"
#include "EntryOperations.h"
#include "Clipboard.h"
#include "ArchiveManager.h"
#include <wx/aui/auibook.h>
#include <wx/filename.h>


/*******************************************************************
 * ARCHIVEPANEL CLASS FUNCTIONS
 *******************************************************************/

/* ArchivePanel::ArchivePanel
 * ArchivePanel class constructor
 *******************************************************************/
ArchivePanel::ArchivePanel(wxWindow* parent, Archive* archive)
: wxPanel(parent, -1) {
	this->archive = archive;
	listenTo(archive);

	// Create entry panels
	entry_area = new EntryPanel(this, "nil");
	default_area = new DefaultEntryPanel(this);
	text_area = new TextEntryPanel(this);
	gfx_area = new GfxEntryPanel(this);
	pal_area = new PaletteEntryPanel(this);
	multi_area = new MultiEntryPanel(this);
	animated_area = new AnimatedEntryPanel(this);
	switches_area = new SwitchesEntryPanel(this);

	// Bind events
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ArchivePanel::onEntryMenuClick, this, MENU_ENTRY_RENAME, MENU_ENTRY_END);
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

	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, "Entries");
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_hbox->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// Create entry list panel
	entry_list = new ArchiveEntryList(this);
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
	//entry_list->Bind(wxEVT_LEFT_DOWN, &ArchivePanel::onEntryListLeftClick, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, &ArchivePanel::onEntryListRightClick, this);
	entry_list->Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &ArchivePanel::onEntryListActivated, this);

	// Update size+layout
	entry_list->updateWidth();
	Layout();
}

/* ArchivePanel::save
 * Saves the archive
 *******************************************************************/
void ArchivePanel::save() {
	// Check the archive exists
	if (!archive)
		return;

	// Check the archive has been previously saved
	if (!archive->canSave()) {
		saveAs();
		return;
	}

	// Save the archive
	if (!archive->save()) {
		// If there was an error pop up a message box
		wxMessageBox(s_fmt(_T("Error:\n%s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
	}

	// Refresh entry list
	entry_list->updateList();
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

	// Refresh entry list
	entry_list->updateList();
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
		index = -1;	// If not add to the end of the list

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
			_T("Any File (*.*)|*.*"), wxFD_OPEN|wxFD_MULTIPLE|wxFD_FILE_MUST_EXIST, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_open.ShowModal() == wxID_OK) {
		// Get file selection
		wxArrayString files;
		dialog_open.GetPaths(files);

		// Get the entry index of the last selected list item
		int index = archive->entryIndex(entry_list->getLastSelectedEntry());

		// If something was selected, add 1 to the index so we add the new entry after the last selected
		if (index >= 0)
			index++;
		else
			index = -1;	// If not add to the end of the list
			//index = entry_list->getListSize(); // If not add to the end of the list

		// Go through the list of files
		bool ok = false;
		for (size_t a = 0; a < files.size(); a++) {
			// If only 1 file was selected, prompt for a name
			string name = wxFileName(files[a]).GetName().Upper().Truncate(8);
			if (files.size() == 1)
				name = wxGetTextFromUser(_T("Enter new entry name:"), _T("New Entry"), name);

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

	return true;
}

/* ArchivePanel::deleteEntry
 * Deletes the selected entries
 *******************************************************************/
bool ArchivePanel::deleteEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Clear selection
	entry_list->clearSelection();

	// Go through the list
	for (int a = selection.size() - 1; a >= 0; a--) {
		// Remove the current selected entry
		archive->removeEntry(selection[a]);
	}

	return true;
}

/* ArchivePanel::copyEntry
 * Copies the selected entries
 *******************************************************************/
bool ArchivePanel::copyEntry() {
	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// If something is selected, clear the clipboard
	if (selection.size() > 0)
		theClipboard->clear();
	else
		return false;

	// Copy all selected entries to the clipboard
	for (size_t a = 0; a < selection.size(); a++)
		theClipboard->addItem(new EntryClipboardItem(selection[a]));

	return true;
}

/* ArchivePanel::cutEntry
 * Cuts the selected entries (copy+delete)
 *******************************************************************/
bool ArchivePanel::cutEntry() {
	if (copyEntry())
		return deleteEntry();
	else
		return false;
}

/* ArchivePanel::pasteEntry
 * Pastes any entries on the clipboard either after the last selected
 * entry or at the end of the entry list if nothing is selected
 *******************************************************************/
bool ArchivePanel::pasteEntry() {
	// Get the entry index of the last selected list item
	int index = archive->entryIndex(entry_list->getLastSelectedEntry());

	// If something was selected, add 1 to the index so we add the new entry after the last selected
	if (index >= 0)
		index++;
	else
		index = -1;	// If not add to the end of the list

	// Go through all items on the clipboard
	for (uint32_t a = 0; a < theClipboard->nItems(); a++) {
		// Get the item
		EntryClipboardItem* item = (EntryClipboardItem*)theClipboard->getItem(a);

		// Check it is of the correct type
		if (item->getType() != CLIPBOARD_ENTRY)
			continue;

		// Add a copy of the entry to the archive
		archive->addExistingEntry(item->getEntry(), index, true);
		index++;
	}

	// Force entrylist width update
	//entry_list->columnsUpdate(true);
	//entry_list->updateListWidth();
	Layout();

	return true;
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

	return true;
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

		// Add appropriate extension if needed
		fn.SetExt(selection[a]->getType()->getExtension());

		// Create save file dialog
		wxFileDialog *dialog_save = new wxFileDialog(this, s_fmt(_T("Export Entry \"%s\""), selection[a]->getName().c_str()),
											wxEmptyString, fn.GetFullName(), _T("Any File (*.*)|*.*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

		// Run the dialog & check that the user didn't cancel
		if (dialog_save->ShowModal() == wxID_OK) {
			// If a filename was selected, export it
			selection[a]->exportFile(dialog_save->GetPath());
		}
	}

	return true;
}

/* ArchivePanel::exportEntryWad
 * Exports the selected entries to a Wad archive
 *******************************************************************/
bool ArchivePanel::exportEntryWad() {
	// Create save file dialog
	wxFileDialog dialog_save(this, _T(""), wxEmptyString, wxEmptyString,
								_T("Doom Wad File (*.wad)|*.wad"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_save.ShowModal() == wxID_OK) {
		// If a filename was selected, export the selected entries to a wad archive
		WadArchive::exportEntriesAsWad(dialog_save.GetPath(), entry_list->getSelectedEntries());
	}

	return true;
}

/* ArchivePanel::moveUp
 * Moves selected entries up
 *******************************************************************/
bool ArchivePanel::moveUp() {
	// Get selection
	vector<int> selection = entry_list->getSelection();

	// If nothing is selected, do nothing
	if (selection.size() == 0)
		return false;

	// If the first selected item is at the top of the list
	// then don't move anything up
	if (selection[0] == 0)
		return false;

	// Move each one up by swapping it with the entry above it
	for (size_t a = 0; a < selection.size(); a++) {
		// Get the entries to swap
		ArchiveEntry* entry = archive->getEntry(selection[a]);
		ArchiveEntry* above = archive->getEntry(selection[a]-1);

		// Swap them in the archive
		archive->swapEntries(entry, above);
	}

	// Return success
	return true;
}

/* ArchivePanel::moveDown
 * Moves selected entries down
 *******************************************************************/
bool ArchivePanel::moveDown() {
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
		ArchiveEntry* entry = archive->getEntry(selection[a]);
		ArchiveEntry* below = archive->getEntry(selection[a]+1);

		// Swap them in the archive
		archive->swapEntries(entry, below);
	}

	// Return success
	return true;
}

/* ArchivePanel::gfxConvert
 * Runs the Gfx Conversion Dialog on all selected entries
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
 * Runs the Modify Offsets Dialog and applies the selected offset
 * settings to all selected entries
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

/* ArchivePanel::basConvert
 * Create a new entry after the current one and fill it with an
 * ANIMDEFS conversion of a Boom ANIMATED or SWITCHES lump.
 *******************************************************************/
bool ArchivePanel::basConvert() {
	// Get the entry index of the last selected list item
	int index = archive->entryIndex(entry_list->getLastSelectedEntry());

	// If something was selected, add 1 to the index so we add the new entry after the last selected
	if (index >= 0)
		index++;
	else
		return false; // If not there's a problem

	// Get a list of selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	//entry_list->columnsUpdate(false);

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

		wxLogMessage("Treatment complete\n");
		// Identify the new lump as what it is
		EntryType::detectEntryType(animdef);
		// Failsafe is detectEntryType doesn't accept to work, grumble
		if (animdef->getType() == EntryType::unknownType())
			animdef->setType(EntryType::getType(_T("animdefs")));
	}

	// Force entrylist width update
	//entry_list->columnsUpdate(true);
	//entry_list->updateListWidth();
	Layout();

	// Load entry data into the text editor
	DefaultEntryPanel * meh = (DefaultEntryPanel *)default_area;
	meh->openTextEntry(animdef);

	return true;
}

/* ArchivePanel::palConvert
 * turns a palette from 6-bit to 8-bit
 *******************************************************************/
bool ArchivePanel::palConvert() {
	// Get the entry index of the last selected list item
	ArchiveEntry* pal6bit = entry_list->getLastSelectedEntry();
	const uint8_t * mehmeh = pal6bit->getData(true);
	uint8_t * meh = new uint8_t[pal6bit->getSize()];
	memcpy(meh, mehmeh, pal6bit->getSize());
	for (size_t i = 0; i < pal6bit->getSize(); ++i)
	{
		meh[i] = ((meh[i] << 2) | (meh[i] >> 6));
	}
	pal6bit->importMem(meh, pal6bit->getSize());
	delete[] meh;
	return true;
}

/* ArchivePanel::onAnnouncement
 * Called when an announcement is recieved from the archive that
 * this ArchivePanel is managing
 *******************************************************************/
void ArchivePanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Reset event data for reading
	event_data.seek(0, SEEK_SET);

	// If the archive was closed
	if (announcer == archive && event_name == "close") {
		// Remove the archive panel from the parent notebook
		wxAuiNotebook* parent = (wxAuiNotebook*)GetParent();
		parent->RemovePage(parent->GetPageIndex(this));

		// Clear list archive
		entry_list->setArchive(NULL);

		// Destroy the archive panel
		this->Destroy();
	}

	// If the archive was saved
	if (announcer == archive && event_name == "saved") {
		// Update this tab's name in the parent notebook (if filename was changed)
		wxAuiNotebook* parent = (wxAuiNotebook*)GetParent();
		parent->SetPageText(parent->GetPageIndex(this), archive->getFileName(false));
	}

	/*
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
	*/
}

/* ArchivePanel::openEntry
 * Shows the appropriate entry area and sends the given entry to it
 *******************************************************************/
bool ArchivePanel::openEntry(ArchiveEntry* entry) {
	// Null entry, do nothing
	if (!entry) {
		wxLogMessage("Warning: NULL entry focused in the list");
		return false;
	}

	// Do nothing if the entry is already open
	if (cur_area->getEntry() == entry)
		return false;

	// Detect entry type if it hasn't been already
	if (entry->getType() == EntryType::unknownType())
		EntryType::detectEntryType(entry);

	// Get the appropriate entry panel for the entry's type
	EntryPanel* new_area = default_area;
	if (!entry->getType()->getEditor().Cmp("gfx"))
		new_area = gfx_area;
	else if (!entry->getType()->getEditor().Cmp("palette"))
		new_area = pal_area;
	else if (!entry->getType()->getEditor().Cmp("text"))
		new_area = text_area;
	else if (!entry->getType()->getEditor().Cmp("animated"))
		new_area = animated_area;
	else if (!entry->getType()->getEditor().Cmp("switches"))
		new_area = switches_area;
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

	return true;
}

bool ArchivePanel::showEntryPanel(EntryPanel* new_area, bool ask_save) {
	// If the current entry area has unsaved changes, ask the user if they wish to save the changes
	if (cur_area->isModified() && cur_area->getEntry() && ask_save) {
		int result = wxMessageBox(s_fmt("Save changes to entry \"%s\"?", cur_area->getEntry()->getName().c_str()),
									"Unsaved Changes", wxYES_NO|wxICON_QUESTION);

		if (result == wxYES)
			cur_area->saveEntry();	// Save changes to the entry if yes clicked
	}

	// Get the panel sizer
	wxSizer* sizer = GetSizer();

	// If the new panel is different than the current, swap them
	if (new_area != cur_area) {
		cur_area->Show(false);				// Hide current
		sizer->Replace(cur_area, new_area);	// Swap the panels
		cur_area = new_area;				// Set the new panel to current
		cur_area->Show(true);				// Show current
	}

	// Update panel layout
	Layout();

	return true;
}


/*******************************************************************
 * ARCHIVEPANEL EVENTS
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
		showEntryPanel(multi_area);
		((MultiEntryPanel*)multi_area)->loadEntries(selection);

		// Update panel layout
		Layout();
	}
}

/* ArchivePanel::onEntryListSelect
 * Called when an entry list item is selected
 *******************************************************************/
void ArchivePanel::onEntryListSelect(wxListEvent& e) {
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
		showEntryPanel(multi_area);
		((MultiEntryPanel*)multi_area)->loadEntries(selection);

		// Update panel layout
		Layout();
	}
}

/* ArchivePanel::onEntryListDeselect
 * Called when an entry list item is deselected
 *******************************************************************/
void ArchivePanel::onEntryListDeselect(wxListEvent& e) {
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
		showEntryPanel(multi_area);
		((MultiEntryPanel*)multi_area)->loadEntries(selection);

		// Update panel layout
		Layout();
	}
}

void ArchivePanel::onEntryListLeftClick(wxMouseEvent& e) {
	// Do usual stuff first
	e.Skip();

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
		showEntryPanel(multi_area);
		((MultiEntryPanel*)multi_area)->loadEntries(selection);

		// Update panel layout
		Layout();
	}
}

/* ArchivePanel::onEntryListRightClick
 * Called when the entry list is right clicked
 *******************************************************************/
void ArchivePanel::onEntryListRightClick(wxListEvent& e) {
	// Generate context menu
	wxMenu* context = new wxMenu();
	context->Append(MENU_ENTRY_RENAME, _T("Rename"));
	context->Append(MENU_ENTRY_DELETE, _T("Delete"));
	context->AppendSeparator();
	context->Append(MENU_ENTRY_COPY, _T("Copy"));
	context->Append(MENU_ENTRY_CUT, _T("Cut"));
	context->Append(MENU_ENTRY_PASTE, _T("Paste"));
	context->AppendSeparator();
	context->Append(MENU_ENTRY_IMPORT, _T("Import"));
	context->Append(MENU_ENTRY_EXPORT, _T("Export"));
	context->Append(MENU_ENTRY_EXPORTWAD, _T("Export as Wad"));
	context->AppendSeparator();
	context->Append(MENU_ENTRY_MOVEUP, _T("Move Up"));
	context->Append(MENU_ENTRY_MOVEDOWN, _T("Move Down"));

	// Get selected entries
	vector<ArchiveEntry*> selection = entry_list->getSelectedEntries();

	// Check what types exist in the selection
	bool gfx_selected = false;
	bool bas_selected = false;
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
	}

	// Add gfx-related menu items if gfx are selected
	if (gfx_selected) {
		context->AppendSeparator();
		context->Append(MENU_ENTRY_GFX_CONVERT, _T("Convert Gfx to..."));
		context->Append(MENU_ENTRY_GFX_OFFSETS, _T("Modify Gfx Offsets"));
	}

	// Add Boom Animations/Switches related menu items if they are selected
	if (bas_selected) {
		context->AppendSeparator();
		context->Append(MENU_ENTRY_BAS_CONVERT, _T("Convert to ANIMDEFS"));
	}
	context->Append(MENU_ENTRY_PAL_CONVERT, _T("Pal 6-bit to 8-bit"));

	// Popup the context menu
	PopupMenu(context);
}

/* ArchivePanel::onEntryMenuClick
 * Called when an entry menu item is selected
 *******************************************************************/
void ArchivePanel::onEntryMenuClick(wxCommandEvent& e) {
	switch (e.GetId()) {
		case MENU_ENTRY_RENAME:
			renameEntry();
			break;
		case MENU_ENTRY_DELETE:
			deleteEntry();
			break;
		case MENU_ENTRY_COPY:
			copyEntry();
			break;
		case MENU_ENTRY_CUT:
			cutEntry();
			break;
		case MENU_ENTRY_PASTE:
			pasteEntry();
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
		case MENU_ENTRY_GFX_CONVERT:
			gfxConvert();
			break;
		case MENU_ENTRY_GFX_OFFSETS:
			gfxModifyOffsets();
			break;
		case MENU_ENTRY_BAS_CONVERT:
			basConvert();
			break;
		case MENU_ENTRY_PAL_CONVERT:
			palConvert();
			break;
	}
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

	// Export entry to wad (Shift+Ctrl+E)
	else if (e.GetKeyCode() == 'E' && e.ShiftDown() && e.ControlDown())
		exportEntryWad();

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

	// New entry from file (Shift+Ctrl+N)
	else if (e.GetKeyCode() == 'N' && e.ControlDown() && e.ShiftDown())
		newEntryFromFile();

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
		theArchiveManager->openTextureEditor(theArchiveManager->archiveIndex(archive));

	e.Skip();
}
