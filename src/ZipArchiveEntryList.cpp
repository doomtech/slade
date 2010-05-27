
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ZipArchiveEntryList.cpp
 * Description: A list widget that shows all entries in a tree-based
 *              archive (eg ZipArchive). Keeps in sync with it's
 *              associated archive automatically.
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
#include "ZipArchiveEntryList.h"


/*******************************************************************
 * ZIPARCHIVEENTRYLIST CLASS FUNCTIONS
 *******************************************************************/

/* ZipArchiveEntryList::ZipArchiveEntryList
 * ZipArchiveEntryList class constructor
 *******************************************************************/
ZipArchiveEntryList::ZipArchiveEntryList(wxWindow* parent) : ArchiveEntryList(parent) {
	// Bind events
	Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, &ZipArchiveEntryList::onListItemActivated, this);

	// Create dummy folder entry (all folder list items will point to this entry)
	entry_folder_back = new ArchiveEntry();
	entry_folder_back->setType(EntryType::folderType());
	entry_folder_back->setState(0);
	entry_folder_back->setName("..");
}

/* ZipArchiveEntryList::~ZipArchiveEntryList
 * ZipArchiveEntryList class destructor
 *******************************************************************/
ZipArchiveEntryList::~ZipArchiveEntryList() {
}

/* ZipArchiveEntryList::setArchive
 * Sets the archive for this widget to handle (can be NULL for no
 * archive)
 *******************************************************************/
void ZipArchiveEntryList::setArchive(Archive* archive) {
	// Stop listening to current archive (if any)
	if (this->archive)
		stopListening(this->archive);

	// Set archive (allow null)
	this->archive = archive;

	// Init new archive if given
	if (archive) {
		// Check that the archive is a zip
		if (archive->getType() != ARCHIVE_ZIP) {
			wxLogMessage("Can't open a non-zip archive with ZipArchiveEntryList");
			this->archive = NULL;
			current_dir = NULL;
			return;
		}

		// Listen to it
		listenTo(archive);

		// Open root directory
		current_dir = ((ZipArchive*)archive)->getRootDirectory();
		updateList();
	}
}

/* ZipArchiveEntryList::updateList
 * Updates + refreshes the list
 *******************************************************************/
void ZipArchiveEntryList::updateList(bool clear) {
	// If no current directory, set size to 0
	if (!current_dir) {
		SetItemCount(0);
		Refresh();
		return;
	}

	// Clear all items if needed
	if (clear)
		DeleteAllItems();

	// Determine if we need a 'back folder' entry
	int back_folder = 0;
	if (current_dir->parent_dir)
		back_folder = 1;

	// Update list
	if (filter_active)
		SetItemCount(filter.size());
	else
		SetItemCount(current_dir->numEntries() + current_dir->numSubDirs() + back_folder);
	Refresh();
}

/* ZipArchiveEntryList::filterList
 * Filters the list to only entries with names matching [filter]
 *******************************************************************/
void ZipArchiveEntryList::filterList(string filter) {
	if (filter.IsEmpty()) {
		filter_active = false;
		updateList();
		return;
	}
	else
		filter_active = true;

	// Clear current filter list
	this->filter.clear();

	// Add * to filter string
	filter += "*";

	// Convert filter to lowercase (to avoid case-sensitivity)
	filter = filter.Lower();

	// Go through entries
	filter_active = false;
	unsigned index = 0;
	ArchiveEntry* entry = getEntry(index);
	while (entry) {
		// Check for name match with filter
		if (entry == entry_folder_back || entry->getName().Lower().Matches(filter))
			this->filter.push_back(index);

		// Go to next unfiltered entry
		entry = getEntry(++index);
	}
	filter_active = true;

	// Update the list
	updateList();
}

/* ZipArchiveEntryList::entriesBegin
 * Returns the index of the first list item that is an entry (rather
 * than a directory), or -1 if no directory/archive is open)
 *******************************************************************/
int ZipArchiveEntryList::entriesBegin() {
	// Check directory is open
	if (!current_dir)
		return -1;

	// Determine first entry index
	int index = 0;
	if (current_dir->parent_dir)		// Offset if '..' item exists
		index++;
	index += current_dir->numSubDirs();	// Offset by number of subdirs

	return index;
}

/* ZipArchiveEntryList::isFolder
 * Returns true if the list item at [index] is a folder
 *******************************************************************/
bool ZipArchiveEntryList::isFolder(int index) {
	// Return false if no directory is open
	if (!current_dir)
		return false;

	// Index modifier if 'up folder' entry exists
	if (current_dir->parent_dir)
		index--;

	return (index < (signed)current_dir->numSubDirs() && index >= 0);
}

/* ZipArchiveEntryList::isEntry
 * Returns true if the list item at [index] is an entry
 *******************************************************************/
bool ZipArchiveEntryList::isEntry(int index) {
	// Return false if no directory is open
	if (!current_dir)
		return false;

	// Index modifier if 'up folder' entry exists
	if (current_dir->parent_dir)
		index--;

	return ((unsigned)index >= current_dir->numSubDirs() && (unsigned)index < current_dir->numSubDirs() + current_dir->numEntries());
}

