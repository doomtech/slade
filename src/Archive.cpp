
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Archive.cpp
 * Description: Archive, the 'base' archive class (Abstract)
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
#include "Archive.h"
#include <wx/filename.h>

/* Archive Directory Layout:
 * ---------------------
 * [root entries]
 * [subdir 1]
 * [subdir 1 entries]
 * [subdir 1/subdir 1.1]
 * [subdir 1/subdir 1.1 entries]
 * [subdir 1/subdir 1.2]
 * [subdir 1/subdir 1.2 entries]
 * [subdir 2] (has no entries)
 * [subdir 2/subdir 2.1]
 * [subdir 2/subdir 2.1 entries]
 * etc...
 */


 /*******************************************************************
 * VARIABLES
 *******************************************************************/
 CVAR(Bool, archive_load_data, false, CVAR_SAVE)


/*******************************************************************
 * ARCHIVETREENODE CLASS FUNCTIONS
 *******************************************************************/

/* ArchiveTreeNode::ArchiveTreeNode
 * ArchiveTreeNode class constructor
 *******************************************************************/
ArchiveTreeNode::ArchiveTreeNode(ArchiveTreeNode* parent) : STreeNode(parent) {
	// Init dir entry
	dir_entry = new ArchiveEntry();
	dir_entry->type = EntryType::folderType();
	dir_entry->parent = parent;

	// Init variables
	archive = NULL;
}

/* ArchiveTreeNode::~ArchiveTreeNode
 * ArchiveTreeNode class destructor
 *******************************************************************/
ArchiveTreeNode::~ArchiveTreeNode() {
	// Delete dir entry
	delete dir_entry;

	// Delete entries
	for (unsigned a = 0; a < entries.size(); a++)
		delete entries[a];
}

void ArchiveTreeNode::addChild(STreeNode* child) {
	// Do default child add
	STreeNode::addChild(child);

	// The child node's dir_entry should have this as parent
	((ArchiveTreeNode*)child)->dir_entry->parent = this;
}

/* ArchiveTreeNode::getArchive
 * Returns the parent archive of this node (gets the parent archive
 * of the *root* parent of this node)
 *******************************************************************/
Archive* ArchiveTreeNode::getArchive() {
	if (parent)
		return ((ArchiveTreeNode*)parent)->getArchive();
	else
		return archive;
}

/* ArchiveTreeNode::getName
 * Returns the node (directory) name
 *******************************************************************/
string ArchiveTreeNode::getName() {
	// Check dir entry exists
	if (!dir_entry)
		return "ERROR";

	return dir_entry->getName();
}

/* ArchiveTreeNode::entryIndex
 * Returns the index of [entry] within this directory, or -1 if
 * the entry doesn't exist
 *******************************************************************/
int ArchiveTreeNode::entryIndex(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return -1;

	// Search for it
	for (unsigned a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return (int)a;
	}

	// Not found
	return -1;
}

/* ArchiveTreeNode::getEntry
 * Returns the entry at [index] in this directory, or NULL if [index]
 * is out of bounds
 *******************************************************************/
ArchiveEntry* ArchiveTreeNode::getEntry(unsigned index) {
	// Check index
	if (index >= entries.size())
		return NULL;

	return entries[index];
}

/* ArchiveTreeNode::getEntry
 * Returns the entry matching [name] in this directory, or NULL if
 * no entries match
 *******************************************************************/
ArchiveEntry* ArchiveTreeNode::getEntry(string name, bool cut_ext) {
	// Check name was given
	if (name == "")
		return NULL;

	// Go through entries
	for (unsigned a = 0; a < entries.size(); a++) {
		// Check for (non-case-sensitive) name match
		if (s_cmpnocase(entries[a]->getName(cut_ext), name))
			return entries[a];
	}

	// Not found
	return NULL;
}

/* ArchiveTreeNode::numEntries
 * Returns the number of entries in this directory
 *******************************************************************/
unsigned ArchiveTreeNode::numEntries(bool inc_subdirs) {
	if (!inc_subdirs)
		return entries.size();
	else {
		unsigned count = entries.size();
		for (unsigned a = 0; a < children.size(); a++)
			count += ((ArchiveTreeNode*)children[a])->numEntries(true);

		return count;
	}
}

