
#ifndef __ARCHIVEENTRY_H__
#define __ARCHIVEENTRY_H__

#include "EntryType.h"
#include "PropertyList.h"

class ArchiveTreeNode;
class Archive;

class ArchiveEntry {
	friend class ArchiveTreeNode;
	friend class Archive;
private:
	// Entry Info
	string				name;
	uint32_t			size;
	MemChunk			data;
	EntryType*			type;
	ArchiveTreeNode*	parent;
	PropertyList		ex_props;

	// Entry status
	uint8_t			state;			// 0 = unmodified, 1 = modified, 2 = newly created (not saved to disk)
	bool			state_locked;	// If true the entry state cannot be changed (used for initial loading)
	bool			locked;			// If true the entry data+info cannot be changed
	bool			data_loaded;	// True if the entry's data is currently loaded into the data MemChunk

public:
	ArchiveEntry(string name = "", uint32_t size = 0);
	ArchiveEntry(ArchiveEntry& copy);
	~ArchiveEntry();

	// Accessors
	string				getName(bool cut_ext = false);
	uint32_t			getSize()			{ if (data_loaded) return data.getSize(); else return size; }
	MemChunk&			getMCData(bool allow_load = true);
	const uint8_t*		getData(bool allow_load = true);
	ArchiveTreeNode*	getParentDir()		{ return parent; }
	Archive*			getParent();
	string				getPath(bool name = false);
	EntryType*			getType()			{ return type; }
	PropertyList&		exProps()			{ return ex_props; }
	Property&			exProp(string key)	{ return ex_props[key]; }
	uint8_t				getState()			{ return state; }
	bool				isLocked()			{ return locked; }
	bool				isLoaded()			{ return data_loaded; }

	// Modifiers (won't change entry state, except setState of course :P)
	void		setName(string name) { this->name = name; }
	void		setLoaded(bool loaded = true) { data_loaded = loaded; }
	void		setType(EntryType* type) { this->type = type; }
	void		setState(uint8_t state);
	void		unloadData();
	void		lock();
	void		unlock();
	void		lockState() { state_locked = true; }
	void		unlockState() { state_locked = false; }

	// Entry modification (will change entry state)
	bool	rename(string new_name);
	bool	resize(uint32_t new_size, bool preserve_data);

	// Data modification
	void	clearData();

	// Data import
	bool	importMem(const void* data, uint32_t size);
	bool	importMemChunk(MemChunk& mc);
	bool	importFile(string filename, uint32_t offset = 0, uint32_t size = 0);
	bool	importFileStream(wxFile& file, uint32_t len = 0);
	bool	importEntry(ArchiveEntry* entry);

	// Data export
	bool	exportFile(string filename);

	// Data access
	bool		write(const void* data, uint32_t size);
	bool		read(void* buf, uint32_t size);
	bool		seek(uint32_t offset, uint32_t start) { return data.seek(offset, start); }
	uint32_t	currentPos() { return data.currentPos(); }

	// Misc
	string	getSizeString();
	string	getTypeString() { if (type) return type->getName(); else return "Unknown"; }
	void	stateChanged();
};

#endif//__ARCHIVEENTRY_H__
