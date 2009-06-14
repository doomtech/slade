/*
#ifndef __ZIPARCHIVE_H__
#define	__ZIPARCHIVE_H__

#include "Archive.h"

class ZipDirectory {
private:
	string					name;
	vector<ZipDirectory*>	subdirectories;
	vector<ArchiveEntry*>	entries;
	ZipDirectory*			parent_dir;

public:
	ZipDirectory(string name, ZipDirectory* parent);
	~ZipDirectory();

	vector<ZipDirectory*>	getSubDirs() { return subdirectories; }
	ZipDirectory*			getSubDir(DWORD index);
	vector<ArchiveEntry*>	getEntries() { return entries; }
	ArchiveEntry*			getEntry(DWORD index);
	ArchiveEntry*			getEntry(string name);
	bool					entryExists(ArchiveEntry* entry);

	string			getName() { return name; }
	ZipDirectory*	getParent() { return parent_dir; }
	ZipDirectory*	getSubDir(string dir);
	string			getPath();

	bool			addEntry(ArchiveEntry* entry, int index = -1);
	bool			addSubDir(ZipDirectory* dir);
	ZipDirectory*	addSubDir(string dir);

	bool operator> (ZipDirectory& rhs) { return name > rhs.getName(); }
	bool operator< (ZipDirectory& rhs) { return name < rhs.getName(); }

	int				getEntryIndex(ArchiveEntry* entry, int start = 0);
	ArchiveEntry*	getEntryAtIndex(DWORD index, DWORD start = 0);
	ZipDirectory*	dirWithEntry(ArchiveEntry* entry);

	string	dumpStructure();
};

class ZipArchive : public Archive {
private:
	ZipDirectory*	directory;

public:
	ZipArchive();
	~ZipArchive();

	int				entryIndex(ArchiveEntry* entry);
	ArchiveEntry*	getEntry(DWORD index);
	ArchiveEntry*	getEntry(string name);
	string			getFileExtensionString();

	bool	openFile(string filename);
	bool	save(string filename);
	void	close();

	bool	loadEntryData(ArchiveEntry* entry);
	DWORD	numEntries();

	// Entry directory stuff
	void			setEntryDirectory(ArchiveEntry* entry, string dir);
	string			getEntryDirectory(ArchiveEntry* entry);
	string			getEntryFullPath(ArchiveEntry* entry);
	ZipDirectory*	getEntryZipDir(ArchiveEntry* entry);

	// Entry zip index stuff
	void	setEntryZipIndex(ArchiveEntry* entry, int index);
	int		getEntryZipIndex(ArchiveEntry* entry);

	// Directory manipulation/retrieval
	vector<ArchiveEntry*>	getDirectory(string dir);
	vector<string>			getSubDirs(string dir);

	bool			addEntry(ArchiveEntry* entry, DWORD position = 0);
	ArchiveEntry*	addNewEntry(string name = _T(""), DWORD position = 0);
	ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, DWORD position = 0, bool copy = false);
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);

	bool	swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);

	bool	renameEntry(ArchiveEntry* entry, string new_name);

	vector<mapdesc_t>	detectMaps();
};

#endif //__ZIPARCHIVE_H__

 */