/* ArchiveTreeNode::addEntry
 * Adds [entry] to this directory at [index], or at the end if
 * [index] is out of bounds
 *******************************************************************/
bool ArchiveTreeNode::addEntry(ArchiveEntry* entry, unsigned index) {
	// Check entry
	if (!entry)
		return false;

	// Check index
	if (index >= entries.size())
		entries.push_back(entry);	// Invalid index, add to end of list
	else
		entries.insert(entries.begin() + index, entry);

	// Set entry's parent to this node
	entry->parent = this;

	return true;
}

/* ArchiveTreeNode::removeEntry
 * Removes the entry at [index] in this directory (but doesn't delete
 * it). Returns false if [index] was out of bounds, true otherwise
 *******************************************************************/
bool ArchiveTreeNode::removeEntry(unsigned index) {
	// Check index
	if (index >= entries.size())
		return false;

	// De-parent entry
	entries[index]->parent = NULL;

	// Remove it from the entry list
	entries.erase(entries.begin() + index);

	return true;
}

/* ArchiveTreeNode::swapEntries
 * Swaps the entry at [index1] with the entry at [index2] within this
 * directory. Returns false if either index was invalid,
 * true otherwise
 *******************************************************************/
bool ArchiveTreeNode::swapEntries(unsigned index1, unsigned index2) {
	// Check indices
	if (index1 >= entries.size() || index2 >= entries.size())
		return false;

	// Do swap
	ArchiveEntry* temp = entries[index1];
	entries[index1] = entries[index2];
	entries[index2] = temp;

	return true;
}

/* ArchiveTreeNode::clone
 * Returns a clone of this node
 *******************************************************************/
ArchiveTreeNode* ArchiveTreeNode::clone() {
	// Create copy
	ArchiveTreeNode* copy = new ArchiveTreeNode();
	copy->setName(dir_entry->getName());

	// Copy entries
	for (unsigned a = 0; a < entries.size(); a++)
		copy->addEntry(new ArchiveEntry(*(entries[a])));

	// Copy subdirectories
	for (unsigned a = 0; a < children.size(); a++)
		copy->addChild(((ArchiveTreeNode*)children[a])->clone());

	return copy;
}

/* ArchiveTreeNode::merge
 * Merges [node] with this node. Entries within [node] are added
 * at [position] within this node. Returns false if [node] is invalid,
 * true otherwise
 *******************************************************************/
bool ArchiveTreeNode::merge(ArchiveTreeNode* node, unsigned position) {
	// Chech node was given to merge
	if (!node)
		return false;

	// Merge entries
	for (unsigned a = 0; a < node->numEntries(); a++) {
		addEntry(new ArchiveEntry(*(node->getEntry(a))), position);

		if (position < entries.size())
			position++;
	}

	// Merge subdirectories
	for (unsigned a = 0; a < node->nChildren(); a++) {
		ArchiveTreeNode* child = (ArchiveTreeNode*)STreeNode::addChild(node->getChild(a)->getName());
		child->merge((ArchiveTreeNode*)node->getChild(a));
	}

	return true;
}


/*******************************************************************
 * ARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* Archive::Archive
 * Archive class constructor
 *******************************************************************/
Archive::Archive(uint8_t type) {
	// Init variables
	this->type = type;
	modified = true;
	on_disk = false;
	parent = NULL;
	read_only = false;

	// Create root directory
	dir_root = new ArchiveTreeNode();
	dir_root->archive = this;
}

/* Archive::~Archive
 * Archive class destructor
 *******************************************************************/
Archive::~Archive() {
	if (dir_root)
		delete dir_root;
}

/* Archive::getFilename
 * Returns the archive's filename, including the path if specified
 *******************************************************************/
string Archive::getFilename(bool full) {
	// If the archive is within another archive, return "<parent archive>/<entry name>"
	if (parent) {
		string parent_archive = "";
		if (getParentArchive())
			parent_archive = getParentArchive()->getFilename(false) + "/";

		wxFileName fn(parent->getName());
		return parent_archive + fn.GetName();
	}

	if (full)
		return filename;
	else {
		// Get the filename without the path
		wxFileName fn(filename);
		return fn.GetFullName();
	}
}

/* Archive::setModified
 * Sets the Archive's modified status and announces the change
 *******************************************************************/
