
#ifndef __ZIPARCHIVE_H__
#define __ZIPARCHIVE_H__

#include "Archive.h"

struct zipdir_t {
	string					name;
	vector<zipdir_t*>		subdirectories;
	vector<ArchiveEntry*>	entries;
	zipdir_t*				parent_dir;

	bool entryExists(ArchiveEntry* entry) {
		if (!entry)
			return false;

		for (size_t a = 0; a < entries.size(); a++) {
			if (entries[a] == entry)
				return true;
		}

		return false;
	}

	ArchiveEntry* getEntry(string name) {
		for (size_t a = 0; a < entries.size(); a++) {
			if (!entries[a]->getName().Cmp(name))
				return entries[a];
		}

		return NULL;
	}

	int entryIndex(ArchiveEntry* entry) {
		if (!entry)
			return -1;

		for (size_t a = 0; a < entries.size(); a++) {
			if (entries[a] == entry)
				return a;
		}

		return -1;
	}

	zipdir_t* getSubDir(string name) {
		for (size_t a = 0; a < subdirectories.size(); a++) {
			if (!subdirectories[a]->name.Cmp(name))
				return subdirectories[a];
		}

		return NULL;
	}

	string getFullPath() {
		if (parent_dir)
			return parent_dir->getFullPath() + name;
		else
			return name;
	}

	DWORD numEntries(bool include_subdirs = false) {
		DWORD num = entries.size();

		if (include_subdirs) {
			for (size_t a = 0; a < subdirectories.size(); a++)
				num += subdirectories[a]->numEntries(true);
		}

		return num;
	}
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
	zipdir_t*	getDirectory(string name, zipdir_t* dir = NULL);
	zipdir_t*	addDirectory(string name, zipdir_t* dir = NULL);
	zipdir_t*	getRootDirectory() { return directory; }
	void		dumpDirectoryTree(zipdir_t* start = NULL);
};

#endif//__ZIPARCHIVE_H__
