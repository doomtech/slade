
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
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(Bool, archive_load_data)


/* Zip Directory Layout:
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
 * ZIPDIR_T STRUCT FUNCTIONS
 *******************************************************************/

/* zipdir_t::zipdir_t
 * zipdir_t struct constructor
 *******************************************************************/
zipdir_t::zipdir_t() {
	entry = new ArchiveEntry();
	entry->setType(ETYPE_FOLDER);
	entry->setState(0);
}

/* zipdir_t::entryExists
 * Checks if the given entry is in this zipdir. If include_subdirs is
 * true, also recursively checks any subdirectories in the zipdir.
 * Returns true if entry is found, false otherwise
 *******************************************************************/
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

/* zipdir_t::getEntry
 * Gets the entry within this zipdir that matches the given name.
 * Returns the matching entry, or NULL if none matched
 *******************************************************************/
ArchiveEntry* zipdir_t::getEntry(string name) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (!entries[a]->getName().Cmp(name))
			return entries[a];
	}

	return NULL;
}

/* zipdir_t::entryIndex
 * Returns the entry index of the given entry within the zipdir, or
 * -1 if the entry doesn't exist in the directory
 *******************************************************************/
int zipdir_t::entryIndex(ArchiveEntry* entry) {
	if (!entry)
		return -1;

	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return a;
	}

	return -1;
}

/* zipdir_t::dirIndex
 * Returns the index of the given subdirectory within the zipdir, or
 * -1 if the subdirectory doesn't exist in the zipdir
 *******************************************************************/
int zipdir_t::dirIndex(zipdir_t* dir) {
	if (!dir)
		return -1;

	for (size_t a = 0; a < subdirectories.size(); a++) {
		if (subdirectories[a] == dir)
			return a;
	}

	return -1;
}

/* zipdir_t::getSubDir
 * Returns the subdirectory of this zipdir matching the given name,
 * or NULL if none matched
 *******************************************************************/
zipdir_t* zipdir_t::getSubDir(string name) {
	for (size_t a = 0; a < subdirectories.size(); a++) {
		if (!subdirectories[a]->getName().Cmp(name))
			return subdirectories[a];
	}

	return NULL;
}

/* zipdir_t::getFullPath
 * Gets the full path of this zipdir (includes parent directories)
 *******************************************************************/
string zipdir_t::getFullPath() {
	if (parent_dir)
		return parent_dir->getFullPath() + getName();
	else
		return getName();
}

/* zipdir_t::numEntries
 * Returns the number of entries in the zipdir, or if include_subdirs
 * is true, the number of entries in the zipdir and all it's
 * subdirectories
 *******************************************************************/
uint32_t zipdir_t::numEntries(bool include_subdirs) {
	uint32_t num = entries.size();

	if (include_subdirs) {
		for (size_t a = 0; a < subdirectories.size(); a++)
			num += subdirectories[a]->numEntries(true);
	}

	return num;
}

/* zipdir_t::numSubDirs
 * Returns the number of subdirectories in the zipdir, or if
 * include_subdirs is true, the number of subdirectories in the
 * zipdir and all it's subdirectories
 *******************************************************************/
uint32_t zipdir_t::numSubDirs(bool include_subdirs) {
	uint32_t num = subdirectories.size();

	if (include_subdirs) {
		for (size_t a = 0; a < subdirectories.size(); a++)
			num += subdirectories[a]->numSubDirs(true);
	}

	return num;
}

/* zipdir_t::copy
 * Returns a copy of the zipdir. The copy includes copies of all
 * entries, and if <include_subdirs> is true, all subdirectories are
 * also recursively copied
 *******************************************************************/
