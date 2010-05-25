
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    DatArchive.cpp
 * Description: DatArchive, archive class to handle ravdata.dat
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
#include "DatArchive.h"
#include "SplashWindow.h"
#include <wx/log.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/

/*******************************************************************
 * DATARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* DatArchive::DatArchive
 * DatArchive class constructor
 *******************************************************************/
DatArchive::DatArchive()
: Archive(ARCHIVE_DAT) {
}

/* DatArchive::~DatArchive
 * DatArchive class destructor
 *******************************************************************/
DatArchive::~DatArchive() {
	// Delete entries
	for (size_t a = 0; a < entries.size(); a++)
		delete entries[a];
}

/* DatArchive::entryIndex
 * Returns the given entry's index in the Archive entry list.
 * Returns -1 if the entry doesn't exist in the Archive.
 *******************************************************************/
int DatArchive::entryIndex(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return -1;

	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return a;
	}

	return -1;
}

/* DatArchive::getEntry
 * Returns the entry at the index specified,
 * or NULL if the index is invalid
 *******************************************************************/
ArchiveEntry* DatArchive::getEntry(uint32_t index) {
	// Check index
	if (index >= entries.size())
		return NULL;

	return entries[index];
}

/* DatArchive::getEntry
 * Returns the first entry with the specified name,
 * or NULL if no entry exists with that name
 *******************************************************************/
ArchiveEntry* DatArchive::getEntry(string name) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (getEntry(a)->getName() == name)
			return getEntry(a);
	}

	return NULL;
}

/* DatArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string DatArchive::getFileExtensionString() {
	return _T("Wad Files (*.dat)|*.dat");
}

/* DatArchive::open
 * Reads a wad format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool DatArchive::open(string filename) {
	// Read the file into a MemChunk
	MemChunk mc;
	if (!mc.importFile(filename)) {
		Global::error = _T("Unable to open file. Make sure it isn't in use by another program.");
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

/* DatArchive::open
 * Reads wad format data from an ArchiveEntry
 * Returns true if successful, false otherwise
 *******************************************************************/
