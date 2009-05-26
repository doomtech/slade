
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


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/


/* ArchiveManager::ArchiveManager
 * ArchiveManager class constructor
 *******************************************************************/
ArchiveManager::ArchiveManager() {
}

/* ArchiveManager::~ArchiveManager
 * ArchiveManager class destructor
 *******************************************************************/
ArchiveManager::~ArchiveManager() {
}

/* ArchiveManager::addArchive
 * Adds an archive to the archive list
 *******************************************************************/
bool ArchiveManager::addArchive(Archive* archive) {
	// Only add if archive is a valid pointer
	if (archive) {
		open_archives.push_back(archive);
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
		open_archives.push_back(new_archive);
		announce(_T("archive_opened"));
		return new_archive;
	} else {
		delete new_archive;
		return NULL;
	}
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
	announce(_T("archive_closed"));

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

	// If no wad is found with a matching filename, return false
	return false;
}

/* ArchiveManager::closeArchive
 * Closes the specified archive and removes it from the list, if it
 * exists in the list. Returns false if it doesn't exist, else true
 *******************************************************************/
bool ArchiveManager::closeArchive(Archive* archive) {
	// Go through all open wads
	for (int a = 0; a < (int) open_archives.size(); a++) {
		// If the wad exists in the list, remove it
		if (open_archives[a] == archive)
			return closeArchive(a);
	}

	// If the wad isn't in the list, return false
	return false;
}



/* Console Command - "list_archives"
 * Lists the filenames of all open archives
 *******************************************************************/
//CONSOLE_COMMAND(list_wads, 0,

void c_list_archives(vector<string> args) {
	Console::getInstance().logMessage(s_fmt(_T("%d Open Archives:"), ArchiveManager::getInstance().numArchives()));

	for (int a = 0; a < ArchiveManager::getInstance().numArchives(); a++) {
		Archive* archive = ArchiveManager::getInstance().getArchive(a);
		Console::getInstance().logMessage(s_fmt(_T("%d: \"%s\""), a + 1, archive->getFileName().c_str()));
	}
}
ConsoleCommand list_archives(_T("list_archives"), &c_list_archives, 0);
//})
