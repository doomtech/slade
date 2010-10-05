
#ifndef __GRPARCHIVE_H__
#define __GRPARCHIVE_H__

#include "Archive.h"

class GrpArchive : public TreelessArchive {
public:
	GrpArchive();
	~GrpArchive();

	// GRP specific
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
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);

	// Entry modification
	bool	renameEntry(ArchiveEntry* entry, string name);

	// Detection
	vector<mapdesc_t>	detectMaps();
	string				detectNamespace(ArchiveEntry* entry);

	// Static functions
	static bool isGrpArchive(MemChunk& mc);
	static bool isGrpArchive(string filename);

	static bool exportEntriesAsWad(string filename, vector<ArchiveEntry*> entries) {
		GrpArchive grp;

		// Add entries to grp archive
		for (size_t a = 0; a < entries.size(); a++) {
			// Add each entry to the grp archive
			grp.addEntry(entries[a], entries.size(), NULL, true);
		}

		return grp.save(filename);
	}
};

#endif//__GRPARCHIVE_H__
