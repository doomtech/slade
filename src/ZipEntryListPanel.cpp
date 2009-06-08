
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ZipEntryListPanel.cpp
 * Description: ZipEntryListPanel class, same as EntryListPanel,
 *              except handles zip stuff (folders etc)
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
#include "ZipEntryListPanel.h"
#include "ArchiveEntry.h"
#include <wx/filename.h>


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
extern wxColor col_new;
extern wxColor col_modified;


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

	// Init beginning index of entries
	entries_begin = 0;
}

/* ZipEntryListPanel::~ZipEntryListPanel
 * ZipEntryListPanel class destructor
 *******************************************************************/
ZipEntryListPanel::~ZipEntryListPanel() {
	delete dummy_folder_entry;
}

void ZipEntryListPanel::updateDirectoryEntry(int index) {
	// Check the given index is a directory
	if (index >= entriesBegin())
		return;

	// Update the entry
	entry_list->updateEntry(index, true);

	// Get the associated zipdir
	zipdir_t* dir = getCurrentDir()->parent_dir;
	if (dir && index > 0)
		dir = getCurrentDir()->subdirectories[index - 1];
	else if (!dir)
		dir = getCurrentDir()->subdirectories[index];

	// Set it's size column manually
	wxListItem li;
	li.SetId(index);
	li.SetColumn(1);
	li.SetText(s_fmt(_T("%d Entries"), dir->numEntries(false) + dir->numSubDirs(false)));
	entry_list->SetItem(li);
}

vector<zipdir_t*> ZipEntryListPanel::getSelectedDirectories() {
	vector<zipdir_t*> ret;
	vector<int> selection = getSelection();

	for (size_t a = 0; a < selection.size(); a++) {
		int sel_index = selection[a];
		if (sel_index < entriesBegin()) {
			if (getCurrentDir()->parent_dir)
				sel_index--;

			ret.push_back(getCurrentDir()->subdirectories[sel_index]);
		}
	}

	return ret;
}

int ZipEntryListPanel::entriesBegin() {
	int begin = 0;

	if (getCurrentDir()->parent_dir)
		begin++;

	return begin + (int)getCurrentDir()->subdirectories.size();
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
		li.SetData(subdir->entry);

		// Add it to the list
		entry_list->InsertItem(li);
		//entry_list->updateEntry(index);
		updateDirectoryEntry(index);

		/*
		// Set size manually
		li.SetColumn(1);
		li.SetText(s_fmt(_T("%d Entries"), subdir->numEntries(false) + subdir->numSubDirs(false)));
		entry_list->SetItem(li);
		 */

		index++;
	}

	// Go through and add all entries in the current directory
	entries_begin = index;
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
			entry_list->SetItemTextColour(index, col_new);
		else if (entry->getState() == 1)
			entry_list->SetItemTextColour(index, col_modified);

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
		updateDirectoryEntry(0);

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

/* ZipEntryListPanel::swapItems
 * Swaps two list items (including their focused/selected states)
 * Override to work correctly with zip directories (as the entry list
 * won't be a 1-1 match index-wise with the archive)
 *******************************************************************/
bool ZipEntryListPanel::swapItems(int item1, int item2, ArchiveEntry* e1, ArchiveEntry* e2) {
	// Check that both entries are in the current directory
	// (shouldn't ever really happen that two entries in different directories are swapped)
	zipdir_t* dir = (zipdir_t*)cur_directory;
	if (!dir->entryExists(e1) || !dir->entryExists(e2))
		return false;

	// Do default entry swap, keeping in mind the offset to the beginning of the entries
	return EntryListPanel::swapItems(entriesBegin() + item1, entriesBegin() + item2, e1, e2);
}

/* ZipEntryListPanel::addEntry
 * Called when an entry is added to the archive. If the added entry
 * is in the current directory it is added to the list. If the added
 * entry is in a *new* subdirectory of the current directory then
 * a new subdirectory entry is added, otherwise it does nothing
 *******************************************************************/
bool ZipEntryListPanel::addEntry(DWORD archive_index, ArchiveEntry* e) {
	// Get the current directory
	zipdir_t* cdir = (zipdir_t*)cur_directory;

	// Check if the added entry was in this directory or any of it's subdirectories
	if (!cdir->entryExists(e, true))
		return true;

	// Get the directory the entry was added to
	zipdir_t* dir = ((ZipArchive*)archive)->getEntryDirectory(e, cdir);

	// If that directory is the current just add the entry to the list at the right index
	if (dir == cdir) {
		// Setup new entry
		wxListItem li;
		li.SetId(entriesBegin() + archive_index);
		li.SetData(e);

		// Add it to the list
		entry_list->InsertItem(li);
		entry_list->updateEntry(entriesBegin() + archive_index);

		return true;
	}

	// Otherwise, update that subdirectory's list item to refresh the number of entries within it
	for (size_t a = 0; a < cdir->subdirectories.size(); a++) {
		if (cdir->subdirectories[a]->entryExists(e, true))
		{
			if (cdir->parent_dir)
				updateDirectoryEntry(a-1);
			else
				updateDirectoryEntry(a);
			break;
		}
	}

	return true;
}

/* ZipEntryListPanel::updateEntry
 * Updates an entry in the list, if it is in the current directory
 *******************************************************************/
