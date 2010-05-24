
#ifndef __LIBARCHIVE_H__
#define	__LIBARCHIVE_H__

#include "Archive.h"

class LibArchive : public Archive {
private:
	vector<ArchiveEntry*>	entries;

public:
	LibArchive();
	~LibArchive();

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
	ArchiveEntry*	addNewEntry(string name = _T(""), uint32_t position = 0);
	ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, uint32_t position = 0, bool copy = false);
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);
	vector<mapdesc_t>	detectMaps();

	bool	swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);
	bool	renameEntry(ArchiveEntry *,string){return false;}

	ArchiveEntry*			findEntry(string search, bool includesubdirs = true);
	ArchiveEntry*			findEntry(int edftype, bool includesubdirs = true);
	vector<ArchiveEntry*>	findEntries(string search, bool includesubdirs = true);
	vector<ArchiveEntry*>	findEntries(int edftype, bool includesubdirs = true);

	static bool isLibArchive(MemChunk& mc);
	static bool isLibArchive(string filename);
};

#endif	/* __LIBARCHIVE_H__ */
