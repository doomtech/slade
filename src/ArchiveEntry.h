
#ifndef __ARCHIVEENTRY_H__
#define	__ARCHIVEENTRY_H__

class Archive;

#include "ListenerAnnouncer.h"

// Entry types enum
enum {
	ETYPE_UNKNOWN = 0,
	ETYPE_MARKER,
	ETYPE_TEXT,

	ETYPE_PATCH,
	ETYPE_SPRITE,
	ETYPE_FLAT,
	ETYPE_GFX,
	ETYPE_GFX2,
	ETYPE_PNG,
	ETYPE_IMAGE,

	ETYPE_SOUND,
	ETYPE_WAV,
	ETYPE_MP3,
	ETYPE_FLAC,
	ETYPE_MUS,
	ETYPE_MIDI,
	ETYPE_MOD,

	ETYPE_TEXTURES,
	ETYPE_PNAMES,

	ETYPE_MAP,
	ETYPE_LINEDEFS,
	ETYPE_SIDEDEFS,
	ETYPE_VERTEXES,
	ETYPE_SECTORS,
	ETYPE_THINGS,
	ETYPE_NODES,
	ETYPE_SEGS,
	ETYPE_SSECTS,
	ETYPE_REJECT,
	ETYPE_BLOCKMAP,
	ETYPE_BEHAVIOR,

	ETYPE_PLAYPAL,
	ETYPE_COLORMAP,

	ETYPE_WAD,

	ETYPE_FOLDER,


	ETYPE_COUNT,
};

class ArchiveEntry : public Announcer {
private:
	Archive*		parent;
	string			name;
	uint8_t*		data;
	uint32_t		size;
	bool			data_loaded;
	uint8_t			type;
	uint8_t			state;		// 0 = unmodified, 1 = modified, 2 = newly created
	bool			locked;
	PropertyList	ex_props;

public:
	ArchiveEntry(string name = _T(""), Archive* parent = NULL);
	ArchiveEntry(ArchiveEntry& copy);
	virtual ~ArchiveEntry();

	// Accessors
	Archive*	getParent() { return parent; }
	string		getName() { return name; }
	uint32_t	getSize() { return size; }
	uint8_t*	getData(bool allow_load = true);
	bool		isLoaded() { return data_loaded; }
	uint8_t		getType() { return type; }
	uint8_t		getState() { return state; }
	bool		isLocked() { return locked; }

	// Modifiers (won't change entry state)
	void		setParent(Archive* parent) { this->parent = parent; }
	void		setName(string name) { this->name = name; }
	void		setLoaded(bool loaded = true) { data_loaded = loaded; }
	void		setSize(uint32_t size) { this->size = size; }
	void		setData(uint8_t* data) { this->data = data; }
	void		setType(uint8_t type) { this->type = type; }
	void		setState(uint8_t state);
	void		unloadData();
	void		lock() { locked = true; }
	void		unlock() { locked = false; }

	// Entry modification (will change entry state)
	void	rename(string new_name);

	// Data modification
	void	clearData();

	// Data import
	bool	importMem(void* data, uint32_t size);
	bool	importMemChunk(MemChunk& mc);
	bool	importFile(string filename, uint32_t offset = 0, uint32_t size = 0);
	bool	importEntry(ArchiveEntry* entry);

	// Data export
	bool	exportFile(string filename);

	// Type detection
	void	detectType(bool data_check, bool force = false);
	string	getTypeString();

	// Misc
	string	getSizeString();

	// Extra properties stuff
	bool	hasExProp(string key);
	string	getExProp(string key);
	bool	setExProp(string key, string value);
	bool	removeExProp(string key);
};


#endif //__ARCHIVEENTRY_H__
