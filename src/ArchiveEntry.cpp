
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
#include <wx/log.h>
#include <wx\hash.h>


/*******************************************************************
 * STRUCTS
 *******************************************************************/
struct patch_header_t
{
	short	width;
	short	height;
	short	left;
	short	top;
};


/* ArchiveEntry::ArchiveEntry
 * ArchiveEntry class constructor
 *******************************************************************/
ArchiveEntry::ArchiveEntry(string name, Archive* parent) {
	// Initialise attributes
	this->parent = parent;
	this->name = name;
	this->data = NULL;
	this->size = 0;
	this->data_loaded = true;
	this->state = 2;
	this->type = ETYPE_UNKNOWN;
}

/* ArchiveEntry::ArchiveEntry
 * ArchiveEntry class copy constructor
 *******************************************************************/
ArchiveEntry::ArchiveEntry(ArchiveEntry& copy) {
	// Copy attributes
	parent = copy.getParent();
	name = copy.getName();
	size = copy.getSize();
	data_loaded = true;

	// Get the data to copy
	BYTE* copy_data = copy.getData(true);

	if (copy_data) {
		// Allocate memory and copy the data
		data = new BYTE[size];
		memcpy(data, copy_data, size);
	} else {
		// If we didn't get any data to copy for some reason, print an error
		// message and set the entry's size to 0
		wxLogMessage(s_fmt(_T("Unable to copy data from entry %s"), copy.getName().c_str()));
		data = NULL;
		size = 0;
	}

	// Set entry state
	state = 2;
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
bool ArchiveEntry::hasExProp(string key) {
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
string ArchiveEntry::getExProp(string key) {
	if (hasExProp(key))
		return ex_props[key];
	else
		return _T("");
}

/* ArchiveEntry::setExProp
 * Sets an extra property value, returns true if [key] already
 * existed and was overwritten
 *******************************************************************/
bool ArchiveEntry::setExProp(string key, string value) {
	bool exists = hasExProp(key);
	ex_props[key] = value;
	return exists;
}

/* ArchiveEntry::removeExProp
 * Removes an extra property value, returns true if [key] was removed
 * or false if key didn't exist
 *******************************************************************/
bool ArchiveEntry::removeExProp(string key) {
	PropertyList::iterator i = ex_props.find(key);

	if (i != ex_props.end()) {
		ex_props.erase(i);
		return true;
	}
	else
		return false;
}

/* ArchiveEntry::setState
 * Sets the entry's state. If the state is already 'new' and we try
 * to set it to 'modified' then don't change the state
 *******************************************************************/
void ArchiveEntry::setState(BYTE state) {
	if (state == 0)
		this->state = 0;
	else {
		if (state > this->state)
			this->state = state;
	}

	// Notify parent archive this entry has been modified
	if (parent)
		parent->entryModified(this);
}

void ArchiveEntry::unloadData() {
	// Check there is any data to be 'unloaded'
	if (!data || !data_loaded)
		return;

	// Only unload if the data wasn't modified
	if (getState() > 0)
		return;

	// Delete any data
	delete[] data;

	// Update variables etc
	setLoaded(false);
}

/* ArchiveEntry::rename
 * Renames the entry
 *******************************************************************/
void ArchiveEntry::rename(string new_name) {
	setName(new_name);
	setState(1);
}

/* ArchiveEntry::clearData
 * Clears entry data and resets it's size to zero
 *******************************************************************/
void ArchiveEntry::clearData() {
	// Delete the data
	if (data)
		delete data;

	// Reset attributes
	size = 0;
	data_loaded = false;
	setState(1);
}

/* ArchiveEntry::importMem
 * Imports a chunk of memory into the entry, resizing it and clearing
 * any currently existing data.
 * Returns false if data pointer is invalid, true otherwise
 *******************************************************************/
bool ArchiveEntry::importMem(void* data, DWORD size) {
	// Check parameters
	if (!data)
		return false;

	// Clear any current data
	clearData();

	// Copy data into the entry
	this->data = new BYTE[size];
	memcpy(this->data, data, size);

	// Update attributes
	this->size = size;
	setState(1);
	setLoaded();

	return true;
}

/* ArchiveEntry::importMemChunk
 * Imports data from a MemChunk object into the entry, resizing it
 * and clearing any currently existing data.
 * Returns false if the MemChunk has no data, or true otherwise.
 *******************************************************************/
bool ArchiveEntry::importMemChunk(MemChunk& mc) {
	// Check that the given MemChunk has data
	if (mc.hasData()) {
		// Copy the data from the MemChunk into the entry
		importMem(mc.getData(), mc.getSize());
		return true;
	} else
		return false;
}

/* ArchiveEntry::importFile
 * Loads a portion of a file into the entry, overwriting any existing
 * data currently in the entry. A size of 0 means load from the
 * offset to the end of the file.
 * Returns false if the file does not exist or the given offset/size
 * are out of bounds, otherwise returns true.
 *******************************************************************/
bool ArchiveEntry::importFile(string filename, DWORD offset, DWORD size) {
	// Open the file
	FILE* fp = fopen(filename.ToAscii(), "rb");

	// Check that it opened ok
	if (!fp) {
		Global::error = _T("Unable to open file for reading");
		return false;
	}

	// Get the file's size
	fseek(fp, 0, SEEK_END);
	DWORD fsize = ftell(fp);

	// Get the size to read, if zero
	if (size == 0)
		size = fsize - offset;

	// Check offset/size bounds
	if (offset + size > fsize || offset < 0) {
		fclose(fp);
		return false;
	}

	// Create temporary buffer and load file contents
	BYTE* temp_buf = new BYTE[size];
	fseek(fp, 0, SEEK_SET);
	fseek(fp, offset, SEEK_CUR);
	fread(temp_buf, 1, size, fp);

	// Close the file
	fclose(fp);

	// Import data into entry
	importMem(temp_buf, size);

	// Delete temp buffer
	delete[] temp_buf;

	return true;
}

/* ArchiveEntry::importEntry
 * Imports data from another entry into this entry, resizing it
 * and clearing any currently existing data.
 * Returns false if the entry is null, true otherwise
 *******************************************************************/
bool ArchiveEntry::importEntry(ArchiveEntry* entry) {
	// Check parameters
	if (!entry)
		return false;

	// Copy entry data
	importMem(entry->getData(), entry->getSize());

	return true;
}

/* ArchiveEntry::exportFile
 * Exports entry data to a file.
 * Returns false if file cannot be written, true otherwise
 *******************************************************************/
bool ArchiveEntry::exportFile(string filename) {
	// Attempt to open file
	FILE* fp = fopen(filename.ToAscii(), "wb");

	// Check it opened ok
	if (!fp) {
		Global::error = _T("Unable to open file for writing");
		return false;
	}

	// Write entry data to the file
	fwrite(getData(), 1, this->size, fp);

	// Close the file
	fclose(fp);

	return true;
}

/* ArchiveEntry::detectType
 * Attempts to detect what type of data the entry holds
 *******************************************************************/
void ArchiveEntry::detectType(bool data_check, bool force) {
	// If the type is already known and we're not forcing a detect, don't bother
	if (!force && type != ETYPE_UNKNOWN)
		return;

	// Don't bother checking if it's been set as a folder
	if (type == ETYPE_FOLDER)
		return;

	// Remove any current type property
	removeExProp(_T("EntryType"));
	removeExProp(_T("TextFormat"));

	// If the entry has a parent archive attempt to detect type based on it's position in the archive
	if (parent)
		parent->detectEntryType(this);

	// Marker
	if (size == 0 && type != ETYPE_MAP) {
		type = ETYPE_MARKER;
		return;
	}

	// THINGS
	if (name == _T("THINGS"))
	{
		if (size % 10 == 0)
		{
			type = ETYPE_THINGS;
			return;
		}
	}

	// LINEDEFS
	if (name == _T("LINEDEFS"))
	{
		if (size % 14 == 0 || size % 16 == 0)
		{
			type = ETYPE_LINEDEFS;
			return;
		}
	}

	// SIDEDEFS
	if (name == _T("SIDEDEFS"))
	{
		if (size % 30 == 0)
		{
			type = ETYPE_SIDEDEFS;
			return;
		}
	}

	// SECTORS
	if (name == _T("SECTORS"))
	{
		if (size % 26 == 0)
		{
			type = ETYPE_SECTORS;
			return;
		}
	}

	// VERTEXES
	if (name == _T("VERTEXES"))
	{
		if (size % 4 == 0)
		{
			type = ETYPE_VERTEXES;
			return;
		}
	}

	// NODES
	if (name == _T("NODES"))
	{
		type = ETYPE_NODES;
		return;
	}

	// SSECTORS
	if (name == _T("SSECTORS"))
	{
		type = ETYPE_SSECTS;
		return;
	}

	// SEGS
	if (name == _T("SEGS"))
	{
		type = ETYPE_SEGS;
		return;
	}

	// REJECT
	if (name == _T("REJECT"))
	{
		type = ETYPE_REJECT;
		return;
	}

	// BLOCKMAP
	if (name == _T("BLOCKMAP"))
	{
		type = ETYPE_BLOCKMAP;
		return;
	}

	// BEHAVIOR
	if (name == _T("BEHAVIOR"))
	{
		type = ETYPE_BEHAVIOR;
		return;
	}


	// PLAYPAL
	if (name.StartsWith(_T("PLAYPAL")))
	{
		type = ETYPE_PLAYPAL;
		return;
	}

	// COLORMAP
	if (name.StartsWith(_T("COLORMAP")))
	{
		type = ETYPE_COLORMAP;
		return;
	}

	// TEXTUREx
	if (name.StartsWith(_T("TEXTURE1")) || name.StartsWith(_T("TEXTURE2")))
	{
		type = ETYPE_TEXTURES;
		return;
	}

	// PNames
	if (name.StartsWith(_T("PNAMES")))
	{
		type = ETYPE_PNAMES;
		return;
	}

	// If [data_check] is false we don't want to check the entry's data for type info
	if (!data_check)
		return;

	// Check if the entry's data is currently loaded, if it is we won't unload it
	// after data checks are done
	bool no_unload = data_loaded;

	// Load data if needed
	getData(true);

	// PNG
	if (size > 8) {
		if (data[0] == 137 && data[1] == 80 &&
			data[2] == 78 && data[3] == 71 &&
			data[4] == 13 && data[5] == 10 &&
			data[6] == 26 && data[7] == 10) {
			type = ETYPE_PNG;
			return;
		}
	}

	// BMP
	if (size > 14) {
		if (data[0] == 'B' && data[1] == 'M' &&
			data[6] == 0 && data[7] == 0 && data[8] == 0 && data[9] == 0) {
			type = ETYPE_IMAGE;
			setExProp(_T("EntryType"), _T("BMP Image"));
		}
	}

	// JPEG
	if (size > 128) {
		if ((data[6] == 'J' && data[7] == 'F' && data[8] == 'I' && data[9] == 'F') ||
			(data[6] == 'E' && data[7] == 'x' && data[8] == 'i' && data[9] == 'f')) {
			if (data[0] == 255 && data[1] == 216 && data[2] == 255) {
				type = ETYPE_IMAGE;
				setExProp(_T("EntryType"), _T("JPEG Image"));
				//gfx_checked = true;
			}
		}
	}

	// WAD
	if (size >= 12 && type == ETYPE_UNKNOWN)
	{
		if (data[1] == 'W' && data[2] == 'A' && data[3] == 'D' &&
			(data[0] == 'P' || data[0] == 'I')) {
			type = ETYPE_WAD;
			return;
		}
	}

	// MUS
	if (size > 16)
	{
		if (data[0] == 'M' && data[1] == 'U' && data[2] == 'S' && data[3] == 0x1A) {
			type = ETYPE_MUS;
			return;
		}
	}

	// MIDI
	if (size > 16)
	{
		if (data[0] == 'M' && data[1] == 'T' && data[2] == 'h' && data[3] == 'd') {
			type = ETYPE_MIDI;
			return;
		}
	}

	// IT Module
	if (size > 32)
	{
		if (data[0] == 'I' && data[1] == 'M' && data[2] == 'P' && data[3] == 'M') {
			type = ETYPE_MOD;
			setExProp(_T("LumpType"), _T("Music (IT Module)"));
			return;
		}
	}

	// XM Module
	if (size > 80)
	{
		char temp[17] = "";
		memcpy(temp, data, 17);

		if (strnicmp(temp, "Extended module: ", 17) == 0) {
			if (data[37] == 0x1a) {
				type = ETYPE_MOD;
				setExProp(_T("LumpType"), _T("Music (XM Module)"));
				return;
			}
		}
	}

	// S3M Module
	if (size > 60)
	{
		if (data[44] == 'S' && data[45] == 'C' && data[46] == 'R' && data[47] == 'M') {
			type = ETYPE_MOD;
			setExProp(_T("LumpType"), _T("Music (S3M Module)"));
			return;
		}
	}

	// MOD Module
	if (size > 1084)
	{
		if (data[950] >= 1 && data[950] <= 128 && data[951] == 127) {
			if ((data[1080] == 'M' && data[1081] == '.' && data[1082] == 'K' && data[1083] == '.') ||
				(data[1080] == 'M' && data[1081] == '!' && data[1082] == 'K' && data[1083] == '!') ||
				(data[1080] == 'F' && data[1081] == 'L' && data[1082] == 'T' && data[1083] == '4') ||
				(data[1080] == 'F' && data[1081] == 'L' && data[1082] == 'T' && data[1083] == '8') ||
				(data[1081] == 'C' && data[1082] == 'H' && data[1083] == 'N')) {
				type = ETYPE_MOD;
				setExProp(_T("LumpType"), _T("Music (MOD Module)"));
			}
		}
	}

	// Doom Sound
	if (size > 8) {
		if ((WORD)data[0] == 3 && (WORD)data[6] == 0 && (WORD)data[4] <= size - 8) {
			type = ETYPE_SOUND;

			if (name.Left(2) == _T("DS"))
				return;
		}
	}

	// WAV Sound
	if (size > 8) {
		if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F') {
			type = ETYPE_WAV;
		}
	}

	// Detect doom patch format
	if (size > sizeof(patch_header_t) && (type == ETYPE_UNKNOWN || type == ETYPE_SOUND)) {
		patch_header_t *header = (patch_header_t *)data;

		if (header->height > 0 && header->height < 4096 &&
			header->width > 0 && header->width < 4096 &&
			header->top > -2000 && header->top < 2000 &&
			header->left > -2000 && header->left < 2000) {
			long *col_offsets = (long *)((BYTE *)data + sizeof(patch_header_t));

			if (size < sizeof(patch_header_t) + (header->width * sizeof(long))) {
				//wxLogMessage("lump %s not a patch, col_offsets error 1", lump->Name().c_str());
				return;
			}

			for (int a = 0; a < header->width; a++) {
				if (col_offsets[a] > size || col_offsets[a] < 0) {
					//wxLogMessage("lump %s not a patch, col_offsets error 2", lump->Name().c_str());
					return;
				}
			}

			type = ETYPE_GFX;
		}
	}

	// Unload entry data if it wasn't loaded before checking the type
	if (!no_unload)
		unloadData();
}

/* ArchiveEntry::getTypeString
 * Returns a string representation of the entry's type
 *******************************************************************/
string ArchiveEntry::getTypeString() {
	// Check for a custom type string
	if (hasExProp(_T("EntryType")))
		return getExProp(_T("EntryType"));

	if (type == ETYPE_MARKER) return _T("Marker");
	if (type == ETYPE_TEXT) return _T("Text");

	if (type == ETYPE_PATCH) return _T("Patch");
	if (type == ETYPE_SPRITE) return _T("Sprite");
	if (type == ETYPE_FLAT) return _T("Flat");
	if (type == ETYPE_GFX) return _T("Graphic");
	if (type == ETYPE_GFX2) return _T("Graphic");
	if (type == ETYPE_PNG) return _T("PNG Image");
	if (type == ETYPE_IMAGE) return _T("Image");

	if (type == ETYPE_SOUND) return _T("Sound");
	if (type == ETYPE_WAV) return _T("WAV Sound");
	if (type == ETYPE_MP3) return _T("MP3 Audio");
	if (type == ETYPE_FLAC) return _T("FLAC Audio");
	if (type == ETYPE_MUS) return _T("Music (MUS)");
	if (type == ETYPE_MIDI) return _T("Music (MIDI)");
	if (type == ETYPE_MOD) return _T("Music (Module)");

	if (type == ETYPE_TEXTURES) return _T("TEXTUREx");
	if (type == ETYPE_PNAMES) return _T("PNames");

	if (type == ETYPE_MAP) return _T("Map");
	if (type == ETYPE_LINEDEFS) return _T("Map Linedefs");
	if (type == ETYPE_SIDEDEFS) return _T("Map Sidedefs");
	if (type == ETYPE_VERTEXES) return _T("Map Vertices");
	if (type == ETYPE_SECTORS) return _T("Map Sectors");
	if (type == ETYPE_THINGS) return _T("Map Things");
	if (type == ETYPE_NODES) return _T("Map Nodes");
	if (type == ETYPE_SEGS) return _T("Map Segs");
	if (type == ETYPE_SSECTS) return _T("Map Ssectors");
	if (type == ETYPE_REJECT) return _T("Reject Table");
	if (type == ETYPE_BLOCKMAP) return _T("Blockmap");
	if (type == ETYPE_BEHAVIOR) return _T("Compiled ACS");

	if (type == ETYPE_PLAYPAL) return _T("Palette");
	if (type == ETYPE_COLORMAP) return _T("Colormap");

	if (type == ETYPE_WAD) return _T("Wad File");

	if (type == ETYPE_FOLDER) return _T("Directory");

	return _T("Unknown");
}