/* ZipArchiveEntryList::getEntry
 * Returns the ArchiveEntry associated with the list item at [index].
 * Returns NULL if the index is out of bounds or no archive is open
 *******************************************************************/
ArchiveEntry* ZipArchiveEntryList::getEntry(int index) const {
	// Check index & archive
	if (index < 0 || !archive)
		return NULL;

	// Check if filtering is active
	if (filter_active) {
		// If it is, modify index for filtered list
		if (index < 0 || index >= filter.size())
			return NULL;

		index = filter[index];
	}

	// Index modifier if 'up folder' entry exists
	if (current_dir->parent_dir) {
		if (index == 0)
			return entry_folder_back;
		else
			index--;
	}

	// Subdirectories
	int subdirs = current_dir->numSubDirs();
	if (index < subdirs)
		return current_dir->subdirectories[index]->entry;

	// Entries
	if ((unsigned)index < subdirs + current_dir->numEntries())
		return current_dir->entries[index - subdirs];

	// Out of bounds
	return NULL;
}

/* ZipArchiveEntryList::getSelectedDirectories
 * Returns a vector of all currently selected directories
 *******************************************************************/
vector<zipdir_t*> ZipArchiveEntryList::getSelectedDirectories() {
	vector<zipdir_t*> ret;

	// Get all selected items
	vector<ArchiveEntry*> selection = getSelectedEntries();

	// Go through the selection
	for (size_t a = 0; a < selection.size(); a++) {
		ArchiveEntry* entry = selection[a];

		// If the selected entry is the 'back folder', ignore it
		if (entry == entry_folder_back)
			continue;
		else if (entry->getType() == EntryType::folderType()) {
			// If the entry is a folder type, get it's zipdir_t counterpart
			zipdir_t* dir = ((ZipArchive*)archive)->getDirectory(entry->getName(), current_dir);

			// Add it to the return list
			if (dir)
				ret.push_back(dir);
		}
	}

	return ret;
}

/* ZipArchiveEntryList::getEntryListIndex
 * Returns the list index of the entry at [index] in the current
 * directory
 *******************************************************************/
int ZipArchiveEntryList::getEntryListIndex(int index) {
	// Check the index and current directory is ok
	if (!current_dir || index < 0 || (unsigned)index >= current_dir->numEntries())
		return -1;

	// Determine where the entry indices start
	int start = 0;
	if (current_dir->parent_dir)
		start++;
	start += current_dir->numSubDirs();

	return start + index;
}

/* ZipArchiveEntryList::getDirListIndex
 * Returns the list index of the subdirectory at [subdir] in the
 * currentdirectory
 *******************************************************************/
int ZipArchiveEntryList::getDirListIndex(int subdir) {
	// Check the index and current directory is ok
	if (!current_dir || subdir < 0 || (unsigned)subdir >= current_dir->numSubDirs())
		return -1;

	// Only need to check for .. entry
	if (current_dir->parent_dir)
		return subdir + 1;
	else
		return subdir;
}

/* ArchiveEntryList::OnGetItemText
 * Called when the widget requests the text for [item] at [column]
 *******************************************************************/
string ZipArchiveEntryList::OnGetItemText(long item, long column) const {
	// Get entry
	ArchiveEntry* entry = getEntry(item);

	// Check entry
	if (!entry)
		return "INVALID INDEX";

	// Determine what column we want
	int col = columnType(column);

	if (col == AEL_COLUMN_NAME)
		return entry->getName();	// Name column
	else if (col == AEL_COLUMN_SIZE) {
		// Size column

		if (entry->getType() == EntryType::folderType()) {
			// Entry is a folder, return the number of entries+subdirectories in it
			zipdir_t* dir = NULL;

			// Get selected directory
			if (entry == entry_folder_back)
				dir = current_dir->parent_dir;	// If it's the 'back directory', get the current dir's parent
			else
				dir = dir = ((ZipArchive*)archive)->getDirectory(entry->getName(), current_dir);

			// If it's null, return error
			if (!dir)
				return "INVALID DIRECTORY";

			// Return the number of items in the directory
			return s_fmt("%d entries", dir->numEntries() + dir->numSubDirs());
		}
		else
			return entry->getSizeString();	// Not a folder, just return the normal size string
	}
	else if (col == AEL_COLUMN_TYPE)
		return entry->getTypeString();	// Type column
	else
		return "INVALID COLUMN";		// Invalid column
}

/* ZipArchiveEntryList::onAnnouncement
 * Called when an announcement is recieved from the archive being
 * managed
 *******************************************************************/
