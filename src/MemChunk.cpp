
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MemChunk.cpp
 * Description: MemChunk class, a simple data structure for
 *              storing/handling arbitrary sized chunks of memory.
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
#include "MemChunk.h"
#include <wx/log.h>


/*******************************************************************
 * MEMCHUNK CLASS FUNCTIONS
 *******************************************************************/

/* MemChunk::MemChunk
 * MemChunk class constructor
 *******************************************************************/
MemChunk::MemChunk(uint32_t size) {
	// Init variables
	this->size = size;
	this->cur_ptr = 0;

	// If a size is specified, allocate that much memory
	if (size)
		data = new uint8_t[size];
	else
		data = NULL;
}

/* MemChunk::MemChunk
 * MemChunk class constructor taking initial data
 *******************************************************************/
MemChunk::MemChunk(const uint8_t* data, uint32_t size) {
	// Init variables
	this->cur_ptr = 0;
	this->data = NULL;
	this->size = size;

	// Load given data
	importMem(data, size);
}

/* MemChunk::~MemChunk
 * MemChunk class destructor
 *******************************************************************/
MemChunk::~MemChunk() {
	// Free memory
	if (data)
		delete[] data;
}

/* MemChunk::hasData
 * Returns true if the chunk contains data
 *******************************************************************/
bool MemChunk::hasData() {
	if (size > 0 && data)
		return true;
	else
		return false;
}

/* MemChunk::clear
 * Deletes the memory chunk.
 * Returns false if no data exists, true otherwise.
 *******************************************************************/
bool MemChunk::clear() {
	if (hasData()) {
		delete[] data;
		data = NULL;
		size = 0;
		return true;
	}

	return false;
}

/* MemChunk::reSize
 * Resizes the memory chunk, preserving existing data if specified
 * Returns false if new size is invalid, true otherwise
 *******************************************************************/
bool MemChunk::reSize(uint32_t new_size, bool preserve_data) {
	// Check for invalid new size
	if (new_size == 0) {
		wxLogMessage(_T("MemChunk::reSize: new_size cannot be 0"));
		return false;
	}

	// Resize data
	if (preserve_data)
		data = (uint8_t*)realloc(data, new_size);
	else {
		clear();
		data = new uint8_t[new_size];
	}

	// Update variables
	size = new_size;

	// Check position
	if (cur_ptr > size)
		cur_ptr = size;

	return true;
}

/* MemChunk::importFile
 * Loads a file (or part of it) into the MemChunk
 * Returns false if file couldn't be opened, true otherwise
 *******************************************************************/
bool MemChunk::importFile(string filename, uint32_t offset, uint32_t len) {
	// Open the file
	FILE *fp = fopen(chr(filename), "rb");

	// Return false if file open failed
	if (!fp) {
		wxLogMessage(_T("MemChunk::loadFile: Unable to open file %s"), filename.c_str());
		return false;
	}

	// Clear current data if it exists
	clear();

	// Get file length
	uint32_t flen = 0;
	fseek(fp, 0, SEEK_END);
	flen = (uint32_t) ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// If length isn't specified or exceeds the file length,
	// only read to the end of the file
	if (offset + len > flen || len == 0)
		len = flen - offset;

	// Setup variables
	size = len;

	// Read the file
	if (size > 0) {
		data = new uint8_t[size];

		// Read the file
		fseek(fp, offset, SEEK_SET);
		fread(data, 1, size, fp);
		fclose(fp);
	}

	return true;
}

/* MemChunk::importFileStream
 * Loads a file (or part of it) from a currently open file stream
 * into the MemChunk
 * Returns false if file couldn't be opened, true otherwise
 *******************************************************************/
bool MemChunk::importFileStream(FILE* fp, uint32_t len) {
	// Check file
	if (!fp)
		return false;

	// Clear current data if it exists
	clear();

	// Get current file position
	uint32_t offset = ftell(fp);

	// Get file length
	uint32_t flen = 0;
	fseek(fp, 0, SEEK_END);
	flen = (uint32_t) ftell(fp);
	fseek(fp, offset, SEEK_SET);

	// If length isn't specified or exceeds the file length,
	// only read to the end of the file
	if (offset + len > flen || len == 0)
		len = flen - offset;


	// Setup variables
	size = len;

	// Read the file
	if (size > 0) {
		data = new uint8_t[size];
		fread(data, 1, size, fp);
	}

	return true;
}

/* MemChunk::importMem
 * Loads a chunk of memory into the MemChunk
 * Returns false if size or data pointer is invalid, true otherwise
 *******************************************************************/
bool MemChunk::importMem(const uint8_t* start, uint32_t len) {
	// Check that length & data to be loaded are valid
	if (!start)
		return false;

	// Clear current data if it exists
	clear();

	// Setup variables
	size = len;

	// Load new data
	if (size > 0) {
		data = new uint8_t[size];
		memcpy(data, start, size);
	}

	return true;
}

/* MemChunk::saveFile
 * Writes the MemChunk data to a new file of [filename]
 *******************************************************************/
bool MemChunk::exportFile(string filename) {
	// Open file for writing
	FILE* fp = fopen(chr(filename), "wb");
	if (!fp) {
		wxLogMessage(s_fmt(_T("Unable to write to file %s"), filename.c_str()));
		return false;
	}

	// Write the data
	fwrite(data, 1, size, fp);

	// Close the file
	fclose(fp);

	return true;
}

/* MemChunk::write
 * Writes the given data at the current position. Expands the memory
 * chunk if necessary.
 *******************************************************************/
bool MemChunk::write(const void* data, uint32_t size) {
	// Check pointers
	if (!data)
		return false;

	// If we're trying to write past the end of the memory chunk,
	// resize it so we can write at this point
	if (cur_ptr + size > this->size)
		reSize(cur_ptr + size, true);

	// Write the data and move to the byte after what was written
	memcpy(this->data + cur_ptr, data, size);
	cur_ptr += size;

	// Success
	return true;
}

/* MemChunk::read
 * Reads data from the current position into <buf>. Returns false if
 * attempting to read data outside of the chunk, true otherwise
 *******************************************************************/
bool MemChunk::read(void* buf, uint32_t size) {
	// Check pointers
	if (!this->data || !buf)
		return false;

	// If we're trying to read past the end
	// of the memory chunk, return failure
	if (cur_ptr + size > this->size)
		return false;

	// Read the data and move to the byte after what was read
	memcpy(buf, this->data + cur_ptr, size);
	cur_ptr += size;

	return true;
}

/* MemChunk::seek
 * Moves the current position, works the same as fseek() etc.
 *******************************************************************/
bool MemChunk::seek(uint32_t offset, uint32_t start) {
	if (start == SEEK_CUR) {
		// Move forward from the current position
		cur_ptr += offset;
		if (cur_ptr > size)
			cur_ptr = size;
	}
	else if (start == SEEK_SET) {
		// Move to the specified offset
		cur_ptr = offset;
		if (cur_ptr > size)
			cur_ptr = size;
	}
	else if (start == SEEK_END) {
		// Move to <offset> bytes before the end of the chunk
		if (offset > size)
			cur_ptr = 0;
		else
			cur_ptr = size - offset;
	}

	// Success
	return true;
}
