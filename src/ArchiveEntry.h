
#ifndef __ARCHIVEENTRY_H__
#define	__ARCHIVEENTRY_H__

class Archive;

#include "ListenerAnnouncer.h"
#include "MemChunk.h"

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
	MemChunk		data;
	uint32_t		size;
	bool			data_loaded;
	uint8_t			type;
	uint8_t			state;		// 0 = unmodified, 1 = modified, 2 = newly created
	bool			locked;
	PropertyList	ex_props;

public:
	ArchiveEntry(string name = _T(""), uint32_t size = 0, Archive* parent = NULL);
	ArchiveEntry(ArchiveEntry& copy);
	virtual ~ArchiveEntry();

	// Accessors
	Archive*		getParent() { return parent; }
	string			getName(bool cut_ext = false);
	uint32_t		getSize() { return size; }
	bool			isLoaded() { return data_loaded; }
	uint8_t			getType() { return type; }
	uint8_t			getState() { return state; }
	bool			isLocked() { return locked; }
	const uint8_t*	getData(bool allow_load = true);
	MemChunk&		getMCData() { data.writeAllowed(false); return data; }

	// Modifiers (won't change entry state)
	void		setParent(Archive* parent) { this->parent = parent; }
	void		setName(string name) { this->name = name; }
	void		setLoaded(bool loaded = true) { data_loaded = loaded; }
	void		setType(uint8_t type) { this->type = type; }
	void		setState(uint8_t state);
	void		unloadData();
	void		lock() { locked = true; }
	void		unlock() { locked = false; }

	// Entry modification (will change entry state)
	bool	rename(string new_name);
	bool	resize(uint32_t new_size, bool preserve_data);

	// Data modification
	void	clearData();

	// Data import
	bool	importMem(const void* data, uint32_t size);
	bool	importMemChunk(MemChunk& mc);
	bool	importFile(string filename, uint32_t offset = 0, uint32_t size = 0);
	bool	importFileStream(FILE* fp, uint32_t len = 0);
	bool	importEntry(ArchiveEntry* entry);

	// Data export
	bool	exportFile(string filename);

	// Type detection
	void	detectType(bool data_check, bool force = false);
	string	getTypeString();

	// Misc
	string	getSizeString();

	// Data access
	bool		write(const void* data, uint32_t size);
	bool		read(void* buf, uint32_t size);
	bool		seek(uint32_t offset, uint32_t start) { return data.seek(offset, start); }
	uint32_t	currentPos() { return data.currentPos(); }

	// Extra properties stuff
	bool	hasExProp(string key);
	string	getExProp(string key);
	bool	setExProp(string key, string value);
	bool	removeExProp(string key);
	void	allExProps(vector<string>& keys, vector<string>& values);
};


#endif //__ARCHIVEENTRY_H__
