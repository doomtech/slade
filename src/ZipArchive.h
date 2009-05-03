
#ifndef __ZIPARCHIVE_H__
#define	__ZIPARCHIVE_H__

#include "Archive.h"

class ZipArchive : public Archive {
private:

public:
	ZipArchive();
	~ZipArchive();

	bool	openFile(string filename);
	bool	loadEntryData(ArchiveEntry* entry);

	vector<mapdesc_t>	detectMaps();
};

#endif //__ZIPARCHIVE_H__