void Archive::setModified(bool modified) {
	// Set modified
	this->modified = modified;

	// Announce
	announce("modified");
}

/* Archive::checkEntry
 * Checks that the given entry is valid and part of this archive
 *******************************************************************/
bool Archive::checkEntry(ArchiveEntry* entry) {
	// Check entry is valid
	if (!entry)
		return false;

	// Check entry is part of this archive
	if (entry->getParent() != this)
		return false;

	// Entry is ok
	return true;
}

/* Archive::getEntry
 * Returns the entry matching [name] within [dir]. If no dir is given
 * the root dir is used
 *******************************************************************/
ArchiveEntry* Archive::getEntry(string name, bool cut_ext, ArchiveTreeNode* dir) {
	// Check if dir was given
	if (!dir)
		dir = dir_root;	// None given, use root

	return dir->getEntry(name, cut_ext);
}

/* Archive::getEntry
 * Returns the entry at [index] within [dir]. If no dir is given the
 * root dir is used. Returns NULL if [index] is out of bounds
 *******************************************************************/
ArchiveEntry* Archive::getEntry(unsigned index, ArchiveTreeNode* dir) {
	// Check if dir was given
	if (!dir)
		dir = dir_root;	// None given, use root

	return dir->getEntry(index);
}

/* Archive::entryIndex
 * Returns the index of [entry] within [dir]. IF no dir is given the
 * root dir is used. Returns -1 if [entry] doesn't exist within [dir]
 *******************************************************************/
int	Archive::entryIndex(ArchiveEntry* entry, ArchiveTreeNode* dir) {
	// Check if dir was given
	if (!dir)
		dir = dir_root;	// None given, use root

	return dir->entryIndex(entry);
}

ArchiveEntry* Archive::entryAtPath(string path) {
	// Get path as wxFileName for processing
	wxFileName fn(path);

	// Get directory from path
	ArchiveTreeNode* dir = getDir(fn.GetPath(true));

	// If dir doesn't exist, return null
	if (!dir)
		return NULL;

	// Return entry
	return dir->getEntry(fn.GetFullName());
}

/* Archive::save
 * This is the general, all-purpose 'save archive' function. Takes
 * into account whether the archive is contained within another,
 * is already on the disk, etc etc. Does a 'save as' if [filename]
 * is specified, unless the archive is contained within another.
 * Returns false if saving was unsuccessful, true otherwise
 *******************************************************************/
bool Archive::save(string filename) {
	bool success = false;

	// If the archive has a parent ArchiveEntry, just write it to that
	if (parent) {
		success = write(parent->getMCData());
		parent->setState(1);
	}
	else {
		// Otherwise, file stuff
		if (!filename.IsEmpty()) {
			// New filename is given (ie 'save as'), write to new file and change archive filename accordingly
			success = write(filename);
			if (success) this->filename = filename;

			// Update variables
			this->on_disk = true;
		}
		else if (!this->filename.IsEmpty()) {
			// No filename is given, but the archive has a filename, so overwrite it (and make a backup)

			// Create backup
			if (wxFileName::FileExists(this->filename)) {
				string bakfile = this->filename + ".bak";

				// Remove old backup file
				wxRemoveFile(bakfile);

				// Copy current file contents to new backup file
				wxLogMessage("Creating backup %s", bakfile.c_str());
				wxCopyFile(this->filename, bakfile);
			}

			// Write it to the file
			success = write(this->filename);

			// Update variables
			this->on_disk = true;
		}
	}

	// If saving was successful, update variables and announce save
	if (success) {
		setModified(false);
		announce("saved");
	}

	return success;
}

/* Archive::numEntries
 * Returns the total number of entries in the archive
 *******************************************************************/
unsigned Archive::numEntries() {
	return dir_root->numEntries(true);
}

/* Archive::close
 * 'Closes' the archive
 *******************************************************************/
void Archive::close() {
	// Announce
	announce("closing");

	// Delete root directory
	delete dir_root;

	// Recreate root directory
	dir_root = new ArchiveTreeNode();
	dir_root->archive = this;

	// Unlock parent entry if it exists
	if (parent)
		parent->unlock();

	// Announce
	announce("closed");
}

/* Archive::entryStateChanged
 * Updates the archive variables and announces if necessary that an
 * entry's state has changed
 *******************************************************************/