void ZipArchiveEntryList::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	event_data.seek(0, SEEK_SET);

	// Archive entries were swapped
	if (announcer == archive && event_name == "entries_swapped") {
		int i1, i2;
		wxUIntPtr e1, e2;

		// Check all event data is there
		if (!event_data.read(&i1, sizeof(int)) || !event_data.read(&i2, sizeof(int)) ||
			!event_data.read(&e1, sizeof(wxUIntPtr)) || !event_data.read(&e2, sizeof(wxUIntPtr)))
			return;

		// Get pointers to entries
		ArchiveEntry* entry1 = (ArchiveEntry*)wxUIntToPtr(e1), * entry2 = (ArchiveEntry*)wxUIntToPtr(e2);

		// Check they exist (should)
		if (!entry1 || !entry2)
			return;

		// Check that they are both in the current directory
		if (!current_dir->entryExists(entry1) || !current_dir->entryExists(entry2))
			return;	// Not in this directory, ignore

		// Swap selection
		int index1 = getEntryListIndex(i1);
		int index2 = getEntryListIndex(i2);
		long state1 = GetItemState(index1, wxLIST_STATE_FOCUSED|wxLIST_STATE_SELECTED);
		long state2 = GetItemState(index2, wxLIST_STATE_FOCUSED|wxLIST_STATE_SELECTED);
		SetItemState(index1, state2, wxLIST_STATE_FOCUSED|wxLIST_STATE_SELECTED);
		SetItemState(index2, state1, wxLIST_STATE_FOCUSED|wxLIST_STATE_SELECTED);

		// Update affected items
		RefreshItem(index1);
		RefreshItem(index2);
	}

	// An entry was added to the archive
	if (announcer == archive && event_name == "entry_added") {
		uint32_t index = 0;
		wxUIntPtr e = 0;

		// Check all event data is there
		if (!event_data.read(&index, sizeof(uint32_t)) || !event_data.read(&e, sizeof(wxUIntPtr)))
			return;

		// Get pointer to the entry
		ArchiveEntry* entry = (ArchiveEntry*)wxUIntToPtr(e);

		// Get the entry's directory
		zipdir_t* dir = ((ZipArchive*)archive)->getEntryDirectory(entry);

		// If the directory is the current, a direct subdirectory or the parent, update the list
		if (dir == current_dir || dir == current_dir->parent_dir || current_dir->dirIndex(dir) > 0)
			updateList();
	}

	// An entry in the archive was modified
	if (announcer == archive && event_name == "entry_modified") {
		uint32_t index = 0;
		wxUIntPtr e = 0;

		// Check all event data is there
		if (!event_data.read(&index, sizeof(uint32_t)) || !event_data.read(&e, sizeof(wxUIntPtr)))
			return;

		// Get pointer to the entry
		ArchiveEntry* entry = (ArchiveEntry*)wxUIntToPtr(e);

		// Get the entry's directory
		zipdir_t* dir = ((ZipArchive*)archive)->getEntryDirectory(entry);

		// If the directory is the current dir, update the list
		if (dir == current_dir)
			updateList();
	}

	// An entry in the archive is about to be removed
	if (announcer == archive && event_name == "entry_removing") {
		uint32_t index = 0;
		wxUIntPtr e = 0;

		// Check all event data is there
		if (!event_data.read(&index, sizeof(uint32_t)) || !event_data.read(&e, sizeof(wxUIntPtr)))
			return;

		// Get pointer to the entry
		ArchiveEntry* entry = (ArchiveEntry*)wxUIntToPtr(e);

		// Get the entry's directory
		zipdir_t* dir = ((ZipArchive*)archive)->getEntryDirectory(entry);

		// If the directory is the current dir, deselect it
		if (dir == current_dir)
			SetItemState(getEntryListIndex(index), 0x0000, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
	}

	// An entry in the archive was removed
	if (announcer == archive && event_name == "entry_removed") {
		// Update the list (we can't check if the entry is in the current folder as it's already been deleted
		updateList();
	}

	// A directory was added to the archive
	if (announcer == archive && event_name == "directory_added") {
		// Update the list
		updateList();
	}

	// A directory was removed from the archive
	if (announcer == archive && event_name == "directory_removed") {
		// Update the list
		updateList();
	}

	// A directory was renamed in the archive
	if (announcer == archive && event_name == "directory_modified") {
		// Update the list
		updateList();
	}
}


/*******************************************************************
 * ZIPARCHIVEENTRYLIST EVENTS
 *******************************************************************/

/* ZipArchiveEntryList::onListItemActivated
 * Called when a list item is 'activated' (double-click or enter)
 *******************************************************************/
void ZipArchiveEntryList::onListItemActivated(wxListEvent& e) {
	// Get item entry
	ArchiveEntry* entry = getEntry(e.GetIndex());

	// Do nothing if NULL (shouldn't be)
	if (!entry)
		return;

	// If it's a folder, open it
	if (entry->getType() == EntryType::folderType()) {
		// Get directory to open
		zipdir_t* dir = NULL;
		if (entry == entry_folder_back)
			dir = current_dir->parent_dir;	// 'Back directory' entry, open current dir's parent
		else
			dir = ((ZipArchive*)archive)->getDirectory(entry->getName(), current_dir);

		// Check it exists (really should)
		if (!dir) {
			wxLogMessage("Error: Trying to open nonexistant directory");
			return;
		}

		// Set current dir
		current_dir = dir;

		// Clear current selection
		clearSelection();

		// Update filter
		if (text_filter)
			filterList(text_filter->GetValue());

		// Update list
		updateList();
	}
}
