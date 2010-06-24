
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
#include "SplashWindow.h"
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
// Used for map detection
string map_lumps[12] = {
	"THINGS",
	"VERTEXES",
	"LINEDEFS",
	"SIDEDEFS",
	"SECTORS",
	"SEGS",
	"SSECTORS",
	"NODES",
	"BLOCKMAP",
	"REJECT",
	"SCRIPTS",
	"BEHAVIOR"
};


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
CVAR(Bool, iwad_lock, true, CVAR_SAVE)
EXTERN_CVAR(Bool, archive_load_data)

/*******************************************************************
 * WADARCHIVE CLASS FUNCTIONS
 *******************************************************************/

/* WadArchive::WadArchive
 * WadArchive class constructor
 *******************************************************************/
WadArchive::WadArchive() : TreelessArchive(ARCHIVE_WAD) {
	// Init variables
	iwad = false;
}

/* WadArchive::~WadArchive
 * WadArchive class destructor
 *******************************************************************/
WadArchive::~WadArchive() {
}

/* WadArchive::getEntryOffset
 * Returns the file byte offset for [entry]
 *******************************************************************/
uint32_t WadArchive::getEntryOffset(ArchiveEntry* entry) {
	// Check entry
	if (!checkEntry(entry))
		return 0;

	return (uint32_t)(int)entry->exProp("Offset");
}

/* WadArchive::setEntryOffset
 * Sets the file byte offset for [entry]
 *******************************************************************/
void WadArchive::setEntryOffset(ArchiveEntry* entry, uint32_t offset) {
	// Check entry
	if (!checkEntry(entry))
		return;

	entry->exProp("Offset") = (int)offset;
}

/* WadArchive::updateNamespaces
 * Updates the namespace list
 *******************************************************************/
void WadArchive::updateNamespaces() {
	// Clear current namespace info
	while (namespaces.size() > 0)
		namespaces.pop_back();

	// Go through all entries
	for (unsigned a = 0; a < numEntries(); a++) {
		ArchiveEntry* entry = getRoot()->getEntry(a);

		// Check for namespace begin
		if (entry->getName().Matches("*_START")) {
			// Create new namespace
			wad_ns_pair_t ns(entry, NULL);
			string name = entry->getName();
			ns.name = name.Left(name.Length() - 6).Lower();

			// Add to namespace list
			namespaces.push_back(ns);
		}
		// Check for namespace end
		else if (entry->getName().Matches("?_END") || entry->getName().Matches("??_END")) {
			// Get namespace 'name'
			int len = entry->getName().Length() - 4;
			string ns_name = entry->getName().Left(len).Lower();

			// Check if it's the end of an existing namespace
			for (unsigned a = 0; a < namespaces.size(); a++) {
				if (s_cmp(ns_name, namespaces[a].name)) {
					namespaces[a].end = entry;
				}
			}
		}
	}

	// Check namespaces
	for (unsigned a = 0; a < namespaces.size(); a++) {
		wad_ns_pair_t& ns = namespaces[a];

		// Check the namespace has an end
		if (!ns.end) {
			// If not, remove the namespace as it is invalid
			namespaces.erase(namespaces.begin() + a);
			a--;
			continue;
		}

		// Check namespace name for special cases (at the moment these are just mapping to zdoom's "zip as wad" namespace folders)
		// http://zdoom.org/wiki/Using_ZIPs_as_WAD_replacement#How_to
		if (s_cmp(ns.name, "p") || s_cmp(ns.name, "pp"))
			ns.name = "patches";
		else if (s_cmp(ns.name, "s") || s_cmp(ns.name, "ss"))
			ns.name = "sprites";
		else if (s_cmp(ns.name, "f") || s_cmp(ns.name, "ff"))
			ns.name = "flats";
		else if (s_cmp(ns.name, "tx"))
			ns.name = "textures";
		else if (s_cmp(ns.name, "hi"))
			ns.name = "hires";
		else if (s_cmp(ns.name, "c"))
			ns.name = "colormaps";
		else if (s_cmp(ns.name, "a"))
			ns.name = "acs";
		else if (s_cmp(ns.name, "v"))
			ns.name = "voices";

		// Testing
		//wxLogMessage("Namespace %s", chr(ns.name));
	}
}

/* WadArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string WadArchive::getFileExtensionString() {
	return "Wad Files (*.wad)|*.wad";
}

/* WadArchive::getFormat
 * Returns the EntryDataFormat id of this archive type
 *******************************************************************/
string WadArchive::getFormat() {
	return "archive_wad";
}

/* WadArchive::open
 * Reads a wad format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WadArchive::open(string filename) {
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

/* WadArchive::open
 * Reads wad format data from an ArchiveEntry
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WadArchive::open(ArchiveEntry* entry) {
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

/* WadArchive::open
 * Reads wad format data from a MemChunk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WadArchive::open(MemChunk& mc) {
	// Check data was given
	if (!mc.hasData())
		return false;

	// Read wad header
	uint32_t	num_lumps = 0;
	uint32_t	dir_offset = 0;
	char		wad_type[4] = "";
	mc.seek(0, SEEK_SET);
	mc.read(&wad_type, 4);		// Wad type
	mc.read(&num_lumps, 4);		// No. of lumps in wad
	mc.read(&dir_offset, 4);	// Offset to directory

	// Byteswap values for big endian if needed
	num_lumps = wxINT32_SWAP_ON_BE(num_lumps);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);

	// Check the header
	if (wad_type[1] != 'W' || wad_type[2] != 'A' || wad_type[3] != 'D') {
		wxLogMessage("WadArchive::openFile: File %s has invalid header", filename.c_str());
		Global::error = "Invalid wad header";
		return false;
	}

	// Check for iwad
	if (wad_type[0] == 'I')
		iwad = true;

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Read the directory
	mc.seek(dir_offset, SEEK_SET);
	theSplashWindow->setProgressMessage("Reading wad archive data");
	for (uint32_t d = 0; d < num_lumps; d++) {
		// Update splash window progress
		theSplashWindow->setProgress(((float)d / (float)num_lumps));

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
			wxLogMessage("WadArchive::open: Wad archive is invalid or corrupt");
			Global::error = "Archive is invalid and/or corrupt";
			setMuted(false);
			return false;
		}

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(wxString::FromAscii(name), size);
		nlump->setLoaded(false);
		nlump->exProp("Offset") = (int)offset;
		nlump->setState(0);

		// Add to entry list
		getRoot()->addEntry(nlump);
	}

	// Detect namespaces (needs to be done before type detection as some types
	// rely on being within certain namespaces)
	updateNamespaces();

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

		// Unload entry data if needed
		if (!archive_load_data)
			entry->unloadData();

		// Lock entry if IWAD
		if (wad_type[0] == 'I' && iwad_lock)
			entry->lock();

		// Set entry to unchanged
		entry->setState(0);
	}

	// Detect maps (will detect map entry types)
	theSplashWindow->setProgressMessage("Detecting maps");
	detectMaps();

	// Setup variables
	setMuted(false);
	setModified(false);
	if (iwad && iwad_lock) read_only = true;
	announce("opened");

	theSplashWindow->setProgressMessage("");

	return true;
}

/* WadArchive::write
 * Writes the wad archive to a file
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WadArchive::write(string filename, bool update) {
	// Write to a MemChunk, then export it to a file
	MemChunk mc;
	if (write(mc, true))
		return mc.exportFile(filename);
	else
		return false;
}

/* WadArchive::write
 * Writes the wad archive to a MemChunk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WadArchive::write(MemChunk& mc, bool update) {
	// Determine directory offset & individual lump offsets
	uint32_t dir_offset = 12;
	ArchiveEntry* entry = NULL;
	for (uint32_t l = 0; l < numEntries(); l++) {
		entry = getEntry(l);
		setEntryOffset(entry, dir_offset);
		dir_offset += entry->getSize();
	}

	// Clear/init MemChunk
	mc.clear();
	mc.seek(0, SEEK_SET);
	mc.reSize(dir_offset + numEntries() * 16);

	// Setup wad type
	char wad_type[4] = { 'P', 'W', 'A', 'D' };
	if (iwad) wad_type[0] = 'I';

	// Write the header
	uint32_t num_lumps = numEntries();
	mc.write(wad_type, 4);
	mc.write(&num_lumps, 4);
	mc.write(&dir_offset, 4);

	// Write the lumps
	for (uint32_t l = 0; l < num_lumps; l++) {
		entry = getEntry(l);
		mc.write(entry->getData(), entry->getSize());
	}

	// Write the directory
	for (uint32_t l = 0; l < num_lumps; l++) {
		entry = getEntry(l);
		char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		long offset = getEntryOffset(entry);
		long size = entry->getSize();

		for (size_t c = 0; c < entry->getName().length() && c < 8; c++)
			name[c] = entry->getName()[c];

		mc.write(&offset, 4);
		mc.write(&size, 4);
		mc.write(name, 8);

		if (update) {
			entry->setState(0);
			entry->exProp("Offset") = (int)offset;
		}
	}

	return true;
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
	wxFile file(filename);

	// Check if opening the file failed
	if (!file.IsOpened()) {
		wxLogMessage("WadArchive::loadEntryData: Failed to open wadfile %s", filename.c_str());
		return false;
	}

	// Seek to lump offset in file and read it in
	file.Seek(getEntryOffset(entry), wxFromStart);
	entry->importFileStream(file, entry->getSize());

	// Set the lump to loaded
	entry->setLoaded();

	return true;
}

/* WadArchive::addEntry
 * Override of Archive::addEntry to force entry addition to the root
 * directory, update namespaces if needed and rename the entry if
 * necessary to be wad-friendly (8 characters max and no file
 * extension)
 *******************************************************************/