zipdir_t* zipdir_t::copy(bool include_subdirs) {
	// Init copied directory
	zipdir_t* copy_dir = new zipdir_t;
	copy_dir->parent_dir = NULL;

	// Copy dir entry
	copy_dir->entry = new ArchiveEntry(*entry);

	// Copy entries
	for (size_t a = 0; a < entries.size(); a++)
		copy_dir->entries.push_back(new ArchiveEntry(*entries[a]));

	// Copy subdirectories if needed
	if (include_subdirs) {
		for (size_t a = 0; a < subdirectories.size(); a++) {
			copy_dir->subdirectories.push_back(subdirectories[a]->copy(true));
			copy_dir->subdirectories[a]->parent_dir = copy_dir;
		}
	}

	// Return copied zipdir
	return copy_dir;
}

void zipdir_t::clear(bool delete_entries) {
	// Delete dir entry
	if (entry)
		delete entry;

	// Delete entries
	if (delete_entries) {
		for (size_t a = 0; a < entries.size(); a++)
			delete entries[a];
	}
	entries.clear();

	// Delete subdirectories
	for (size_t a = 0; a < subdirectories.size(); a++)
		subdirectories[a]->clear(delete_entries);
	subdirectories.clear();
}

void zipdir_t::addToList(vector<ArchiveEntry*>& list) {
	// Add the directory entry to the list
	wxFileName fn(getFullPath());
	fn.RemoveLastDir();
	entry->setExProp(_T("Directory"), fn.GetPath(true, wxPATH_UNIX));
	list.push_back(entry);

	// Add all entries in the directory to the list
	for (size_t a = 0; a < entries.size(); a++) {
		entries[a]->setExProp(_T("Directory"), getFullPath());
		list.push_back(entries[a]);
	}

	// Lastly go through all subdirectories and add them to the list
	for (size_t a = 0; a < subdirectories.size(); a++)
		subdirectories[a]->addToList(list);
}


/*******************************************************************
 * ZIPARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* ZipArchive::ZipArchive
 * ZipArchive class constructor
 *******************************************************************/
ZipArchive::ZipArchive()
: Archive(ARCHIVE_ZIP) {
	directory = new zipdir_t;
	directory->parent_dir = NULL;
	directory->setName(_T(""));
}

/* ZipArchive::~ZipArchive
 * ZipArchive class destructor
 *******************************************************************/
ZipArchive::~ZipArchive() {
}

/* ZipArchive::entryIndex
 * Returns the entry index of the given entry relative to it's
 * directory, rather than to the whole archive.
 * Returns the index or -1 if the entry is invalid/not in the archive
 *******************************************************************/
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

/* ZipArchive::getEntry
 * Get the entry with the given name AND directory (must match both)
 * Returns NULL if the entry is not found or invalid
 *******************************************************************/
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

/* ZipArchive::getEntry
 * Getting entry by index is not implemented for ZipArchive, just
 * prints a warning to the logfile and returns NULL
 *******************************************************************/
ArchiveEntry* ZipArchive::getEntry(uint32_t index) {
	wxLogMessage(_T("Warning: Attempt to access ZipArchive entry by index"));
	return NULL;
}

/* ZipArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string ZipArchive::getFileExtensionString() {
	return _T("Any Zip Format File (*.zip;*.pk3;*.jdf)|*.zip;*.pk3;*.jdf|Zip File (*.zip)|*.zip|Pk3 File (*.pk3)|*.pk3|JDF File (*.jdf)|*.jdf");
}

/* ZipArchive::openFile
 * Opens the given file and loads it into the archive.
 * Returns false if file wasn't found or zip was invalid,
 * true otherwise
 *******************************************************************/
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

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Create 'root' directory
	if (directory)
		deleteDirectory(directory);
	directory = new zipdir_t;
	directory->parent_dir = NULL;
	directory->setName(_T(""));

	// Go through all zip entries
	int entry_index = 0;
	wxZipEntry* entry = zip.GetNextEntry();
	while (entry) {
		if (entry->GetMethod() != wxZIP_METHOD_DEFLATE && entry->GetMethod() != wxZIP_METHOD_STORE) {
			Global::error = _T("Unsupported zip compression method");
			setMuted(false);
			return false;
		}

		if (!entry->IsDir()) {
			// Get the entry name as a wxFileName (so we can break it up)
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);

			// Create entry
			ArchiveEntry *new_entry = new ArchiveEntry(fn.GetFullName(), entry->GetSize(), this);

			// Setup entry info
			new_entry->setLoaded(false);
			new_entry->setExProp(_T("ZipIndex"), s_fmt(_T("%d"), entry_index));
			new_entry->setState(0);

			// Add entry and directory to directory tree
			zipdir_t* ndir = addDirectory(fn.GetPath(true, wxPATH_UNIX));
			ndir->entries.push_back(new_entry);

			// Read the data
			uint8_t* data = new uint8_t[entry->GetSize()];
			zip.Read(data, entry->GetSize());
			new_entry->importMem(data, entry->GetSize());
			new_entry->setLoaded(true);
			new_entry->setState(0);

			// Determine it's type
			new_entry->detectType(true, true);

			// Unload data if needed
			if (!archive_load_data)
				new_entry->unloadData();

			// Clean up
			delete[] data;
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

	// Enable announcements
	setMuted(false);

	// Setup variables
	this->filename = filename;
	setModified(false);
	on_disk = true;

	return true;
}

