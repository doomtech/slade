
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
: TreelessArchive(ARCHIVE_DAT) {
}

/* DatArchive::~DatArchive
 * DatArchive class destructor
 *******************************************************************/
DatArchive::~DatArchive() {
}

/* DatArchive::getEntryOffset
 * Gets a lump entry's offset
 * Returns the lump entry's offset, or zero if it doesn't exist
 *******************************************************************/
uint32_t DatArchive::getEntryOffset(ArchiveEntry* entry) {
	return uint32_t((int)entry->exProp("Offset"));
}

/* DatArchive::setEntryOffset
 * Sets a lump entry's offset
 *******************************************************************/
void DatArchive::setEntryOffset(ArchiveEntry* entry, uint32_t offset) {
	entry->exProp("Offset") = (int)offset;
}


/* DatArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string DatArchive::getFileExtensionString() {
	return "Raven Software Data Files (*.dat; *.cd; *.hd)|*.dat;*.cd;*.hd";
}

/* DatArchive::getFormat
 * Gives the "archive_dat" string
 *******************************************************************/
string DatArchive::getFormat() {
	return "archive_dat";
}

/* DatArchive::open
 * Reads a wad format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool DatArchive::open(string filename) {
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
	theSplashWindow->setProgressMessage("Reading dat archive data");
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
		// the data file is invalid
		if (offset + size > mc.getSize()) {
			wxLogMessage("DatArchive::open: Dat archive is invalid or corrupt");
			Global::error = "Archive is invalid and/or corrupt";
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
			myname = s_fmt("%s+%d", lastname, ++namecount);
		}

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(myname, size);
		nlump->setLoaded(false);
		nlump->exProp("Offset") = (int)offset;
		nlump->setState(0);

		// Check for markers
		if (!nlump->getName().Cmp("startflats"))
			flats[0] = d;
		if (!nlump->getName().Cmp("endflats"))
			flats[1] = d;
		if (!nlump->getName().Cmp("startsprites"))
			sprites[0] = d;
		if (!nlump->getName().Cmp("endmonsters"))
			sprites[1] = d;
		if (!nlump->getName().Cmp("startwalls"))
			walls[0] = d;
		if (!nlump->getName().Cmp("endwalls"))
			walls[1] = d;

		// Add to entry list
		getRoot()->addEntry(nlump);
	}

	// Detect all entry types
	MemChunk edata;
	theSplashWindow->setProgressMessage("Detecting entry types");
	for (size_t a = 0; a < numEntries(); a++) {
		// Update splash window progress
		theSplashWindow->setProgress((((float)a / (float)num_lumps)));

		// Get entry
		ArchiveEntry* entry = getEntry(a);

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

string DatArchive::detectNamespace(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return "global";

	// Check if entry is within any markers
	int index = entryIndex(entry);

	// Textures
	if (index > walls[0] && index < walls[1])
		return "textures";

	// Flats
	if (index > flats[0] && index < flats[1])
		return "flats";

	// Sprites
	if (index > sprites[0] && index < sprites[1])
		return "sprites";

	return "global";
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

/* DatArchive::loadEntryData
 * Loads an entry's data from the datfile
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
		wxLogMessage("DatArchive::loadEntryData: Failed to open datfile %s", filename.c_str());
		return false;
	}

	// Seek to lump offset in file and read it in
	file.Seek(getEntryOffset(entry), wxFromStart);
	entry->importFileStream(file, entry->getSize());

	// Set the lump to loaded
	entry->setLoaded();

	return true;
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
	return true;
}

bool DatArchive::isDatArchive(string filename) {
	// Open file for reading
	wxFile file(filename);

	// Check it opened ok
	if (!file.IsOpened())
		return false;

	// Read dat header
	file.Seek(0, wxFromStart);
	uint16_t num_lumps;
	uint32_t dir_offset, junk;
	file.Read(&num_lumps, 2);	// Size
	file.Read(&dir_offset, 4);	// Directory offset
	file.Read(&junk, 4);		// Unknown value
	num_lumps	= wxINT16_SWAP_ON_BE(num_lumps);
	dir_offset	= wxINT32_SWAP_ON_BE(dir_offset);
	junk		= wxINT32_SWAP_ON_BE(junk);

	if (dir_offset >= file.Length())
		return false;

	// Read the directory
	file.Seek(dir_offset, wxFromStart);
	// Read lump info
	uint32_t offset = 0;
	uint32_t size = 0;
	uint16_t nameofs = 0;
	uint16_t unknown = 0;

	file.Read(&offset,	4);		// Offset
	file.Read(&size,	4);		// Size
	file.Read(&nameofs,	2);		// Name offset
	file.Read(&unknown,	2);		// Name offset

	// Byteswap values for big endian if needed
	offset	= wxINT32_SWAP_ON_BE(offset);
	size	= wxINT32_SWAP_ON_BE(size);
	nameofs = wxINT16_SWAP_ON_BE(nameofs);
	unknown = wxINT16_SWAP_ON_BE(unknown);

	// The first lump should have a name (subsequent lumps need not have one).
	// Also, sanity check the values.
	if (nameofs == 0 || nameofs >=  file.Length() || offset + size >= file.Length()) {
		return false;
	}

	string name;
	size_t len = 1;
	size_t start = nameofs+dir_offset;
	// Sanity checks again. Make sure there is actually a name.
	if (start > file.Length())
		return false;
	uint8_t temp;
	file.Seek(start, wxFromStart);
	file.Read(&temp, 1);
	if (temp < 33)
		return false;
	for (size_t i = start; i < file.Length(); ++i, ++len) {
		file.Read(&temp, 1);
		// Found end of name
		if (temp == 0)
			break;
		// Names should not contain garbage characters
		if (temp < 32)
			return false;
	}
	// Let's be reasonable here. While names aren't limited, if it's too long, it's suspicious.
	if (len > 60)
		return false;
	return true;
}
