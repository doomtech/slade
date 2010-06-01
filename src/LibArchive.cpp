
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    LibArchive.cpp
 * Description: LibArchive, archive class to handle shadow.lib
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
#include "LibArchive.h"
#include "SplashWindow.h"
#include <wx/log.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/

/*******************************************************************
 * LIBARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* LibArchive::LibArchive
 * LibArchive class constructor
 *******************************************************************/
LibArchive::LibArchive()
: Archive(ARCHIVE_LIB) {
}

/* LibArchive::~LibArchive
 * LibArchive class destructor
 *******************************************************************/
LibArchive::~LibArchive() {
	// Delete entries
	for (size_t a = 0; a < entries.size(); a++)
		delete entries[a];
}

/* LibArchive::entryIndex
 * Returns the given entry's index in the Archive entry list.
 * Returns -1 if the entry doesn't exist in the Archive.
 *******************************************************************/
int LibArchive::entryIndex(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return -1;

	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return a;
	}

	return -1;
}

/* LibArchive::getEntry
 * Returns the entry at the index specified,
 * or NULL if the index is invalid
 *******************************************************************/
ArchiveEntry* LibArchive::getEntry(uint32_t index) {
	// Check index
	if (index >= entries.size())
		return NULL;

	return entries[index];
}

/* LibArchive::getEntry
 * Returns the first entry with the specified name,
 * or NULL if no entry exists with that name
 *******************************************************************/
ArchiveEntry* LibArchive::getEntry(string name) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (getEntry(a)->getName() == name)
			return getEntry(a);
	}

	return NULL;
}

/* LibArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string LibArchive::getFileExtensionString() {
	return "Shadowcaster Lib Files (*.lib)|*.lib";
}

/* LibArchive::getFormat
 * Gives the "archive_lib" string
 *******************************************************************/
string LibArchive::getFormat() {
	return "archive_lib";
}

/* LibArchive::open
 * Reads a wad format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool LibArchive::open(string filename) {
	// Read the file into a MemChunk
	MemChunk mc;
	if (!mc.importFile(filename)) {
		Global::error = "Unable to open file. Make sure it isn't in use by another program.";
		return false;
	}

	// Load from MemChunk
	if (open(mc)) {
		// Update variables
		this->filename = filename;
		this->on_disk = true;

		return true;
	}
	else
		return false;
}

/* LibArchive::open
 * Reads wad format data from an ArchiveEntry
 * Returns true if successful, false otherwise
 *******************************************************************/
bool LibArchive::open(ArchiveEntry* entry) {
	// Load from entry's data
	if (entry && open(entry->getMCData())) {
		// Update variables and return success
		parent = entry;
		parent->lock();
		return true;
	}
	else
		return false;
}

/* LibArchive::open
 * Reads wad format data from a MemChunk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool LibArchive::open(MemChunk& mc) {
	// Check data was given
	if (!mc.hasData())
		return false;

	// Read lib footer
	mc.seek(2, SEEK_END);
	uint32_t num_lumps = 0;
	mc.read(&num_lumps, 2);		// Size
	num_lumps = wxINT16_SWAP_ON_BE(num_lumps);
	uint32_t dir_offset = mc.getSize() - (2 + (num_lumps * 21));

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Read the directory
	mc.seek(dir_offset, SEEK_SET);
	theSplashWindow->setProgressMessage("Reading lib archive data");
	for (uint32_t d = 0; d < num_lumps; d++) {
		// Update splash window progress
		theSplashWindow->setProgress(((float)d / (float)num_lumps));

		// Read lump info
		char myname[13] = "";
		uint32_t offset = 0;
		uint32_t size = 0;
		uint8_t dummy = 0;

		mc.read(&size, 4);		// Size
		mc.read(&offset, 4);	// Offset
		mc.read(myname, 12);	// Name
		mc.read(&dummy, 1);		// Separator
		myname[12] = '\0';

		// Byteswap values for big endian if needed
		offset = wxINT32_SWAP_ON_BE(offset);
		size = wxINT32_SWAP_ON_BE(size);

		// If the lump data goes past the directory,
		// the wadfile is invalid
		if (offset + size > dir_offset) {
			wxLogMessage("LibArchive::open: Lib archive is invalid or corrupt");
			Global::error = "Archive is invalid and/or corrupt";
			setMuted(false);
			return false;
		}

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(wxString::FromAscii(myname), size, this);
		nlump->setLoaded(false);
		nlump->extraProp("Offset") = (int)offset;
		nlump->setState(0);

		// Add to entry list
		entries.push_back(nlump);
	}

	// Detect all entry types
	MemChunk edata;
	theSplashWindow->setProgressMessage("Detecting entry types");
	for (size_t a = 0; a < entries.size(); a++) {
		// Update splash window progress
		theSplashWindow->setProgress((((float)a / (float)num_lumps)));

		// Get entry
		ArchiveEntry* entry = entries[a];

		// Read entry data if it isn't zero-sized
		if (entry->getSize() > 0) {
			// Read the entry data
			mc.exportMemChunk(edata, getEntryOffset(entry), entry->getSize());
			entry->importMemChunk(edata);
		}

		// Detect entry type
		EntryType::detectEntryType(entry);

		// Set entry to unchanged
		entry->setState(0);
	}

	// Detect maps (will detect map entry types)
	theSplashWindow->setProgressMessage("Detecting maps");
	detectMaps();

	// Setup variables
	setMuted(false);
	setModified(false);
	announce("opened");

	theSplashWindow->setProgressMessage("");

	return true;
}

/* LibArchive::write
 * Writes the wad archive to a file
 * Returns true if successful, false otherwise
 *******************************************************************/