/* ZipArchive::save
 * Saves the archive to the given filename, or the current archive
 * filename if none given. Makes a backup of the file if it already
 * exists. Returns false if file could not be written, true otherwise
 *******************************************************************/
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
			zip.PutNextDirEntry(entries[a]->getExProp(_T("Directory")) + entries[a]->getName());
			entries[a]->setState(0);
			continue;
		}

		if (!inzip.IsOk() || entries[a]->getState() > 0 || !entries[a]->hasExProp(_T("ZipIndex"))) {
			// If the current entry has been changed, or doesn't exist in the old zip,
			// (re)compress it's data and write it to the zip
			wxZipEntry* zipentry = new wxZipEntry(getEntryFullPath(entries[a]));
			zip.PutNextEntry(zipentry);
			zip.Write(entries[a]->getData(), entries[a]->getSize());
		}
		else {
			// If the entry is unmodified and exists in the old zip, just copy it over
			int index = atoi(entries[a]->getExProp(_T("ZipIndex")).ToAscii());
			c_entries[index]->SetName(getEntryFullPath(entries[a]));
			zip.CopyEntry(c_entries[index], inzip);
			inzip.Reset();
		}

		// Update entry info
		entries[a]->setState(0);
		entries[a]->setExProp(_T("ZipIndex"), s_fmt(_T("%d"), a));
	}

	// Clean up and update variables
	delete[] c_entries;
	zip.Close();
	this->filename = filename;
	on_disk = true;
	setModified(false);
	announce(_T("saved"));

	return true;
}

/* ZipArchive::loadEntryData
 * Loads an entry's data from the saved copy of the archive if any.
 * Returns false if the entry is invalid, doesn't belong to the
 * archive or doesn't exist in the saved copy, true otherwise.
 *******************************************************************/
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
	if (entry->hasExProp(_T("ZipIndex")))
		zip_index = atoi(chr(entry->getExProp(_T("ZipIndex"))));
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
	uint8_t* data = new uint8_t[zentry->GetSize()];
	zip.Read(data, zentry->GetSize());
	entry->importMem(data, zentry->GetSize());

	// Set the entry to loaded
	entry->setLoaded();

	return true;
}

/* ZipArchive::numEntries
 * Returns the total number of entries in the archive
 *******************************************************************/
uint32_t ZipArchive::numEntries() {
	if (directory)
		return directory->numEntries(true);
	else
		return 0;
}

/* ZipArchive::close
 * 'Closes' the archive, deletes all enries.
 *******************************************************************/
void ZipArchive::close() {
	// Delete all directories and entries
	deleteDirectory();

	// Announce
	announce(_T("close"));
}

/* ZipArchive::addEntry
 * Adds an entry to the archive, at the directory given in the
 * entry's 'directory' extra property. If the extra property doesn't
 * exist the entry is added to the root directory. If the given
 * directory doesn't exist it will be created.
 * The position is relative to the *directory* it will be added to,
 * rather than relative to the beginning of the archive.
 * Returns false if the entry is invalid, false otherwise.
 *******************************************************************/
