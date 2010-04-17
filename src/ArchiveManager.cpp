
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ArchiveManager.cpp
 * Description: ArchiveManager class. Manages all open archives and
 *              the interactions between them.
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
#include "ArchiveManager.h"
#include "WadArchive.h"
#include "ZipArchive.h"
#include "Console.h"
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
ArchiveManager* ArchiveManager::instance = NULL;
CVAR(String, base_resource_path, "", CVAR_SAVE)


/*******************************************************************
 * ARCHIVEMANAGER CLASS FUNCTIONS
 *******************************************************************/

/* ArchiveManager::ArchiveManager
 * ArchiveManager class constructor
 *******************************************************************/
ArchiveManager::ArchiveManager() {
	// Init variables
	res_archive_open = true;
	base_resource_archive = NULL;

	// Find slade3.pk3 directory
	string dir_slade_pk3 = appPath(_T("slade.pk3"), DIR_DATA);
	if (!wxFileExists(dir_slade_pk3))
		dir_slade_pk3 = appPath(_T("slade.pk3"), DIR_APP);
	if (!wxFileExists(dir_slade_pk3))
		dir_slade_pk3 = appPath(_T("slade.pk3"), DIR_USER);
	if (!wxFileExists(dir_slade_pk3))
		dir_slade_pk3 = _T("slade.pk3");

	// Open slade.pk3
	program_resource_archive = new ZipArchive();
	if (!program_resource_archive->open(dir_slade_pk3)) {
		wxLogMessage(_T("Unable to find slade.pk3!"));
		res_archive_open = false;
	}

	// Open base resource archive (if any)
	openBaseResource(base_resource_path);
}

/* ArchiveManager::~ArchiveManager
 * ArchiveManager class destructor
 *******************************************************************/
ArchiveManager::~ArchiveManager() {
	clearAnnouncers();
	if (program_resource_archive) delete program_resource_archive;
	if (base_resource_archive) delete base_resource_archive;
}

/* ArchiveManager::addArchive
 * Adds an archive to the archive list
 *******************************************************************/
bool ArchiveManager::addArchive(Archive* archive) {
	// Only add if archive is a valid pointer
	if (archive) {
		// Add to the list
		archive_t n_archive;
		n_archive.archive = archive;
		n_archive.resource = false;
		open_archives.push_back(n_archive);

		// Listen to the archive
		listenTo(archive);

		// Announce the addition
		announce(_T("archive_added"));

		return true;
	} else
		return false;
}

/* ArchiveManager::getArchive
 * Returns the archive at the index specified
 * (NULL if it doesn't exist)
 *******************************************************************/
Archive* ArchiveManager::getArchive(int index) {
	// Check that index is valid
	if (index < 0 || index >= (int) open_archives.size())
		return NULL;
	else
		return open_archives[index].archive;
}

/* ArchiveManager::getArchive
 * Returns the archive with the specified filename
 * (NULL if it doesn't exist)
 *******************************************************************/
Archive* ArchiveManager::getArchive(string filename) {
	// Go through all open archives
	for (int a = 0; a < (int) open_archives.size(); a++) {
		// If the filename matches, return it
		if (open_archives[a].archive->getFileName().compare(filename) == 0)
			return open_archives[a].archive;
	}

	// If no archive is found with a matching filename, return NULL
	return NULL;
}

/* ArchiveManager::openArchive
 * Opens and adds a archive to the list, returns a pointer to the
 * newly opened and added archive, or NULL if an error occurred
 *******************************************************************/
Archive* ArchiveManager::openArchive(string filename) {
	Archive* new_archive = NULL;

	// Check that the file isn't already open
	if (getArchive(filename)) {
		Global::error = _T("Archive is already open");
		return NULL;
	}

	// Determine file format
	if (WadArchive::isWadArchive(filename))
		new_archive = new WadArchive();
	else if (ZipArchive::isZipArchive(filename))
		new_archive = new ZipArchive();
	else
		return NULL;	// Unsupported format

	// If it opened successfully, add it to the list & return it,
	// Otherwise, delete it and return NULL
	if (new_archive->open(filename)) {
		addArchive(new_archive);
		return new_archive;
	} else {
		wxLogMessage(_T("Error: ") + Global::error);
		delete new_archive;
		return NULL;
	}
}

/* ArchiveManager::openArchive
 * Same as the above function, except it opens from an ArchiveEntry
 *******************************************************************/
