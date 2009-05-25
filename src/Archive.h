
#ifndef __ARCHIVE_H__
#define	__ARCHIVE_H__

#include "ArchiveEntry.h"

// Define archive types
#define ARCHIVE_WAD	1
#define ARCHIVE_ZIP	2

class Archive : public Announcer {
protected:
	string					filename;
	vector<ArchiveEntry*>	entries;
	BYTE					type;
	bool					modified;

public:
	struct mapdesc_t {
		string			name;
		ArchiveEntry*	head;
		ArchiveEntry*	end;
		BYTE			format;	// 0=doom 1=hexen 2=udmf
	};

	Archive();
	virtual ~Archive();

	BYTE					getType() { return type; }
	string					getFileName(bool fullpath = true);
	bool					isModified() { return modified; }
	int						entryIndex(ArchiveEntry* entry);
	virtual ArchiveEntry*	getEntry(DWORD index);
	virtual ArchiveEntry*	getEntry(string name);

	virtual bool	openFile(string filename);
	virtual bool	save(string filename = _T(""));
	virtual bool	loadEntryData(ArchiveEntry* entry);
	virtual DWORD	numEntries();
	virtual void	close();

	// Entry addition/removal
	virtual bool			addEntry(ArchiveEntry* entry, DWORD position = 0);
	virtual ArchiveEntry*	addNewEntry(string name = _T(""), DWORD position = 0);
	virtual ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, DWORD position = 0, bool copy = false);
	virtual bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);

	// Entry moving
	virtual bool			swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);

	// Entry modification
	virtual bool			renameEntry(ArchiveEntry* entry, string new_name);

	virtual void	entryModified(ArchiveEntry* entry);

	virtual vector<mapdesc_t>	detectMaps();
};

#endif //__ARCHIVE_H__