ArchiveEntry* WadArchive::addEntry(ArchiveEntry* entry, unsigned position, ArchiveTreeNode* dir, bool copy) {
	// Check entry
	if (!entry)
		return NULL;

	// Check if read-only
	if (isReadOnly())
		return NULL;

	// Copy if necessary
	if (copy)
		entry = new ArchiveEntry(*entry);

	// Process name (must be 8 characters max, also cut any extension as wad entries don't usually want them)
	wxFileName fn(entry->getName());
	string name = fn.GetName().Truncate(8);
	name.MakeUpper();

	// Set new wad-friendly name
	entry->setName(name);

	// Do default entry addition (to root directory)
	Archive::addEntry(entry, position);

	// Update namespaces if necessary
	if (name.Upper().Matches("*_START") ||
		name.Upper().Matches("*_END"))
		updateNamespaces();

	return entry;
}

/* WadArchive::addEntry
 * Adds [entry] to the end of the namespace matching [add_namespace].
 * If [copy] is true a copy of the entry is added. Returns the added
 * entry or NULL if the entry is invalid
 *******************************************************************/
ArchiveEntry* WadArchive::addEntry(ArchiveEntry* entry, string add_namespace, bool copy) {
	// Find requested namespace
	for (unsigned a = 0; a < namespaces.size(); a++) {
		if (s_cmpnocase(namespaces[a].name, add_namespace)) {
			// Namespace found, add entry before end marker
			return addEntry(entry, entryIndex(namespaces[a].end), NULL, copy);
		}
	}

	// Namespace not found, add to global namespace (ie end of archive)
	return addEntry(entry, 0xFFFFFFFF, NULL, copy);
}

/* WadArchive::removeEntry
 * Override of Archive::removeEntry to update namespaces if needed
 *******************************************************************/
bool WadArchive::removeEntry(ArchiveEntry* entry, bool delete_entry) {
	// Check entry
	if (!checkEntry(entry))
		return false;

	// Get entry name (for later)
	string name = entry->getName();

	// Do default remove
	bool ok = Archive::removeEntry(entry, delete_entry);

	if (ok) {
		// Update namespaces if necessary
		if (name.Upper().Matches("*_START") ||
			name.Upper().Matches("*_END"))
			updateNamespaces();

		return true;
	}
	else
		return false;
}

/* WadArchive::renameEntry
 * Override of Archive::renameEntry to update namespaces if needed
 * and rename the entry if necessary to be wad-friendly (8 characters
 * max and no file extension)
 *******************************************************************/