bool DatArchive::open(ArchiveEntry* entry) {
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

/* DatArchive::open
 * Reads wad format data from a MemChunk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool DatArchive::open(MemChunk& mc) {
	// Check data was given
	if (!mc.hasData())
		return false;

	const uint8_t * mcdata = mc.getData();

	// Read dat header
	mc.seek(0, SEEK_SET);
	uint16_t num_lumps;
	uint32_t dir_offset, unknown;
	mc.read(&num_lumps, 2);		// Size
	mc.read(&dir_offset, 4);	// Directory offset
	mc.read(&unknown, 4);		// Unknown value
	num_lumps	= wxINT16_SWAP_ON_BE(num_lumps);
	dir_offset	= wxINT32_SWAP_ON_BE(dir_offset);
	unknown		= wxINT32_SWAP_ON_BE(unknown);
	string lastname(wxString::FromAscii("-noname-"));
	size_t namecount = 0;

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Read the directory
	mc.seek(dir_offset, SEEK_SET);
	theSplashWindow->setProgressMessage(_T("Reading dat archive data"));
	for (uint32_t d = 0; d < num_lumps; d++) {
		// Update splash window progress
		theSplashWindow->setProgress(((float)d / (float)num_lumps));

		// Read lump info
		uint32_t offset = 0;
		uint32_t size = 0;
		uint16_t nameofs = 0;
		uint16_t unknown = 0;
		
		mc.read(&offset,	4);		// Offset
		mc.read(&size,		4);		// Size
		mc.read(&nameofs,	2);		// Name offset
		mc.read(&unknown,	2);		// Name offset

		// Byteswap values for big endian if needed
		offset = wxINT32_SWAP_ON_BE(offset);
		size = wxINT32_SWAP_ON_BE(size);
		nameofs = wxINT16_SWAP_ON_BE(nameofs);
		unknown = wxINT16_SWAP_ON_BE(unknown);

		// If the lump data goes past the directory,
		// the wadfile is invalid
		if (offset + size > mc.getSize()) {
			wxLogMessage(_T("DatArchive::open: Dat archive is invalid or corrupt"));
			Global::error = _T("Archive is invalid and/or corrupt");
			setMuted(false);
			return false;
		}

		string myname;
		if (nameofs != 0) {
			size_t len = 1;
			size_t start = nameofs+dir_offset;
			for (size_t i = start; mcdata[i] != 0; ++i, ++len) {}
			lastname = myname = wxString::FromAscii(mcdata+start, len);
			namecount = 0;
		}
		else
		{
			myname = s_fmt(_T("%s+%d"), lastname, ++namecount);
		}

//		wxLogMessage(s_fmt(_T("Entry %d in %s: %s @ %x, size %d"), d, name, myname, offset, size));

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(myname, size, this);
		nlump->setLoaded(false);
		nlump->extraProp(_T("Offset")) = (int)offset;
		nlump->setState(0);

		// Check for markers
		if (!nlump->getName().Cmp(_T("startflats")))
			flats[0] = d;
		if (!nlump->getName().Cmp(_T("endflats")))
			flats[1] = d;
		if (!nlump->getName().Cmp(_T("startsprites")))
			sprites[0] = d;
		if (!nlump->getName().Cmp(_T("endmonsters")))
			sprites[1] = d;
		if (!nlump->getName().Cmp(_T("startwalls")))
			walls[0] = d;
		if (!nlump->getName().Cmp(_T("endwalls")))
			walls[1] = d;

		// Add to entry list
		entries.push_back(nlump);
	}

//	wxLogMessage(s_fmt(_T("Total: %d entries"), entries.size()));

	// Detect all entry types
	MemChunk edata;
	theSplashWindow->setProgressMessage(_T("Detecting entry types"));
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
	theSplashWindow->setProgressMessage(_T("Detecting maps"));
	detectMaps();

	// Setup variables
	setMuted(false);
	setModified(false);
	announce(_T("opened"));

	theSplashWindow->setProgressMessage(_T(""));

	return true;
}

string DatArchive::detectEntrySection(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return _T("none");

	// Check if entry is within any markers
	int index = entryIndex(entry);

	// Textures
	if (index > walls[0] && index < walls[1])
		return _T("textures");

	// Flats
	if (index > flats[0] && index < flats[1])
		return _T("flats");

	// Sprites
	if (index > sprites[0] && index < sprites[1])
		return _T("sprites");

	return _T("none");
}

/* DatArchive::write
 * Writes the wad archive to a file
 * Returns true if successful, false otherwise
 *******************************************************************/
bool DatArchive::write(string filename, bool update) {
	return false;
}

/* DatArchive::write
 * Writes the wad archive to a MemChunk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool DatArchive::write(MemChunk& mc, bool update) {
	return false;
}

/* DatArchive::close
 * 'Closes' the archive
 *******************************************************************/
void DatArchive::close() {
	// Delete all entries
	while (entries.size() > 0) {
		delete entries[0];
		entries.erase(entries.begin());
	}

	// Unlock parent entry if it exists
	if (parent)
		parent->unlock();

	// Announce
	announce(_T("close"));
}

/* DatArchive::getEntryOffset
 * Gets a lump entry's offset
 * Returns the lump entry's offset, or zero if it doesn't exist
 *******************************************************************/
uint32_t DatArchive::getEntryOffset(ArchiveEntry* entry) {
	return uint32_t((int)entry->extraProp(_T("Offset")));
}

/* DatArchive::setEntryOffset
 * Sets a lump entry's offset
 *******************************************************************/
void DatArchive::setEntryOffset(ArchiveEntry* entry, uint32_t offset) {
	entry->extraProp(_T("Offset")) = (int)offset;
}

/* DatArchive::loadEntryData
 * Loads an entry's data from the wadfile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool DatArchive::loadEntryData(ArchiveEntry* entry) {
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
		wxLogMessage(_T("DatArchive::loadEntryData: Failed to open libfile %s"), filename.c_str());
		return false;
	}

	// Seek to lump offset in file and read it in
	file.Seek(getEntryOffset(entry), wxFromStart);
	entry->importFileStream(file, entry->getSize());

	// Set the lump to loaded
	entry->setLoaded();

	return true;
}

/* DatArchive::numEntries
 * Returns the number of entries in the archive
 *******************************************************************/
uint32_t DatArchive::numEntries() {
	return entries.size();
}

/* DatArchive::addEntry
 * Adds an entry to the entry list before position. If position is
 * invalid the entry will be added at the end of the list. Truncates
 * the given name to 8 characters, and also stores the original full
 * name as an extra property of the entry. Returns false if the given
 * entry was invalid, true otherwise
 *******************************************************************/
bool DatArchive::addEntry(ArchiveEntry* entry, uint32_t position) {
	return false;
}

/* DatArchive::addNewEntry
 * Creates a new ArchiveEntry and adds it to the archive before the
 * position specified. Returns the created entry
 *******************************************************************/
ArchiveEntry* DatArchive::addNewEntry(string name, uint32_t position) {
	return NULL;
}

/* DatArchive::addExistingEntry
 * Adds an existing ArchiveEntry to the archive before the position
 * specified. Returns the added archive entry
 *******************************************************************/
ArchiveEntry* DatArchive::addExistingEntry(ArchiveEntry* entry, uint32_t position, bool copy) {
	return NULL;
}

/* DatArchive::removeEntry
 * Removes an entry from the archive, and deletes it if delete_entry
 * is true. Returns false if the given entry doesn't exist in the
 * archive, true otherwise
 *******************************************************************/
bool DatArchive::removeEntry(ArchiveEntry* entry, bool delete_entry) {
	return false;
}

/* DatArchive::swapEntries
 * Swaps the specified entries. Returns false if either entry is
 * invalid or not part of this Archive, true otherwise.
 *******************************************************************/
bool DatArchive::swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) {
	return false;
}