void Archive::entryStateChanged(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return;

	// Get the entry index and announce the change
	if (!isMuted()) {
		MemChunk mc;
		mc.reSize(8, false);
		wxUIntPtr ptr = wxPtrToUInt(entry);
		uint32_t index = entryIndex(entry);
		mc.write(&index, sizeof(uint32_t));
		mc.write(&ptr, sizeof(wxUIntPtr));
		announce("entry_state_changed", mc);
	}

	// If entry was set to unmodified, don't set the archive to modified
	if (entry->getState() == 0)
		return;

	// Set the archive state to modified
	setModified(true);
}

/* Archive::getEntryTreeAsList
 * Adds the directory structure starting from [start] to [list]
 *******************************************************************/
void Archive::getEntryTreeAsList(vector<ArchiveEntry*>& list, ArchiveTreeNode* start) {
	// If no start dir is specified, use the root dir
	if (!start)
		start = dir_root;

	// Add the directory entry to the list if it isn't the root dir
	if (start != dir_root)
		list.push_back(start->dir_entry);

	// Add all entries to the list
	for (unsigned a = 0; a < start->numEntries(); a++)
		list.push_back(start->getEntry(a));

	// Go through subdirectories and add them to the list
	for (unsigned a = 0; a < start->nChildren(); a++)
		getEntryTreeAsList(list, (ArchiveTreeNode*)start->getChild(a));
}

/* Archive::paste
 * 'Pastes' the [tree] into the archive, with its root entries
 * starting at [position] in [base] directory. If [base] is null,
 * the root directory is used
 *******************************************************************/
bool Archive::paste(ArchiveTreeNode* tree, unsigned position, ArchiveTreeNode* base) {
	// Check tree was given to paste
	if (!tree)
		return false;

	// Paste to root dir if none specified
	if (!base)
		base = getRoot();

	// Just do a merge
	return base->merge(tree, position);
}

/* Archive::getDir
 * Gets the directory matching [path], starting from [base]. If
 * [base] is NULL, the root directory is used. Returns NULL if
 * the requested directory does not exist
 *******************************************************************/
ArchiveTreeNode* Archive::getDir(string path, ArchiveTreeNode* base) {
	// Check if base dir was given
	if (!base)
		base = dir_root;	// None given, use root

	return (ArchiveTreeNode*)base->getChild(path);
}

/* Archive::createDir
 * Creates a directory at [path], starting from [base]. If
 * [base] is NULL, the root directory is used. Returns the created
 * directory. If the directory requested to be created already
 * exists, it will be returned
 *******************************************************************/
ArchiveTreeNode* Archive::createDir(string path, ArchiveTreeNode* base) {
	// Abort if read only
	if (read_only)
		return dir_root;

	// If no base dir specified, set it to root
	if (!base)
		base = dir_root;

	if (path.IsEmpty())
		return base;

	// Create the directory
	ArchiveTreeNode* dir = (ArchiveTreeNode*)((STreeNode*)base)->addChild(path);

	// Set the archive state to modified
	setModified(true);

	// Announce
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(dir);
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce("directory_added", mc);

	return dir;
}

/* Archive::removeDir
 * Deletes the directory matching [path], starting from [base]. If
 * [base] is NULL, the root directory is used. Returns false if
 * the directory does not exist, true otherwise
 *******************************************************************/
bool Archive::removeDir(string path, ArchiveTreeNode* base) {
	// Abort if read only
	if (read_only)
		return false;

	// Get the dir to remove
	ArchiveTreeNode* dir = getDir(path, base);

	// Check it exists (and that it isn't the root dir)
	if (!dir || dir == getRoot())
		return false;

	// Remove the directory from its parent
	if (dir->getParent())
		dir->getParent()->removeChild(dir);

	// Delete the directory
	delete dir;

	// Set the archive state to modified
	setModified(true);

	return true;
}

/* Archive::renameDir
 * Renames [dir] to [new_name]. Returns false if [dir] isn't part of
 * the archive, true otherwise
 *******************************************************************/