Archive* ArchiveManager::openArchive(ArchiveEntry* entry) {
	Archive* new_archive = NULL;

	// Check entry was given
	if (!entry)
		return NULL;

	// Check if the entry is already opened
	for (size_t a = 0; a < open_archives.size(); a++) {
		if (open_archives[a].archive->getParent() == entry)
			return open_archives[a].archive;
	}

	// Check entry type
	if (WadArchive::isWadArchive(entry->getMCData()))
		new_archive = new WadArchive();
	else if (ZipArchive::isZipArchive(entry->getMCData()))
		new_archive = new ZipArchive();
	else
		return NULL;	// Unsupported format

	// If it opened successfully, add it to the list & return it,
	// Otherwise, delete it and return NULL
	if (new_archive->open(entry)) {
		// Add to parent's child list if parent is open in the manager (it should be)
		int index_parent = -1;
		if (entry->getParent())
			index_parent = archiveIndex(entry->getParent());
		if (index_parent >= 0)
			open_archives[index_parent].open_children.push_back(new_archive);

		// Add the new archive and return it
		addArchive(new_archive);
		return new_archive;
	} else {
		wxLogMessage(_T("Error: ") + Global::error);
		delete new_archive;
		return NULL;
	}
}

/* ArchiveManager::newArchive
 * Creates a new archive of the specified format and adds it to the
 * list of open archives. Returns the created archive, or NULL if an
 * invalid archive type was given
 *******************************************************************/
Archive* ArchiveManager::newArchive(uint8_t type) {
	// Init variables
	Archive* new_archive = NULL;
	string format_str = _T("");

	// Create a new archive depending on the type specified
	switch (type) {
		case ARCHIVE_WAD:
			new_archive = new WadArchive();
			format_str = _T("wad");
			break;
		case ARCHIVE_ZIP:
			new_archive = new ZipArchive();
			format_str = _T("zip");
			break;
	}

	// If the archive was created, set its filename and add it to the list
	if (new_archive) {
		new_archive->setFileName(s_fmt(_T("UNSAVED (%s)"), format_str.c_str()));
		addArchive(new_archive);
	}

	// Return the created archive, if any
	return new_archive;
}

/* ArchiveManager::closeArchive
 * Closes the archive at index, and removes it from the list if the
 * index is valid. Returns false on invalid index, true otherwise
 *******************************************************************/
bool ArchiveManager::closeArchive(int index) {
	// Check for invalid index
	if (index < 0 || index >= (int) open_archives.size())
		return false;

	// Close any open child archives
	for (size_t a = 0; a < open_archives[index].open_children.size(); a++) {
		int ci = archiveIndex(open_archives[index].open_children[a]);
		if (ci >= 0)
			closeArchive(ci);
	}

	// Close the archive
	open_archives[index].archive->close();

	// Delete the archive object
	delete open_archives[index].archive;

	// Remove the archive at index from the list
	open_archives.erase(open_archives.begin() + index);

	// Announce it
	MemChunk mc;
	mc.write(&index, sizeof(int));
	announce(_T("archive_closed"), mc);

	return true;
}

/* ArchiveManager::closeArchive
 * Finds the archive with a matching filename, deletes it and removes
 * it from the list. Returns false if it doesn't exist or can't be
 * removed, true otherwise
 *******************************************************************/
bool ArchiveManager::closeArchive(string filename) {
	// Go through all open archives
	for (int a = 0; a < (int) open_archives.size(); a++) {
		// If the filename matches, remove it
		if (open_archives[a].archive->getFileName().compare(filename) == 0)
			return closeArchive(a);
	}

	// If no archive is found with a matching filename, return false
	return false;
}

/* ArchiveManager::closeArchive
 * Closes the specified archive and removes it from the list, if it
 * exists in the list. Returns false if it doesn't exist, else true
 *******************************************************************/
bool ArchiveManager::closeArchive(Archive* archive) {
	// Go through all open archives
	for (int a = 0; a < (int) open_archives.size(); a++) {
		// If the archive exists in the list, remove it
		if (open_archives[a].archive == archive)
			return closeArchive(a);
	}

	// If the archive isn't in the list, return false
	return false;
}

/* ArchiveManager::closeAll
 * Closes all opened archives
 *******************************************************************/
void ArchiveManager::closeAll() {
	// Close the first archive in the list until no archives are open
	while (open_archives.size() > 0)
		closeArchive(0);
}

/* ArchiveManager::archiveIndex
 * Returns the index in the list of the given archive, or -1 if the
 * archive doesn't exist in the list
 *******************************************************************/
int ArchiveManager::archiveIndex(Archive* archive) {
	// Go through all open archives
	for (size_t a = 0; a < open_archives.size(); a++) {
		// If the archive we're looking for is this one, return the index
		if (open_archives[a].archive == archive)
			return (int)a;
	}

	// If we get to here the archive wasn't found, so return -1
	return -1;
}

