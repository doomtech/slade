
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

	int				entryIndex(ArchiveEntry* entry);
	ArchiveEntry*	getEntry(DWORD index);
	ArchiveEntry*	getEntry(string name);
	string			getFileExtensionString();

	bool	openFile(string filename);
	bool	save(string filename = _T(""));
	void	close();

	DWORD	getEntryOffset(ArchiveEntry* entry);
	void	setEntryOffset(ArchiveEntry* entry, DWORD offset);
	bool	loadEntryData(ArchiveEntry* entry);
	DWORD	numEntries();

	bool			addEntry(ArchiveEntry* entry, DWORD position = 0);
	ArchiveEntry*	addNewEntry(string name = _T(""), DWORD position = 0);
	ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, DWORD position = 0, bool copy = false);
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);

	bool	swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);

	bool	renameEntry(ArchiveEntry* entry, string new_name);

	vector<mapdesc_t>	detectMaps();
	bool				detectEntryType(ArchiveEntry* entry);
};

#endif	/* _WADARCHIVE_H */
