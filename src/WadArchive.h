
#ifndef __WADARCHIVE_H__
#define	__WADARCHIVE_H__

#include "Archive.h"

class WadArchive : public Archive {
private:

public:
	WadArchive();
	~WadArchive();

	bool	openFile(string filename);

	DWORD	getEntryOffset(ArchiveEntry* entry);
	bool	loadEntryData(ArchiveEntry* entry);
};

#endif	/* _WADARCHIVE_H */