bool ZipArchive::addEntry(ArchiveEntry* entry, uint32_t position) {
	// Check valid entry
	if (!entry)
		return false;

	// Get directory to add to, adding it if necessary
	zipdir_t* dir = addDirectory(entry->getExProp(_T("Directory")));

	// Add the entry to the directory
	if (position >= dir->numEntries()) {
		dir->entries.push_back(entry);
		position = dir->entries.size() - 1;
	}
	else
		dir->entries.insert(dir->entries.begin() + position, entry);

	// Update variables etc
	setModified(true);
	entry->setParent(this);
	entry->setState(2);

	// Announce
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(entry);
	mc.write(&position, sizeof(uint32_t));
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce(_T("entry_added"), mc);

	return true;
}

/* ZipArchive::addNewEntry
 * Creates an entry with the given name and adds it to the archive.
 * If the name contains an absolute or relative directory then the
 * entry will be created in that directory. The position is relative
 * to the directory rather than the beginning of the archive.
 * Returns the created entry, or NULL if no name was given.
 *******************************************************************/
ArchiveEntry* ZipArchive::addNewEntry(string name, uint32_t position) {
	// Convert name to wxFileName for processing
	wxFileName fn(name);

	// If the entry name is empty do nothing
	if (fn.GetFullName().IsEmpty()) {
		wxLogMessage(_T("Cannot create entry with no name"));
		return NULL;
	}

	// Create the entry to add
	ArchiveEntry* new_entry = new ArchiveEntry(fn.GetFullName());

	// Setup entry directory
	new_entry->setExProp(_T("Directory"), fn.GetPath(true, wxPATH_UNIX));

	// Add it to the archive
	addEntry(new_entry, position);

	// Return the new entry
	return new_entry;
}

/* ZipArchive::addExistingEntry
 * Adds an existing entry to the archive, copying it if specified.
 *******************************************************************/
ArchiveEntry* ZipArchive::addExistingEntry(ArchiveEntry* entry, uint32_t position, bool copy) {
	// Make a copy of the entry to add if needed
	if (copy)
		entry = new ArchiveEntry(*entry);

	// Add the entry to the list
	addEntry(entry, position);

	// Return the added entry
	return entry;
}

/* ZipArchive::removeEntry
 * Removes the given entry from the archive, and deletes the entry
 * object if specified. Returns false if the entry is invalid or
 * doesn't exist in the archive, true otherwise.
 *******************************************************************/
bool ZipArchive::removeEntry(ArchiveEntry* entry, bool delete_entry) {
	// Check valid entry
	if (!entry)
		return false;

	// Check if entry is locked
	if (entry->isLocked())
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
	uint32_t index = (uint32_t)dir->entryIndex(entry);
	wxUIntPtr ptr = wxPtrToUInt(entry);
	mc.write(&index, sizeof(uint32_t));
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce(_T("entry_removed"), mc);

	// Remove it from the directory
	dir->entries.erase(dir->entries.begin() + index);

	// Delete the entry if specified
	if (delete_entry)
		delete entry;

	// Update variables etc
	setModified(true);

	// Return success
	return true;
}

/* ZipArchive::swapEntries
 * Swaps two entries in the archive. Returns false if either entry is
 * invalid or doesn't exist in the archive, true otherwise.
 *******************************************************************/
bool ZipArchive::swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) {
	// Check valid entries
	if (!entry1 || !entry2)
		return false;

	// Check entries are both unlocked
	if (entry1->isLocked() || entry2->isLocked())
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

	// Update variables etc
	setModified(true);

	// Return success
	return true;
}

// Have to check for directory structure in the given name
/* ZipArchive::renameEntry
 * Renames the specified entry. If a directory is given in the new
 * name, the entry will be moved to that directory.
 * Returns false if the entry is invalid or doesn't exist in the
 * archive, true otherwise.
 *******************************************************************/
