
#ifndef __ARCHIVE_H__
#define	__ARCHIVE_H__

#include "ArchiveEntry.h"

class Archive {
protected:
	string					filename;
	vector<ArchiveEntry*>	entries;

public:
	struct mapdesc_t
	{
		string			name;
		ArchiveEntry*	head;
		ArchiveEntry*	end;
		BYTE			format;	// 0=doom 1=hexen 2=udmf
	};

	Archive();
	virtual ~Archive();

	string					getFileName(bool fullpath = true);
	virtual ArchiveEntry*	getEntry(DWORD index);
	virtual ArchiveEntry*	getEntry(string name);

	virtual bool	openFile(string filename);
	virtual bool	loadEntryData(ArchiveEntry* entry);
	virtual DWORD	numEntries();

	virtual vector<mapdesc_t>	detectMaps();
};

#endif //__ARCHIVE_H__