bool Archive::renameDir(ArchiveTreeNode* dir, string new_name) {
	// Abort if read only
	if (read_only)
		return false;

	// Check the directory is part of this archive
	if (dir->getArchive() != this)
		return false;

	// Rename the directory if needed
	if (!(s_cmpnocase(dir->getName(), new_name))) {
		dir->setName(new_name);
		dir->getDirEntry()->setState(1);
	}
	else
		return true;

	// Announce
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(dir);
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce("directory_modified", mc);

	// Update variables etc
	setModified(true);

	return true;
}

/* Archive::addEntry
 * Adds [entry] to [dir] at [position]. If [dir] is NULL it is added
 * to the root dir. If [position] is out of bounds, it is added to
 * the end of the dir. If [copy] is true, a copy of [entry] is added
 * (rather than [entry] itself). Returns the added entry, or NULL if
 * [entry] is invalid or the archive is read-only
 *******************************************************************/
ArchiveEntry* Archive::addEntry(ArchiveEntry* entry, unsigned position, ArchiveTreeNode* dir, bool copy) {
	// Abort if read only
	if (read_only)
		return NULL;

	// Check valid entry
	if (!entry)
		return NULL;

	// If no dir given, set it to the root dir
	if (!dir)
		dir = dir_root;

	// Make a copy of the entry to add if needed
	if (copy)
		entry = new ArchiveEntry(*entry);

	// Add the entry
	dir->addEntry(entry, position);

	// Update variables etc
	setModified(true);
	entry->state = 2;

	// Announce
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(entry);
	mc.write(&position, sizeof(uint32_t));
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce("entry_added", mc);

	return entry;
}

/* Archive::addNewEntry
 * Creates a new entry with [name] and adds it to [dir] at [position].
 * If [dir] is NULL it is added to the root dir. If [position] is out
 * of bounds, it is added tothe end of the dir. Return false if the
 * entry is invalid, true otherwise
 *******************************************************************/
ArchiveEntry* Archive::addNewEntry(string name, unsigned position, ArchiveTreeNode* dir) {
	// Abort if read only
	if (read_only)
		return NULL;

	// Create the new entry
	ArchiveEntry* entry = new ArchiveEntry(name);

	// Add it to the archive
	addEntry(entry, position, dir);

	// Return the newly created entry
	return entry;
}

/* Archive::removeEntry
 * Removes [entry] from the archive. If [delete_entry] is true, the
 * entry will also be deleted. Returns true if the removal succeeded
 *******************************************************************/
bool Archive::removeEntry(ArchiveEntry* entry, bool delete_entry) {
	// Abort if read only
	if (read_only)
		return false;

	// Check entry
	if (!checkEntry(entry))
		return false;

	// Check if entry is locked
	if (entry->isLocked())
		return false;

	// Get it's directory
	ArchiveTreeNode* dir = entry->getParentDir();

	// Error if entry has no parent directory
	if (!dir)
		return false;

	// Get the entry index
	int index = dir->entryIndex(entry);

	// Announce (before actually removing in case entry is still needed)
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(entry);
	mc.write(&index, sizeof(int));
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce("entry_removing", mc);

	// Remove it from its directory
	bool ok = dir->removeEntry(index);

	// If it was removed ok
	if (ok) {
		// Announce removed
		announce("entry_removed", mc);

		// Delete if necessary
		if (delete_entry)
			delete entry;

		// Update variables etc
		setModified(true);
	}

	return ok;
}

/* Archive::swapEntries
 * Swaps [entry1] and [entry2]. Returns false if either entry is
 * invalid or if both entries are not in the same directory, true
 * otherwise
 *******************************************************************/
bool Archive::swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) {
	// Abort if read only
	if (read_only)
		return false;

	// Check both entries
	if (!checkEntry(entry1) || !checkEntry(entry2))
		return false;

	// Check neither entry is locked
	if (entry1->isLocked() || entry2->isLocked())
		return false;

	// Get their directory
	ArchiveTreeNode* dir = entry1->getParentDir();

	// Error if no dir
	if (!dir)
		return false;

	// Check they are both in the same directory
	if (entry2->getParentDir() != dir) {
		wxLogMessage("Error: Can't swap two entries in different directories");
		return false;
	}

	// Get entry indices
	int i1 = dir->entryIndex(entry1);
	int i2 = dir->entryIndex(entry2);

	// Check indices
	if (i1 < 0 || i2 < 0)
		return false;

	// Swap entries
	dir->swapEntries(i1, i2);

	// Announce the swap
	MemChunk mc(16);
	wxUIntPtr ptr1 = wxPtrToUInt(entry1);
	wxUIntPtr ptr2 = wxPtrToUInt(entry2);
	mc.write(&i1, sizeof(int));
	mc.write(&i2, sizeof(int));
	mc.write(&ptr1, sizeof(wxUIntPtr));
	mc.write(&ptr2, sizeof(wxUIntPtr));
	announce("entries_swapped", mc);

	// Return success
	return true;
}

