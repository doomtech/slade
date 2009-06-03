
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    EntryListPanel.cpp
 * Description: EntryListPanel class (and related classes).
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
#include "Archive.h"
#include "ZipArchive.h"
#include "EntryListPanel.h"
#include "ArchiveEntry.h"
#include <wx/filename.h>
#include "ArchiveManager.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
wxColor col_new(0, 150, 0);
wxColor col_modified(0, 80, 180);


/* get_entry_icon
 * Gets an entry icon image from the SLADE resource pk3 and returns
 * it as a wxImage (will be empty if the specified icon isn't found)
 *******************************************************************/
wxImage get_entry_icon(string name, int type) {
	// Init
	wxImage image;

	// Get the needed entry from the SLADE resource pk3
	Archive* resource_pk3 = theArchiveManager.resourceArchive();
	string entry_path = _T("entry_icons/") + name;
	ArchiveEntry* entry = resource_pk3->getEntry(entry_path);

	if (entry) {
		// Export entry data to a temporary file
		entry->exportFile(_T("sladetemp"));

		// Load the data to a wxImage
		image.LoadFile(_T("sladetemp"), type);

		// Delete the temporary file
		wxRemoveFile(_T("sladetemp"));
	}

	// Return the image, loaded or not
	return image;
}

#ifdef _WIN32
#define ENTRYLIST_FLAGS wxLC_REPORT|wxLC_VRULES|wxLC_HRULES // No wxLC_EDIT_LABELS on win32 as it doesn't work in wxMSW :(
#else
#define ENTRYLIST_FLAGS wxLC_REPORT|wxLC_VRULES|wxLC_HRULES|wxLC_EDIT_LABELS
#endif

/* EntryList::EntryList
 * EntryList class constructor
 *******************************************************************/
EntryList::EntryList(EntryListPanel *parent, int id)
: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, ENTRYLIST_FLAGS) {
	this->parent = parent;
}

/* EntryList::~EntryList
 * EntryList class destructor
 *******************************************************************/
EntryList::~EntryList() {
}

/* EntryList::updateEntry
 * Updates the list item at index with it's associated archive
 * entry's information (name/size/type).
 * Returns false on invalid index or missing entry, true otherwise
 *******************************************************************/
bool EntryList::updateEntry(int index) {
	// Check that index is valid
	if (index < 0 || index >= this->GetItemCount())
		return false;

	// Get the Entry associated with entry index
	ArchiveEntry* entry = (ArchiveEntry*) GetItemData(index);

	// Check that it exists
	if (!entry) {
		wxLogMessage(_T("EntryList item %d has no associated archive entry!"), index);
		return false;
	}

	// -- Setup entry --
	// Name
	wxListItem li;
	li.SetId(index);
	li.SetText(entry->getName());
	SetItem(li);
	SetColumnWidth(0, wxLIST_AUTOSIZE);

	// Size
	li.SetText(s_fmt(_T("%d"), entry->getSize()));
	li.SetColumn(1);
	SetItem(li);
	SetColumnWidth(1, wxLIST_AUTOSIZE);

	// Type
	li.SetText(entry->getTypeString());
	li.SetColumn(2);
	SetItem(li);
	SetColumnWidth(2, wxLIST_AUTOSIZE);

	// Set default text colour
	SetItemTextColour(index, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));

	// Set entry status text colour if needed
	if (entry->getState() == 2)
		SetItemTextColour(index, col_new);
	else if (entry->getState() == 1)
		SetItemTextColour(index, col_modified);

	// Setup size
	SetMinSize(wxSize(getWidth(), -1));

	return true;
}

int EntryList::getWidth() {
	// For the moment. Kinda annoying I have to do this actually, it should be automatic >_<
	return GetColumnWidth(0) + GetColumnWidth(1) + GetColumnWidth(2) + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this) + 4;
}

/* EntryListPanel::EntryListPanel
 * EntryListPanel class constructor
 *******************************************************************/
EntryListPanel::EntryListPanel(wxWindow *parent, int id, Archive* archive)
: wxPanel(parent, id) {
	// Init variables
	this->archive = archive;

	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Entries"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);

	entry_list = new EntryList(this, ENTRY_LIST);
	framesizer->Add(entry_list, 1, wxEXPAND | wxALL, 4);

	Layout();
}

/* EntryListPanel::~EntryListPanel
 * EntryListPanel class destructor
 *******************************************************************/
EntryListPanel::~EntryListPanel() {
}

/* EntryListPanel::populateEntryList
 * Clears & populates the entry list with all the entries in
 * the archive
 *******************************************************************/