string ArchiveManager::getArchiveExtensionsString(bool wad, bool zip, bool pk3, bool jdf) {
	string ext_wad = _T("*.wad;*.WAD;*.Wad");
	string ext_zip = _T("*.zip;*.ZIP;*.Zip");
	string ext_pk3 = _T("*.pk3;*.PK3;*.Pk3");
	string ext_jdf = _T("*.jdf;*.JDF;*.Jdf");
	
	// Create extensions string
	string extensions = s_fmt(_T("Any Supported File (*.wad; *.zip; *.pk3; *.jdf)|%s;%s;%s;%s"), ext_wad.c_str(), ext_zip.c_str(), ext_pk3.c_str(), ext_jdf.c_str());
	extensions += s_fmt(_T("|Doom Wad files (*.wad)|%s"), ext_wad.c_str());
	extensions += s_fmt(_T("|Zip files (*.zip)|%s"), ext_zip.c_str());
	extensions += s_fmt(_T("|Pk3 (zip) files (*.pk3)|%s"), ext_pk3.c_str());
	extensions += s_fmt(_T("|JDF (zip) files (*.jdf)|%s"), ext_jdf.c_str());
	
	return extensions;
}

bool ArchiveManager::openBaseResource(string filename) {
	// Close/delete current base resource archive
	if (base_resource_archive) {
		delete base_resource_archive;
		base_resource_archive = NULL;
	}

	// Check filename was given
	if (filename.IsEmpty())
		return false;

	// Create archive based on file type
	if (WadArchive::isWadArchive(filename))
		base_resource_archive = new WadArchive();
	else if (ZipArchive::isZipArchive(filename))
		base_resource_archive = new ZipArchive();
	else
		return false;

	// Attempt to open the file
	if (base_resource_archive->open(filename)) {
		base_resource_path = filename;
		return true;
	}
	else {
		delete base_resource_archive;
		base_resource_archive = NULL;
		return false;
	}
}

string ArchiveManager::baseResourcePath(uint32_t index) {
	// Check index
	if (index >= base_resource_list.size())
		return _T("");

	return base_resource_list[index];
}

void ArchiveManager::addBaseResourcePath(string path) {
	// Add the path
	base_resource_list.Add(path);
	
	// Announce
	announce(_T("base_resource_path_added"));
}

ArchiveEntry* ArchiveManager::getResourceEntry(string name) {
	// Go through all open archives
	for (size_t a = 0; a < open_archives.size(); a++) {
		// If it isn't a resource archive, skip it
		if (!open_archives[a].resource)
			continue;

		// Try to find the entry in the archive
		ArchiveEntry* entry = open_archives[a].archive->getEntry(name);
		if (entry)
			return entry;
	}

	// If entry isn't found yet, search the base resource archive
	if (base_resource_archive)
		return base_resource_archive->getEntry(name);

	return NULL;
}

/* ArchivePanel::openTextureEditor
 * Announces that the texture editor for archive [index] should be
 * opened
 *******************************************************************/
bool ArchiveManager::openTextureEditor(uint32_t index) {
	// Check index
	if (index >= open_archives.size())
		return false;

	// Announce
	MemChunk mc(4);
	mc.write(&index, 4);
	announce(_T("open_tex_editor"), mc);

	return true;
}


/* ArchivePanel::onAnnouncement
 * Called when an announcement is recieved from one of the archives
 * in the list
 *******************************************************************/
void ArchiveManager::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// Reset event data for reading
	event_data.seek(0, SEEK_SET);

	// Check that the announcement came from an archive in the list
	int32_t index = archiveIndex((Archive*)announcer);
	if (index >= 0) {
		// If the archive was saved
		if (event_name == _T("saved")) {
			MemChunk mc;
			mc.write(&index, 4);
			announce(_T("archive_saved"), mc);
		}

		// If the archive was modified
		if (event_name == _T("modified") || event_name == _T("entry_modified")) {
			MemChunk mc;
			mc.write(&index, 4);
			announce(_T("archive_modified"), mc);
		}
	}
}


/* Console Command - "list_archives"
 * Lists the filenames of all open archives
 *******************************************************************/
void c_list_archives(vector<string> args) {
	wxLogMessage(s_fmt(_T("%d Open Archives:"), theArchiveManager->numArchives()));

	for (int a = 0; a < theArchiveManager->numArchives(); a++) {
		Archive* archive = theArchiveManager->getArchive(a);
		wxLogMessage(s_fmt(_T("%d: \"%s\""), a + 1, archive->getFileName().c_str()));
	}
}
ConsoleCommand am_list_archives(_T("list_archives"), &c_list_archives, 0);

/* Console Command - "open"
 * Attempts to open each given argument (filenames)
 *******************************************************************/
void c_open(vector<string> args) {
	for (size_t a = 0; a < args.size(); a++)
		theArchiveManager->openArchive(args[a]);
}
ConsoleCommand am_open(_T("open"), &c_open, 1);
