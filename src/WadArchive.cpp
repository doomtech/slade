
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    WadArchive.cpp
 * Description: WadArchive, archive class to handle doom format
 *              wad archives
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
#include "WadArchive.h"
#include <wx/log.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, archive_load_data, false, CVAR_SAVE)

// Used for map detection
string map_lumps[12] = {
	_T("THINGS"),
	_T("VERTEXES"),
	_T("LINEDEFS"),
	_T("SIDEDEFS"),
	_T("SECTORS"),
	_T("SEGS"),
	_T("SSECTORS"),
	_T("NODES"),
	_T("BLOCKMAP"),
	_T("REJECT"),
	_T("SCRIPTS"),
	_T("BEHAVIOR")
};


/*******************************************************************
 * WADARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* WadArchive::WadArchive
 * WadArchive class constructor
 *******************************************************************/
WadArchive::WadArchive()
: Archive(ARCHIVE_WAD) {
	// Default wad type
	wad_type[0] = 'P';
	wad_type[1] = 'W';
	wad_type[2] = 'A';
	wad_type[3] = 'D';

	// Init entry type ranges
	patches[0] = -1;
	patches[1] = -1;
	flats[0] = -1;
	flats[1] = -1;
	sprites[0] = -1;
	sprites[1] = -1;
	tx[0] = -1;
	tx[1] = -1;
}

/* WadArchive::~WadArchive
 * WadArchive class destructor
 *******************************************************************/
WadArchive::~WadArchive() {
}

/* WadArchive::isIWAD
 * Returns true if the wad is an IWAD, false otherwise
 *******************************************************************/
bool WadArchive::isIWAD() {
	if (wad_type[0] == 'I')
		return true;
	else
		return false;
}

/* WadArchive::entryIndex
 * Returns the given entry's index in the Archive entry list.
 * Returns -1 if the entry doesn't exist in the Archive.
 *******************************************************************/
int WadArchive::entryIndex(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return -1;

	for (size_t a = 0; a < entries.size(); a++) {
		if (entries[a] == entry)
			return a;
	}

	return -1;
}

/* WadArchive::getEntry
 * Returns the entry at the index specified,
 * or NULL if the index is invalid
 *******************************************************************/
ArchiveEntry* WadArchive::getEntry(uint32_t index) {
	// Check index
	if (index >= entries.size())
		return NULL;

	return entries[index];
}

/* WadArchive::getEntry
 * Returns the first entry with the specified name,
 * or NULL if no entry exists with that name
 *******************************************************************/
ArchiveEntry* WadArchive::getEntry(string name) {
	for (size_t a = 0; a < entries.size(); a++) {
		if (getEntry(a)->getName() == name)
			return getEntry(a);
	}

	return NULL;
}

/* WadArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string WadArchive::getFileExtensionString() {
	return _T("Wad Files (*.wad)|*.wad");
}

bool WadArchive::open(MemChunk& mc) {
	// Check data was given
	if (!mc.hasData())
		return false;
	
	// Read wad header
	uint32_t num_lumps = 0;
	uint32_t dir_offset = 0;
	mc.seek(0, SEEK_SET);
	mc.read(&wad_type, 4);		// Wad type
	mc.read(&num_lumps, 4);		// No. of lumps in wad
	mc.read(&dir_offset, 4);	// Offset to directory
	
	// Byteswap values for big endian if needed
	num_lumps = wxINT32_SWAP_ON_BE(num_lumps);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);
	
	// Check the header
	if (wad_type[1] != 'W' || wad_type[2] != 'A' || wad_type[3] != 'D') {
		wxLogMessage(_T("WadArchive::openFile: File %s has invalid header"), filename.c_str());
		Global::error = _T("Invalid wad header");
		return false;
	}

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Read the directory
	mc.seek(dir_offset, SEEK_SET);
	for (uint32_t d = 0; d < num_lumps; d++) {
		// Read lump info
		char name[9] = "";
		uint32_t offset = 0;
		uint32_t size = 0;

		mc.read(&offset, 4);	// Offset
		mc.read(&size, 4);		// Size
		mc.read(name, 8);		// Name
		name[8] = '\0';

		// Byteswap values for big endian if needed
		offset = wxINT32_SWAP_ON_BE(offset);
		size = wxINT32_SWAP_ON_BE(size);

		// If the lump data goes past the end of the file,
		// the wadfile is invalid
		if (offset + size > mc.getSize()) {
			wxLogMessage(_T("WadArchive::open: Wad archive is invalid or corrupt"));
			Global::error = _T("Archive is invalid and/or corrupt");
			setMuted(false);
			return false;
		}

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(wxString::FromAscii(name), size, this);
		nlump->setLoaded(false);
		nlump->setExProp(_T("Offset"), s_fmt(_T("%d"), offset));
		nlump->setState(0);

		// Check for markers
		if (!nlump->getName().Cmp(_T("P_START")))
			patches[0] = d;
		if (!nlump->getName().Cmp(_T("PP_START")))
			patches[0] = d;
		if (!nlump->getName().Cmp(_T("P_END")))
			patches[1] = d;
		if (!nlump->getName().Cmp(_T("PP_END")))
			patches[1] = d;
		if (!nlump->getName().Cmp(_T("F_START")))
			flats[0] = d;
		if (!nlump->getName().Cmp(_T("FF_START")))
			flats[0] = d;
		if (!nlump->getName().Cmp(_T("F_END")))
			flats[1] = d;
		if (!nlump->getName().Cmp(_T("FF_END")))
			flats[1] = d;
		if (!nlump->getName().Cmp(_T("S_START")))
			sprites[0] = d;
		if (!nlump->getName().Cmp(_T("SS_START")))
			sprites[0] = d;
		if (!nlump->getName().Cmp(_T("S_END")))
			sprites[1] = d;
		if (!nlump->getName().Cmp(_T("SS_END")))
			sprites[1] = d;
		if (!nlump->getName().Cmp(_T("TX_START")))
			tx[0] = d;
		if (!nlump->getName().Cmp(_T("TX_END")))
			tx[1] = d;

		// Add to entry list
		entries.push_back(nlump);
	}

	// Detect all entry types
	MemChunk edata;
	for (size_t a = 0; a < entries.size(); a++) {
		ArchiveEntry* entry = entries[a];

		// Read entry data if it isn't zero-sized
		if (entry->getSize() > 0) {
			// Read the entry data
			mc.exportMemChunk(edata, getEntryOffset(entry), entry->getSize());
			entry->importMemChunk(edata);
		}

		// Detect entry type
		EntryType::detectEntryType(entry);

		// Unload entry data if needed
		if (!archive_load_data)
			entry->unloadData();

		// Lock entry if IWAD
		if (wad_type[0] == 'I')
			entry->lock();

		// Set entry to unchanged
		entry->setState(0);
	}

	// Detect maps (will detect map entry types)
	detectMaps();

	// Setup variables
	setMuted(false);
	setModified(false);
	announce(_T("opened"));

	return true;
}

/* WadArchive::openFile
 * Reads a wad format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WadArchive::openFile(string filename) {
	// Open file
	MemChunk mc;
	mc.importFile(filename);
	if (open(mc)) {
		// Setup variables
		this->filename = filename;
		on_disk = true;

		return true;
	}
	else
		return false;
}

/* WadArchive::save
 * Saves the wad archive to the specified file, if no filename is
 * given, saves to the current archive filename.
 * Returns false if the file couldn't be written to, true otherwise
 *******************************************************************/
bool WadArchive::save(string filename) {
	// If no filename specified, just use the current filename
	if (filename == _T(""))
		filename = this->filename;

	// Create a backup copy if needed
	if (wxFileName::FileExists(filename)) {
		string bakfile = filename + _T(".bak");

		// Remove old backup file
		remove(chr(bakfile));

		// Copy current file contents to new backup file
		wxCopyFile(this->filename, bakfile);
	}

	// Determine directory offset & individual lump offsets
	uint32_t dir_offset = 12;
	for (uint32_t l = 0; l < numEntries(); l++) {
		setEntryOffset(entries[l], dir_offset);
		dir_offset += entries[l]->getSize();
	}

	// Open wadfile for writing
	FILE *fp = fopen(filename.ToAscii(), "wb");
	if (!fp) {
		Global::error = _T("Unable to open file for saving. Make sure it isn't in use by another program.");
		return false;
	}

	// Write the header
	uint32_t num_lumps = numEntries();
	fwrite(wad_type, 1, 4, fp);
	fwrite(&num_lumps, 4, 1, fp);
	fwrite(&dir_offset, 4, 1, fp);

	// Write the lumps
	for (uint32_t l = 0; l < num_lumps; l++)
		fwrite(entries[l]->getData(), entries[l]->getSize(), 1, fp);

	// Write the directory
	for (uint32_t l = 0; l < num_lumps; l++) {
		char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		long offset = getEntryOffset(entries[l]);
		long size = entries[l]->getSize();

		for (size_t c = 0; c < entries[l]->getName().length(); c++)
			name[c] = entries[l]->getName()[c];

		fwrite(&offset, 4, 1, fp);
		fwrite(&size, 4, 1, fp);
		fwrite(name, 1, 8, fp);

		entries[l]->setState(0);
		entries[l]->setExProp(_T("Offset"), s_fmt(_T("%d"), l));
	}

	// Close the file
	fclose(fp);

	// Set variables and return success
	this->filename = filename;
	setModified(false);
	on_disk = true;
	announce(_T("saved"));

	return true;
}