/* Archive::moveEntry
 * Moves [entry] to [position] in [dir]. If [dir] is NULL, the root
 * dir is used. Returns false if the entry was invalid, true
 * otherwise
 *******************************************************************/
bool Archive::moveEntry(ArchiveEntry* entry, unsigned position, ArchiveTreeNode* dir) {
	// Abort if read only
	if (read_only)
		return false;

	// Check the entry
	if (!checkEntry(entry))
		return false;

	// Check if the entry is locked
	if (entry->isLocked())
		return false;

	// Get the entry's current directory
	ArchiveTreeNode* cdir = entry->getParentDir();

	// Error if no dir
	if (!cdir)
		return false;

	// If no destination dir specified, use root
	if (!dir)
		dir = dir_root;

	// Remove the entry from its current dir
	removeEntry(entry, false);

	// Add it to the destination dir
	addEntry(entry, position, dir);

	// Return success
	return true;
}

/* Archive::renameEntry
 * Renames [entry] with [name]. Returns false if the entry was
 * invalid, true otherwise
 *******************************************************************/
bool Archive::renameEntry(ArchiveEntry* entry, string name) {
	// Abort if read only
	if (read_only)
		return false;

	// Check entry
	if (!checkEntry(entry))
		return false;

	// Check if entry is locked
	if (entry->isLocked())
		return false;

	// Rename the entry
	entry->rename(name);

	// Announce modification
	entryStateChanged(entry);

	return true;
}

/* Archive::findFirst
 * Returns the first entry matching the search criteria in [options],
 * or NULL if no matching entry was found
 *******************************************************************/
ArchiveEntry* Archive::findFirst(search_options_t& options) {
	// Init search variables
	ArchiveTreeNode* dir = options.dir;
	if (!dir) dir = dir_root;

	// Begin search

	// Search entries
	for (unsigned a = 0; a < dir->numEntries(); a++) {
		ArchiveEntry* entry = dir->getEntry(a);

		// Check type
		if (options.match_type) {
			if (entry->getType() == EntryType::unknownType()) {
				if (!options.match_type->isThisType(entry))
					continue;
			}
			else if (options.match_type != entry->getType())
				continue;
		}

		// Check name
		if (!options.match_name.IsEmpty()) {
			wxFileName fn(entry->getName());
			string name;

			// Cut extension if ignoring
			if (options.ignore_ext)
				name = fn.GetName();
			else
				name = fn.GetFullName();

			// Force case insensitivity
			name.MakeLower();
			options.match_name.MakeLower();

			if (!options.match_name.Matches(name))
				continue;
		}

		// Check namespace
		if (!options.match_namespace.IsEmpty()) {
			if (!s_cmpnocase(detectNamespace(entry), options.match_namespace))
				continue;
		}

		// Entry passed all checks so far, so we found a match
		return entry;
	}

	// Search subdirectories (if needed)
	if (options.search_subdirs) {
		for (unsigned a = 0; a < dir->nChildren(); a++) {
			search_options_t opt = options;
			opt.dir = (ArchiveTreeNode*)dir->getChild(a);
			ArchiveEntry* match = findFirst(opt);

			// If a match was found in this subdir, return it
			if (match)
				return match;
		}
	}

	// No matches found
	return NULL;
}

/* Archive::findLast
 * Returns the last entry matching the search criteria in [options],
 * or NULL if no matching entry was found
 *******************************************************************/
