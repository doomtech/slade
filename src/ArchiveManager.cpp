
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


/*******************************************************************
 * ARCHIVEMANAGER CLASS FUNCTIONS
 *******************************************************************/

/* ArchiveManager::ArchiveManager
 * ArchiveManager class constructor
 *******************************************************************/
ArchiveManager::ArchiveManager() {
	// Init variables
	res_archive_open = true;

	// Find slade3.pk3 directory
	string dir_slade_pk3 = appPath(_T("slade.pk3"), DIR_DATA);
	if (!wxFileExists(dir_slade_pk3))
		dir_slade_pk3 = appPath(_T("slade.pk3"), DIR_APP);
	if (!wxFileExists(dir_slade_pk3))
		dir_slade_pk3 = appPath(_T("slade.pk3"), DIR_USER);
	if (!wxFileExists(dir_slade_pk3))
		dir_slade_pk3 = _T("slade.pk3");

	// Open slade.pk3
	resource_archive = new ZipArchive();
	if (!resource_archive->openFile(dir_slade_pk3)) {
		wxLogMessage(_T("Unable to find slade.pk3!"));
		res_archive_open = false;
	}
}

/* ArchiveManager::~ArchiveManager
 * ArchiveManager class destructor
 *******************************************************************/
ArchiveManager::~ArchiveManager() {
	clearAnnouncers();
	if (instance)
		delete instance;
}

/* ArchiveManager::addArchive
 * Adds an archive to the archive list
 *******************************************************************/
bool ArchiveManager::addArchive(Archive* archive) {
	// Only add if archive is a valid pointer
	if (archive) {
		// Add to the list
		open_archives.push_back(archive);

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
		return open_archives[index];
}

/* ArchiveManager::getArchive
 * Returns the archive with the specified filename
 * (NULL if it doesn't exist)
 *******************************************************************/
Archive* ArchiveManager::getArchive(string filename) {
	// Go through all open archives
	for (int a = 0; a < (int) open_archives.size(); a++) {
		// If the filename matches, return it
		if (open_archives[a]->getFileName().compare(filename) == 0)
			return open_archives[a];
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

	// Create either a wad or zip file, depending on filename extension
	wxFileName fn(filename);

	if (!fn.GetExt().CmpNoCase(_T("wad"))) // Wad File
		new_archive = new WadArchive();
	else if (!fn.GetExt().CmpNoCase(_T("zip")) || !fn.GetExt().CmpNoCase(_T("pk3"))|| !fn.GetExt().CmpNoCase(_T("jdf"))) // Zip/Pk3/JDF file
		new_archive = new ZipArchive();
	else
		return NULL; // Unsupported format

	// If it opened successfully, add it to the list & return it,
	// Otherwise, delete it and return NULL
	if (new_archive->openFile(filename)) {
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

	// If the archive was created, set it's filename and add it to the list
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

	// Close the archive
	open_archives[index]->close();

	// Delete the archive object
	delete open_archives[index];

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
		if (open_archives[a]->getFileName().compare(filename) == 0)
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
		if (open_archives[a] == archive)
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
		if (open_archives[a] == archive)
			return (int)a;
	}

	// If we get to here the archive wasn't found, so return -1
	return -1;
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