/* WadArchive::close
 * 'Closes' the archive
 *******************************************************************/
void WadArchive::close() {
	// Delete all entries
	while (entries.size() > 0) {
		delete entries[0];
		entries.erase(entries.begin());
	}

	// Announce
	announce(_T("close"));
}

/* WadArchive::getEntryOffset
 * Gets a lump entry's offset
 * Returns the lump entry's offset, or zero if it doesn't exist
 *******************************************************************/
uint32_t WadArchive::getEntryOffset(ArchiveEntry* entry) {
	if (entry->hasExProp(_T("Offset"))) {
		return (uint32_t)atoi(chr(entry->getExProp(_T("Offset"))));
	}
	else
		return 0;
}

/* WadArchive::setEntryOffset
 * Sets a lump entry's offset
 *******************************************************************/
void WadArchive::setEntryOffset(ArchiveEntry* entry, uint32_t offset) {
	entry->setExProp(_T("Offset"), s_fmt(_T("%d"), offset));
}

/* WadArchive::loadEntryData
 * Loads an entry's data from the wadfile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WadArchive::loadEntryData(ArchiveEntry* entry) {
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
	FILE *fp = fopen(filename.ToAscii(), "rb");

	// Check if opening the file failed
	if (!fp) {
		wxLogMessage(_T("WadArchive::loadEntryData: Failed to open wadfile %s"), filename.c_str());
		return false;
	}

	// Seek to lump offset in file and read it in
	fseek(fp, getEntryOffset(entry), SEEK_SET);
	entry->importFileStream(fp, entry->getSize());
	//fread(data, entry->getSize(), 1, fp);

	// Close file
	fclose(fp);

	// Set the lump to loaded
	entry->setLoaded();

	return true;
}

/* WadArchive::numEntries
 * Returns the number of entries in the archive
 *******************************************************************/
uint32_t WadArchive::numEntries() {
	return entries.size();
}

/* WadArchive::detectMaps
 * Searches for any maps in the wad and adds them to the map list
 *******************************************************************/
