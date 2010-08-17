
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
#include "LibArchive.h"
#include "DatArchive.h"
#include "ResArchive.h"
#include "Console.h"
#include "SplashWindow.h"
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
ArchiveManager* ArchiveManager::instance = NULL;
CVAR(Int, base_resource, -1, CVAR_SAVE)


/*******************************************************************
 * ARCHIVEMANAGER CLASS FUNCTIONS
 *******************************************************************/

/* ArchiveManager::ArchiveManager
 * ArchiveManager class constructor
 *******************************************************************/
ArchiveManager::ArchiveManager() {
	// Init variables
	res_archive_open = false;
	base_resource_archive = NULL;
}

/* ArchiveManager::~ArchiveManager
 * ArchiveManager class destructor
 *******************************************************************/
ArchiveManager::~ArchiveManager() {
	clearAnnouncers();
	if (program_resource_archive) delete program_resource_archive;
	if (base_resource_archive) delete base_resource_archive;
}

bool ArchiveManager::init() {
	// Find slade3.pk3 directory
	string dir_slade_pk3 = appPath("slade.pk3", DIR_DATA);
	if (!wxFileExists(dir_slade_pk3))
		dir_slade_pk3 = appPath("slade.pk3", DIR_APP);
	if (!wxFileExists(dir_slade_pk3))
		dir_slade_pk3 = appPath("slade.pk3", DIR_USER);
	if (!wxFileExists(dir_slade_pk3))
		dir_slade_pk3 = "slade.pk3";

	// Open slade.pk3
	program_resource_archive = new ZipArchive();
	if (!program_resource_archive->open(dir_slade_pk3)) {
		wxLogMessage("Unable to find slade.pk3!");
		res_archive_open = false;
	}
	else
		res_archive_open = true;

	// Open base resource archive (if any)
	bool bro = openBaseResource((int)base_resource);

	return res_archive_open && bro;
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
		announce("archive_added");

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
		if (open_archives[a].archive->getFilename().compare(filename) == 0)
			return open_archives[a].archive;
	}

	// If no archive is found with a matching filename, return NULL
	return NULL;
}

/* ArchiveManager::openArchive
 * Opens and adds a archive to the list, returns a pointer to the
 * newly opened and added archive, or NULL if an error occurred
 *******************************************************************/
Archive* ArchiveManager::openArchive(string filename, bool manage) {
	Archive* new_archive = getArchive(filename);

	wxLogMessage(s_fmt("Opening archive %s", filename));

	// If the archive is already open, just return it
	if (new_archive) {
		// Announce open
		MemChunk mc;
		uint32_t index = archiveIndex(new_archive);
		mc.write(&index, 4);
		announce("archive_opened", mc);

		return new_archive;
	}

	// Determine file format
	if (WadArchive::isWadArchive(filename))
		new_archive = new WadArchive();
	else if (ZipArchive::isZipArchive(filename))
		new_archive = new ZipArchive();
	else if (ResArchive::isResArchive(filename))
		new_archive = new ResArchive();
	else if (DatArchive::isDatArchive(filename))
		new_archive = new DatArchive();
	else if (LibArchive::isLibArchive(filename))
		new_archive = new LibArchive();
	else
		return NULL;	// Unsupported format

	// If it opened successfully, add it to the list if needed & return it,
	// Otherwise, delete it and return NULL
	if (new_archive->open(filename)) {
		if (manage) {
			// Add the archive
			addArchive(new_archive);

			// Announce open
			MemChunk mc;
			uint32_t index = archiveIndex(new_archive);
			mc.write(&index, 4);
			announce("archive_opened", mc);

			// Add to recent files
			addRecentFile(filename);
		}

		// Return the opened archive
		return new_archive;
	}
	else {
		wxLogMessage("Error: " + Global::error);
		delete new_archive;
		return NULL;
	}
}