bool WadArchive::renameEntry(ArchiveEntry* entry, string name) {
	// Check entry
	if (!checkEntry(entry))
		return false;

	// Process name (must be 8 characters max, also cut any extension as wad entries don't usually want them)
	wxFileName fn(name);
	name = fn.GetName().Truncate(8);
	name.MakeUpper();

	// Do default rename
	bool ok = Archive::renameEntry(entry, name);

	if (ok) {
		// Update namespaces if necessary
		if (entry->getName().Upper().Matches("*_START") ||
			entry->getName().Upper().Matches("*_END"))
			updateNamespaces();

		return true;
	}
	else
		return false;
}

/* WadArchive::swapEntries
 * Override of Archive::swapEntries to update namespaces if needed
 *******************************************************************/
bool WadArchive::swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) {
	// Check entries
	if (!checkEntry(entry1) || !checkEntry(entry2))
		return false;

	// Do default swap (force root dir)
	bool ok = Archive::swapEntries(entry1, entry2);

	if (ok) {
		// Update namespaces if needed
		if (entry1->getName().Upper().Matches("*_START") ||
			entry1->getName().Upper().Matches("*_END") ||
			entry2->getName().Upper().Matches("*_START") ||
			entry2->getName().Upper().Matches("*_END"))
			updateNamespaces();

		return true;
	}
	else
		return false;
}

/* WadArchive::moveEntry
 * Override of Archive::moveEntry to update namespaces if needed
 *******************************************************************/
bool WadArchive::moveEntry(ArchiveEntry* entry, unsigned position, ArchiveTreeNode* dir) {
	// Check entry
	if (!checkEntry(entry))
		return false;

	// Do default move (force root dir)
	bool ok = Archive::moveEntry(entry, position, NULL);

	if (ok) {
		// Update namespaces if necessary
		if (entry->getName().Upper().Matches("*_START") ||
			entry->getName().Upper().Matches("*_END"))
			updateNamespaces();

		return true;
	}
	else
		return false;
}

/* WadArchive::detectMaps
 * Searches for any maps in the wad and adds them to the map list
 *******************************************************************/