bool ZipArchive::renameEntry(ArchiveEntry* entry, string new_name) {
	// Check valid entry
	if (!entry)
		return false;

	// Check if entry is locked
	if (entry->isLocked())
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
		entry->setExProp(_T("Directory"), ndir->getFullPath());
		addEntry(entry, 999999);

		// Rename it
		entry->rename(fn.GetFullName());

		return true;
	}
}

/* ZipArchive::detectMaps
 * Detects all the maps in the archive and returns a vector of
 * information about them.
 *******************************************************************/
vector<Archive::mapdesc_t> ZipArchive::detectMaps() {
	vector<mapdesc_t> ret;

	// Get the maps directory
	zipdir_t* mapdir = getDirectory(_T("maps"));
	if (!mapdir)
		return ret;

	return ret;
}

/* ZipArchive::detectEntryType
 * Performs preliminary entry type checking based on it's position
 * in the zip and it's name/extension (will be overridden if the
 * entry's data later proves it to be another format)
 *******************************************************************/
bool ZipArchive::detectEntryType(ArchiveEntry* entry) {
	// Check the entry is valid and belongs to this archive
	if (!checkEntry(entry))
		return false;

	// Get the entry name as a wxFileName for processing
	wxFileName fn(entry->getName());

	// .txt extension, treat the entry as text (unless it's data proves otherwise)
	if (fn.GetExt().CmpNoCase(_T("txt")) == 0)
		entry->setType(ETYPE_TEXT);

	/* Below types really need to be detected via data rather than extension
	if (fn.GetExt().CmpNoCase(_T("png")) == 0)
		entry->setType(ETYPE_PNG);

	if (fn.GetExt().CmpNoCase(_T("wad")) == 0)
		entry->setType(ETYPE_WAD);
	 */

	// .acs extension - usually an acs script in text format
	if (fn.GetExt().CmpNoCase(_T("acs")) == 0) {
		entry->setType(ETYPE_TEXT);
		entry->setExProp(_T("TextFormat"), _T("SCRIPTS"));
		entry->setExProp(_T("EntryType"), _T("ACS Script (Text)"));
	}

	if (fn.GetName().CmpNoCase(_T("animdefs")) == 0)
		entry->setExProp(_T("TextFormat"), _T("ANIMDEFS"));

	if (fn.GetName().CmpNoCase(_T("sndinfo")) == 0)
		entry->setExProp(_T("TextFormat"), _T("SNDINFO"));

	if (fn.GetName().CmpNoCase(_T("decorate")) == 0)
		entry->setExProp(_T("TextFormat"), _T("DECORATE"));

	if (fn.GetName().CmpNoCase(_T("language")) == 0)
		entry->setType(ETYPE_TEXT);

	return true;
}

ArchiveEntry* ZipArchive::findEntry(string search) {
	return NULL;
}

vector<ArchiveEntry*> ZipArchive::findEntries(string search) {
	vector<ArchiveEntry*> ret;
	return ret;
}



/* ZipArchive::getEntryDirectory
 * Recursively searches subdirectories of [dir] for the given
 * entry. Returns the directory the entry is in, or NULL if the entry
 * is invalid or doesn't exist in any of the subdirectories. If no
 * dir is specified the root directory is searched.
 *******************************************************************/
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

	// If the entry exissrc/ZipArchive.cpp:842: error: ‘WadArchive’ has not been declaredts in the current directory, return it
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

/* ZipArchive::getEntryFullPath
 * Returns the full path of the given entry (folder+name+extension),
 * or an empty string if the entry is invalid or doesn't exist in
 * the archive.
 *******************************************************************/
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

/* ZipArchive::getDirectory
 * Recursive function to find a subdirectory of [dir] that matches
 * the given name (relative to [dir]). If [dir] isn't specified, the
 * root directory is searched. Returns the matching directory or NULL
 * if no matches are found.
 *******************************************************************/
