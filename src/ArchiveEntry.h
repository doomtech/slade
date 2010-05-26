
#ifndef __ARCHIVEENTRY_H__
#define	__ARCHIVEENTRY_H__

class Archive;

#include "ListenerAnnouncer.h"
#include "MemChunk.h"
#include "EntryType.h"
#include "PropertyList.h"

class ArchiveEntry : public Announcer {
private:
	Archive*		parent;
	string			name;
	MemChunk		data;
	uint32_t		size;
	bool			data_loaded;
	EntryType*		type;
	uint8_t			state;			// 0 = unmodified, 1 = modified, 2 = newly created
	bool			locked;
	bool			state_locked;	// If true entry state cannot be changed (for initial loading etc)
	PropertyList	ex_props;

public:
	ArchiveEntry(string name = "", uint32_t size = 0, Archive* parent = NULL);
	ArchiveEntry(ArchiveEntry& copy);
	virtual ~ArchiveEntry();

	// Accessors
	Archive*		getParent() { return parent; }
	string			getName(bool cut_ext = false);
	uint32_t		getSize() { return size; }
	bool			isLoaded() { return data_loaded; }
	EntryType*		getType() { return type; }
	uint8_t			getState() { return state; }
	bool			isLocked() { return locked; }
	const uint8_t*	getData(bool allow_load = true);
	MemChunk&		getMCData(bool allow_load = true);
	PropertyList&	extraProps() { return ex_props; }
	Property&		extraProp(string key) { return ex_props[key]; }

	// Modifiers (won't change entry state)
	void		setParent(Archive* parent) { this->parent = parent; }
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

	// Type detection
	//void	detectType(bool data_check, bool force = false);
	string	getTypeString();

	// Misc
	string	getSizeString();

	// Data access
	bool		write(const void* data, uint32_t size);
	bool		read(void* buf, uint32_t size);
	bool		seek(uint32_t offset, uint32_t start) { return data.seek(offset, start); }
	uint32_t	currentPos() { return data.currentPos(); }

	// Extra properties stuff
	/*
	bool	hasExProp(string key);
	string	getExProp(string key);
	bool	setExProp(string key, string value);
	bool	removeExProp(string key);
	void	allExProps(vector<string>& keys, vector<string>& values);
	*/
};


#endif //__ARCHIVEENTRY_H__
