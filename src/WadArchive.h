
#ifndef __WADARCHIVE_H__
#define	__WADARCHIVE_H__

#include "Archive.h"

class WadArchive : public Archive {
private:
	vector<ArchiveEntry*>	entries;
	char					wad_type[4];
	int						patches[2];
	int						sprites[2];
	int						flats[2];
	int						tx[2];

public:
	WadArchive();
	~WadArchive();

	bool	isIWAD();

	int				entryIndex(ArchiveEntry* entry);
	ArchiveEntry*	getEntry(uint32_t index);
	ArchiveEntry*	getEntry(string name);
	string			getFileExtensionString();

	bool	open(string filename);
	bool	open(ArchiveEntry* entry);
	bool	open(MemChunk& mc);

	bool	write(MemChunk& mc, bool update = true);
	bool	write(string filename, bool update = true);

	void	close();

	uint32_t	getEntryOffset(ArchiveEntry* entry);
	void		setEntryOffset(ArchiveEntry* entry, uint32_t offset);
	bool		loadEntryData(ArchiveEntry* entry);
	uint32_t	numEntries();

	bool			addEntry(ArchiveEntry* entry, uint32_t position = 0);
	ArchiveEntry*	addNewEntry(string name = "", uint32_t position = 0);
	ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, uint32_t position = 0, bool copy = false);
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);

	bool	swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);

	bool	renameEntry(ArchiveEntry* entry, string new_name);

	vector<mapdesc_t>	detectMaps();
	string				detectEntrySection(ArchiveEntry* entry);

	ArchiveEntry*			findEntry(string search, bool includesubdirs = true);
	ArchiveEntry*			findEntry(int edftype, bool includesubdirs = true);
	vector<ArchiveEntry*>	findEntries(string search, bool includesubdirs = true);
	vector<ArchiveEntry*>	findEntries(int edftype, bool includesubdirs = true);

	static bool exportEntriesAsWad(string filename, vector<ArchiveEntry*> entries) {
		WadArchive wad;

		// Add entries to wad archive
		for (size_t a = 0; a < entries.size(); a++) {
			// Add each entry to the wad archive
			wad.addExistingEntry(entries[a], entries.size(), true);
		}

		return wad.save(filename);
	}

	static bool isWadArchive(MemChunk& mc);
	static bool isWadArchive(string filename);
};

#endif	/* _WADARCHIVE_H */
