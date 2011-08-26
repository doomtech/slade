
#ifndef __WADARCHIVE_H__
#define __WADARCHIVE_H__

#include "Archive.h"

// Struct to hold namespace info
struct wad_ns_pair_t {
	ArchiveEntry*	start;	// eg. P_START
	int				start_index;
	ArchiveEntry*	end;	// eg. P_END
	int				end_index;
	string			name;	// eg. "P" (since P or PP is a special case will be set to "patches")

	wad_ns_pair_t(ArchiveEntry* start, ArchiveEntry* end) {
		this->start = start;
		this->end = end;
	}
};

class WadArchive : public TreelessArchive {
private:
	bool					iwad;
	vector<wad_ns_pair_t>	namespaces;

public:
	WadArchive();
	~WadArchive();

	// Wad specific
	bool		isIWAD() { return iwad; }
	bool		isWritable();
	uint32_t	getEntryOffset(ArchiveEntry* entry);
	void		setEntryOffset(ArchiveEntry* entry, uint32_t offset);
	void		updateNamespaces();

	// Archive type info
	string	getFileExtensionString();
	string	getFormat();

	// Opening
	bool	open(MemChunk& mc);			// Open from MemChunk

	// Writing/Saving
	bool	write(MemChunk& mc, bool update = true);	// Write to MemChunk

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
	mapdesc_t			getMapInfo(ArchiveEntry* maphead);
	vector<mapdesc_t>	detectMaps();
	string				detectNamespace(ArchiveEntry* entry);

	// Search
	ArchiveEntry*			findFirst(search_options_t& options);
	ArchiveEntry*			findLast(search_options_t& options);
	vector<ArchiveEntry*>	findAll(search_options_t& options);

	// Static functions
	static bool isWadArchive(MemChunk& mc);
	static bool isWadArchive(string filename);

	static bool exportEntriesAsWad(string filename, vector<ArchiveEntry*> entries) {
		WadArchive wad;

		// Add entries to wad archive
		for (size_t a = 0; a < entries.size(); a++) {
			// Add each entry to the wad archive
			wad.addEntry(entries[a], entries.size(), NULL, true);
		}

		return wad.save(filename);
	}

	friend class WadJArchive;
};

enum MapLumpNames {
	LUMP_THINGS,
	LUMP_VERTEXES,
	LUMP_LINEDEFS,
	LUMP_SIDEDEFS,
	LUMP_SECTORS,
	LUMP_SEGS,
	LUMP_SSECTORS,
	LUMP_NODES,
	LUMP_BLOCKMAP,
	LUMP_REJECT,
	LUMP_SCRIPTS,
	LUMP_BEHAVIOR,
	LUMP_LEAFS,
	LUMP_LIGHTS,
	LUMP_MACROS,
	NUMMAPLUMPS
};

#endif//__WADARCHIVE_H__
