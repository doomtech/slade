
#ifndef __ZIPARCHIVE_H__
#define __ZIPARCHIVE_H__

#include "Archive.h"

struct zipdir_t {
	ArchiveEntry*			entry;
	vector<zipdir_t*>		subdirectories;
	vector<ArchiveEntry*>	entries;
	zipdir_t*				parent_dir;

	zipdir_t();

	string	getName() { return entry->getName(); }
	void	setName(string name) { entry->setName(name); }

	bool			entryExists(ArchiveEntry* entry, bool include_subdirs = false);
	ArchiveEntry*	getEntry(string name);
	int				entryIndex(ArchiveEntry* entry);
	zipdir_t*		getSubDir(string name);
	string			getFullPath();
	DWORD			numEntries(bool include_subdirs = false);
	DWORD			numSubDirs(bool include_subdirs = false);
};

class ZipArchive : public Archive {
private:
	zipdir_t*	directory;

public:
	ZipArchive();
	~ZipArchive();

	int				entryIndex(ArchiveEntry* entry);
	ArchiveEntry*	getEntry(DWORD index);
	ArchiveEntry*	getEntry(string name);
	string			getFileExtensionString();

	bool	openFile(string filename);
	bool	save(string filename = _T(""));
	bool	loadEntryData(ArchiveEntry* entry);
	DWORD	numEntries();
	void	close();

	// Entry addition/removal
	bool			addEntry(ArchiveEntry* entry, DWORD position = 0);
	ArchiveEntry*	addNewEntry(string name = _T(""), DWORD position = 0);
	ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, DWORD position = 0, bool copy = false);
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);

	// Entry moving
	bool			swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);

	// Entry modification
	bool	renameEntry(ArchiveEntry* entry, string new_name);

	vector<mapdesc_t>	detectMaps();

	// ---- Zip-specific ----
	zipdir_t*	getEntryDirectory(ArchiveEntry* entry, zipdir_t* dir = NULL);
	string		getEntryFullPath(ArchiveEntry* entry);
	zipdir_t*	getDirectory(string name, zipdir_t* dir = NULL);
	zipdir_t*	addDirectory(string name, zipdir_t* dir = NULL);
	void		deleteDirectory(zipdir_t* dir = NULL);
	zipdir_t*	getRootDirectory() { return directory; }
	void		dumpDirectoryTree(zipdir_t* start = NULL);
	void		getTreeAsList(vector<ArchiveEntry*>& list, zipdir_t* start = NULL);
};

#endif//__ZIPARCHIVE_H__