bool LibArchive::write(string filename, bool update) {
	return false;
}

/* LibArchive::write
 * Writes the wad archive to a MemChunk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool LibArchive::write(MemChunk& mc, bool update) {
	return false;
}

/* LibArchive::close
 * 'Closes' the archive
 *******************************************************************/
void LibArchive::close() {
	// Delete all entries
	while (entries.size() > 0) {
		delete entries[0];
		entries.erase(entries.begin());
	}

	// Unlock parent entry if it exists
	if (parent)
		parent->unlock();

	// Announce
	announce("close");
}

/* LibArchive::getEntryOffset
 * Gets a lump entry's offset
 * Returns the lump entry's offset, or zero if it doesn't exist
 *******************************************************************/
uint32_t LibArchive::getEntryOffset(ArchiveEntry* entry) {
	return uint32_t((int)entry->extraProp("Offset"));
}

/* LibArchive::setEntryOffset
 * Sets a lump entry's offset
 *******************************************************************/
void LibArchive::setEntryOffset(ArchiveEntry* entry, uint32_t offset) {
	entry->extraProp("Offset") = (int)offset;
}

/* LibArchive::loadEntryData
 * Loads an entry's data from the wadfile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool LibArchive::loadEntryData(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return false;

	// Do nothing if the lump's size is zero,
	// or if it has already been loaded
	if (entry->getSize() == 0 || entry->isLoaded()) {
		entry->setLoaded();
		return true;
	}

	// Open wadfile
	wxFile file(filename);

	// Check if opening the file failed
	if (!file.IsOpened()) {
		wxLogMessage("LibArchive::loadEntryData: Failed to open libfile %s", filename.c_str());
		return false;
	}

	// Seek to lump offset in file and read it in
	file.Seek(getEntryOffset(entry), wxFromStart);
	entry->importFileStream(file, entry->getSize());

	// Set the lump to loaded
	entry->setLoaded();

	return true;
}

/* LibArchive::numEntries
 * Returns the number of entries in the archive
 *******************************************************************/
uint32_t LibArchive::numEntries() {
	return entries.size();
}

/* LibArchive::addEntry
 * Adds an entry to the entry list before position. If position is
 * invalid the entry will be added at the end of the list. Truncates
 * the given name to 8 characters, and also stores the original full
 * name as an extra property of the entry. Returns false if the given
 * entry was invalid, true otherwise
 *******************************************************************/
bool LibArchive::addEntry(ArchiveEntry* entry, uint32_t position) {
	return false;
}

/* LibArchive::addNewEntry
 * Creates a new ArchiveEntry and adds it to the archive before the
 * position specified. Returns the created entry
 *******************************************************************/
ArchiveEntry* LibArchive::addNewEntry(string name, uint32_t position) {
	return NULL;
}

/* LibArchive::addExistingEntry
 * Adds an existing ArchiveEntry to the archive before the position
 * specified. Returns the added archive entry
 *******************************************************************/
ArchiveEntry* LibArchive::addExistingEntry(ArchiveEntry* entry, uint32_t position, bool copy) {
	return NULL;
}