vector<Archive::mapdesc_t> WadArchive::detectMaps() {
	vector<mapdesc_t> maps;

	// Go through all lumps
	size_t i = 0;
	while (i < numEntries()) {
		ArchiveEntry* entry = getEntry(i);

		// UDMF format map check ********************************************************

		// Check for UDMF format map lump (TEXTMAP lump)
		if (entry->getName() == "TEXTMAP" && i > 0) {
			// Get map info
			mapdesc_t md;
			md.head = getEntry(i - 1); // Header lump
			md.name = getEntry(i - 1)->getName(); // Map title
			md.format = 2; // Format = 2 (UDMF)

			// Skip lumps until we find the ENDMAP marker
			bool done = false;
			while (!done) {
				// If we've somehow reached the end of the wad without finding ENDMAP,
				// log an error and return
				if (i == numEntries()) {
					wxLogMessage("UDMF Map with no ENDMAP marker in %s", filename.c_str());
					return maps;
				}

				// If ENDMAP marker is here, exit the loop, otherwise skip to next lump
				if (entry->getName() == "ENDMAP")
					done = true;
				else
					i++;
			}

			// Set end lump
			md.end = entry;

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
			if (entry->getName() == map_lumps[a]) {
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
					if (entry->getName() == map_lumps[a]) {
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
				md.head = getEntry(header_index);				// Header lump
				md.name = getEntry(header_index)->getName();	// Map title
				md.end = getEntry(i - 1);						// End lump

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

/* WadArchive::detectNamespace
 * Returns the namespace that [entry] is within
 *******************************************************************/
string WadArchive::detectNamespace(ArchiveEntry* entry) {
	// Get entry index
	int index = entryIndex(entry);

	// Go through namespaces
	for (unsigned a = 0; a < namespaces.size(); a++) {
		// Get namespace start and end indices
		int start = entryIndex(namespaces[a].start);
		int end = entryIndex(namespaces[a].end);

		// Check if the entry is within this namespace
		if (start < index && index < end)
			return namespaces[a].name;
	}

	// In no namespace
	return "global";
}

/* WadArchive::findFirst
 * Returns the first entry matching the search criteria in [options],
 * or NULL if no matching entry was found
 *******************************************************************/
ArchiveEntry* WadArchive::findFirst(search_options_t& options) {
	// Init search variables
	int start = 0;
	int end = numEntries();
	options.match_name = options.match_name.Lower();

	// Check for namespace to search
	if (!options.match_namespace.IsEmpty()) {
		// Find matching namespace
		for (unsigned a = 0; a < namespaces.size(); a++) {
			if (namespaces[a].name == options.match_namespace) {
				start = entryIndex(namespaces[a].start) + 1;
				end = entryIndex(namespaces[a].end);
				break;
			}
		}
	}

	// Begin search
	ArchiveEntry* entry = NULL;
	for (int a = start; a < end; a++) {
		entry = getEntry(a);

		// Check type
		if (options.match_type) {
			if (options.match_type != entry->getType())
				continue;
		}

		// Check name
		if (!options.match_name.IsEmpty()) {
			if (!options.match_name.Matches(entry->getName().Lower()))
				continue;
		}

		// Entry passed all checks so far, so we found a match
		return entry;
	}

	// No match found
	return NULL;
}

/* WadArchive::findLast
 * Returns the last entry matching the search criteria in [options],
 * or NULL if no matching entry was found
 *******************************************************************/
ArchiveEntry* WadArchive::findLast(search_options_t& options) {
	// Init search variables
	int start = 0;
	int end = numEntries();
	options.match_name = options.match_name.Lower();

	// Check for namespace to search
	if (!options.match_namespace.IsEmpty()) {
		// Find matching namespace
		for (unsigned a = 0; a < namespaces.size(); a++) {
			if (namespaces[a].name == options.match_namespace) {
				start = entryIndex(namespaces[a].start) + 1;
				end = entryIndex(namespaces[a].end);
				break;
			}
		}
	}

	// Begin search (bottom-up)
	ArchiveEntry* entry = NULL;
	for (int a = end - 1; a >= start; a--) {
		entry = getEntry(a);

		// Check type
		if (options.match_type) {
			if (options.match_type != entry->getType())
				continue;
		}

		// Check name
		if (!options.match_name.IsEmpty()) {
			if (!options.match_name.Matches(entry->getName().Lower()))
				continue;
		}

		// Entry passed all checks so far, so we found a match
		return entry;
	}

	// No match found
	return NULL;
}

/* WadArchive::findAll
 * Returns all entries matching the search criteria in [options]
 *******************************************************************/
vector<ArchiveEntry*> WadArchive::findAll(search_options_t& options) {
	// Init search variables
	int start = 0;
	int end = numEntries();
	options.match_name = options.match_name.Lower();
	vector<ArchiveEntry*> ret;

	// Check for namespace to search
	if (!options.match_namespace.IsEmpty()) {
		// Find matching namespace
		for (unsigned a = 0; a < namespaces.size(); a++) {
			if (namespaces[a].name == options.match_namespace) {
				start = entryIndex(namespaces[a].start) + 1;
				end = entryIndex(namespaces[a].end);
				break;
			}
		}
	}

	// Begin search
	ArchiveEntry* entry = NULL;
	for (int a = start; a < end; a++) {
		entry = getEntry(a);

		// Check type
		if (options.match_type) {
			if (options.match_type != entry->getType())
				continue;
		}

		// Check name
		if (!options.match_name.IsEmpty()) {
			if (!options.match_name.Matches(entry->getName().Lower()))
				continue;
		}

		// Entry passed all checks so far, so we found a match
		ret.push_back(entry);
	}

	// Return search result
	return ret;
}



/* WadArchive::isWadArchive
 * Checks if the given data is a valid Doom wad archive
 *******************************************************************/
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

/* WadArchive::isWadArchive
 * Checks if the file at [filename] is a valid Doom wad archive
 *******************************************************************/
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
