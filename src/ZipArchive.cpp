
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ZipArchive.cpp
 * Description: ZipArchive, archive class to handle zip format
 *              archives
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
#include "ZipArchive.h"
#include <wx/log.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/ptr_scpd.h>
#include <wx/filename.h>
#include <algorithm>


/*******************************************************************
 * VARIABLES
 *******************************************************************/


/* Zip Directory Layout:
 * ---------------------
 * [root entries]
 * [subdir 1]
 * [subdir 1 entries]
 * [subdir 1/1]
 * [subdir 1/1 entries]
 * [subdir 1/2]
 * [subdir 1/2 entries]
 * [subdir 2] (has no entries)
 * [subdir 2/1]
 * [subdir 2/1 entries]
 * etc...
 */


ZipArchive::ZipArchive()
: Archive() {
	type = ARCHIVE_ZIP;
}

ZipArchive::~ZipArchive() {
}

// Return the entry index of the given entry within it's directory, not within the whole archive
int ZipArchive::entryIndex(ArchiveEntry* entry) {
	// Check valid entry
	if (!entry)
		return -1;

	// Check entry is part of this archive
	if (entry->getParent() != this)
		return -1;

	// Get the directory the entry is in
	zipdir_t* dir = getEntryDirectory(entry);

	// If it's in a directory return it's index within that directory
	if (dir)
		return dir->entryIndex(entry);

	// Otherwise return -1
	return -1;
}

// Get the entry with the given name AND directory (must match both)
ArchiveEntry* ZipArchive::getEntry(string name) {
	// Convert to wxFileName for processing
	wxFileName fn(name);

	// Get the directory
	zipdir_t* dir = getDirectory(fn.GetPath(true, wxPATH_UNIX));

	// If the directory doesn't exist return NULL
	if (!dir)
		return NULL;

	// Return the entry from the directory (will be NULL if it doesn't exist in the directory)
	return dir->getEntry(fn.GetFullName());
}

// Not implemented for zip archives, use zip-specific instead if needed
ArchiveEntry* ZipArchive::getEntry(DWORD index) {
	return NULL;
}

string ZipArchive::getFileExtensionString() {
	return _T("Any Zip Format File (*.zip;*.pk3;*.jdf)|*.zip;*.pk3;*.jdf|Zip File (*.zip)|*.zip|Pk3 File (*.pk3)|*.pk3|JDF File (*.jdf)|*.jdf");
}

bool ZipArchive::openFile(string filename) {
	// Open the file
	wxFFileInputStream in(filename);
	if (!in.IsOk()) {
		Global::error = _T("Unable to open file");
		return false;
	}

	// Create zip stream
	wxZipInputStream zip(in);
	if (!zip.IsOk()) {
		Global::error = _T("Invalid zip file");
		return false;
	}

	// Create 'root' directory
	directory = new zipdir_t;
	directory->parent_dir = NULL;
	directory->name = _T("");

	// Go through all zip entries
	int entry_index = 0;
	wxZipEntry* entry = zip.GetNextEntry();
	while (entry) {
		if (!entry->IsDir()) {
			// Get the entry name as a wxFileName (so we can break it up)
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);

			// Create entry
			ArchiveEntry *new_entry = new ArchiveEntry(fn.GetFullName(), this);

			// Setup entry info
			new_entry->setSize(entry->GetSize());
			new_entry->setLoaded(false);
			//setEntryZipIndex(new_entry, entry_index);
			new_entry->setExProp(_T("zip_index"), s_fmt(_T("%d"), entry_index));
			new_entry->setState(0);

			// Add entry and directory to directory tree
			zipdir_t* ndir = addDirectory(fn.GetPath(true, wxPATH_UNIX));
			ndir->entries.push_back(new_entry);
		}
		else {
			// Zip entry is a directory, add it to the directory tree
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);
			addDirectory(fn.GetPath(true, wxPATH_UNIX));
		}

		// Go to next entry in the zip file
		entry = zip.GetNextEntry();
		entry_index++;
	}
	
	dumpDirectoryTree();

	// Setup variables
	this->filename = filename;
	modified = false;
	on_disk = true;

	return true;
}

bool ZipArchive::save(string filename) {
	return false;
}