vector<Archive::mapdesc_t> WadArchive::detectMaps() {
	vector<mapdesc_t> maps;

	// Go through all lumps
	size_t i = 0;
	while (i < numEntries()) {
		// UDMF format map check ********************************************************

		// Check for UDMF format map lump (TEXTMAP lump)
		if (entries[i]->getName() == _T("TEXTMAP") && i > 0) {
			// Get map info
			mapdesc_t md;
			md.head = entries[i - 1]; // Header lump
			md.name = entries[i - 1]->getName(); // Map title
			md.format = 2; // Format = 2 (UDMF)

			// Skip lumps until we find the ENDMAP marker
			bool done = false;
			while (!done) {
				// If we've somehow reached the end of the wad without finding ENDMAP,
				// log an error and return
				if (i == numEntries()) {
					wxLogMessage(_T("UDMF Map with no ENDMAP marker in %s"), filename.c_str());
					return maps;
				}

				// If ENDMAP marker is here, exit the loop, otherwise skip to next lump
				if (entries[i]->getName() == _T("ENDMAP"))
					done = true;
				else
					i++;
			}

			// Set end lump
			md.end = entries[i];

			// Add to map list
			maps.push_back(md);

			// Current index is ENDMAP, we don't want to check for a doom/hexen format
			// map so just go to the next index and continue the loop
			i++;
			continue;
		}



		// Doom/Hexen format map check **************************************************

		// Array to keep track of what doom/hexen map lumps have been found
		uint8_t existing_map_lumps[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		// Check if the current lump is a doom/hexen map lump
		bool maplump_found = false;
		for (int a = 0; a < 5; a++) {
			// Compare with all base map lump names
			if (entries[i]->getName() == map_lumps[a]) {
				maplump_found = true;
				existing_map_lumps[a] = 1;
				break;
			}
		}

		// If we've found what might be a map
		if (maplump_found) {
			// Save position of map header lump
			int header_index = i - 1;

			// Check off map lumps until we find a non-map lump
			bool done = false;
			while (!done) {
				// Loop will end if no map lump is found
				done = true;

				// If we're at the end of the wad, exit the loop
				if (i == numEntries()) {
					i--;
					break;
				}

				// Compare with all map lump names
				for (int a = 0; a < 12; a++) {
					// Compare with all base map lump names
					if (entries[i]->getName() == map_lumps[a]) {
						existing_map_lumps[a] = 1;
						done = false;
						break;
					}
				}

				// Go to next lump
				i++;
			}

			// Go back to the lump just after the last map lump found
			i--;

			// Check that we have all the required map lumps: VERTEXES, LINEDEFS, SIDEDEFS, THINGS & SECTORS
			if (!memchr(existing_map_lumps, 0, 5)) {
				// Get map info
				mapdesc_t md;
				md.head = entries[header_index]; // Header lump
				md.name = entries[header_index]->getName(); // Map title
				md.end = entries[i - 1]; // End lump

				// If BEHAVIOR lump exists, it's a hexen format map, otherwise it's doom format
				if (existing_map_lumps[11])
					md.format = 1;
				else
					md.format = 0;

				// Add map info to the maps list
				maps.push_back(md);
			} else {
				// If we found a non-map lump before all needed map lumps were found,
				// it's an invalid map, so just continue the loop
				continue;
			}
		}




		// Not a UDMF or Doom/Hexen map lump, go to next lump
		i++;
	}

	// Set all map header entries to ETYPE_MAP type
	for (size_t a = 0; a < maps.size(); a++)
		maps[a].head->setType(EntryType::mapMarkerType());

	return maps;
}

/* WadArchive::addEntry
 * Adds an entry to the entry list before position. If position is
 * invalid the entry will be added at the end of the list. Truncates
 * the given name to 8 characters, and also stores the original full
 * name as an extra property of the entry. Returns false if the given
 * entry was invalid, true otherwise
 *******************************************************************/
bool WadArchive::addEntry(ArchiveEntry* entry, uint32_t position) {
	// Check valid entry
	if (!entry)
		return false;

	// Check for IWAD
	if (isIWAD())
		return false;

	// Truncate name to 8 characters if needed
	string name = entry->getName();
	if (name.size() > 8) {
		entry->setExProp(_T("full_name"), name); // Add full name as extra property in case it's needed later
		name.Truncate(8);
		entry->setName(name);
	}

	// Check the position is valid
	if (position < 0 || position > entries.size()) {
		// If it isn't valid, we'll add the entry to the end of the list
		position = entries.size();
	}

	// Add it to the entry list
	if (position < entries.size())
		entries.insert(entries.begin() + position, entry);
	else
		entries.push_back(entry);

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

/* WadArchive::addNewEntry
 * Creates a new ArchiveEntry and adds it to the archive before the
 * position specified. Returns the created entry
 *******************************************************************/
ArchiveEntry* WadArchive::addNewEntry(string name, uint32_t position) {
	// Check for IWAD
	if (isIWAD())
		return NULL;

	// Create the new entry
	ArchiveEntry* new_entry = new ArchiveEntry(name);

	// Add it to the entry list at position
	addEntry(new_entry, position);

	// Return the newly created entry
	return new_entry;
}

/* WadArchive::addExistingEntry
 * Adds an existing ArchiveEntry to the archive before the position
 * specified. Returns the added archive entry
 *******************************************************************/
ArchiveEntry* WadArchive::addExistingEntry(ArchiveEntry* entry, uint32_t position, bool copy) {
	// Check for IWAD
	if (isIWAD())
		return NULL;

	// Make a copy of the entry to add if needed
	if (copy)
		entry = new ArchiveEntry(*entry);

	// Add the entry to the list
	addEntry(entry, position);

	// Return the added entry
	return entry;
}

/* WadArchive::removeEntry
 * Removes an entry from the archive, and deletes it if delete_entry
 * is true. Returns false if the given entry doesn't exist in the
 * archive, true otherwise
 *******************************************************************/
bool WadArchive::removeEntry(ArchiveEntry* entry, bool delete_entry) {
	// Check for IWAD
	if (isIWAD())
		return false;

	// Check if the entry is locked
	if (entry->isLocked())
		return false;

	// Check the entry exists in this archive
	int index = entryIndex(entry);
	if (index == -1)
		return false;

	// Announce (before actually removing in case entry is still needed)
	MemChunk mc;
	wxUIntPtr ptr = wxPtrToUInt(entry);
	mc.write(&index, sizeof(int));
	mc.write(&ptr, sizeof(wxUIntPtr));
	announce(_T("entry_removed"), mc);

	// Remove it from the entry list
	entries.erase(entries.begin() + index);

	// Delete the entry if needed
	if (delete_entry)
		delete entry;

	// Update variables etc
	setModified(true);

	return true;
}

/* WadArchive::swapEntries
 * Swaps the specified entries. Returns false if either entry is
 * invalid or not part of this Archive, true otherwise.
 *******************************************************************/
bool WadArchive::swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) {
	// Check for IWAD
	if (isIWAD())
		return false;

	// Check if either entry is locked
	if (entry1->isLocked() || entry2->isLocked())
		return false;

	// Get entry indices
	int i1 = entryIndex(entry1);
	int i2 = entryIndex(entry2);

	// Check indices
	if (i1 == -1 || i2 == -1)
		return false;

	// Swap the entries in the list
	entries[i1] = entry2;
	entries[i2] = entry1;

	// Announce the swap
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

/* WadArchive::renameEntry
 * Override of ArchiveEntry::renameEntry to also truncate the given
 * name to 8 characters
 *******************************************************************/
bool WadArchive::renameEntry(ArchiveEntry* entry, string new_name) {
	// Check for IWAD
	if (isIWAD())
		return false;

	// Check if entry is locked
	if (entry->isLocked())
		return false;

	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return false;

	// Truncate name to 8 characters if needed
	if (new_name.size() > 8)
		new_name.Truncate(8);

	// Rename the entry
	entry->rename(new_name);

	return true;
}

string WadArchive::detectEntrySection(ArchiveEntry* entry) {
	// Check the entry is valid and part of this archive
	if (!checkEntry(entry))
		return _T("none");

	// Check if entry is within any markers
	int index = entryIndex(entry);

	// Patches
	if (index > patches[0] && index < patches[1])
		return _T("patches");

	// Flats
	if (index > flats[0] && index < flats[1])
		return _T("flats");

	// Sprites
	if (index > sprites[0] && index < sprites[1])
		return _T("sprites");

	return _T("none");
}

ArchiveEntry* WadArchive::findEntry(string search) {
	return NULL;
}

vector<ArchiveEntry*> WadArchive::findEntries(string search) {
	vector<ArchiveEntry*> ret;
	return ret;
}


bool WadArchive::isWadArchive(MemChunk& mc) {
	// Check size
	if (mc.getSize() < 12)
		return false;

	// Check for IWAD/PWAD header
	if (!(mc[1] == 'W' && mc[2] == 'A' && mc[3] == 'D' &&
	        (mc[0] == 'P' || mc[0] == 'I')))
		return false;

	// Get number of lumps and directory offset
	uint32_t num_lumps = 0;
	uint32_t dir_offset = 0;
	mc.seek(4, SEEK_SET);
	mc.read(&num_lumps, 4);
	mc.read(&dir_offset, 4);

	// Reset MemChunk (just in case)
	mc.seek(0, SEEK_SET);

	// Byteswap values for big endian if needed
	num_lumps = wxINT32_SWAP_ON_BE(num_lumps);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);

	// Check directory offset is decent
	if ((dir_offset + (num_lumps * 16)) > mc.getSize() ||
	        dir_offset < 12)
		return false;

	// If it's passed to here it's probably a wad file
	return true;
}

bool WadArchive::isWadArchive(string filename) {
	// Open file for reading
	wxFile file(filename);
	
	// Check it opened ok
	if (!file.IsOpened())
		return false;
	
	// Read header
	char header[4];
	file.Read(header, 4);
	
	// Check for IWAD/PWAD header
	if (!(header[1] == 'W' && header[2] == 'A' && header[3] == 'D' &&
	        (header[0] == 'P' || header[0] == 'I')))
		return false;
	
	// Get number of lumps and directory offset
	uint32_t num_lumps = 0;
	uint32_t dir_offset = 0;
	file.Read(&num_lumps, 4);
	file.Read(&dir_offset, 4);
	
	// Byteswap values for big endian if needed
	num_lumps = wxINT32_SWAP_ON_BE(num_lumps);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);

	// Check directory offset is decent
	if ((dir_offset + (num_lumps * 16)) > file.Length() ||
	        dir_offset < 12)
		return false;

	// If it's passed to here it's probably a wad file
	return true;
}
