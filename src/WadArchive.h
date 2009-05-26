
#ifndef __WADARCHIVE_H__
#define	__WADARCHIVE_H__

#include "Archive.h"

class WadArchive : public Archive {
private:
	char	wad_type[4];

public:
	WadArchive();
	~WadArchive();

	string	getFileExtensionString();

	bool	openFile(string filename);
	bool	save(string filename = _T(""));

	DWORD	getEntryOffset(ArchiveEntry* entry);
	void	setEntryOffset(ArchiveEntry* entry, DWORD offset);
	bool	loadEntryData(ArchiveEntry* entry);

	bool	addEntry(ArchiveEntry* entry, DWORD position = 0);
	bool	renameEntry(ArchiveEntry* entry, string new_name);

	vector<mapdesc_t>	detectMaps();
};

#endif	/* _WADARCHIVE_H */
