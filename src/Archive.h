
#ifndef __ARCHIVE_H__
#define	__ARCHIVE_H__

#include "ArchiveEntry.h"

// Define archive types
#define ARCHIVE_WAD	1
#define ARCHIVE_ZIP	2

class Archive : public Announcer {
protected:
	string					filename;
	//vector<ArchiveEntry*>	entries;
	BYTE					type;
	bool					modified;
	
	// Specifies whether the archive exists on disk (as opposed to being newly created)
	bool					on_disk;

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
	bool					isOnDisk() { return on_disk; }
	void					setFileName(string fn) { filename = fn; }
	virtual int				entryIndex(ArchiveEntry* entry) = 0;
	virtual ArchiveEntry*	getEntry(DWORD index) = 0;
	virtual ArchiveEntry*	getEntry(string name) = 0;
	virtual string			getFileExtensionString() = 0;

	virtual bool	openFile(string filename) = 0;
	virtual bool	save(string filename = _T("")) = 0;
	virtual bool	loadEntryData(ArchiveEntry* entry) = 0;
	virtual DWORD	numEntries() = 0;
	virtual void	close() = 0;

	// Entry addition/removal
	virtual bool			addEntry(ArchiveEntry* entry, DWORD position = 0) = 0;
	virtual ArchiveEntry*	addNewEntry(string name = _T(""), DWORD position = 0) = 0;
	virtual ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, DWORD position = 0, bool copy = false) = 0;
	virtual bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true) = 0;

	// Entry moving
	virtual bool			swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) = 0;

	// Entry modification
	virtual bool	renameEntry(ArchiveEntry* entry, string new_name) = 0;
	void			entryModified(ArchiveEntry* entry);

	// Detection
	virtual vector<mapdesc_t>	detectMaps() = 0;
	virtual bool				detectEntryType(ArchiveEntry* entry) = 0;
};

#endif //__ARCHIVE_H__
