
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
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
// Used for map detection
string map_lumps[12] =
{
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


/* Wad::Wad
 * Wad class constructor
 *******************************************************************/
Wad::Wad()
{
	// Init variables
	filename = _T("");
}

/* Wad::~Wad
 * Wad class destructor
 *******************************************************************/
Wad::~Wad()
{
}

/* Wad::getFileName
 * Returns the wad's filename, including the path if specified
 *******************************************************************/
string Wad::getFileName(bool fullpath)
{
	if (fullpath)
		return filename;
	else
	{
		// Get the filename without the path
		wxFileName fn(filename);
		return fn.GetName() + _T(".") + fn.GetExt();
	}
}

/* Wad::isIWAD
 * Returns true if the wad is an IWAD
 *******************************************************************/
bool Wad::isIWAD()
{
	return iwad;
}

/* Wad::checkLumpIndex
 * Checks if a lump index is valid
 *******************************************************************/
bool Wad::checkLumpIndex(int index)
{
	return (index >= 0 && index < (int)lumps.size());
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
	// Check for invalid index
	if (!checkLumpIndex(index))
		return NULL;

	return lumps[index];
}

/* Wad::lumpIndex
 * Returns the directory index of the specified lump,
 * if the lump doesn't exist in the wad returns -1
 *******************************************************************/
int Wad::lumpIndex(Lump* lump)
{
	// Check lump's parent
	if (lump->getParent() != this)
		return -1;

	// Go through all lumps
	for (int a = 0; a < numLumps(); a++)
	{
		// If lump matches return the current index
		if (lumps[a] == lump)
			return a;
	}

	// Lump not found, return -1 (shouldn't ever happen)
	return -1;
}

/* Wad::openWadFile
 * Reads a wad format file from disk
 * Returns true if successful, false otherwise
 *******************************************************************/
bool Wad::openFile(string filename, string &error)
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
		nlump->setLoaded(false);

		lumps.push_back(nlump);
		wxLogMessage(_T("%s"), nlump->getName().c_str());
	}

	// Close the file
	fclose(fp);

	// Setup variables
	this->filename = filename;

	return true;
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

	// Do nothing if the lump's size is zero,
	// or if it has already been loaded
	if (lump->getSize() == 0 || lump->isLoaded())
	{
		lump->setLoaded();
		return true;
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

	// Set the lump to loaded
	lump->setLoaded();

	return true;
}

/* Wad::detectMaps
 * Searches for any maps in the wad and adds them to the map list
 *******************************************************************/
vector<Wad::mapdesc_t> Wad::detectMaps()
{
	vector<mapdesc_t> maps;

	// Go through all lumps
	int i = 0;
	while (i < numLumps())
	{
		// UDMF format map check ********************************************************

		// Check for UDMF format map lump (TEXTMAP lump)
		if (lumps[i]->getName() == _T("TEXTMAP") && i > 0)
		{
			// Get map info
			mapdesc_t md;
			md.head = lumps[i-1];				// Header lump
			md.name = lumps[i-1]->getName();	// Map title
			md.format = 2;						// Format = 2 (UDMF)

			// Skip lumps until we find the ENDMAP marker
			bool done = false;
			while (!done)
			{
				// If we've somehow reached the end of the wad without finding ENDMAP,
				// log an error and return
				if (i == numLumps())
				{
					wxLogMessage(_T("UDMF Map with no ENDMAP marker in %s"), filename.c_str());
					return maps;
				}

				// If ENDMAP marker is here, exit the loop, otherwise skip to next lump
				if (lumps[i]->getName() == _T("ENDMAP"))
					done = true;
				else
					i++;
			}

			// Set end lump
			md.end = lumps[i];

			// Add to map list
			maps.push_back(md);

			// Current index is ENDMAP, we don't want to check for a doom/hexen format
			// map so just go to the next index and continue the loop
			i++;
			continue;
		}



		// Doom/Hexen format map check **************************************************

		// Array to keep track of what doom/hexen map lumps have been found
		BYTE existing_map_lumps[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		// Check if the current lump is a doom/hexen map lump
		bool maplump_found = false;
		for (int a = 0; a < 5; a++)
		{
			// Compare with all base map lump names
			if (lumps[i]->getName() == map_lumps[a])
			{
				maplump_found = true;
				existing_map_lumps[a] = 1;
				break;
			}
		}

		// If we've found what might be a map
		if (maplump_found)
		{
			// Save position of map header lump
			int header_index = i-1;

			// Check off map lumps until we find a non-map lump
			bool done = false;
			while (!done)
			{
				// Loop will end if no map lump is found
				done = true;

				// If we're at the end of the wad, exit the loop
				if (i == numLumps())
				{
				    i--;
                    break;
				}

				// Compare with all map lump names
				for (int a = 0; a < 12; a++)
				{
					// Compare with all base map lump names
					if (lumps[i]->getName() == map_lumps[a])
					{
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
			if (!memchr(existing_map_lumps, 0, 5))
			{
				// Get map info
				mapdesc_t md;
				md.head = lumps[header_index];				// Header lump
				md.name = lumps[header_index]->getName();	// Map title
				md.end = lumps[i-1];						// End lump

				// If BEHAVIOR lump exists, it's a hexen format map, otherwise it's doom format
				if (existing_map_lumps[11])
					md.format = 1;
				else
					md.format = 0;

				// Add map info to the maps list
				maps.push_back(md);
			}
			else
			{
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

/* Wad::addLump
 * Adds the specified lump to the list before the specified index.
 * Returns false if the index is invalid or lump is NULL, true otherwise.
 *******************************************************************/
bool Wad::addLump(Lump* lump, int index)
{
	// Check for invalid index
	if (!checkLumpIndex(index))
		return false;

	// Set the lump's parent
	lump->setParent(this);

	// Add the lump to the list before index
	lumps.insert(lumps.begin() + index, lump);

	return true;
}

/* Wad::addLumpEnd
 * Adds the specified lump to the end of the list.
 * Returns false if lump is NULL, true otherwise.
 *******************************************************************/
bool Wad::addLumpEnd(Lump* lump)
{
	// Check for null lump
	if (!lump)
		return false;

	// Set the lump's parent
	lump->setParent(this);

	// Add the lump to the end of the list
	lumps.push_back(lump);

	return true;
}

/* Wad::removeLump
 * Removes the lump at index from the lump list, but doesn't delete
 * the lump itself. Returns false if index is invalid, true otherwise
 *******************************************************************/
bool Wad::removeLump(int index)
{
	// Check for invalid index
	if (!checkLumpIndex(index))
		return false;

	// De-parent the lump
	lumps[index]->setParent(NULL);

	// Remove the lump from the list
	lumps.erase(lumps.begin() + index);

	return true;
}

/* Wad::removeLump
 * Removes the lump specified from the lump list, but doesn't delete
 * the lump itself. Returns false if the lump does not exist in the
 * wad.
 *******************************************************************/
bool Wad::removeLump(Lump* lump)
{
	// Check for null lump
	if (!lump)
		return false;

	// Check the lump's parent
	if (lump->getParent() != this)
		return false;

	// Get the index of the lump
	int index = lumpIndex(lump);

	// If the index is valid (not -1), remove the lump from the list
	if (index >= 0)
	{
		// De-parent the lump
		lump->setParent(NULL);

		// Remove
		lumps.erase(lumps.begin() + index);
		return true;
	}
	else
		return false;
}