bool ZipArchive::loadEntryData(ArchiveEntry* entry) {
	// Check that the entry belongs to this archive
	if (entry->getParent() != this) {
		wxLogMessage(_T("ZipArchive::loadEntryData: Entry %s attempting to load data from wrong parent!"), entry->getName().c_str());
		return false;
	}

	// Do nothing if the entry's size is zero,
	// or if it has already been loaded
	if (entry->getSize() == 0 || entry->isLoaded()) {
		entry->setLoaded();
		return true;
	}

	// Check that the entry has a zip index
	int zip_index = 0;
	if (entry->hasExProp(_T("zip_index")))
		zip_index = atoi(chr(entry->getExProp(_T("zip_index"))));
	else {
		wxLogMessage(_T("ZipArchive::loadEntryData: Entry %s has no zip entry index!"), entry->getName().c_str());
		return false;
	}

	// Open the file
	wxFFileInputStream in(filename);
	if (!in.IsOk()) {
		wxLogMessage(_T("ZipArchive::loadEntryData: Unable to open zip file \"%s\"!"), filename.c_str());
		return false;
	}

	// Create zip stream
	wxZipInputStream zip(in);
	if (!zip.IsOk()) {
		wxLogMessage(_T("ZipArchive::loadEntryData: Invalid zip file \"%s\"!"), filename.c_str());
		return false;
	}

	// Skip to correct entry in zip
	wxZipEntry* zentry = zip.GetNextEntry();
	for (long a = 0; a < zip_index; a++)
		zentry = zip.GetNextEntry();

	// Read the data
	BYTE* data = new BYTE[zentry->GetSize()];
	zip.Read(data, zentry->GetSize());
	entry->setData(data);

	// Set the entry to loaded
	entry->setLoaded();

	return true;
}

DWORD ZipArchive::numEntries() {
	if (directory)
		return directory->numEntries(true);
	else
		return 0;
}

void ZipArchive::close() {
	// Announce
	announce(_T("close"));
}

// The entry to be added needs a 'directory' extra property to be
// added to any non-root directory (will be added at position in the directory, not archive)
bool ZipArchive::addEntry(ArchiveEntry* entry, DWORD position) {
	return false;
}

ArchiveEntry* ZipArchive::addNewEntry(string name, DWORD position) {
	// See addEntry
	return NULL;
}

ArchiveEntry* ZipArchive::addExistingEntry(ArchiveEntry* entry, DWORD position, bool copy) {
	// See addEntry
	return NULL;
}

bool ZipArchive::removeEntry(ArchiveEntry* entry, bool delete_entry) {
	// Check valid entry
	if (!entry)
		return false;

	// Check entry is part of this archive
	if (entry->getParent() != this)
		return false;

	// Get the directory the entry is in
	zipdir_t* dir = getEntryDirectory(entry);

	// If it doesn't exist in any directory do nothing
	if (!dir)
		return false;

	// Remove it from the directory
	DWORD index = (DWORD)dir->entryIndex(entry);
	dir->entries.erase(dir->entries.begin() + index);

	// Announce
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(entry);
	mc.write(&index, sizeof(DWORD));
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce(_T("entry_removed"), mc);

	// Delete the entry if specified
	if (delete_entry)
		delete entry;

	// Return success
	return true;
}

bool ZipArchive::swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) {
	// Check valid entries
	if (!entry1 || !entry2)
		return false;

	// Check entries are both part of the archive
	if (entry1->getParent() != this || entry2->getParent() != this)
		return false;

	// Get directories for both entries
	zipdir_t* dir1 = getEntryDirectory(entry1);
	zipdir_t* dir2 = getEntryDirectory(entry2);

	// If either entry doesn't exist in a directory return false (something is wrong)
	if (!dir1 || !dir2)
		return false;

	// Swap the entries
	int i1 = dir1->entryIndex(entry1);
	int i2 = dir2->entryIndex(entry2);
	ArchiveEntry* temp = dir2->entries[i2];
	dir2->entries[i2] = dir1->entries[i1];
	dir1->entries[i1] = temp;

	// Announce
	MemChunk mc;
	wxUIntPtr ptr1 = wxPtrToUInt(entry1);
	wxUIntPtr ptr2 = wxPtrToUInt(entry2);
	mc.write(&i1, sizeof(int));
	mc.write(&i2, sizeof(int));
	mc.write(&ptr1, sizeof(wxUIntPtr));
	mc.write(&ptr2, sizeof(wxUIntPtr));
	announce(_T("entries_swapped"), mc);

	// Return success
	return true;
}

