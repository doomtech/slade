
#ifndef __BSP_ARCHIVE_H__
#define __BSP_ARCHIVE_H__

#include "Archive.h"

class BSPArchive : public Archive {
private:

public:
	BSPArchive();
	~BSPArchive();

	// Archive type info
	string		getFileExtensionString();
	string		getFormat();

	// Opening
	bool		open(string filename);		// Open from File
	bool		open(ArchiveEntry* entry);	// Open from ArchiveEntry
	bool		open(MemChunk& mc);			// Open from MemChunk

	// Writing/Saving
	bool		write(MemChunk& mc, bool update = true);	// Write to MemChunk
	bool		write(string filename, bool update = true);	// Write to File

	// Misc
	bool		loadEntryData(ArchiveEntry* entry);
	uint32_t	getEntryOffset(ArchiveEntry* entry);

	// Detection
	virtual		vector<mapdesc_t>	detectMaps() { return vector<mapdesc_t>(); }

	// Static functions
	static bool isBSPArchive(MemChunk& mc);
	static bool isBSPArchive(string filename);
};

#endif//__BSP_ARCHIVE_H__
