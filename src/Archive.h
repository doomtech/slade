
#ifndef __ARCHIVE_H__
#define	__ARCHIVE_H__

#include "ArchiveEntry.h"

// Define archive types
#define ARCHIVE_INVALID	0
#define ARCHIVE_WAD		1
#define ARCHIVE_ZIP		2

class Archive : public Announcer {
private:
	bool	modified;
	uint8_t	type;

protected:
	string			filename;
	ArchiveEntry*	parent;

	// Specifies whether the archive exists on disk (as opposed to being newly created)
	bool	on_disk;

public:
	struct mapdesc_t {
		string			name;
		ArchiveEntry*	head;
		ArchiveEntry*	end;
		uint8_t			format;	// 0=doom 1=hexen 2=udmf
	};

	Archive(uint8_t type = ARCHIVE_INVALID);
	virtual ~Archive();

	uint8_t					getType() { return type; }
	string					getFileName(bool fullpath = true);
	ArchiveEntry*			getParent() { return parent; }
	bool					isModified() { return modified; }
	bool					isOnDisk() { return on_disk; }
	void					setFileName(string fn) { filename = fn; }
	virtual int				entryIndex(ArchiveEntry* entry) = 0;
	virtual ArchiveEntry*	getEntry(uint32_t index) = 0;
	virtual ArchiveEntry*	getEntry(string name) = 0;
	virtual string			getFileExtensionString() = 0;
	bool					checkEntry(ArchiveEntry* entry);
	void					setModified(bool mod);
	bool					canSave() { return parent || on_disk; }


	virtual bool	open(string filename) = 0;
	virtual bool	open(ArchiveEntry* entry) = 0;
	virtual bool	open(MemChunk& mc) = 0;

	virtual bool	write(MemChunk& mc, bool update = true) = 0;
	virtual bool	write(string filename, bool update = true) = 0;
	virtual bool	save(string filename = _T(""));

	virtual bool		loadEntryData(ArchiveEntry* entry) = 0;
	virtual uint32_t	numEntries() = 0;
	virtual void		close() = 0;

	// Entry addition/removal
	virtual bool			addEntry(ArchiveEntry* entry, uint32_t position = 0) = 0;
	virtual ArchiveEntry*	addNewEntry(string name = _T(""), uint32_t position = 0) = 0;
	virtual ArchiveEntry*	addExistingEntry(ArchiveEntry* entry, uint32_t position = 0, bool copy = false) = 0;
	virtual bool			removeEntry(ArchiveEntry* entry, bool delete_entry = true) = 0;

	// Entry moving
	virtual bool			swapEntries(ArchiveEntry* entry1, ArchiveEntry* entry2) = 0;

	// Entry modification
	virtual bool	renameEntry(ArchiveEntry* entry, string new_name) = 0;
	void			entryModified(ArchiveEntry* entry);

	// Detection
	virtual vector<mapdesc_t>	detectMaps() = 0;
	virtual string				detectEntrySection(ArchiveEntry* entry) { return _T("none"); }

	// Search
	virtual ArchiveEntry*			findEntry(string search) = 0;
	virtual vector<ArchiveEntry*>	findEntries(string search) = 0;
};

#endif //__ARCHIVE_H__
