
#ifndef __LIBARCHIVE_H__
#define	__LIBARCHIVE_H__

#include "Archive.h"

class LibArchive : public TreelessArchive {
public:
	LibArchive();
	~LibArchive();

	// Lib specific
	uint32_t	getEntryOffset(ArchiveEntry* entry);
	void		setEntryOffset(ArchiveEntry* entry, uint32_t offset);

	// Archive type info
	string			getFileExtensionString();
	string			getFormat();

	// Opening
	bool	open(string filename);		// Open from File
	bool	open(ArchiveEntry* entry);	// Open from ArchiveEntry
	bool	open(MemChunk& mc);			// Open from MemChunk

	// Writing/Saving
	bool	write(MemChunk& mc, bool update = true);	// Write to MemChunk
	bool	write(string filename, bool update = true);	// Write to File

	// Misc
	bool		loadEntryData(ArchiveEntry* entry);
	unsigned 	numEntries() { return getRoot()->numEntries(); }

	// Entry addition/removal (unimplemented)
	ArchiveEntry*	addEntry(ArchiveEntry* entry, unsigned position = 0xFFFFFFFF, ArchiveTreeNode* dir = NULL, bool copy = false) { return NULL; }
	ArchiveEntry*	addNewEntry(string name = "", unsigned position = 0xFFFFFFFF, ArchiveTreeNode* dir = NULL) { return NULL; }
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true) { return false; }

	// Entry moving (unimplemented)
	bool	swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) { return false; }
	bool	moveEntry(ArchiveEntry* entry, unsigned position = 0xFFFFFFFF, ArchiveTreeNode* dir = NULL) { return false; }

	// Entry modification (unimplemented)
	bool	renameEntry(ArchiveEntry* entry, string name) { return false; }

	// Detection
	vector<mapdesc_t>	detectMaps() { vector<mapdesc_t> ret; return ret; }
	string				detectNamespace(ArchiveEntry* entry) { return "global"; }

	static bool isLibArchive(MemChunk& mc);
	static bool isLibArchive(string filename);
};

#endif	/* __LIBARCHIVE_H__ */
