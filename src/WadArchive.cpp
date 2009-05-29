
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


/* WadArchive::WadArchive
 * WadArchive class constructor
 *******************************************************************/
WadArchive::WadArchive()
: Archive() {
	type = ARCHIVE_WAD;

	// Default wad type
	wad_type[0] = 'P';
	wad_type[1] = 'W';
	wad_type[2] = 'A';
	wad_type[3] = 'D';
}

/* WadArchive::~WadArchive
 * WadArchive class destructor
 *******************************************************************/
WadArchive::~WadArchive() {
}

/* WadArchive::getFileExtensionString
 * Gets the wxWidgets file dialog filter string for the archive type
 *******************************************************************/
string WadArchive::getFileExtensionString() {
	return _T("Wad Files (*.wad)|*.wad");
}

/* WadArchive::openFile
 * Reads a wad format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WadArchive::openFile(string filename) {
	// Try to open the file
	FILE *fp = fopen(filename.ToAscii(), "rb");

	// Check if opening the file failed
	if (!fp) {
		wxLogMessage(_T("WadArchive::openFile: Failed to open wadfile %s"), filename.c_str());
		Global::error = _T("Unable to open file");
		return false;
	}

	// Get file size
	long filesize = 0;
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Read wad header
	DWORD num_lumps = 0;
	DWORD dir_offset = 0;
	fread(&wad_type, 1, 4, fp);		// Wad type
	fread(&num_lumps, 4, 1, fp);	// No. of lumps in wad
	fread(&dir_offset, 4, 1, fp);	// Offset to directory

	// Byteswap values for big endian if needed
	num_lumps = wxINT32_SWAP_ON_BE(num_lumps);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);

	// Check the header
	if (wad_type[1] != 'W' || wad_type[2] != 'A' || wad_type[3] != 'D') {
		wxLogMessage(_T("WadArchive::openFile: File %s has invalid header"), filename.c_str());
		Global::error = _T("Invalid wad header");
		fclose(fp);
		return false;
	}

	// Read the directory
	fseek(fp, dir_offset, SEEK_SET);
	for (DWORD d = 0; d < num_lumps; d++) {
		// Read lump info
		char name[9] = "";
		DWORD offset = 0;
		DWORD size = 0;

		fread(&offset, 4, 1, fp);	// Offset
		fread(&size, 4, 1, fp);		// Size
		fread(name, 1, 8, fp);		// Name
		name[8] = '\0';

		// Byteswap values for big endian if needed
		offset = wxINT32_SWAP_ON_BE(offset);
		size = wxINT32_SWAP_ON_BE(size);

		// If the lump data goes past the end of the file,
		// the wadfile is invalid
		if (offset + size > (DWORD)filesize) {
			wxLogMessage(_T("WadArchive::openFile: File %s is invalid or corrupt"), filename.c_str());
			Global::error = _T("File is invalid and/or corrupt");
			fclose(fp);
			return false;
		}

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(wxString::FromAscii(name), this);
		nlump->setSize(size);
		nlump->setLoaded(false);
		nlump->setExProp(_T("offset"), s_fmt(_T("%d"), offset));
		nlump->setState(0);

		// Add to entry list
		entries.push_back(nlump);
	}

	// Close the file
	fclose(fp);

	// Setup variables
	this->filename = filename;
	modified = false;
	on_disk = true;
	announce(_T("opened"));

	return true;
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
	long dir_offset = 12;
	for (DWORD l = 0; l < numEntries(); l++) {
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
	long num_lumps = numEntries();
	fwrite(wad_type, 1, 4, fp);
	fwrite(&num_lumps, 4, 1, fp);
	fwrite(&dir_offset, 4, 1, fp);

	// Write the lumps
	for (DWORD l = 0; l < num_lumps; l++)
		fwrite(entries[l]->getData(), entries[l]->getSize(), 1, fp);

	// Write the directory
	for (DWORD l = 0; l < num_lumps; l++) {
		char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		long offset = getEntryOffset(entries[l]);
		long size = entries[l]->getSize();

		for (int c = 0; c < entries[l]->getName().length(); c++)
			name[c] = entries[l]->getName()[c];

		fwrite(&offset, 4, 1, fp);
		fwrite(&size, 4, 1, fp);
		fwrite(name, 1, 8, fp);

		entries[l]->setState(0);
	}

	// Close the file
	fclose(fp);

	// Set variables and return success
	this->filename = filename;
	modified = false;
	on_disk = true;
	announce(_T("saved"));
	return true;
}

/* WadArchive::getEntryOffset
 * Gets a lump entry's offset
 * Returns the lump entry's offset, or zero if it doesn't exist
 *******************************************************************/
DWORD WadArchive::getEntryOffset(ArchiveEntry* entry) {
	if (entry->hasExProp(_T("offset"))) {
		return (DWORD)atoi(chr(entry->getExProp(_T("offset"))));
	}
	else
		return 0;
}

/* WadArchive::setEntryOffset
 * Sets a lump entry's offset
 *******************************************************************/
void WadArchive::setEntryOffset(ArchiveEntry* entry, DWORD offset) {
	entry->setExProp(_T("offset"), s_fmt(_T("%d"), offset));
}

/* WadArchive::loadEntryData
 * Loads an entry's data from the wadfile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool WadArchive::loadEntryData(ArchiveEntry* entry) {
	// Check that the lump belongs to this wadfile
	if (entry->getParent() != this) {
		wxLogMessage(_T("WadArchive::loadEntryData: Entry %s attempting to load data from wrong parent!"), entry->getName().c_str());
		return false;
	}

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

	// Allocate memory for lump data
	BYTE* data = new BYTE[entry->getSize()];

	// Seek to lump offset in file and read it in
	fseek(fp, getEntryOffset(entry), SEEK_SET);
	fread(data, entry->getSize(), 1, fp);

	// Set lump data to newly allocated memory
	entry->setData(data);

	// Close file
	fclose(fp);

	// Set the lump to loaded
	entry->setLoaded();

	return true;
}

/* WadArchive::detectMaps
 * Searches for any maps in the wad and adds them to the map list
 *******************************************************************/
vector<Archive::mapdesc_t> WadArchive::detectMaps() {
	vector<mapdesc_t> maps;

	// Go through all lumps
	int i = 0;
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
		BYTE existing_map_lumps[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

	return maps;
}

/* WadArchive::addEntry
 * Override of ArchiveEntry::addEntry to also truncate the given
 * name to 8 characters, and also stores the original full name as
 * an extra property of the entry
 *******************************************************************/
bool WadArchive::addEntry(ArchiveEntry* entry, DWORD position) {
	// Truncate name to 8 characters if needed
	string name = entry->getName();
	if (name.size() > 8) {
		entry->setExProp(_T("full_name"), name); // Add full name as extra property in case it's needed later
		name.Truncate(8);
		entry->setName(name);
	}

	// Now we can add it
	return Archive::addEntry(entry, position);
}

/* WadArchive::renameEntry
 * Override of ArchiveEntry::renameEntry to also truncate the given
 * name to 8 characters
 *******************************************************************/
bool WadArchive::renameEntry(ArchiveEntry* entry, string new_name) {
	// Truncate name to 8 characters if needed
	if (new_name.size() > 8)
		new_name.Truncate(8);

	// Now we can rename it
	return Archive::renameEntry(entry, new_name);
}
