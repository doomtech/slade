
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Archive.cpp
 * Description: Archive, the 'base' archive class.
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


/* Archive::Archive
 * Archive class constructor
 *******************************************************************/
Archive::Archive() {
	type = 0;
	modified = true;
}

/* Archive::~Archive
 * Archive class destructor
 *******************************************************************/
Archive::~Archive() {
}

/* Archive::entryIndex
 * Returns the given entry's index in the Archive entry list.
 * Returns -1 if the entry doesn't exist in the Archive.
 *******************************************************************/
int Archive::entryIndex(ArchiveEntry* entry) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return a;
	}

	return -1;
}

/* Archive::getFileName
 * Returns the archive's filename, including the path if specified
 *******************************************************************/
string Archive::getFileName(bool fullpath)
{
	if (fullpath)
		return filename;
	else {
		// Get the filename without the path
		wxFileName fn(filename);
		return fn.GetName() + _T(".") + fn.GetExt();
	}
}

/* Archive::getEntry
 * Returns the entry at the index specified,
 * or NULL if the index is invalid
 *******************************************************************/
ArchiveEntry* Archive::getEntry(DWORD index) {
	// Check index
	if (index >= entries.size())
		return NULL;

	return entries[index];
}

/* Archive::getEntry
 * Returns the first entry with the specified name,
 * or NULL if no lump exists with that name
 *******************************************************************/
ArchiveEntry* Archive::getEntry(string name) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a]->getName() == name)
			return entries[a];
	}

	return NULL;
}

/* Archive::openFile
 * Function to open a file, does nothing here, to be overridden by
 * any subclass of Archive
 *******************************************************************/
bool Archive::openFile(string filename) {
	Global::error = _T("Action not implemented for this Archive type");
	return false;
}

/* Archive::save
 * Function to save to a file, does nothing here, to be overridden by
 * any subclass of Archive
 *******************************************************************/
bool Archive::save(string filename) {
	Global::error = _T("Action not implemented for this Archive type");
	return false;
}

/* Archive::loadEntryData
 * Function to load in an entry's data, does nothing here,
 * to be overridden by any subclass of Archive
 *******************************************************************/
bool Archive::loadEntryData(ArchiveEntry* entry) {
	Global::error = _T("Action not implemented for this Archive type");
	return false;
}

/* Archive::numLumps
 * Returns the number of entries in the archive
 *******************************************************************/
DWORD Archive::numEntries() {
	return entries.size();
}

/* Archive::close
 * 'Closes' the archive
 *******************************************************************/
void Archive::close() {
	announce(_T("close"));
}

/* Archive::detectMaps
 * Function to detect any maps in the archive, does nothing here,
 * to be overridden by any subclass of Archive
 *******************************************************************/
vector<Archive::mapdesc_t> Archive::detectMaps() {
	vector<mapdesc_t> maps;
	return maps;
}

/* Archive::addNewEntry
 * Creates a new ArchiveEntry and adds it to the archive before the
 * position specified. Returns the created entry
 *******************************************************************/
ArchiveEntry* Archive::addNewEntry(string name, DWORD position) {
	// Check the position, if it's out of range, change it
	if (position < 0)
		position = 0;
	if (position >= entries.size())
		position = entries.size() - 1;

	// Create the new entry
	ArchiveEntry* new_entry = new ArchiveEntry(name);

	// Add it to the entry list
	entries.insert(entries.begin() + position, new_entry);

	// Update variables etc
	modified = true;
	MemChunk mc;
	mc.write(new_entry, sizeof(ArchiveEntry*));
	announce(_T("entry_added"), mc);

	// Return the newly created entry
	return new_entry;
}

/* Archive::addExistingEntry
 * Adds an existing ArchiveEntry to the archive before the position
 * specified. Returns the added archive entry
 *******************************************************************/
ArchiveEntry* Archive::addExistingEntry(ArchiveEntry* entry, DWORD position, bool copy) {
	// Check the given entry exists
	if (!entry)
		return false;

	// Check the position, if it's out of range, change it
	if (position < 0)
		position = 0;
	if (position >= entries.size())
		position = entries.size() - 1;

	// Make a copy of the entry to add if needed
	if (copy)
		entry = new ArchiveEntry(*entry);

	// Add the entry to the list
	entries.insert(entries.begin() + position, entry);

	// Update variables etc
	modified = true;
	MemChunk mc;
	mc.write(entry, sizeof(ArchiveEntry*));
	announce(_T("entry_added"), mc);

	// Return the added entry
	return entry;
}

/* Archive::swapEntries
 * Swaps the specified entries. Returns false if either entry is
 * invalid or not part of this Archive, true otherwise.
 *******************************************************************/
bool Archive::swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) {
	// Get entry indices
	int i1 = entryIndex(entry1);
	int i2 = entryIndex(entry2);

	// Check indices
	if (i1 == -1 || i2 == -1)
		return false;

	// Swap the entries in the list
	entries[i1] = entry2;
	entries[i2] = entry1;

	// Announce the swap
	MemChunk mc;
	mc.write(&i1, sizeof(int));
	mc.write(&i2, sizeof(int));
	announce(_T("entries_swapped"), mc);

	// Return success
	return true;
}
