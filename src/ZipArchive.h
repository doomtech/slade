
#ifndef __ZIPARCHIVE_H__
#define	__ZIPARCHIVE_H__

#include "Archive.h"

class ZipArchive : public Archive {
private:

public:
	ZipArchive();
	~ZipArchive();

	string	getFileExtensionString();

	bool	openFile(string filename);
	bool	save(string filename);
	bool	loadEntryData(ArchiveEntry* entry);

	// Entry directory stuff
	void	setEntryDirectory(ArchiveEntry* entry, string dir);
	string	getEntryDirectory(ArchiveEntry* entry);
	string	getEntryFullPath(ArchiveEntry* entry);

	// Entry zip index stuff
	void	setEntryZipIndex(ArchiveEntry* entry, int index);
	int		getEntryZipIndex(ArchiveEntry* entry);

	// Directory manipulation/retrieval
	vector<ArchiveEntry*>	getDirectory(string dir);

	vector<mapdesc_t>	detectMaps();
};

#endif //__ZIPARCHIVE_H__
