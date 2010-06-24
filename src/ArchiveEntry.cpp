
#include "Main.h"
#include "ArchiveEntry.h"
#include "Archive.h"
#include "Misc.h"
#include <wx/filename.h>

/* ArchiveEntry::ArchiveEntry
 * ArchiveEntry class constructor
 *******************************************************************/
ArchiveEntry::ArchiveEntry(string name, uint32_t size) {
	// Initialise attributes
	this->parent = NULL;
	this->name = name;
	this->data = NULL;
	this->size = size;
	this->data_loaded = true;
	this->state = 2;
	this->type = EntryType::unknownType();
	this->locked = false;
	this->state_locked = false;
}

/* ArchiveEntry::ArchiveEntry
 * ArchiveEntry class copy constructor
 *******************************************************************/
ArchiveEntry::ArchiveEntry(ArchiveEntry& copy) {
	// Copy attributes
	//parent = copy.getParent();
	name = copy.getName();
	size = copy.getSize();
	data_loaded = true;
	type = copy.type;
	locked = false;

	// Copy data
	data.importMem(copy.getData(true), copy.getSize());

	// Copy extra properties
	copy.exProps().copyTo(ex_props);

	// Set entry state
	state = 2;
	state_locked = false;
}

ArchiveEntry::~ArchiveEntry() {
}

string ArchiveEntry::getName(bool cut_ext) {
	if (!cut_ext)
		return name;

	wxFileName fn(name);
	return fn.GetName();
}

Archive* ArchiveEntry::getParent() {
	if (parent)
		return parent->getArchive();
	else
		return NULL;
}

string ArchiveEntry::getPath(bool name) {
	// Get the entry path
	string path = parent->getPath();

	if (name)
		return path + getName();
	else
		return path;
}

/* ArchiveEntry::getData
 * Returns a pointer to the entry data. If no entry data exists and
 * allow_load is true, entry data will be loaded from its parent
 * archive (if it exists)
 *******************************************************************/
const uint8_t* ArchiveEntry::getData(bool allow_load) {
	// Return entry data
	return getMCData(allow_load).getData();
}

/* ArchiveEntry::getMCData
 * Returns the entry data MemChunk. If no entry data exists and
 * allow_load is true, entry data will be loaded from its parent
 * archive (if it exists)
 *******************************************************************/
MemChunk& ArchiveEntry::getMCData(bool allow_load) {
	// Get parent archive
	Archive* parent_archive = getParent();

	// Load the data if needed (and possible)
	if (allow_load && !isLoaded() && parent_archive && size > 0) {
		data_loaded = parent_archive->loadEntryData(this);
		setState(0);
	}

	return data;
}

/* ArchiveEntry::setState
 * Sets the entry's state. Won't change state if the change would be
 * redundant (eg new->modified, unmodified->unmodified)
 *******************************************************************/
void ArchiveEntry::setState(uint8_t state) {
	if (state_locked || (state == 0 && this->state == 0))
		return;

	if (state == 0)
		this->state = 0;
	else {
		if (state > this->state)
			this->state = state;
	}

	// Notify parent archive this entry has been modified
	stateChanged();
}

/* ArchiveEntry::unloadData
 * 'Unloads' entry data from memory
 *******************************************************************/
void ArchiveEntry::unloadData() {
	// Check there is any data to be 'unloaded'
	if (!data.hasData() || !data_loaded)
		return;

	// Only unload if the data wasn't modified
	if (getState() > 0)
		return;

	// Delete any data
	data.clear();

	// Update variables etc
	setLoaded(false);
}

void ArchiveEntry::lock() {
	// Lock the entry
	locked = true;

	// Inform parent
	stateChanged();
}

void ArchiveEntry::unlock() {
	// Unlock the entry
	locked = false;

	// Inform parent
	stateChanged();
}

/* ArchiveEntry::rename
 * Renames the entry
 *******************************************************************/
bool ArchiveEntry::rename(string new_name) {
	// Check if locked
	if (locked) {
		Global::error = "Entry is locked";
		return false;
	}

	// Update attributes
	name = new_name;
	setState(1);

	return true;
}