void EntryListPanel::populateEntryList() {
	// Clear the list
	entry_list->ClearAll();

	// Hide the list
	entry_list->Show(false);

	// Create the "Name" column
	entry_list->InsertColumn(0, _T("Name"));

	// Create the "Size" column
	entry_list->InsertColumn(1, _T("Size"));

	// Create the "Type" column
	entry_list->InsertColumn(2, _T("Type"));

	// Go through all entries and add them to the list
	for (int a = 0; a < archive->numEntries(); a++) {
		// Setup new entry
		ArchiveEntry* entry = archive->getEntry(a);
		wxListItem li;
		li.SetId(a);
		li.SetData(entry);
		entry_list->InsertItem(li);

		// Name
		li.SetId(a);
		li.SetText(entry->getName());
		entry_list->SetItem(li);

		// Size
		li.SetText(s_fmt(_T("%d"), entry->getSize()));
		li.SetColumn(1);
		entry_list->SetItem(li);

		// Type
		li.SetText(entry->getTypeString());
		li.SetColumn(2);
		entry_list->SetItem(li);
	}

	// Setup column widths
	entry_list->SetColumnWidth(0, wxLIST_AUTOSIZE);
	entry_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
	entry_list->SetColumnWidth(2, wxLIST_AUTOSIZE);

	// Setup size
	entry_list->SetMinSize(wxSize(entry_list->getWidth(), -1));

	// Show the list
	entry_list->Show();
}

/* EntryListPanel::getFocusedEntry
 * Gets the archive entry associated with the currently focused list
 * item. Returns NULL if nothing is focused
 *******************************************************************/
ArchiveEntry* EntryListPanel::getFocusedEntry() {
	// Get the focus index
	int focus = getFocus();

	// Check that the focus index is valid
	if (focus < 0 || focus > entry_list->GetItemCount())
		return NULL;

	// Return the focused archive entry
	return (ArchiveEntry*) (entry_list->GetItemData(focus));
}

/* EntryListPanel::getFocus
 * Gets the list index of the currently focused list item
 *******************************************************************/
int EntryListPanel::getFocus() {
	// Get the item in the list that is focused
	return entry_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
}

/* EntryListPanel::getSelectedEntries
 * Returns a vector of all selected archive entries
 *******************************************************************/
vector<ArchiveEntry*> EntryListPanel::getSelectedEntries() {
	// Init vector
	vector<ArchiveEntry*> ret;

	// Go through all items
	long item = -1;
	while (true) {
		// Get the next item in the list that is selected
		item = entry_list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// If -1 then none were selected
		if (item == -1)
			break;

		// Otherwise add the selected entry to the vector
		ret.push_back((ArchiveEntry*)(entry_list->GetItemData(item)));
	}

	return ret;
}

/* EntryListPanel::getSelection
 * Returns a vector of all selected list item indices
 *******************************************************************/
vector<int> EntryListPanel::getSelection() {
	// Init vector
	vector<int> ret;

	// Go through all items
	long item = -1;
	while (true) {
		// Get the next item in the list that is selected
		item = entry_list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// If -1 then none were selected
		if (item == -1)
			break;

		// Otherwise add the selected index to the vector
		ret.push_back(item);
	}

	return ret;
}

/* EntryListPanel::getLastSelected
 * Gets the index of the last selected item in the list, or -1 if no
 * item is selected
 *******************************************************************/
int EntryListPanel::getLastSelected() {
	// Go through all items
	int item = -1;
	while (true) {
		// Get the next item in the list that is selected
		int index = entry_list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		// Exit loop if there is no selected item after the current
		if (index == -1)
			break;

		// Otherwise set the item to this index and keep going
		item = index;
	}

	return item;
}

/* EntryListPanel::getLastSelectedEntry
 * Gets the archive entry associated with the last selected item in
 * the list. Returns NULL if no item is selected
 *******************************************************************/
ArchiveEntry* EntryListPanel::getLastSelectedEntry() {
	int index = getLastSelected();

	if (index >= 0)
		return (ArchiveEntry*)(entry_list->GetItemData(index));
	else
		return NULL;
}

/* EntryListPanel::swapItems
 * Swaps two list items (including their focused/selected states)
 *******************************************************************/
