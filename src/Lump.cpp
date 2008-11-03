
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Lump.cpp
 * Description: Lump class functions
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

/* Lump::Lump
 * Lump class constructor
 *******************************************************************/
Lump::Lump(Wad* parent_)
{
	// Init variables
	name = _T("");
	this->parent = parent;
	data = NULL;
	size = 0;
	offset = -1;
	data_loaded = true;
}

/* Lump::~Lump
 * Lump class destructor
 *******************************************************************/
Lump::~Lump()
{
	if (parent)
		parent->removeLump(this);
}

/* Lump::getData
 * Returns a pointer to the lump data. If no lump data exists and
 * allow_load is true, lump data will be loaded from it's
 * parent wadfile (if it exists)
 *******************************************************************/
BYTE* Lump::getData(bool allow_load)
{
	if (allow_load && parent && !data_loaded)
		parent->loadLump(this);

	return data;
}

/* Lump::hasExProp
 * Checks if the lump has the specified extra property value
 *******************************************************************/
bool Lump::hasExProp(string key)
{
	// Try to find specified key
	if (ex_props.find(key) == ex_props.end())
		return false;
	else
		return true;
}

/* Lump::getExProp
 * Gets an extra property value, if it doesn't exist returns
 * an empty string
 *******************************************************************/
string Lump::getExProp(string key)
{
	if (hasExProp(key))
		return ex_props[key];
	else
		return _T("");
}

/* Lump::setExProp
 * Sets an extra property value, returns true if key already
 * existed and was overwritten
 *******************************************************************/
bool Lump::setExProp(string key, string value)
{
	bool exists = hasExProp(key);
	ex_props[key] = value;
	return exists;
}

/* Lump::clearData
 * Clears lump data and resets it's size to zero
 *******************************************************************/
void Lump::clearData()
{
	// Delete the data
	if (data)
		delete data;

	// Reset attributes
	size = 0;
	offset = -1;
}

/* Lump::importMem
 * Imports a chunk of memory into the lump, resizing it and clearing
 * any currently existing data.
 * Returns false if data pointer is invalid or size is negative,
 * or true otherwise.
 *******************************************************************/
bool Lump::importMem(void* data, DWORD size)
{
	// Check parameters
	if (!data || size < 0)
		return false;

	// Clear any current data
	clearData();

	// Copy data into the lump
	this->data = new BYTE[size];
	memcpy(this->data, data, size);

	// Update attributes
	this->size = size;

	return true;
}

/* Lump::importMemChunk
 * Imports data from a MemChunk object into the lump, resizing it
 * and clearing any currently existing data.
 * Returns false if the MemChunk has no data, or true otherwise.
 *******************************************************************/
bool Lump::importMemChunk(MemChunk& mc)
{
	// Check that the given MemChunk has data
	if (mc.hasData())
	{
		// Copy the data from the MemChunk into the lump
		importMem(mc.getData(), mc.getSize());
		return true;
	}
	else
		return false;
}

/* Lump::importFile
 * Loads a portion of a file into the lump, overwriting any existing
 * data currently in the lump. A size of -1 means load from the
 * offset to the end of the file.
 * Returns false if the file does not exist or the given offset/size
 * are out of bounds, otherwise returns true.
 *******************************************************************/
bool Lump::importFile(string filename, DWORD offset = 0, DWORD size = -1)
{
	// Open the file
	FILE* fp = fopen(filename.ToAscii(), "rb");

	// Check that it opened ok
	if (!fp)
		return false;

	// Get the file's size
	fseek(fp, 0, SEEK_END);
	DWORD fsize = ftell(fp);

	// Get the size to read, if negative
	if (size < 0)
		size = fsize - offset;

	// Check offset/size bounds
	if (offset + size > fsize || offset < 0)
	{
		fclose(fp);
		return false;
	}

	// Create temporary buffer and load file contents
	BYTE* temp_buf = new BYTE[size];
	fseek(fp, offset, SEEK_SET);
	fread(temp_buf, size, 1, fp);

	// Import data into lump
	importMem(temp_buf, size);

	// Delete temp buffer
	delete temp_buf;

	return true;
}

/* Lump::importLump
 * Imports data from another lump into this lump, resizing it
 * and clearing any currently existing data.
 * Returns false if the MemChunk has no data, or true otherwise.
 *******************************************************************/
bool Lump::importLump(Lump* lump)
{
	// Check parameters
	if (!lump)
		return false;

	// Copy lump data
	importMem(lump->getData(), lump->getSize());

	return true;
}