bool ArchiveEntry::resize(uint32_t new_size, bool preserve_data) {
	// Check if locked
	if (locked) {
		Global::error = "Entry is locked";
		return false;
	}

	// Update attributes
	setState(1);

	return data.reSize(new_size, preserve_data);
}

/* ArchiveEntry::clearData
 * Clears entry data and resets its size to zero
 *******************************************************************/
void ArchiveEntry::clearData() {
	// Check if locked
	if (locked) {
		Global::error = "Entry is locked";
		return;
	}

	// Delete the data
	data.clear();

	// Reset attributes
	size = 0;
	data_loaded = false;
}

/* ArchiveEntry::importMem
 * Imports a chunk of memory into the entry, resizing it and clearing
 * any currently existing data.
 * Returns false if data pointer is invalid, true otherwise
 *******************************************************************/
bool ArchiveEntry::importMem(const void* data, uint32_t size) {
	// Check parameters
	if (!data)
		return false;

	// Check if locked
	if (locked) {
		Global::error = "Entry is locked";
		return false;
	}

	// Clear any current data
	clearData();

	// Copy data into the entry
	this->data.importMem((const uint8_t*)data, size);

	// Update attributes
	this->size = size;
	setLoaded();
	setType(EntryType::unknownType());
	setState(1);

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
		return importMem(mc.getData(), mc.getSize());
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
bool ArchiveEntry::importFile(string filename, uint32_t offset, uint32_t size) {
	// Check if locked
	if (locked) {
		Global::error = "Entry is locked";
		return false;
	}

	// Open the file
	wxFile file(filename);

	// Check that it opened ok
	if (!file.IsOpened()) {
		Global::error = "Unable to open file for reading";
		return false;
	}

	// Get the size to read, if zero
	if (size == 0)
		size = file.Length() - offset;

	// Check offset/size bounds
	if (offset + size > file.Length() || offset < 0)
		return false;

	// Create temporary buffer and load file contents
	uint8_t* temp_buf = new uint8_t[size];
	file.Seek(offset, wxFromStart);
	file.Read(temp_buf, size);

	// Import data into entry
	importMem(temp_buf, size);

	// Delete temp buffer
	delete[] temp_buf;

	return true;
}

bool ArchiveEntry::importFileStream(wxFile& file, uint32_t len) {
	// Check if locked
	if (locked) {
		Global::error = "Entry is locked";
		return false;
	}

	// Import data from the file stream
	if (data.importFileStream(file, len)) {
		// Update attributes
		this->size = data.getSize();
		setLoaded();
		setType(EntryType::unknownType());
		setState(1);

		return true;
	}

	return false;
}

/* ArchiveEntry::importEntry
 * Imports data from another entry into this entry, resizing it
 * and clearing any currently existing data.
 * Returns false if the entry is null, true otherwise
 *******************************************************************/
bool ArchiveEntry::importEntry(ArchiveEntry* entry) {
	// Check if locked
	if (locked) {
		Global::error = "Entry is locked";
		return false;
	}

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
	wxFile file(filename, wxFile::write);

	// Check it opened ok
	if (!file.IsOpened()) {
		Global::error = s_fmt("Unable to open file %s for writing", filename);
		return false;
	}

	// Write entry data to the file
	file.Write(getData(), this->getSize());

	return true;
}

bool ArchiveEntry::write(const void* data, uint32_t size) {
	// Check if locked
	if (locked) {
		Global::error = "Entry is locked";
		return false;
	}

	// Load data if it isn't already
	if (isLoaded())
		getData(true);

	// Perform the write
	if (this->data.write(data, size)) {
		// Update attributes
		this->size = this->data.getSize();
		setState(1);

		return true;
	}

	return false;
}

bool ArchiveEntry::read(void* buf, uint32_t size) {
	// Load data if it isn't already
	if (isLoaded())
		getData(true);

	return data.read(buf, size);
}

string ArchiveEntry::getSizeString() {
	return Misc::sizeAsString(getSize());
}

void ArchiveEntry::stateChanged() {
	Archive* parent_archive = getParent();
	if (parent_archive)
		parent_archive->entryStateChanged(this);
}
