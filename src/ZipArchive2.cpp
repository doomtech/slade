
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
/*
#include "Main.h"
#include "ZipArchive.h"
#include <wx/log.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/ptr_scpd.h>
#include <wx/filename.h>
#include <algorithm>
*/

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

/*
ZipDirectory::ZipDirectory(string name, ZipDirectory* parent) {
	this->name = name;
	this->parent_dir = parent;
}

ZipDirectory::~ZipDirectory() {
}

ZipDirectory* ZipDirectory::getSubDir(DWORD index) {
	if (index >= subdirectories.size())
		return NULL;

	return subdirectories[index];
}

ArchiveEntry* ZipDirectory::getEntry(DWORD index) {
	if (index >= entries.size())
		return NULL;

	return entries[index];
}

ArchiveEntry* ZipDirectory::getEntry(string name) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (!entries[a]->getName().Cmp(name))
			return entries[a];
	}

	return NULL;
}

ZipDirectory* ZipDirectory::getSubDir(string dir) {
	wxFileName fn(dir);

	if (fn.GetDirCount() == 0)
		return this;

	string first = fn.GetDirs()[0] + _T("/");
	for (size_t a = 0; a < subdirectories.size(); a++) {
		if (!subdirectories[a]->getName().Cmp(first)) {
			fn.RemoveDir(0);
			return subdirectories[a]->getSubDir(fn.GetPath(true, wxPATH_UNIX));
		}
	}

	return NULL;
}

string ZipDirectory::getPath() {
	if (parent_dir)
		return parent_dir->getPath() + name;
	else
		return name;
}

bool ZipDirectory::entryExists(ArchiveEntry* entry) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return true;
	}

	return false;
}

ZipDirectory* ZipDirectory::dirWithEntry(ArchiveEntry* entry) {
	if (entryExists(entry))
		return this;

	for (size_t a = 0; a < subdirectories.size(); a++) {
		ZipDirectory* dir = subdirectories[a]->dirWithEntry(entry);
		if (dir)
			return dir;
	}

	return NULL;
}

bool ZipDirectory::addEntry(ArchiveEntry* entry, int index) {
	// Check entry is valid
	if (!entry)
		return false;

	// Add it to the entry list
	if (index < 0 || index > entries.size())
		entries.push_back(entry);
	else
		entries.insert(entries.begin() + index, entry);

	return true;
}

bool ZipDirectory::addSubDir(ZipDirectory* dir) {
	// Check dir is valid
	if (!dir)
		return false;

	// Check dir doesn't already exist
	for (size_t a = 0; a < subdirectories.size(); a++) {
		if (!subdirectories[a]->getName().Cmp(dir->getName()))
			return false;
	}

	// Add the directory
	subdirectories.push_back(dir);

	// Sort subdirectory list alphabetically
	std::sort(subdirectories.begin(), subdirectories.end());

	return true;
}

ZipDirectory* ZipDirectory::addSubDir(string dir) {
	wxFileName fn(dir);

	if (fn.GetDirCount() == 0)
		return this;

	// Get first directory in given path
	string first = fn.GetDirs()[0] + _T("/");

	// Check the first dir doesn't already exist
	ZipDirectory* dir_add = NULL;
	for (size_t a = 0; a < subdirectories.size(); a++) {
		if (!subdirectories[a]->getName().Cmp(first))
			dir_add = subdirectories[a];
	}

	if (!dir_add) {
		dir_add = new ZipDirectory(first, this);
		subdirectories.push_back(dir_add);
		std::sort(subdirectories.begin(), subdirectories.end());
	}

	fn.RemoveDir(0);
	return dir_add->addSubDir(fn.GetPath(true, wxPATH_UNIX));
}

int ZipDirectory::getEntryIndex(ArchiveEntry* entry, int start) {
	if (!entry)
		return -1;

	int index = start;
	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return index;

		index++;
	}

	for (size_t a = 0; a < subdirectories.size(); a++) {
		index = subdirectories[a]->getEntryIndex(entry, index);
	}

	return index;
}

ArchiveEntry* ZipDirectory::getEntryAtIndex(DWORD index, DWORD start) {
	DWORD cur_index = start;

	for (size_t a = 0; a < entries.size(); a++) {
		if (cur_index == index)
			return entries[a];

		cur_index++;
	}

	for (size_t a = 0; a < subdirectories.size(); a++) {
		ArchiveEntry* result = subdirectories[a]->getEntryAtIndex(index, cur_index);
		if (result)
			return result;
	}

	return NULL;
}

string ZipDirectory::dumpStructure() {
	string ret = getPath() + _T("\n");

	for (size_t a = 0; a < entries.size(); a++) {
		ret += getPath();
		ret += entries[a]->getName();
		ret += _T("\n");
	}

	for (size_t a = 0; a < subdirectories.size(); a++) {
		ret += subdirectories[a]->dumpStructure();
	}

	return ret;
}


/* ZipArchive::ZipArchive
 * ZipArchive class constructor
 *******************************************************************//*
ZipArchive::ZipArchive()
: Archive() {
	type = ARCHIVE_ZIP;
	directory = NULL;
}

/* ZipArchive::~ZipArchive
 * ZipArchive class destructor
 *******************************************************************//*
ZipArchive::~ZipArchive() {
}

int ZipArchive::entryIndex(ArchiveEntry* entry) {
	/*
	// Check entry is valid
	if (!entry)
		return -1;

	// Check entry is part of this archive
	if (entry->getParent() != this)
		return -1;

	// Slow recursive algorithm to get entry index
	return directory->getEntryIndex(entry);
	 */
