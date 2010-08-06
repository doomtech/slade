
#ifndef __RESARCHIVE_H__
#define __RESARCHIVE_H__

#include "Archive.h"

#define RESDIRENTRYSIZE 39	// The size of a res entry in the res directory

class ResArchive : public Archive {
private:

public:
	ResArchive();
	~ResArchive();

	// Res specific
	uint32_t	getEntryOffset(ArchiveEntry* entry);
	void		setEntryOffset(ArchiveEntry* entry, uint32_t offset);
	void		updateNamespaces();

	// Archive type info
	string	getFileExtensionString();
	string	getFormat();

	// Opening
	bool	open(string filename);		// Open from File
	bool	open(ArchiveEntry* entry);	// Open from ArchiveEntry
	bool	open(MemChunk& mc);			// Open from MemChunk
	bool	readDirectory(MemChunk& mc, size_t dir_offset, size_t num_lumps, ArchiveTreeNode* parent);

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

	// Entry moving
	bool	swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);
	bool	moveEntry(ArchiveEntry* entry, unsigned position = 0xFFFFFFFF, ArchiveTreeNode* dir = NULL);

	// Detection
	vector<mapdesc_t>	detectMaps();
	string				detectNamespace(ArchiveEntry* entry);

	// Search
	ArchiveEntry*			findFirst(search_options_t& options);
	ArchiveEntry*			findLast(search_options_t& options);
	vector<ArchiveEntry*>	findAll(search_options_t& options);

	// Static functions
	static bool isResArchive(MemChunk& mc);
	static bool isResArchive(MemChunk& mc, size_t& d_o, size_t& n_l);
	static bool isResArchive(string filename);

};

#endif//__RESARCHIVE_H__
