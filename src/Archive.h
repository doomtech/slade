
#ifndef __ARCHIVE_H__
#define	__ARCHIVE_H__

#include "ArchiveEntry.h"

class Archive {
protected:
	string					filename;
	vector<ArchiveEntry*>	entries;

public:
	Archive();
	virtual ~Archive();

	string					getFileName(bool fullpath = true);
	virtual ArchiveEntry*	getEntry(DWORD index);
	virtual ArchiveEntry*	getEntry(string name);

	virtual bool	openFile(string filename);
	virtual bool	loadEntryData(ArchiveEntry* entry);
};

#endif //__ARCHIVE_H__