//}

/*
ArchiveEntry* ZipArchive::getEntry(DWORD index) {
	return directory->getEntryAtIndex(index);
}

ArchiveEntry* ZipArchive::getEntry(string name) {
	wxFileName fn(name);

	ZipDirectory* dir = directory->getSubDir(fn.GetPath(true, wxPATH_UNIX));
	if (dir)
		return dir->getEntry(fn.GetFullName());
	else
		return NULL;
}

/* ZipArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************//*
string ZipArchive::getFileExtensionString() {
	return _T("Any Zip Format File (*.zip;*.pk3;*.jdf)|*.zip;*.pk3;*.jdf|Zip File (*.zip)|*.zip|Pk3 File (*.pk3)|*.pk3|JDF File (*.jdf)|*.jdf");
}

/* ZipArchive::openFile
 * Reads a zip format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************//*
bool ZipArchive::openFile(string filename) {
	wxZipEntry* entry;

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
	directory = new ZipDirectory(_T(""), NULL);

	// Go through all zip entries
	int entry_index = 0;
	entry = zip.GetNextEntry();
	while (entry) {
		if (!entry->IsDir()) {
			// Get the entry name as a wxFileName (so we can break it up)
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);

			// Create entry
			ArchiveEntry *new_entry = new ArchiveEntry(fn.GetFullName(), this);

			// Setup entry info
			new_entry->setSize(entry->GetSize());
			new_entry->setLoaded(false);
			setEntryZipIndex(new_entry, entry_index);
			new_entry->setState(0);

			// Add entry and directory to directory tree
			ZipDirectory* ndir = directory->addSubDir(fn.GetPath(true, wxPATH_UNIX));
			ndir->addEntry(new_entry);
		}
		else {
			// Add directory to the directory tree
			wxFileName fn(entry->GetName(wxPATH_UNIX), wxPATH_UNIX);
			directory->addSubDir(fn.GetPath(true, wxPATH_UNIX));
		}

		// Go to next entry in the zip file
		entry = zip.GetNextEntry();
		entry_index++;
	}
	
	wxLogMessage(directory->dumpStructure());

	// Setup variables
	this->filename = filename;
	modified = false;
	on_disk = true;

	return true;
}

/* ZipArchive::save
 * Saves the archive as a zip format file, if no filename is given,
 * the archive's current filename is used.
 * Returns true if successful, false otherwise
 *******************************************************************//*
bool ZipArchive::save(string filename) {
	// If no filename specified, just use the current filename
	if (filename == _T(""))
		filename = this->filename;

	// Create a backup copy if needed
	string bakfile = filename + _T(".bak");
	if (wxFileName::FileExists(filename)) {
		// Remove old backup file
		remove(chr(bakfile));

		// Copy current file contents to new backup file
		wxCopyFile(this->filename, bakfile);
	}
	else
		bakfile = this->filename;

	// Open the file
	wxFFileOutputStream out(filename);
	if (!out.IsOk()) {
		Global::error = _T("Unable to open file for saving. Make sure it isn't in use by another program.");
		return false;
	}

	// Open as a zip to save
	wxZipOutputStream zip(out, 9);
	if (!zip.IsOk()) {
		Global::error = _T("Unable to create zip");
		return false;
	}

	// Open old zip for copying, if it exists. This is used to copy any entries
	// that have been previously saved/compressed and are unmodified, to greatly
	// speed up zip file saving (especially for large zips)
	wxFFileInputStream in(bakfile);
	wxZipInputStream inzip(in);

	// Get a list of all entries in the old zip
	wxZipEntry **c_entries = new wxZipEntry*[inzip.GetTotalEntries()];
	for (int a = 0; a < inzip.GetTotalEntries(); a++)
		c_entries[a] = inzip.GetNextEntry();

	/*
	// Go through all archive entries
	for (size_t a = 0; a < entries.size(); a++) {
		if (!inzip.IsOk() || entries[a]->getState() > 0 || getEntryZipIndex(entries[a]) == -1) {
			// If the current entry has been changed, or doesn't exist in the old zip,
			// (re)compress it's data and write it to the zip
			wxZipEntry* zipentry = new wxZipEntry(getEntryFullPath(entries[a]));
			zip.PutNextEntry(zipentry);
			zip.Write(entries[a]->getData(), entries[a]->getSize());
		}
		else {
			// If the entry is unmodified and was already compressed in the old zip, just copy it over
			zip.CopyEntry(c_entries[getEntryZipIndex(entries[a])], inzip);
			inzip.Reset();
		}

		// Update entry info
		entries[a]->setState(0);
		setEntryZipIndex(entries[a], a);
	}
	 **/

