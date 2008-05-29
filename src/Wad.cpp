
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Wad.cpp
 * Description: Wad file handling class functions
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
#include "Wad.h"
#include "Lump.h"
#include <wx/log.h>

/* Wad::Wad
 * Wad class constructor
 *******************************************************************/
Wad::Wad(BYTE format)
{
	// Init variables
	filename = _T("");
	this->format = format;
}

/* Wad::~Wad
 * Wad class destructor
 *******************************************************************/
Wad::~Wad()
{
}

/* Wad::getFileName
 * Returns the wad's filename
 *******************************************************************/
string Wad::getFileName()
{
	return filename;
}

/* Wad::isIWAD
 * Returns true if the wad is an IWAD
 *******************************************************************/
bool Wad::isIWAD()
{
	return iwad;
}

/* Wad::numLumps
 * Returns the number of lumps in the wad
 *******************************************************************/
int Wad::numLumps()
{
	return (int)lumps.size();
}

/* Wad::lumpAt
 * Returns the lump at index, NULL if index is invalid
 *******************************************************************/
Lump* Wad::lumpAt(int index)
{
	if (index < 0 || index >= (int)lumps.size())
		return NULL;

	return lumps[index];
}

/* Wad::openWadFile
 * Reads a wad format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool Wad::openWadFile(string filename, string &error)
{
	// Try to open the file
	FILE *fp = fopen(filename.ToAscii(), "rb");

	// Check if opening the file failed
	if (!fp)
	{
		wxLogMessage(_T("Wad::openFile: Failed to open wadfile %s"), filename.c_str());
		error = _T("Unable to open file");
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
	if (type[1] != 'W' || type[2] != 'A' || type[3] != 'D')
	{
		wxLogMessage(_T("Wad::openFile: File %s has invalid header"), filename.c_str());
		error = _T("Invalid wad header");
		return false;
	}

	// Read the directory
	fseek(fp, dir_offset, SEEK_SET);
	for (DWORD d = 0; d < num_lumps; d++)
	{
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
			wxLogMessage(_T("Wad::openFile: File %s is invalid or corrupt"), filename.c_str());
			error = _T("File is invalid and/or corrupt");
			return false;
		}

		// Create & setup lump
		Lump* nlump = new Lump(this);
		nlump->setName(wxString::FromAscii(name));
		nlump->setOffset(offset);
		nlump->setSize(size);

		lumps.push_back(nlump);
		wxLogMessage(_T("%s"), nlump->getName().c_str());
	}

	// Close the file
	fclose(fp);

	// Setup variables
	this->filename = filename;
	format = FORMAT_WAD;

	return true;
}

/* Wad::openZipFile
 * Reads a zip/pk3 format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool Wad::openZipFile(string filename, string &error)
{
	// Not yet implemented
	error = _T("Not Implemented");
	return false;
}

/* Wad::loadLump
 * Loads a lump's data from the wadfile
 * Returns true if successful, false otherwise
 *******************************************************************/
bool Wad::loadLump(Lump* lump)
{
	// Check that the lump belongs to this wadfile
	if (lump->getParent() != this)
	{
		wxLogMessage(_T("Wad::LoadLump: Lump %s attempting to load data from wrong parent!"), lump->getName().c_str());
		return false;
	}

	// Open wadfile
	FILE *fp = fopen(filename.ToAscii(), "rb");

	// Check if opening the file failed
	if (!fp)
	{
		wxLogMessage(_T("Wad::LoadLump: Failed to open wadfile %s"), filename.c_str());
		return false;
	}

	// Allocate memory for lump data
	BYTE* data = new BYTE[lump->getSize()];

	// Seek to lump offset in file and read it in
	fseek(fp, lump->getOffset(), SEEK_SET);
	fread(data, lump->getSize(), 1, fp);

	// Set lump data to newly allocated memory
	lump->setData(data);

	// Close file
	fclose(fp);

	return true;
}

/* Wad::detectMaps
 * Searches for any maps in the wad and adds them to the map list
 *******************************************************************/
void Wad::detectMaps()
{
}