zipdir_t* ZipArchive::getDirectory(string name, zipdir_t* dir) {
	// Check if root directory exists
	if (!directory)
		return NULL;

	// If no current directory was specified, set it to the root directory
	if (!dir)
		dir = directory;

	// Append trailing / if none given
	if (!name.EndsWith(_T("/")))
		name += _T("/");

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

/* ZipArchive::getDirectory
 * Same as above, except searches via a directory entry rather than
 * a name.
 *******************************************************************/
zipdir_t* ZipArchive::getDirectory(ArchiveEntry* dir_entry, zipdir_t* dir) {
	// If no current directory was specified, set it to the root directory
	if (!dir)
		dir = directory;
	
	// Check if the given entry is a directory entry
	if (dir_entry->getType() != ETYPE_FOLDER)
		return NULL;

	// Check if the given entry is the current directory's entry
	if (dir_entry == dir->entry)
		return dir;

	// Go through subdirectories and recursively check if the given entry is
	// any of their directory entries
	for (size_t a = 0; a < dir->subdirectories.size(); a++) {
		zipdir_t* sdir = getDirectory(dir_entry, dir->subdirectories[a]);
		if (sdir) return sdir;
	}

	// The given entry wasn't the current directory or any of its subdirectories'
	// archive entry, so return NULL
	return NULL;
}

/* ZipArchive::addDirectory
 * Recursive function that adds the given subdirectory to [dir],
 * including any extra subdirectories needed on the way. If [dir] is
 * not specified, adds to the root directory. Returns the created
 * subdirectory, or if it already existed returns that.
 *******************************************************************/
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

		// Update variables etc
		setModified(true);

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

/* ZipArchive::deleteDirectory
 * Deletes all entries from the given directory, and recursively
 * deletes all subdirectories and their entries from [dir]. If [dir]
 * is not specified, the root directory is deleted.
 *******************************************************************/
void ZipArchive::deleteDirectory(zipdir_t* dir) {
	// If no current directory was specified, set it to the root directory
	if (!dir)
		dir = directory;

	// Announce
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(dir);
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce(_T("directory_removed"), mc);

	// Remove this directory from it's parent's subdirectory list
	if (dir->parent_dir) {
		int p_index = dir->parent_dir->dirIndex(dir);
		if (p_index >= 0)
			dir->parent_dir->subdirectories.erase(dir->parent_dir->subdirectories.begin() + p_index);
	}

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

	// Delete directory entry
	delete dir->entry;

	// Delete the directory itself
	delete dir;

	// Update variables etc
	setModified(true);
}

/* ZipArchive::renameDirectory
 * Renames [dir] with the new name. Returns false if [dir] is invalid,
 * true otherwise
 *******************************************************************/
bool ZipArchive::renameDirectory(zipdir_t* dir, string newname) {
	// Check valid directory
	if (!dir)
		return false;

	// Rename the directory
	dir->entry->rename(newname);

	// Announce
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(dir);
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce(_T("directory_modified"), mc);

	return true;
}

/* ZipArchive::dumpDirectoryTree
 * Dumps the directory structure to the logfile starting from [dir]
 *******************************************************************/
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

/* ZipArchive::getTreeAsList
 * Adds all entries and directory/subdirectory entries to a vector,
 * starting from [dir]. Used for saving the archive.
 *******************************************************************/
void ZipArchive::getTreeAsList(vector<ArchiveEntry*>& list, zipdir_t* dir) {
	// If no starting directory was specified, set it to the root directory
	if (!dir)
		dir = directory;

	// Add the current directory entry to the list if it isn't the root directory
	if (dir != directory) {
		// Update directory entry information
		wxFileName fn(dir->getFullPath());
		fn.RemoveLastDir();
		dir->entry->setExProp(_T("Directory"), fn.GetPath(true, wxPATH_UNIX));

		// Add it
		list.push_back(dir->entry);
	}

	// Add all entries in the current directory to the list
	for (size_t a = 0; a < dir->entries.size(); a++)
		list.push_back(dir->entries[a]);

	// Lastly go through all subdirectories and add them to the list
	for (size_t a = 0; a < dir->subdirectories.size(); a++) {
		getTreeAsList(list, dir->subdirectories[a]);
	}
}