/*
	// Clean up and update variables
	delete[] c_entries;
	zip.Close();
	this->filename = filename;
	on_disk = true;
	announce(_T("saved"));

	return true;
}

/* ZipArchive::close
 * 'Closes' the archive
 *******************************************************************//*
void ZipArchive::close() {
	// Delete directory (will delete all entries)
	if (directory)
		delete directory;

	// Announce
	announce(_T("close"));
}

/* ZipArchive::loadEntryData
 * Loads an entry's data from the zipfile
 * Returns true if successful, false otherwise
 *******************************************************************//*
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
	return 0;
}

/* ZipArchive::setEntryDirectory
 * Sets the specified entry's directory, if it is part of this
 * archive
 *******************************************************************//*
void ZipArchive::setEntryDirectory(ArchiveEntry* entry, string dir) {
	if (entry->getParent() == this) {
		entry->setExProp(_T("directory"), dir);
		entry->setState(1);
	}
}

/* ZipArchive::getEntryDirectory
 * Gets the specified entry's directory, returns an empty string if
 * the entry isn't part of this archive
 *******************************************************************//*
string ZipArchive::getEntryDirectory(ArchiveEntry* entry) {
	if (entry->getParent() != this)
		return _T("");
	else
		return entry->getExProp(_T("directory"));
}

/* ZipArchive::getEntryFullPath
 * Gets the specified entry's full path (directory+name+extension),
 * returns an empty string if the entry isn't part of this archive
 *******************************************************************//*
string ZipArchive::getEntryFullPath(ArchiveEntry* entry) {
	if (entry->getParent() != this)
		return _T("");
	else
		return getEntryDirectory(entry) + entry->getName();
}

ZipDirectory* ZipArchive::getEntryZipDir(ArchiveEntry* entry) {
	if (directory)
		return directory->dirWithEntry(entry);
	else
		return NULL;
}

/* ZipArchive::setEntryZipIndex
 * Sets the entry's zip index. This is the index of the entry in the
 * currently saved zip file, used to copy the compressed entry data
 * when saving if the entry data hasn't changed.
 *******************************************************************//*
void ZipArchive::setEntryZipIndex(ArchiveEntry* entry, int index) {
	if (entry->getParent() == this)
		entry->setExProp(_T("zip_index"), s_fmt(_T("%d"), index));
}

/* ZipArchive::getEntryZipIndex
 * Gets the entry's zip index. Returns -1 if the entry is not part
 * of this archive
 *******************************************************************//*
int ZipArchive::getEntryZipIndex(ArchiveEntry* entry) {
	if (entry->getParent() != this)
		return -1;
	else
		return atoi(entry->getExProp(_T("zip_index")).ToAscii());
}

/* ZipArchive::getDirectory
 * Returns a list of all entries within the specified directory
 *******************************************************************//*
vector<ArchiveEntry*> ZipArchive::getDirectory(string dir) {
	ZipDirectory* zdir = directory->getSubDir(dir);
	if (zdir)
		return zdir->getEntries();
	else
		return vector<ArchiveEntry*>();
}

/* ZipArchive::getSubDirs
 * Gets a list of all the subdirectories within the specified
 * directory
 *******************************************************************//*
vector<string> ZipArchive::getSubDirs(string dir) {
	// Init list
	vector<string> ret;

	ZipDirectory* zdir = directory->getSubDir(dir);

	if (zdir) {
		vector<ZipDirectory*> dirs = zdir->getSubDirs();
		for (size_t a = 0; a < dirs.size(); a++)
			ret.push_back(dirs[a]->getName());
	}

	// Return the list
	return ret;
}

bool ZipArchive::addEntry(ArchiveEntry* entry, DWORD position) {
	// Check valid entry
	if (!entry)
		return false;

	string dir = entry->getExProp(_T("directory"));
	ZipDirectory* zdir = directory->addSubDir(dir);
	zdir->addEntry(entry, position);

	// Update variables etc
	modified = true;
	entry->setParent(this);
	entry->setState(2);

	return false;
}

ArchiveEntry* ZipArchive::addNewEntry(string name, DWORD position) {
	wxFileName fn(name);
	ArchiveEntry* new_entry = new ArchiveEntry(fn.GetFullName(), this);
	new_entry->setExProp(_T("directory"), fn.GetPath(true, wxPATH_UNIX));
	addEntry(new_entry, position);
	return new_entry;
}

ArchiveEntry* ZipArchive::addExistingEntry(ArchiveEntry* entry, DWORD position, bool copy) {
	return NULL;
}

bool ZipArchive::removeEntry(ArchiveEntry* entry, bool delete_entry) {
	if (entry->getParent() != this)
		return false;

	return false;
}

bool ZipArchive::swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) {
	if (entry1->getParent() != this)
		return false;

	if (entry2->getParent() != this)
		return false;

	return false;
}

bool ZipArchive::renameEntry(ArchiveEntry* entry, string new_name) {
	if (entry->getParent() != this)
		return false;

	return false;
}

/* ZipArchive::detectMaps
 * Searches for any maps in the archive and adds them to the map list
 *******************************************************************//*
vector<Archive::mapdesc_t> ZipArchive::detectMaps() {
	vector<mapdesc_t> maps;

	return maps;
}
*/
