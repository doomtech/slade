
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

#include <wx\filename.h>



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


zipdir_t::zipdir_t() {
	entry = new ArchiveEntry();
	entry->setType(ETYPE_FOLDER);
	entry->setState(0);
}

bool zipdir_t::entryExists(ArchiveEntry* entry, bool include_subdirs) {
	if (!entry)
		return false;

	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return true;
	}

	if (include_subdirs) {
		for (size_t a = 0; a < subdirectories.size(); a++) {
			if (subdirectories[a]->entryExists(entry, true))
				return true;
		}
	}

	return false;
}

ArchiveEntry* zipdir_t::getEntry(string name) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (!entries[a]->getName().Cmp(name))
			return entries[a];
	}

	return NULL;
}

int zipdir_t::entryIndex(ArchiveEntry* entry) {
	if (!entry)
		return -1;

	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return a;
	}

	return -1;
}

zipdir_t* zipdir_t::getSubDir(string name) {
	for (size_t a = 0; a < subdirectories.size(); a++) {
		if (!subdirectories[a]->getName().Cmp(name))
			return subdirectories[a];
	}

	return NULL;
}

string zipdir_t::getFullPath() {
	if (parent_dir)
		return parent_dir->getFullPath() + getName();
	else
		return getName();
}

DWORD zipdir_t::numEntries(bool include_subdirs) {
	DWORD num = entries.size();

	if (include_subdirs) {
		for (size_t a = 0; a < subdirectories.size(); a++)
			num += subdirectories[a]->numEntries(true);
	}

	return num;
}

DWORD zipdir_t::numSubDirs(bool include_subdirs) {
	DWORD num = subdirectories.size();

	if (include_subdirs) {
		for (size_t a = 0; a < subdirectories.size(); a++)
			num += subdirectories[a]->numSubDirs(true);
	}

	return num;
}




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
	directory->setName(_T(""));

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
			ndir->entry->setState(0);
		}
		else {
			// Zip entry is a directory, add it to the directory tree
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);
			zipdir_t* ndir = addDirectory(fn.GetPath(true, wxPATH_UNIX));
			ndir->entry->setState(0);
		}

		// Go to next entry in the zip file
		entry = zip.GetNextEntry();
		entry_index++;
	}

	//dumpDirectoryTree();

	// Setup variables
	this->filename = filename;
	modified = false;
	on_disk = true;

	return true;
}

bool ZipArchive::save(string filename) {
	// If no filename was specified, just use the current filename
	if (filename.IsEmpty())
		filename = this->filename;

	// Create backup copy if needed
	string bakfile = filename + _T(".bak");
	if (wxFileName::FileExists(filename)) {
		// Remove old backup file
		wxRemoveFile(bakfile);

		// Copy current file contents to backup file
		wxCopyFile(this->filename, bakfile);
	}
	else {
		// If no backup needed, set bakfile anyway as it's needed for copying zip entries
		bakfile = this->filename;
	}

	// Open the file
	wxFFileOutputStream out(filename);
	if (!out.IsOk()) {
		Global::error = _T("Unable to open file for saving. Make sure it isn't in use by another program.");
		return false;
	}

	// Open as zip for writing
	wxZipOutputStream zip(out, 9);
	if (!zip.IsOk()) {
		Global::error = _T("Unable to create zip for saving");
		return false;
	}

	// Open old zip for copying, if it exists. This is used to copy any entries
	// that have been previously saved/compressed and are unmodified, to greatly
	// speed up zip file saving by not having to recompress unchanged entries
	wxFFileInputStream in(bakfile);
	wxZipInputStream inzip(in);

	// Get a list of all entries in the old zip
	wxZipEntry** c_entries = new wxZipEntry*[inzip.GetTotalEntries()];
	for (int a = 0; a < inzip.GetTotalEntries(); a++)
		c_entries[a] = inzip.GetNextEntry();

	// Get a linear list of all entries in the archive
	vector<ArchiveEntry*> entries;
	getTreeAsList(entries);

	// Go through all entries
	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a]->getType() == ETYPE_FOLDER) {
			// If the current entry is a folder, just write a directory entry and continue
			zip.PutNextDirEntry(entries[a]->getExProp(_T("directory")) + entries[a]->getName());
			entries[a]->setState(0);
			continue;
		}

		if (!inzip.IsOk() || entries[a]->getState() > 0 || !entries[a]->hasExProp(_T("zip_index"))) {
			// If the current entry has been changed, or doesn't exist in the old zip,
			// (re)compress it's data and write it to the zip
			wxZipEntry* zipentry = new wxZipEntry(getEntryFullPath(entries[a]));
			zip.PutNextEntry(zipentry);
			zip.Write(entries[a]->getData(), entries[a]->getSize());
		}
		else {
			// If the entry is unmodified and exists in the old zip, just copy it over
			int index = atoi(entries[a]->getExProp(_T("zip_index")).ToAscii());
			//c_entries[index]->SetName(getEntryFullPath(entries[a]));
			zip.CopyEntry(c_entries[index], inzip);
			inzip.Reset();
		}

		// Update entry info
		entries[a]->setState(0);
		entries[a]->setExProp(_T("zip_index"), s_fmt(_T("%d"), a));
	}

	// Clean up and update variables
	delete[] c_entries;
	zip.Close();
	this->filename = filename;
	on_disk = true;
	announce(_T("saved"));

	return true;
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
	// Delete all directories and entries
	deleteDirectory();

	// Announce
	announce(_T("close"));
}

