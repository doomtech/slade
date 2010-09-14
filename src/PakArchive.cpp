
#include "Main.h"
#include "PakArchive.h"
#include "SplashWindow.h"
#include <wx/filename.h>

EXTERN_CVAR(Bool, archive_load_data)

PakArchive::PakArchive() : Archive(ARCHIVE_PAK) {
}

PakArchive::~PakArchive() {
}

string PakArchive::getFileExtensionString() {
	return "Pak Files (*.pak)|*.pak";
}

string PakArchive::getFormat() {
	return "archive_pak";
}

bool PakArchive::open(string filename) {
	// Read the file into a MemChunk
	MemChunk mc;
	if (!mc.importFile(filename)) {
		Global::error = "Unable to open file. Make sure it isn't in use by another program.";
		return false;
	}

	// Load from MemChunk
	if (open(mc)) {
		// Update variables
		this->filename = filename;
		this->on_disk = true;

		return true;
	}
	else
		return false;
}

bool PakArchive::open(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Read archive from entry
	return open(entry->getMCData());
}

bool PakArchive::open(MemChunk& mc) {
	// Check given data is valid
	if (mc.getSize() < 12)
		return false;

	// Read pak header
	char pack[4];
	long dir_offset;
	long dir_size;
	mc.seek(0, SEEK_SET);
	mc.read(pack, 4);
	mc.read(&dir_offset, 4);
	mc.read(&dir_size, 4);

	// Check it
	if (pack[0] != 'P' || pack[1] != 'A' || pack[2] != 'C' || pack[3] != 'K') {
		wxLogMessage("PakArchive::open: Opening failed, invalid header");
		Global::error = "Invalid pak header";
		return false;
	}

	// Stop announcements (don't want to be announcing modification due to entries being added etc)
	setMuted(true);

	// Read the directory
	int num_entries = dir_size / 64;
	mc.seek(dir_offset, SEEK_SET);
	theSplashWindow->setProgressMessage("Reading pak archive data");
	for (uint32_t d = 0; d < num_entries; d++) {
		// Update splash window progress
		theSplashWindow->setProgress(((float)d / (float)num_entries));

		// Read entry info
		char name[56];
		long offset;
		long size;
		mc.read(name, 56);
		mc.read(&offset, 4);
		mc.read(&size, 4);

		// Byteswap if needed
		offset = wxINT32_SWAP_ON_BE(offset);
		size = wxINT32_SWAP_ON_BE(size);

		// Check offset+size
		if (offset + size > mc.getSize()) {
			wxLogMessage("PakArchive::open: Pak archive is invalid or corrupt (entry goes past end of file)");
			Global::error = "Archive is invalid and/or corrupt";
			setMuted(false);
			return false;
		}

		// Parse name
		wxFileName fn(wxString::FromAscii(name, 56));

		// Create directory if needed
		ArchiveTreeNode* dir = createDir(fn.GetPath(true, wxPATH_UNIX));

		// Create entry
		ArchiveEntry* entry = new ArchiveEntry(fn.GetFullName(), size);
		entry->exProp("Offset") = (int)offset;
		entry->setLoaded(false);
		entry->setState(0);

		// Add to directory
		dir->addEntry(entry);
	}

	// Detect all entry types
	MemChunk edata;
	vector<ArchiveEntry*> all_entries;
	getEntryTreeAsList(all_entries);
	theSplashWindow->setProgressMessage("Detecting entry types");
	for (size_t a = 0; a < all_entries.size(); a++) {
		// Update splash window progress
		theSplashWindow->setProgress((((float)a / (float)num_entries)));

		// Get entry
		ArchiveEntry* entry = all_entries[a];

		// Read entry data if it isn't zero-sized
		if (entry->getSize() > 0) {
			// Read the entry data
			mc.exportMemChunk(edata, (int)entry->exProp("Offset"), entry->getSize());
			entry->importMemChunk(edata);
		}

		// Detect entry type
		EntryType::detectEntryType(entry);

		// Unload entry data if needed
		if (!archive_load_data)
			entry->unloadData();

		// Set entry to unchanged
		entry->setState(0);
	}

	// Setup variables
	setMuted(false);
	setModified(false);
	announce("opened");

	theSplashWindow->setProgressMessage("");

	return true;
}

bool PakArchive::write(MemChunk& mc, bool update) {
	return false;
}

bool PakArchive::write(string filename, bool update) {
	return false;
}

bool PakArchive::loadEntryData(ArchiveEntry* entry) {
	// Check entry is ok
	if (!checkEntry(entry))
		return false;

	// Do nothing if the entry's size is zero,
	// or if it has already been loaded
	if (entry->getSize() == 0 || entry->isLoaded()) {
		entry->setLoaded();
		return true;
	}

	// Open archive file
	wxFile file(filename);
	
	// Check it opened
	if (!file.IsOpened()) {
		wxLogMessage("PakArchive::loadEntryData: Unable to open archive file %s", chr(filename));
		return false;
	}

	// Seek to entry offset in file and read it in
	file.Seek((int)entry->exProp("Offset"), wxFromStart);
	entry->importFileStream(file, entry->getSize());

	// Set the lump to loaded
	entry->setLoaded();

	return true;
}




bool PakArchive::isPakArchive(MemChunk& mc) {
	// Check given data is valid
	if (mc.getSize() < 12)
		return false;

	// Read pak header
	char pack[4];
	long dir_offset;
	long dir_size;
	mc.seek(0, SEEK_SET);
	mc.read(pack, 4);
	mc.read(&dir_offset, 4);
	mc.read(&dir_size, 4);

	// Byteswap values for big endian if needed
	dir_size = wxINT32_SWAP_ON_BE(dir_size);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);

	// Check header
	if (pack[0] != 'P' || pack[1] != 'A' || pack[2] != 'C' || pack[3] != 'K')
		return false;

	// Check directory is sane
	if (dir_offset < 12 || dir_offset + dir_size > mc.getSize())
		return false;

	// That'll do
	return true;
}

bool PakArchive::isPakArchive(string filename) {
	// Open file for reading
	wxFile file(filename);

	// Check it opened ok
	if (!file.IsOpened())
		return false;

	// Read pak header
	char pack[4];
	long dir_offset;
	long dir_size;
	file.Seek(0, wxFromStart);
	file.Read(pack, 4);
	file.Read(&dir_offset, 4);
	file.Read(&dir_size, 4);

	// Byteswap values for big endian if needed
	dir_size = wxINT32_SWAP_ON_BE(dir_size);
	dir_offset = wxINT32_SWAP_ON_BE(dir_offset);

	// Check header
	if (pack[0] != 'P' || pack[1] != 'A' || pack[2] != 'C' || pack[3] != 'K')
		return false;

	// Check directory is sane
	if (dir_offset < 12 || dir_offset + dir_size > file.Length())
		return false;

	// That'll do
	return true;
}