ArchiveEntry* DatArchive::findEntry(string search, bool incsub) {
	for (size_t a = 0; a < numEntries(); a++) {
		if (getEntry(a)->getName().Lower().Matches(search.Lower()))
			return getEntry(a);
	}
	return NULL;
}

ArchiveEntry* DatArchive::findEntry(int edftype, bool incsub) {
//	for (size_t a = 0; a < numEntries(); a++) {
//		if (getEntry(a)->getType()->getFormat() == edftype)
//			return getEntry(a);
//	}
	return NULL;
}

vector<ArchiveEntry*> DatArchive::findEntries(string search, bool incsub) {
	vector<ArchiveEntry*> ret;
	for (size_t a = 0; a < numEntries(); a++) {
		if (getEntry(a)->getName().Lower().Matches(search.Lower()))
			ret.push_back(getEntry(a));
	}
	return ret;
}

vector<ArchiveEntry*> DatArchive::findEntries(int edftype, bool incsub) {
	vector<ArchiveEntry*> ret;
//	for (size_t a = 0; a < numEntries(); a++) {
//		if (getEntry(a)->getType()->getFormat() == edftype)
//			ret.push_back(getEntry(a));
//	}
	return ret;
}

bool DatArchive::isDatArchive(MemChunk& mc) {
	// Read dat header
	mc.seek(0, SEEK_SET);
	uint16_t num_lumps;
	uint32_t dir_offset, junk;
	mc.read(&num_lumps, 2);		// Size
	mc.read(&dir_offset, 4);	// Directory offset
	mc.read(&junk, 4);		// Unknown value
	num_lumps	= wxINT16_SWAP_ON_BE(num_lumps);
	dir_offset	= wxINT32_SWAP_ON_BE(dir_offset);
	junk		= wxINT32_SWAP_ON_BE(junk);

	if (dir_offset >= mc.getSize())
		return false;

	// Read the directory
	mc.seek(dir_offset, SEEK_SET);
	// Read lump info
	uint32_t offset = 0;
	uint32_t size = 0;
	uint16_t nameofs = 0;
	uint16_t unknown = 0;
		
	mc.read(&offset,	4);		// Offset
	mc.read(&size,		4);		// Size
	mc.read(&nameofs,	2);		// Name offset
	mc.read(&unknown,	2);		// Name offset

	// Byteswap values for big endian if needed
	offset = wxINT32_SWAP_ON_BE(offset);
	size = wxINT32_SWAP_ON_BE(size);
	nameofs = wxINT16_SWAP_ON_BE(nameofs);
	unknown = wxINT16_SWAP_ON_BE(unknown);

	// The first lump should have a name (subsequent lumps need not have one).
	// Also, sanity check the values.
	if (nameofs == 0 || nameofs >=  mc.getSize() || offset + size >= mc.getSize()) {
		return false;
	}

	string name;
	size_t len = 1;
	size_t start = nameofs+dir_offset;
	// Sanity checks again. Make sure there is actually a name.
	if (start > mc.getSize() || mc[start] < 33)
		return false;
	for (size_t i = start; (mc[i] != 0 && i < mc.getSize()); ++i, ++len) {
		// Names should not contain garbage characters
		if (mc[i] < 32)
			return false;
	}
	// Let's be reasonable here. While names aren't limited, if it's too long, it's suspicious.
	if (len > 60)
		return false;
	const uint8_t * mcdata = mc.getData();
	name = wxString::FromAscii(mcdata+start, len);
	return true;
}

bool DatArchive::isDatArchive(string filename) {
	// Open file for reading
	wxFile file(filename);

	// Check it opened ok
	if (!file.IsOpened())
		return false;

	if (filename.Contains(string("RAVDATA")))
		return true;
	if (filename.Contains(string("D_CASTR")))
		return true;
	return false;

	return true;
}

/* DatArchive::detectMaps
 * Detects all the maps in the archive and returns a vector of
 * information about them.
 *******************************************************************/
vector<Archive::mapdesc_t> DatArchive::detectMaps() {
	vector<mapdesc_t> ret;
	return ret;
}