bool ZipEntryListPanel::updateEntry(DWORD archive_index, ArchiveEntry* e) {
	// Get the current directory
	zipdir_t* cdir = (zipdir_t*)cur_directory;

	// Check if the updated entry was in this directory
	if (!cdir->entryExists(e, false))
		return true;

	// Update the entry
	entry_list->updateEntry(entriesBegin() + archive_index, true);

	return true;
}

/* ZipEntryListPanel::removeEntry
 * Removes an entry from the list if it is in the current directory
 *******************************************************************/
bool ZipEntryListPanel::removeEntry(DWORD archive_index, ArchiveEntry* e) {
	// Get the current directory
	zipdir_t* cdir = (zipdir_t*)cur_directory;

	// Check if the removed entry was in this directory
	if (!cdir->entryExists(e, false))
		return true;

	// If it was, remove it from the list
	entry_list->DeleteItem(entriesBegin() + archive_index);

	return true;
}

/* ZipEntryListPanel::addDirectory
 * Adds a subdirectory to the list if it is a direct subdirectory of
 * the current directory
 *******************************************************************/
bool ZipEntryListPanel::addDirectory(wxUIntPtr zipdir_ptr) {
	// Get added directory
	zipdir_t* dir = (zipdir_t*)wxUIntToPtr(zipdir_ptr);

	// Get current directory
	zipdir_t* cdir = (zipdir_t*)cur_directory;

	// Check directories
	if (!cdir || !dir)
		return false;

	// Check if the added directory was a direct subdirectory of the current directory
	for (size_t a = 0; a < cdir->subdirectories.size(); a++) {
		if (cdir->subdirectories[a] == dir) {
			// It is a direct subdirectory, so add it the the entry list

			// Setup subdirectory item
			wxListItem li;
			li.SetId(entriesBegin());
			li.SetData(dir->entry);

			// Add it to the list
			entry_list->InsertItem(li);
			entry_list->updateEntry(entriesBegin());

			// Set name manually
			li.SetText(dir->getName());
			entry_list->SetItem(li);

			// Set size manually
			li.SetColumn(1);
			li.SetText(s_fmt(_T("%d Entries"), dir->numEntries(false) + dir->numSubDirs(false)));
			entry_list->SetItem(li);

			return true;
		}
	}

	return true;
}

bool ZipEntryListPanel::removeDirectory(wxUIntPtr zipdir_ptr) {
	// Get added directory
	zipdir_t* dir = (zipdir_t*)wxUIntToPtr(zipdir_ptr);

	// If the directory that was removed was this one, go to root directory
	if (cur_directory == dir) {
		cur_directory = ((ZipArchive*)archive)->getRootDirectory();
		populateEntryList();
		return true;
	}

	// Get current directory
	zipdir_t* cdir = (zipdir_t*)cur_directory;

	// If the directory that was removed was a subdirectory of the current
	// directory, remove it from the list
	int index = cdir->dirIndex(dir);
	if (index >= 0) {
		// If the current directory has a parent directory, increment index
		// to skip the '..' list item
		if (cdir->parent_dir)
			index++;

		// Remove the directory from the list
		entry_list->DeleteItem(index);
	}

	return true;
}

bool ZipEntryListPanel::renameDirectory(wxUIntPtr zipdir_ptr) {
	// Get the renamed directory
	zipdir_t* dir = (zipdir_t*)wxUIntToPtr(zipdir_ptr);

	// If the renamed directory was the current, do nothing
	if (cur_directory == dir)
		return true;

	// Get current directory
	zipdir_t* cdir = (zipdir_t*)cur_directory;

	// If the directory that was renamed is a subdirectory of the current
	// directory, update it in the list
	int index = cdir->dirIndex(dir);
	if (index >= 0) {
		// If the current directory has a parent directory, increment index
		// to skip the '..' list item
		if (cdir->parent_dir)
			index++;

		// Update the directory in the list
		updateDirectoryEntry(index);
	}

	return true;
}

/* ZipEntryListPanel::moveUp
 * Override of EntryListPanel::moveUp to take subdirectories into
 * account (can't move them)
 *******************************************************************/
bool ZipEntryListPanel::moveUp() {
	// Get selection
	vector<int> selection = getSelection();

	// If nothing is selected, do nothing
	if (selection.size() == 0)
		return false;

	// If the first selected item is at the top of the list
	// then don't move anything up
	if (selection[0] <= entriesBegin())
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

/* ZipEntryListPanel::moveDown
 * Override of EntryListPanel::moveDown to take subdirectories into
 * account (can't move them)
 *******************************************************************/
bool ZipEntryListPanel::moveDown() {
	// Get selection
	vector<int> selection = getSelection();

	// If nothing is selected, do nothing
	if (selection.size() == 0)
		return false;

	// If the last selected item is at the end of the list
	// then don't move anything down
	if (selection.back() == entry_list->GetItemCount()-1 || selection[0] < entriesBegin())
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
			cur_directory = getCurrentDir()->getSubDir(event.GetText());
		}
		else {
			// '..' item, go up 1 directory
			if (((zipdir_t*)cur_directory)->parent_dir)
				cur_directory = getCurrentDir()->parent_dir;
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

/* ZipEntryListPanel::onEntryListEditLabel
 * Called when an 'edit label' operation finishes on a list entry
 *******************************************************************/
void ZipEntryListPanel::onEntryListEditLabel(wxListEvent& event) {
	// TODO: Check if renamed entry is a folder and rename it in the archive accordingly
	ArchiveEntry* entry = getFocusedEntry();
	if (entry && !event.IsEditCancelled()) {
		archive->renameEntry(entry, event.GetLabel());
	}
}