/* ArchiveManager::openArchive
 * Same as the above function, except it opens from an ArchiveEntry
 *******************************************************************/
Archive* ArchiveManager::openArchive(ArchiveEntry* entry, bool manage) {
	Archive* new_archive = NULL;

	// Check entry was given
	if (!entry)
		return NULL;

	// Check if the entry is already opened
	for (size_t a = 0; a < open_archives.size(); a++) {
		if (open_archives[a].archive->getParent() == entry) {
			// Announce open
			MemChunk mc;
			uint32_t index = archiveIndex(open_archives[a].archive);
			mc.write(&index, 4);
			announce("archive_opened", mc);

			return open_archives[a].archive;
		}
	}

	// Check entry type
	if (WadArchive::isWadArchive(entry->getMCData()))
		new_archive = new WadArchive();
	else if (ZipArchive::isZipArchive(entry->getMCData()))
		new_archive = new ZipArchive();
	else if (ResArchive::isResArchive(entry->getMCData()))
		new_archive = new ResArchive();
	else if (LibArchive::isLibArchive(entry->getMCData()))
		new_archive = new LibArchive();
	else if (DatArchive::isDatArchive(entry->getMCData()))
		new_archive = new DatArchive();
	else
		return NULL;	// Unsupported format

	// If it opened successfully, add it to the list & return it,
	// Otherwise, delete it and return NULL
	if (new_archive->open(entry)) {
		if (manage) {
			// Add to parent's child list if parent is open in the manager (it should be)
			int index_parent = -1;
			if (entry->getParent())
				index_parent = archiveIndex(entry->getParent());
			if (index_parent >= 0)
				open_archives[index_parent].open_children.push_back(new_archive);

			// Add the new archive
			addArchive(new_archive);

			// Announce open
			MemChunk mc;
			uint32_t index = archiveIndex(new_archive);
			mc.write(&index, 4);
			announce("archive_opened", mc);
		}

		return new_archive;
	} else {
		wxLogMessage("Error: " + Global::error);
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
	string format_str = "";

	// Create a new archive depending on the type specified
	switch (type) {
		case ARCHIVE_WAD:
			new_archive = new WadArchive();
			format_str = "wad";
			break;
		case ARCHIVE_ZIP:
			new_archive = new ZipArchive();
			format_str = "zip";
			break;
		case ARCHIVE_LIB:
			new_archive = new LibArchive();
			format_str = "lib";
			break;
		case ARCHIVE_DAT:
			new_archive = new DatArchive();
			format_str = "dat";
			break;
		case ARCHIVE_RES:
			new_archive = new ResArchive();
			format_str = "res";
			break;
	}

	// If the archive was created, set its filename and add it to the list
	if (new_archive) {
		new_archive->setFilename(s_fmt("UNSAVED (%s)", format_str.c_str()));
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

	// Announce archive closing
	MemChunk mc;
	mc.write(&index, sizeof(int));
	announce("archive_closing", mc);

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

	// Announce closed
	announce("archive_closed", mc);

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
		if (open_archives[a].archive->getFilename().compare(filename) == 0)
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
	string ext_wad = "*.wad;*.WAD;*.Wad";
	string ext_zip = "*.zip;*.ZIP;*.Zip";
	string ext_pk3 = "*.pk3;*.PK3;*.Pk3";
	string ext_jdf = "*.jdf;*.JDF;*.Jdf";
	string ext_dat = "*.dat;*.DAT;*.Dat";
	string ext_chd = "*.cd;*.CD;*.Cd;*.hd;*.HD;*.Hd";
	string ext_lib = "*.lib;*.LIB;*.Lib";
	string ext_res = "*.res;*.RES;*.Res";

	// Create extensions string
	string extensions = s_fmt("Any Supported File (*.wad; *.zip; *.pk3; *.jdf)|%s;%s;%s;%s;%s;%s;%s;%s",
		ext_wad.c_str(), ext_zip.c_str(), ext_pk3.c_str(), ext_jdf.c_str(),
		ext_dat.c_str(), ext_lib.c_str(), ext_chd.c_str(), ext_res.c_str());
	extensions += s_fmt("|Doom Wad files (*.wad)|%s", ext_wad.c_str());
	extensions += s_fmt("|Zip files (*.zip)|%s", ext_zip.c_str());
	extensions += s_fmt("|Pk3 (zip) files (*.pk3)|%s", ext_pk3.c_str());
	extensions += s_fmt("|JDF (zip) files (*.jdf)|%s", ext_jdf.c_str());
	extensions += s_fmt("|Data (dat) files (*.dat)|%s", ext_dat.c_str());
	extensions += s_fmt("|CD/HD (cd/hd) files (*.cd; *.hd)|%s", ext_chd.c_str());
	extensions += s_fmt("|Library (lib) files (*.lib)|%s", ext_lib.c_str());
	extensions += s_fmt("|Resource (res) files (*.res)|%s", ext_res.c_str());

	return extensions;
}

void ArchiveManager::addBaseResourcePath(string path) {
	// First check the path doesn't already exist
	for (unsigned a = 0; a < base_resource_paths.size(); a++) {
		if (s_cmp(base_resource_paths[a], path))
			return;
	}

	// Add it
	base_resource_paths.push_back(path);

	// Announce
	announce("base_resource_path_added");
}

void ArchiveManager::removeBaseResourcePath(unsigned index) {
	// Check index
	if (index >= base_resource_paths.size())
		return;

	// Unload base resource if removed is open
	if (index == base_resource)
		openBaseResource(-1);

	// Modify base_resource cvar if needed
	else if (base_resource > (signed)index)
		base_resource = base_resource - 1;

	// Remove the path
	base_resource_paths.erase(base_resource_paths.begin() + index);

	// Announce
	announce("base_resource_path_removed");
}

string ArchiveManager::getBaseResourcePath(unsigned index) {
	// Check index
	if (index >= base_resource_paths.size())
		return "INVALID INDEX";

	return base_resource_paths[index];
}

bool ArchiveManager::openBaseResource(int index) {
	// Check we're opening a different archive
	if (base_resource_archive && base_resource == index)
		return true;

	// Close/delete current base resource archive
	if (base_resource_archive) {
		delete base_resource_archive;
		base_resource_archive = NULL;
	}

	// Check index
	if (index < 0 || (unsigned)index >= base_resource_paths.size()) {
		base_resource = -1;
		announce("base_resource_changed");
		return false;
	}

	// Create archive based on file type
	string filename = base_resource_paths[index];
	if (WadArchive::isWadArchive(filename))
		base_resource_archive = new WadArchive();
	else if (ZipArchive::isZipArchive(filename))
		base_resource_archive = new ZipArchive();
	else
		return false;

	// Attempt to open the file
	theSplashWindow->show(s_fmt("Opening %s...", chr(filename)), true);
	if (base_resource_archive->open(filename)) {
		base_resource = index;
		theSplashWindow->hide();
		announce("base_resource_changed");
		return true;
	}
	else {
		delete base_resource_archive;
		base_resource_archive = NULL;
		theSplashWindow->hide();
		announce("base_resource_changed");
		return false;
	}
}

ArchiveEntry* ArchiveManager::getResourceEntry(string name, Archive* ignore) {
	// Go through all open archives
	for (size_t a = 0; a < open_archives.size(); a++) {
		// If it isn't a resource archive, skip it
		if (!open_archives[a].resource)
			continue;

		// If it's being ignored, skip it
		if (open_archives[a].archive == ignore)
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

ArchiveEntry* ArchiveManager::findResourceEntry(Archive::search_options_t& options, Archive* ignore) {
	// Go through all open archives
	for (size_t a = 0; a < open_archives.size(); a++) {
		// If it isn't a resource archive, skip it
		if (!open_archives[a].resource)
			continue;

		// If it's being ignored, skip it
		if (open_archives[a].archive == ignore)
			continue;

		// Try to find the entry in the archive
		ArchiveEntry* entry = open_archives[a].archive->findLast(options);
		if (entry)
			return entry;
	}

	// If entry isn't found yet, search the base resource archive
	if (base_resource_archive)
		return base_resource_archive->findLast(options);

	return NULL;
}

vector<ArchiveEntry*> ArchiveManager::findAllResourceEntries(Archive::search_options_t& options, Archive* ignore) {
	vector<ArchiveEntry*> ret;

	// Search the base resource archive first
	if (base_resource_archive) {
		vector<ArchiveEntry*> vec = base_resource_archive->findAll(options);
		ret.insert(ret.end(), vec.begin(), vec.end());
	}

	// Go through all open archives
	for (size_t a = 0; a < open_archives.size(); a++) {
		// If it isn't a resource archive, skip it
		if (!open_archives[a].resource)
			continue;

		// If it's being ignored, skip it
		if (open_archives[a].archive == ignore)
			continue;

		// Add matching entries from this archive
		vector<ArchiveEntry*> vec = open_archives[a].archive->findAll(options);
		ret.insert(ret.end(), vec.begin(), vec.end());
	}

	return ret;
}

string ArchiveManager::recentFile(unsigned index) {
	// Check index
	if (index >= recent_files.size())
		return "";

	return recent_files[index];
}

void ArchiveManager::addRecentFile(string path) {
	// Check the path is valid
	if (!wxFileName::FileExists(path))
		return;

	// Check if the file is already in the list
	for (unsigned a = 0; a < recent_files.size(); a++) {
		if (recent_files[a] == path) {
			// Move this file to the top of the list
			recent_files.erase(recent_files.begin() + a);
			recent_files.insert(recent_files.begin(), path);

			// Announce
			announce("recent_files_changed");

			return;
		}
	}

	// Add the file to the top of the list
	recent_files.insert(recent_files.begin(), path);

	// List is 8 files max
	while (recent_files.size() > 8)
		recent_files.pop_back();

	// Announce
	announce("recent_files_changed");
}

void ArchiveManager::addRecentFiles(vector<string> paths) {
	// Mute annoucements
	setMuted(true);

	// Clear existing list
	recent_files.clear();

	// Add the files
	for (size_t a = 0; a < paths.size(); ++a) {
		addRecentFile(paths[a]);
	}

	// Announce
	setMuted(false);
	announce("recent_files_changed");
}

/* ArchiveManager::openTextureEditor
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
	announce("open_tex_editor", mc);

	return true;
}


/* ArchiveManager::onAnnouncement
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
		if (event_name == "saved") {
			MemChunk mc;
			mc.write(&index, 4);
			announce("archive_saved", mc);
		}

		// If the archive was modified
		if (event_name == "modified" || event_name == "entry_modified") {
			MemChunk mc;
			mc.write(&index, 4);
			announce("archive_modified", mc);
		}
	}
}


/* Console Command - "list_archives"
 * Lists the filenames of all open archives
 *******************************************************************/
CONSOLE_COMMAND (list_archives, 0) {
	wxLogMessage(s_fmt("%d Open Archives:", theArchiveManager->numArchives()));

	for (int a = 0; a < theArchiveManager->numArchives(); a++) {
		Archive* archive = theArchiveManager->getArchive(a);
		wxLogMessage(s_fmt("%d: \"%s\"", a + 1, archive->getFilename().c_str()));
	}
}

/* Console Command - "open"
 * Attempts to open each given argument (filenames)
 *******************************************************************/
void c_open(vector<string> args) {
	for (size_t a = 0; a < args.size(); a++)
		theArchiveManager->openArchive(args[a]);
}
ConsoleCommand am_open("open", &c_open, 1); // Can't use the macro with this name
