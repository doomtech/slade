
#ifndef __RFFARCHIVE_H__
#define __RFFARCHIVE_H__

#include "Archive.h"

class RffArchive : public TreelessArchive {
public:
	RffArchive();
	~RffArchive();

	// RFF specific
	uint32_t	getEntryOffset(ArchiveEntry* entry);
	void		setEntryOffset(ArchiveEntry* entry, uint32_t offset);

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
	ArchiveEntry*	addEntry(ArchiveEntry* entry, string add_namespace, bool copy = false);

	// Entry modification
	bool	renameEntry(ArchiveEntry* entry, string name);

	// Detection
	vector<mapdesc_t>	detectMaps();
	string				detectNamespace(ArchiveEntry* entry);

	// Static functions
	static bool isRffArchive(MemChunk& mc);
	static bool isRffArchive(string filename);

	static bool exportEntriesAsWad(string filename, vector<ArchiveEntry*> entries) {
		RffArchive rff;

		// Add entries to grp archive
		for (size_t a = 0; a < entries.size(); a++) {
			// Add each entry to the grp archive
			rff.addEntry(entries[a], entries.size(), NULL, true);
		}

		return rff.save(filename);
	}
};

#endif//__RFFARCHIVE_H__
