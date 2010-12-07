
#ifndef __WAD2_ARCHIVE_H__
#define __WAD2_ARCHIVE_H__

#include "Archive.h"

// From http://www.gamers.org/dEngine/quake/spec/quake-spec31.html#CWADF
typedef struct
{
	long offset;                 // Position of the entry in WAD
	long dsize;                  // Size of the entry in WAD file
	long size;                   // Size of the entry in memory
	char type;                   // type of entry
	char cmprs;                  // Compression. 0 if none.
	short dummy;                 // Not used
	char name[16];               // 1 to 16 characters, '\0'-padded
} wad2entry_t;

class Wad2Archive : public TreelessArchive {
private:
	bool	wad3;

public:
	Wad2Archive();
	~Wad2Archive();

	// Archive type info
	string	getFileExtensionString();
	string	getFormat();

	// Opening
	bool	open(string filename);		// Open from File
	bool	open(ArchiveEntry* entry);	// Open from ArchiveEntry
	bool	open(MemChunk& mc);			// Open from MemChunk

	// Writing/Saving
	bool	write(MemChunk& mc, bool update = true);	// Write to MemChunk
	bool	write(string filename, bool update = true);	// Write to File

	// Misc
	bool		loadEntryData(ArchiveEntry* entry);

	// Entry addition/removal
	ArchiveEntry*	addEntry(ArchiveEntry* entry, unsigned position = 0xFFFFFFFF, ArchiveTreeNode* dir = NULL, bool copy = false);

	// Entry modification
	bool	renameEntry(ArchiveEntry* entry, string name);

	// Detection
	vector<mapdesc_t>	detectMaps() { return vector<mapdesc_t>(); }

	// Static functions
	static bool isWad2Archive(MemChunk& mc);
	static bool isWad2Archive(string filename);
};

#endif//__WAD2_ARCHIVE_H__