/* LibArchive::removeEntry
 * Removes an entry from the archive, and deletes it if delete_entry
 * is true. Returns false if the given entry doesn't exist in the
 * archive, true otherwise
 *******************************************************************/
bool LibArchive::removeEntry(ArchiveEntry* entry, bool delete_entry) {
	return false;
}

/* LibArchive::swapEntries
 * Swaps the specified entries. Returns false if either entry is
 * invalid or not part of this Archive, true otherwise.
 *******************************************************************/
bool LibArchive::swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) {
	return false;
}

ArchiveEntry* LibArchive::findEntry(string search, bool incsub) {
	for (size_t a = 0; a < numEntries(); a++) {
		if (getEntry(a)->getName().Lower().Matches(search.Lower()))
			return getEntry(a);
	}
	return NULL;
}

ArchiveEntry* LibArchive::findEntry(int edftype, bool incsub) {
//	for (size_t a = 0; a < numEntries(); a++) {
//		if (getEntry(a)->getType()->getFormat() == edftype)
//			return getEntry(a);
//	}
	return NULL;
}

vector<ArchiveEntry*> LibArchive::findEntries(string search, bool incsub) {
	vector<ArchiveEntry*> ret;
	for (size_t a = 0; a < numEntries(); a++) {
		if (getEntry(a)->getName().Lower().Matches(search.Lower()))
			ret.push_back(getEntry(a));
	}
	return ret;
}

vector<ArchiveEntry*> LibArchive::findEntries(int edftype, bool incsub) {
	vector<ArchiveEntry*> ret;
//	for (size_t a = 0; a < numEntries(); a++) {
//		if (getEntry(a)->getType()->getFormat() == edftype)
//			ret.push_back(getEntry(a));
//	}
	return ret;
}

bool LibArchive::isLibArchive(MemChunk& mc) {
	if (mc.getSize() < 64)
		return false;

	// Read lib footer
	mc.seek(2, SEEK_END);
	uint32_t num_lumps = 0;
	mc.read(&num_lumps, 2);		// Size
	num_lumps = wxINT16_SWAP_ON_BE(num_lumps);
	uint32_t dir_offset = mc.getSize() - (2 + (num_lumps * 21));

	// Check directory offset is decent
	mc.seek(dir_offset, SEEK_SET);
	uint32_t offset = 0;
	uint32_t size = 0;
	uint8_t dummy = 0;
	mc.read(&size, 4);		// Size
	mc.read(&offset, 4);	// Offset
	mc.read(&dummy, 1);		// Separator
	offset = wxINT32_SWAP_ON_BE(offset);
	size = wxINT32_SWAP_ON_BE(size);

	// If the lump data goes past the directory,
	// the library is invalid
	if (dummy != 0 || offset != 0 || offset + size > mc.getSize()) {
		return false;
	}

	// If it's passed to here it's probably a lib file
	return true;
}

bool LibArchive::isLibArchive(string filename) {
	// Open file for reading
	wxFile file(filename);

	// Check it opened ok
	if (!file.IsOpened())
		return false;
	// Read lib footer
	file.Seek(2, wxFromEnd);
	uint32_t num_lumps = 0;
	file.Read(&num_lumps, 2);		// Size
	num_lumps = wxINT16_SWAP_ON_BE(num_lumps);
	uint32_t dir_offset = file.Length() - (2 + (num_lumps * 21));

	// Check directory offset is decent
	file.Seek(dir_offset, wxFromStart);
	uint32_t offset = 0;
	uint32_t size = 0;
	uint8_t dummy = 0;
	file.Read(&size, 4);	// Size
	file.Read(&offset, 4);	// Offset
	file.Read(&dummy, 1);	// Separator
	offset = wxINT32_SWAP_ON_BE(offset);
	size = wxINT32_SWAP_ON_BE(size);

	// If the lump data goes past the directory,
	// the library is invalid
	if (dummy != 0 || offset != 0 || offset + size > file.Length()) {
		return false;
	}

	// If it's passed to here it's probably a lib file
	return true;
}

/* LibArchive::detectMaps
 * Detects all the maps in the archive and returns a vector of
 * information about them.
 *******************************************************************/
vector<Archive::mapdesc_t> LibArchive::detectMaps() {
	vector<mapdesc_t> ret;
	return ret;
}