// Have to check for directory structure in the given name
bool ZipArchive::renameEntry(ArchiveEntry* entry, string new_name) {
	// Check valid entry
	if (!entry)
		return false;

	// Check entry is part of the archive
	if (entry->getParent() != this)
		return false;

	// Check if we're dealing with an absolute directory (if new_name starts with a /)
	bool absolute = false;
	if (new_name.StartsWith(_T("/"))) {
		// Absolute, set the flag and remove starting /
		absolute = true;
		new_name.Remove(0, 1);
	}

	// Convert given name to wxFileName for processing
	wxFileName fn(new_name);

	// Check for directories in the new name
	if (fn.GetDirCount() == 0 && !absolute) {
		// If no directory was given, just rename the entry
		entry->rename(new_name);
	}
	else {
		// Otherwise create the directory first if needed
		string new_dir;
		zipdir_t* cdir = getEntryDirectory(entry);

		// Check if the directory was absolute
		if (!absolute) {
			// If it wasn't, add the given directory to the entry's current directory path
			new_dir = cdir->getFullPath() + fn.GetPath(true, wxPATH_UNIX);
		}
		else {
			// If it was absolute, set the directory to what was given
			new_dir = fn.GetPath(true, wxPATH_UNIX);
		}

		// Create the directory (or just get it if it already exists)
		zipdir_t* ndir = addDirectory(new_dir);

		// Remove the entry from it's current directory
		cdir->entries.erase(cdir->entries.begin() + cdir->entryIndex(entry));

		// Add it to the new directory
		ndir->entries.push_back(entry);
	}
}

vector<Archive::mapdesc_t> ZipArchive::detectMaps() {
	return vector<mapdesc_t>();
}



zipdir_t* ZipArchive::getEntryDirectory(ArchiveEntry* entry, zipdir_t* dir) {
	// Check valid entry
	if (!entry)
		return NULL;

	// Check if root directory exists
	if (!directory)
		return NULL;

	// If no directory was specified, set it to the root directory
	if (!dir)
		dir = directory;

	// If the entry exists in the current directory, return it
	if (dir->entryExists(entry))
		return dir;

	// Go through all subirectories in the current directory
	for (size_t a = 0; a < dir->subdirectories.size(); a++) {
		// Check if the entry exists in the subdirectory, or any of it's subdirectories
		zipdir_t* rdir = getEntryDirectory(entry, dir->subdirectories[a]);

		// If it does, return it
		if (rdir)
			return rdir;
	}

	// Entry wasn't found in the given directory or any of it's subdirectories,
	// so return NULL
	return NULL;
}

zipdir_t* ZipArchive::getDirectory(string name, zipdir_t* dir) {
	// Check if root directory exists
	if (!directory)
		return NULL;

	// If no current directory was specified, set it to the root directory
	if (!dir)
		dir = directory;

	// Convert the given dirname to a wxFileName for processing
	wxFileName fn(name);

	// If the dirname has no directories, return the current directory
	if (fn.GetDirCount() == 0)
		return dir;

	// Otherwise get the first directory in the name, and check that it's
	// a subdirectory of the current directory
	string first = fn.GetDirs()[0] + _T("/");
	for (size_t a = 0; a < dir->subdirectories.size(); a++) {
		if (!dir->subdirectories[a]->name.Cmp(first)) {
			// The subdirectory exists within the current directory
			// Remove the first directory from the dirname
			fn.RemoveDir(0);

			// Continue searching from the subdirectory
			return getDirectory(fn.GetPath(true, wxPATH_UNIX), dir->subdirectories[a]);
		}
	}

	// The first subdirectory in the given dirname doesn't exist in the current directory,
	// so return NULL as the directory asked for doesn't exist
	return NULL;
}

zipdir_t* ZipArchive::addDirectory(string name, zipdir_t* dir) {
	// If no current directory was specified, set it to the root directory
	if (!dir)
		dir = directory;

	// Convert the given dirname to a wxFileName for processing
	wxFileName fn(name);

	// If the dirname has no directories, return the current directory
	if (fn.GetDirCount() == 0)
		return dir;

	// Get first directory in given path
	string first = fn.GetDirs()[0] + _T("/");

	// Check the first dir doesn't already exist
	zipdir_t* dir_add = NULL;
	for (size_t a = 0; a < dir->subdirectories.size(); a++) {
		if (!dir->subdirectories[a]->name.Cmp(first))
			dir_add = dir->subdirectories[a];
	}

	// If the first directory didn't exist within the current directory, add it
	if (!dir_add) {
		// Create and setup new directory
		dir_add = new zipdir_t;
		dir_add->name = first;
		dir_add->parent_dir = dir;

		// Add it to the current directory
		dir->subdirectories.push_back(dir_add);
	}

	// If it did exist, remove the first directory from the dirname and
	// continue adding subsequent subdirectories
	fn.RemoveDir(0);
	return addDirectory(fn.GetPath(true, wxPATH_UNIX), dir_add);
}

void ZipArchive::dumpDirectoryTree(zipdir_t* dir) {
	if (!dir)
		dir = directory;

	wxLogMessage(dir->getFullPath());

	for (size_t a = 0; a < dir->entries.size(); a++) {
		string line = dir->getFullPath();
		line += dir->entries[a]->getName();
		wxLogMessage(line);
	}

	for (size_t a = 0; a < dir->subdirectories.size(); a++) {
		dumpDirectoryTree(dir->subdirectories[a]);
	}
}
