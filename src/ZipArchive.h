
#ifndef __ZIPARCHIVE_H__
#define __ZIPARCHIVE_H__

#include "Archive.h"

struct zipdir_t {
	ArchiveEntry*			entry;
	vector<zipdir_t*>		subdirectories;
	vector<ArchiveEntry*>	entries;
	zipdir_t*				parent_dir;

	zipdir_t();
	~zipdir_t();

	string	getName() { return entry->getName(); }
	void	setName(string name) { entry->setName(name); }

	bool			entryExists(ArchiveEntry* entry, bool include_subdirs = false);
	ArchiveEntry*	getEntry(string name, bool include_subdirs = true);
	int				entryIndex(ArchiveEntry* entry);
	zipdir_t*		getSubDir(string name);
	int				dirIndex(zipdir_t* dir);
	string			getFullPath();
	uint32_t		numEntries(bool include_subdirs = false);
	uint32_t		numSubDirs(bool include_subdirs = false);
	zipdir_t*		copy(bool include_subdirs = true);
	void			clear(bool delete_entries);
	void			addToList(vector<ArchiveEntry*>& list);
};

class ZipArchive : public Archive {
private:
	zipdir_t*	directory;

public:
	ZipArchive();
	~ZipArchive();

	int				entryIndex(ArchiveEntry* entry);
	ArchiveEntry*	getEntry(uint32_t index);
	ArchiveEntry*	getEntry(string name);
	string			getFileExtensionString();

	bool	open(string filename);
	bool	open(ArchiveEntry* entry);
	bool	open(MemChunk& mc);

	bool	write(MemChunk& mc, bool update = true);
	bool	write(string filename, bool update = true);

	bool		loadEntryData(ArchiveEntry* entry);
	uint32_t	numEntries();
	void		close();

	// Entry addition/removal
	bool			addEntry(ArchiveEntry* entry, uint32_t position = 0);
	ArchiveEntry*	addNewEntry(string name = _T(""), uint32_t position = 0);
	ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, uint32_t position = 0, bool copy = false);
	bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true);

	// Entry moving
	bool			swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2);

	// Entry modification
	bool	renameEntry(ArchiveEntry* entry, string new_name);

	vector<mapdesc_t>	detectMaps();
	string				detectEntrySection(ArchiveEntry* entry);

	ArchiveEntry*			findEntry(string search);
	vector<ArchiveEntry*>	findEntries(string search);

	// ---- Zip-specific ----
	zipdir_t*	getEntryDirectory(ArchiveEntry* entry, zipdir_t* dir = NULL);
	string		getEntryFullPath(ArchiveEntry* entry);
	zipdir_t*	getDirectory(string name, zipdir_t* dir = NULL);
	zipdir_t*	getDirectory(ArchiveEntry* dir_entry, zipdir_t* dir = NULL);
	zipdir_t*	addDirectory(string name, zipdir_t* dir = NULL);
	bool		renameDirectory(zipdir_t* dir, string newname);
	void		deleteDirectory(zipdir_t* dir = NULL);
	zipdir_t*	getRootDirectory() { return directory; }
	void		dumpDirectoryTree(zipdir_t* start = NULL);
	void		getTreeAsList(vector<ArchiveEntry*>& list, zipdir_t* start = NULL);
	
	static bool	isZipArchive(MemChunk& mc);
	static bool isZipArchive(string filename);
};

#endif//__ZIPARCHIVE_H__
