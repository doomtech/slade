
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ArchiveEntry.cpp
 * Description: ArchiveEntry, the base archive entry class.
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
#include "Archive.h"


/* ArchiveEntry::ArchiveEntry
 * ArchiveEntry class constructor
 *******************************************************************/
ArchiveEntry::ArchiveEntry(string name, Archive* parent) {
	// Initialise attributes
	this->parent = parent;
	this->name = name;
	this->data = NULL;
	this->size = 0;
	this->data_loaded = false;
}

/* ArchiveEntry::~ArchiveEntry
 * ArchiveEntry class destructor
 *******************************************************************/
ArchiveEntry::~ArchiveEntry() {
	// Delete any entry data if it exists
	if (data)
		delete[] data;
}

/* ArchiveEntry::getData
 * Returns a pointer to the entry data. If no entry data exists and
 * allow_load is true, entry data will be loaded from it's parent
 * archive (if it exists)
 *******************************************************************/
BYTE* ArchiveEntry::getData(bool allow_load) {
	// Load the data if needed (and possible)
	if (allow_load && !isLoaded() && parent) {
		data_loaded = parent->loadEntryData(this);
	}

	// Return entry data
	return data;
}

/* ArchiveEntry::hasExProp
 * Checks if the entry has the specified extra property value
 *******************************************************************/
bool ArchiveEntry::hasExProp(string key)
{
	// Try to find specified key
	if (ex_props.find(key) == ex_props.end())
		return false;
	else
		return true;
}

/* ArchiveEntry::getExProp
 * Gets an extra property value, if it doesn't exist returns
 * an empty string
 *******************************************************************/
string ArchiveEntry::getExProp(string key)
{
	if (hasExProp(key))
		return ex_props[key];
	else
		return _T("");
}

/* ArchiveEntry::setExProp
 * Sets an extra property value, returns true if key already
 * existed and was overwritten
 *******************************************************************/
bool ArchiveEntry::setExProp(string key, string value)
{
	bool exists = hasExProp(key);
	ex_props[key] = value;
	return exists;
}

/* ArchiveEntry::clearData
 * Clears entry data and resets it's size to zero
 *******************************************************************/
void ArchiveEntry::clearData()
{
	// Delete the data
	if (data)
		delete data;

	// Reset attributes
	size = 0;
	data_loaded = false;
}

/* ArchiveEntry::importMem
 * Imports a chunk of memory into the entry, resizing it and clearing
 * any currently existing data.
 * Returns false if data pointer is invalid, true otherwise
 *******************************************************************/
bool ArchiveEntry::importMem(void* data, DWORD size)
{
	// Check parameters
	if (!data)
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

/* ArchiveEntry::importMemChunk
 * Imports data from a MemChunk object into the entry, resizing it
 * and clearing any currently existing data.
 * Returns false if the MemChunk has no data, or true otherwise.
 *******************************************************************/
bool ArchiveEntry::importMemChunk(MemChunk& mc)
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

/* ArchiveEntry::importFile
 * Loads a portion of a file into the entry, overwriting any existing
 * data currently in the entry. A size of -1 means load from the
 * offset to the end of the file.
 * Returns false if the file does not exist or the given offset/size
 * are out of bounds, otherwise returns true.
 *******************************************************************/
bool ArchiveEntry::importFile(string filename, DWORD offset = 0, DWORD size = -1)
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

/* ArchiveEntry::importLump
 * Imports data from another entry into this entry, resizing it
 * and clearing any currently existing data.
 * Returns false if the entry is null, true otherwise
 *******************************************************************/
bool ArchiveEntry::importEntry(ArchiveEntry* entry)
{
	// Check parameters
	if (!entry)
		return false;

	// Copy lump data
	importMem(entry->getData(), entry->getSize());

	return true;
}
