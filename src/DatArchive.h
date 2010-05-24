
#ifndef __DATARCHIVE_H__
#define	__DATARCHIVE_H__

#include "Archive.h"

class DatArchive : public Archive {
private:
	vector<ArchiveEntry*>	entries;
	int						sprites[2];
	int						flats[2];
	int						walls[2];

public:
	DatArchive();
	~DatArchive();

	int				entryIndex(ArchiveEntry* entry);
	ArchiveEntry*	getEntry(uint32_t index);
	ArchiveEntry*	getEntry(string name);
	string			getFileExtensionString();

	bool	open(string filename);
	bool	open(ArchiveEntry* entry);
	bool	open(MemChunk& mc, string name = _T(""));

	bool	write(MemChunk& mc, bool update = true);
	bool	write(string filename, bool update = true);

	void	close();

	uint32_t	getEntryOffset(ArchiveEntry* entry);
	void		setEntryOffset(ArchiveEntry* entry, uint32_t offset);
	bool		loadEntryData(ArchiveEntry* entry);
	uint32_t	numEntries();

	bool			addEntry(ArchiveEntry* entry, uint32_t position = 0);
	ArchiveEntry*	addNewEntry(string name = _T(""), uint32_t position = 0);
	ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, uint32_t position = 0, bool copy = false);
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);

	bool	swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);
	bool	renameEntry(ArchiveEntry *,string){return false;}

	vector<mapdesc_t>	detectMaps();
	string				detectEntrySection(ArchiveEntry* entry);

	ArchiveEntry*			findEntry(string search, bool includesubdirs = true);
	ArchiveEntry*			findEntry(int edftype, bool includesubdirs = true);
	vector<ArchiveEntry*>	findEntries(string search, bool includesubdirs = true);
	vector<ArchiveEntry*>	findEntries(int edftype, bool includesubdirs = true);

	static bool isDatArchive(MemChunk& mc);
	static bool isDatArchive(string filename);
};

#endif	/* __DATARCHIVE_H__ */