// The entry to be added needs a 'directory' extra property to be
// added to any non-root directory (will be added at position in the directory, not archive)
bool ZipArchive::addEntry(ArchiveEntry* entry, DWORD position) {
	// Check valid entry
	if (!entry)
		return false;

	// Get directory to add to, adding it if necessary
	zipdir_t* dir = addDirectory(entry->getExProp(_T("directory")));

	// Add the entry to the directory
	if (position >= dir->numEntries()) {
		dir->entries.push_back(entry);
		position = dir->entries.size() - 1;
	}
	else
		dir->entries.insert(dir->entries.begin() + position, entry);

	// Update variables etc
	modified = true;
	entry->setParent(this);
	entry->setState(2);

	// Announce
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(entry);
	mc.write(&position, sizeof(DWORD));
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce(_T("entry_added"), mc);

	return false;
}

ArchiveEntry* ZipArchive::addNewEntry(string name, DWORD position) {
	// Convert name to wxFileName for processing
	wxFileName fn(name);

	// If the entry name is empty do nothing
	if (fn.GetFullName().IsEmpty())
		return NULL;

	// Create the entry to add
	ArchiveEntry* new_entry = new ArchiveEntry(fn.GetFullName());

	// Setup entry directory
	new_entry->setExProp(_T("directory"), fn.GetPath(true, wxPATH_UNIX));

	// Add it to the archive
	addEntry(new_entry, position);

	// Return the new entry
	return new_entry;
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

	// Announce
	MemChunk mc;
	DWORD index = (DWORD)dir->entryIndex(entry);
	wxUIntPtr ptr = wxPtrToUInt(entry);
	mc.write(&index, sizeof(DWORD));
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce(_T("entry_removed"), mc);

	// Remove it from the directory
	dir->entries.erase(dir->entries.begin() + index);

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

	// Check a new name was given
	if (new_name.IsEmpty() || !new_name.Cmp(entry->getName()))
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

	// Load entry data if it hasn't been already (this prevents problems with saving the entry)
	entry->getData();

	// Check for directories in the new name
	if (fn.GetDirCount() == 0 && !absolute) {
		// If no directory was given, just rename the entry
		entry->rename(new_name);
		return true;
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
		removeEntry(entry, false);

		// Add it to the new directory
		entry->setExProp(_T("directory"), ndir->getFullPath());
		addEntry(entry, 999999);

		// Rename it
		entry->rename(fn.GetFullName());

		return true;
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

string ZipArchive::getEntryFullPath(ArchiveEntry* entry) {
	// Check valid entry
	if (!entry)
		return _T("");

	// Check entry is part of the archive
	if (entry->getParent() != this)
		return _T("");

	// Get the entry directory
	zipdir_t* dir = getEntryDirectory(entry);

	// Return the entry path + name
	if (dir)
		return dir->getFullPath() + entry->getName();
	else
		return entry->getName();
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
		if (!dir->subdirectories[a]->getName().Cmp(first)) {
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
		if (!dir->subdirectories[a]->getName().Cmp(first))
			dir_add = dir->subdirectories[a];
	}

	// If the first directory didn't exist within the current directory, add it
	if (!dir_add) {
		// Create and setup new directory
		dir_add = new zipdir_t;
		dir_add->setName(first);
		dir_add->parent_dir = dir;
		dir_add->entry->setState(2);

		// Add it to the current directory
		dir->subdirectories.push_back(dir_add);

		// Announce
		MemChunk mc;
		wxUIntPtr ptr = wxPtrToUInt(dir_add);
		mc.write(&ptr, sizeof(wxUIntPtr));
		announce(_T("directory_added"), mc);
	}

	// If it did exist, remove the first directory from the dirname and
	// continue adding subsequent subdirectories
	fn.RemoveDir(0);
	return addDirectory(fn.GetPath(true, wxPATH_UNIX), dir_add);
}

void ZipArchive::deleteDirectory(zipdir_t* dir) {
	// If no current directory was specified, set it to the root directory
	if (!dir)
		dir = directory;

	// Delete directory entry
	delete dir->entry;

	// Delete any entries in the directory
	for (size_t a = 0; a < dir->entries.size(); a++) {
		delete dir->entries[a];
	}

	// Delete any subdirectories in the directory
	for (size_t a = 0; a < dir->subdirectories.size(); a++) {
		deleteDirectory(dir->subdirectories[a]);
	}

	// Clear vectors
	dir->entries.clear();
	dir->subdirectories.clear();

	// Delete the directory itself
	delete dir;
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

void ZipArchive::getTreeAsList(vector<ArchiveEntry*>& list, zipdir_t* start) {
	// If no starting directory was specified, set it to the root directory
	if (!start)
		start = directory;

	// Add the current directory entry to the list if it isn't the root directory
	if (start != directory) {
		// Update directory entry information
		wxFileName fn(start->getFullPath());
		fn.RemoveLastDir();
		start->entry->setExProp(_T("directory"), fn.GetPath(true, wxPATH_UNIX));

		// Add it
		list.push_back(start->entry);
	}

	// Add all entries in the current directory to the list
	for (size_t a = 0; a < start->entries.size(); a++)
		list.push_back(start->entries[a]);

	// Lastly go through all subdirectories and add them to the list
	for (size_t a = 0; a < start->subdirectories.size(); a++) {
		getTreeAsList(list, start->subdirectories[a]);
	}
}