bool EntryListPanel::swapItems(int item1, int item2) {
	// Check item 1
	if (item1 < 0 || item1 > entry_list->GetItemCount())
		return false;

	// Check item 2
	if (item2 < 0 || item2 > entry_list->GetItemCount())
		return false;

	// Get item 1 info
	wxListItem i1;
	i1.SetId(item1);
	i1.SetMask(0xFFFF);
	entry_list->GetItem(i1);
	long state1 = entry_list->GetItemState(item1, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED); // Not sure why I have to get/set states separately :/

	// Get item 2 info
	wxListItem i2;
	i2.SetId(item2);
	i2.SetMask(0xFFFF);
	entry_list->GetItem(i2);
	long state2 = entry_list->GetItemState(item2, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

	// Swap the items
	i1.SetId(item2);
	i2.SetId(item1);
	entry_list->SetItem(i1);
	entry_list->SetItem(i2);
	entry_list->SetItemState(item1, state2, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
	entry_list->SetItemState(item2, state1, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

	// Update the items
	entry_list->updateEntry(item1);
	entry_list->updateEntry(item2);

	return true;
}

/* EntryListPanel::addEntry
 * Adds an entry to the list
 *******************************************************************/
bool EntryListPanel::addEntry(DWORD archive_index) {
	// Get the entry to add
	ArchiveEntry* entry = archive->getEntry(archive_index);

	// Check it
	if (!entry)
		return false;

	// Setup new entry
	wxListItem li;
	li.SetId(archive_index);
	li.SetData(entry);

	// Add it to the list
	entry_list->InsertItem(li);
	entry_list->updateEntry(archive_index);

	return true;
}

/* EntryListPanel::updateEntry
 * Updates an entry in the list
 *******************************************************************/
bool EntryListPanel::updateEntry(DWORD archive_index) {
	// Just update the list item corresponding with the entry index
	// (it's 1-1 in a normal EntryListPanel, will be different for zip archives)
	entry_list->updateEntry(archive_index);
}

/* EntryListPanel::removeEntry
 * Removes an entry from the list
 *******************************************************************/
bool EntryListPanel::removeEntry(DWORD archive_index) {
	// Just delete the list item corresponding with the entry index
	// (it's 1-1 in a normal EntryListPanel, will be different for zip archives)
	entry_list->DeleteItem(archive_index);
}

/* EntryListPanel::getEntryListItem
 * Gets the list item index of the given entry. Returns -1 if the
 * entry doesn't exist in the list, or the given entry was NULL
 *******************************************************************/
int EntryListPanel::getEntryListItem(ArchiveEntry* entry) {
	if (!entry)
		return -1;

	int count = entry_list->GetItemCount();
	for (int a = 0; a < count; a++) {
		if ((ArchiveEntry*)entry_list->GetItemData(a) == entry)
			return a;
	}

	return -1;
}

/* EntryListPanel::moveUp
 * Moves all selected entries up.
 * Returns false if the first selected item was at the top of the
 * list, true otherwise
 *******************************************************************/
bool EntryListPanel::moveUp() {
	// Get selection
	vector<int> selection = getSelection();

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
		ArchiveEntry* entry = (ArchiveEntry*)(entry_list->GetItemData(selection[a]));
		ArchiveEntry* above = (ArchiveEntry*)(entry_list->GetItemData(selection[a]-1));

		// Swap them in the archive
		archive->swapEntries(entry, above);
	}

	// Return success
	return true;
}

/* EntryListPanel::moveDown
 * Moves all selected entries down.
 * Returns false if the last selected item was at the end of the
 * list, true otherwise
 *******************************************************************/
bool EntryListPanel::moveDown() {
	// Get selection
	vector<int> selection = getSelection();

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
		ArchiveEntry* entry = (ArchiveEntry*)(entry_list->GetItemData(selection[a]));
		ArchiveEntry* below = (ArchiveEntry*)(entry_list->GetItemData(selection[a]+1));

		// Swap them in the archive
		archive->swapEntries(entry, below);
	}

	// Return success
	return true;
}

/* EntryListPanel::newEntry
 * Adds a new entry to the archive after the last selected entry in
 * the list. If nothing is selected it is added at the end of the
 * list. Returns the newly created entry, or NULL if creation failed
 * (shouldn't ever)
 *******************************************************************/
ArchiveEntry* EntryListPanel::newEntry(string name) {
	// Get the entry index of the last selected list item
	int index = archive->entryIndex(getLastSelectedEntry());

	// If something was selected, add 1 to the index so we add the new entry after the last selected
	if (index >= 0)
		index++;

	// Add the entry to the archive
	ArchiveEntry* new_entry = archive->addNewEntry(name, index);

	// Return the newly created entry
	return new_entry;
}

/* EntryListPanel::newEntryFromFile
 * Same as newEntry, but imports the specified file into the new
 * entry
 *******************************************************************/
ArchiveEntry* EntryListPanel::newEntryFromFile(string name, string filename) {
	// Create the new entry
	ArchiveEntry* new_entry = newEntry(name);

	// If it was created, import the given file into it
	if (new_entry)
		new_entry->importFile(filename);

	// Return the created entry
	return new_entry;
}

BEGIN_EVENT_TABLE(EntryListPanel, wxPanel)
	EVT_LIST_ITEM_FOCUSED(ENTRY_LIST, EntryListPanel::onEntryListChange)
	EVT_LIST_ITEM_ACTIVATED(ENTRY_LIST, EntryListPanel::onEntryListActivated)
	EVT_LIST_END_LABEL_EDIT(ENTRY_LIST, EntryListPanel::onEntryListEditLabel)
END_EVENT_TABLE()

/* EntryListPanel::onEntryListChange
 * Called when the current focus on the entry list control is changed
 *******************************************************************/
void EntryListPanel::onEntryListChange(wxListEvent& event) {
	// Pass the event up to the parent window, as the archive panel should deal with it,
	// not this panel.
	event.Skip();
}

void EntryListPanel::onEntryListActivated(wxListEvent& event) {
}

void EntryListPanel::onEntryListEditLabel(wxListEvent& event) {
	ArchiveEntry* entry = getFocusedEntry();

	if (entry && !event.IsEditCancelled()) {
		archive->renameEntry(entry, event.GetLabel());
	}
}


/* ZipEntryListPanel::ZipEntryListPanel
 * ZipEntryListPanel class constructor
 *******************************************************************/
ZipEntryListPanel::ZipEntryListPanel(wxWindow* parent, int id, Archive* archive)
: EntryListPanel(parent, id, archive) {
	// Check the archive type
	if (archive->getType() != ARCHIVE_ZIP) {
		wxMessageBox(_T("Error: Attempt to open a non-zip archive with a zip entry list, this shouldn't happen"), _T("Error"));
		archive = NULL;
	}

	// Init directory
	cur_directory = ((ZipArchive*)archive)->getRootDirectory();

	// Create dummy folder entry (all folder list items will point to this entry)
	dummy_folder_entry = new ArchiveEntry();
	dummy_folder_entry->setType(ETYPE_FOLDER);
	dummy_folder_entry->setSize(0);
	dummy_folder_entry->setState(0);
}

/* ZipEntryListPanel::~ZipEntryListPanel
 * ZipEntryListPanel class destructor
 *******************************************************************/
ZipEntryListPanel::~ZipEntryListPanel() {
	delete dummy_folder_entry;
}

/* ZipEntryListPanel::populateEntryList
 * Clears & populates the entry list with all the entries in
 * the current directory, all subdirectories, and a '..' entry if
 * it is needed
 *******************************************************************/
void ZipEntryListPanel::populateEntryList() {
	// Clear the list
	entry_list->ClearAll();

	// Hide the list
	entry_list->Show(false);

	// Create the "Name" column
	entry_list->InsertColumn(0, _T("Name"));

	// Create the "Size" column
	entry_list->InsertColumn(1, _T("Size"));

	// Create the "Type" column
	entry_list->InsertColumn(2, _T("Type"));

	// Typecast directory
	zipdir_t* dir = (zipdir_t*)cur_directory;

	// Go through and add all subdirectories in the current directory
	int index = 0;
	for (size_t a = 0; a < dir->subdirectories.size(); a++) {
		zipdir_t* subdir = dir->subdirectories[a];

		// Setup subdirectory item
		wxListItem li;
		li.SetId(index);
		li.SetData(dummy_folder_entry);

		// Add it to the list
		entry_list->InsertItem(li);
		entry_list->updateEntry(index);

		// Set name manually
		li.SetText(subdir->name);
		entry_list->SetItem(li);

		// Set size manually
		li.SetColumn(1);
		li.SetText(s_fmt(_T("%d Entries"), subdir->numEntries(false)));
		entry_list->SetItem(li);

		index++;
	}

	// Go through and add all entries in the current directory
	for (size_t a = 0; a < dir->entries.size(); a++) {
		// Setup new item
		ArchiveEntry* entry = dir->entries[a];
		wxListItem li;
		li.SetId(index);
		li.SetData(entry);
		entry_list->InsertItem(li);

		// Name
		li.SetText(entry->getName());
		entry_list->SetItem(li);

		// Size
		li.SetText(s_fmt(_T("%d"), entry->getSize()));
		li.SetColumn(1);
		entry_list->SetItem(li);

		// Type
		li.SetText(entry->getTypeString());
		li.SetColumn(2);
		entry_list->SetItem(li);

		// Set entry status text colour if needed
		if (entry->getState() == 2)
			entry_list->SetItemTextColour(a, col_new);
		else if (entry->getState() == 1)
			entry_list->SetItemTextColour(a, col_modified);

		index++;
	}

	// If we're not in the root directory, add a '..' item
	if (cur_directory != ((ZipArchive*)archive)->getRootDirectory()) {
		// Setup item
		wxListItem li;
		li.SetId(0);
		li.SetData(dummy_folder_entry);

		// Add it to the list
		entry_list->InsertItem(li);
		entry_list->updateEntry(0);

		// Set name manually
		li.SetText(_T(".."));
		entry_list->SetItem(li);
	}

	// Setup column widths
	entry_list->SetColumnWidth(0, wxLIST_AUTOSIZE);
	entry_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
	entry_list->SetColumnWidth(2, wxLIST_AUTOSIZE);

	// Setup size
	entry_list->SetMinSize(wxSize(entry_list->getWidth(), -1));

	// Show the list
	entry_list->Show(true);
}

/* ZipEntryListPanel::onEntryListActivated
 * Called when an item in the list is activated (double clicked or
 * pressed enter). Changes to that difectory if it is a directory
 * entry.
 *******************************************************************/
void ZipEntryListPanel::onEntryListActivated(wxListEvent& event) {
	// Get the entry that was activated
	ArchiveEntry* entry = (ArchiveEntry*)(entry_list->GetItemData(event.GetIndex()));

	// If it doesn't exist, return
	if (!entry)
		return;

	// Check for folder
	if (entry->getType() == ETYPE_FOLDER) {
		// If a folder was activated, open it
		if (entry_list->GetItemText(event.GetIndex()).Cmp(_T(".."))) {
			cur_directory = ((zipdir_t*)cur_directory)->getSubDir(event.GetText());
		}
		else {
			// '..' item, go up 1 directory
			if (((zipdir_t*)cur_directory)->parent_dir)
				cur_directory = ((zipdir_t*)cur_directory)->parent_dir;
		}

		// Refresh the list
		populateEntryList();
		GetParent()->Layout();
	}
	else {
		// Otherwise do default activate action
		EntryListPanel::onEntryListActivated(event);
	}
}

/* EntryListPanel::swapItems
 * Swaps two list items (including their focused/selected states)
 * Override to work correctly with zip directories (as the entry list
 * won't be a 1-1 match index-wise with the archive)
 *******************************************************************/
bool ZipEntryListPanel::swapItems(int item1, int item2) {
	return false;
}

/* ZipEntryListPanel::addEntry
 * Called when an entry is added to the archive. If the added entry
 * is in the current directory it is added to the list. If the added
 * entry is in a *new* subdirectory of the current directory then
 * a new subdirectory entry is added, otherwise it does nothing
 *******************************************************************/
bool ZipEntryListPanel::addEntry(DWORD archive_index) {
	return false;
}

/* ZipEntryListPanel::updateEntry
 * Updates an entry in the list, if it is in the current directory
 *******************************************************************/
bool ZipEntryListPanel::updateEntry(DWORD archive_index) {
	return false;
}

/* ZipEntryListPanel::removeEntry
 * Removes an entry from the list if it is in the current directory
 *******************************************************************/
bool ZipEntryListPanel::removeEntry(DWORD archive_index) {
	return false;
}

/* ZipEntryListPanel::newEntry
 * The same as EntryListPanel::newEntry, but more advanced to deal
 * with directories. If the given name is a relative or absolute
 * directory, the entry is created with the appropriate directory
 * value.
 *******************************************************************/
ArchiveEntry* ZipEntryListPanel::newEntry(string name) {
	return NULL;
}

/* ZipEntryListPanel::newEntryFromFile
 * Same as newEntry, but imports a file into the created entry
 *******************************************************************/
ArchiveEntry* ZipEntryListPanel::newEntryFromFile(string name, string filename) {
	// Create the new entry
	ArchiveEntry* new_entry = newEntry(name);

	// If it was created, import the given file into it
	if (new_entry)
		new_entry->importFile(filename);

	// Return the created entry
	return new_entry;
}

void ZipEntryListPanel::onEntryListEditLabel(wxListEvent& event) {
	// TODO: Check if renamed entry is a folder and rename it in the archive accordingly
	ArchiveEntry* entry = getFocusedEntry();
	if (entry && !event.IsEditCancelled()) {
		archive->renameEntry(entry, event.GetLabel());
	}
}