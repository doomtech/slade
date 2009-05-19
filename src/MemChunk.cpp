
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


/* MemChunk::MemChunk
 * MemChunk class constructor
 *******************************************************************/
MemChunk::MemChunk(DWORD size) {
	// Init variables
	this->size = size;
	this->cur_ptr = 0;

	// If a size is specified, allocate that much memory
	if (size)
		data = new BYTE[size];
	else
		data = NULL;
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
bool MemChunk::reSize(DWORD new_size, bool preserve_data) {
	// Check for invalid new size
	if (new_size == 0) {
		wxLogMessage(_T("MemChunk::reSize: new_size cannot be 0"));
		return false;
	}

	// Allocate new chunk
	BYTE* new_data = new BYTE[new_size];

	// Copy existing data to new chunk if needed & current data exists
	if (preserve_data && hasData()) {
		// Determine size of data to copy
		DWORD copy_size = new_size;
		if (copy_size > size)
			copy_size = size;

		// Copy the data
		memcpy(new_data, data, copy_size);
	}

	// Clear current chunk if it exists
	clear();

	// Set data & size to new data & size
	data = new_data;
	size = new_size;

	return true;
}

/* MemChunk::loadFile
 * Loads a file (or part of it) into the MemChunk
 * Returns false if file couldn't be opened, true otherwise
 *******************************************************************/
bool MemChunk::loadFile(string filename, DWORD offset, DWORD len) {
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
	DWORD flen = 0;
	fseek(fp, 0, SEEK_END);
	flen = (DWORD) ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// If length isn't specified or exceeds the file length,
	// only read to the end of the file
	if (offset + len > flen || len == 0)
		len = flen - offset;

	// Setup variables & allocate memory
	size = len;
	data = new BYTE[size];

	// Read the file
	fseek(fp, offset, SEEK_SET);
	fread(data, 1, size, fp);
	fclose(fp);

	return true;
}

/* MemChunk::loadMem
 * Loads a chunk of memory into the MemChunk
 * Returns false if size or data pointer is invalid, true otherwise
 *******************************************************************/
bool MemChunk::loadMem(BYTE* start, DWORD len) {
	// Check that length & data to be loaded are valid
	if (len == 0 || !start) {
		wxLogMessage(_T("MemChunk::loadMem: Invalid data and/or length"));
		return false;
	}

	// Clear current data if it exists
	clear();

	// Setup variables & allocate memory
	size = len;
	data = new BYTE[size];

	// Read the memory
	memcpy(data, start, size);

	return true;
}

bool MemChunk::write(void* data, DWORD size) {
	if (cur_ptr + size > this->size)
		reSize(cur_ptr + size, true);
	memcpy(this->data + cur_ptr, data, size);
	cur_ptr += size;
	return true;
}

bool MemChunk::read(void* buf, DWORD size) {
	if (cur_ptr + size > this->size)
		return false;

	memcpy(buf, this->data + cur_ptr, size);
	cur_ptr += size;

	return true;
}

bool MemChunk::seek(DWORD offset, DWORD start) {
	if (start == SEEK_CUR) {
		cur_ptr += offset;
		if (cur_ptr > size)
			cur_ptr = size;
	}
	else if (start == SEEK_SET) {
		cur_ptr = offset;
		if (cur_ptr > size)
			cur_ptr = size;
	}
	else if (start == SEEK_END) {
		if (offset > size)
			cur_ptr = 0;
		else
			cur_ptr = size - offset;
	}

	return true;
}