ArchiveEntry* Archive::findLast(search_options_t& options) {
	// Init search variables
	ArchiveTreeNode* dir = options.dir;
	if (!dir) dir = dir_root;

	// Begin search

	// Search subdirectories (if needed) (bottom-up)
	if (options.search_subdirs) {
		for (int a = dir->nChildren() - 1; a >= 0; a--) {
			search_options_t opt = options;
			opt.dir = (ArchiveTreeNode*)dir->getChild(a);
			ArchiveEntry* match = findFirst(opt);

			// If a match was found in this subdir, return it
			if (match)
				return match;
		}
	}

	// Search entries (bottom-up)
	for (int a = dir->numEntries() - 1; a >= 0; a--) {
		ArchiveEntry* entry = dir->getEntry(a);

		// Check type
		if (options.match_type) {
			if (entry->getType() == EntryType::unknownType()) {
				if (!options.match_type->isThisType(entry))
					continue;
			}
			else if (options.match_type != entry->getType())
				continue;
		}

		// Check name
		if (!options.match_name.IsEmpty()) {
			wxFileName fn(entry->getName());
			string name;

			// Cut extension if ignoring
			if (options.ignore_ext)
				name = fn.GetName();
			else
				name = fn.GetFullName();

			// Force case insensitivity
			name.MakeLower();
			options.match_name.MakeLower();

			if (!options.match_name.Matches(name))
				continue;
		}

		// Check namespace
		if (!options.match_namespace.IsEmpty()) {
			if (!s_cmpnocase(detectNamespace(entry), options.match_namespace))
				continue;
		}

		// Entry passed all checks so far, so we found a match
		return entry;
	}

	// No matches found
	return NULL;
}

/* Archive::findAll
 * Returns a list of entries matching the search criteria in
 * [options]
 *******************************************************************/
vector<ArchiveEntry*> Archive::findAll(search_options_t& options) {
	// Init search variables
	ArchiveTreeNode* dir = options.dir;
	if (!dir) dir = dir_root;
	vector<ArchiveEntry*> ret;

	// Begin search

	// Search entries
	for (unsigned a = 0; a < dir->numEntries(); a++) {
		ArchiveEntry* entry = dir->getEntry(a);

		// Check type
		if (options.match_type) {
			if (entry->getType() == EntryType::unknownType()) {
				if (!options.match_type->isThisType(entry))
					continue;
			}
			else if (options.match_type != entry->getType())
				continue;
		}

		// Check name
		if (!options.match_name.IsEmpty()) {
			wxFileName fn(entry->getName());
			string name;

			// Cut extension if ignoring
			if (options.ignore_ext)
				name = fn.GetName();
			else
				name = fn.GetFullName();

			// Force case insensitivity
			name.MakeLower();
			options.match_name.MakeLower();

			if (!options.match_name.Matches(name))
				continue;
		}

		// Check namespace
		if (!options.match_namespace.IsEmpty()) {
			if (!s_cmpnocase(detectNamespace(entry), options.match_namespace))
				continue;
		}

		// Entry passed all checks so far, so we found a match
		ret.push_back(entry);
	}

	// Search subdirectories (if needed)
	if (options.search_subdirs) {
		for (unsigned a = 0; a < dir->nChildren(); a++) {
			search_options_t opt = options;
			opt.dir = (ArchiveTreeNode*)dir->getChild(a);

			// Add any matches to the list
			vector<ArchiveEntry*> vec = findAll(opt);
			ret.insert(ret.end(), vec.begin(), vec.end());
		}
	}

	// Return matches
	return ret;
}


/*******************************************************************
 * TREELESSARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* TreelessArchive::paste
 * Treeless version of Archive::paste. Pastes all entries in [tree]
 * and its subdirectories straight into the root dir at [position]
 *******************************************************************/
bool TreelessArchive::paste(ArchiveTreeNode* tree, unsigned position, ArchiveTreeNode* base) {
	// Check tree was given to paste
	if (!tree)
		return false;

	// Paste root entries only
	for (unsigned a = 0; a < tree->numEntries(); a++) {
		// Add entry to archive
		ArchiveEntry* entry = addEntry(tree->getEntry(a), position, NULL, true);

		// Update [position] if needed
		if (position < 0xFFFFFFFF)
			position++;
	}

	// Go through paste tree subdirs and paste their entries recursively
	for (unsigned a = 0; a < tree->nChildren(); a++) {
		ArchiveTreeNode* dir = (ArchiveTreeNode*)tree->getChild(a);
		paste(dir, position);
	}

	return true;
}
