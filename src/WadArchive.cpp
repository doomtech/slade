
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
#include "WadEntry.h"
#include <wx/log.h>


/* WadArchive::WadArchive
 * WadArchive class constructor
 *******************************************************************/
WadArchive::WadArchive()
: Archive() {
}

/* WadArchive::~WadArchive
 * WadArchive class destructor
 *******************************************************************/
WadArchive::~WadArchive() {
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
	char type[4] = "";
	DWORD num_lumps = 0;
	DWORD dir_offset = 0;
	fread(&type, 1, 4, fp);			// Wad type
	fread(&num_lumps, 4, 1, fp);	// No. of lumps in wad
	fread(&dir_offset, 4, 1, fp);	// Offset to directory

	// Byteswap values for big endian if needed
	num_lumps = wxINT32_SWAP_ON_BE(num_lumps);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);

	// Check the header
	if (type[1] != 'W' || type[2] != 'A' || type[3] != 'D') {
		wxLogMessage(_T("WadArchive::openFile: File %s has invalid header"), filename.c_str());
		Global::error = _T("Invalid wad header");
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
		if (offset + size > (DWORD)filesize)
		{
			wxLogMessage(_T("WadArchive::openFile: File %s is invalid or corrupt"), filename.c_str());
			Global::error = _T("File is invalid and/or corrupt");
			return false;
		}

		// Create & setup lump
		ArchiveEntry* nlump = new ArchiveEntry(_T(""), this);
		nlump->setName(wxString::FromAscii(name));
		nlump->setSize(size);
		nlump->setLoaded(false);
		nlump->setExProp(_T("offset"), s_fmt(_T("%d"), offset));

		entries.push_back(nlump);
		wxLogMessage(_T("%s"), nlump->getName().c_str());
	}

	// Close the file
	fclose(fp);

	// Setup variables
	this->filename = filename;

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
