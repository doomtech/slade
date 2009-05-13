
#ifndef __ARCHIVEENTRY_H__
#define	__ARCHIVEENTRY_H__

class Archive;

#include <wx/hashmap.h>
WX_DECLARE_STRING_HASH_MAP(wxString, PropertyList);

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

class ArchiveEntry {
private:
	Archive*		parent;
	string			name;
	BYTE*			data;
	DWORD			size;
	bool			data_loaded;
	BYTE			type;
	PropertyList	ex_props;

public:
	ArchiveEntry(string name = _T(""), Archive* parent = NULL);
	ArchiveEntry(ArchiveEntry& copy);
	virtual ~ArchiveEntry();

	// Accessors
	Archive*	getParent() { return parent; }
	string		getName() { return name; }
	DWORD		getSize() { return size; }
	BYTE*		getData(bool allow_load = true);
	bool		isLoaded() { return data_loaded; }
	BYTE		getType() { return type; }

	// Modifiers
	void		setParent(Archive* parent) { this->parent = parent; }
	void		setName(string name) { this->name = name; }
	void		setLoaded(bool loaded = true) { data_loaded = loaded; }
	void		setSize(DWORD size) { this->size = size; }
	void		setData(BYTE* data) { this->data = data; }
	void		setType(BYTE type) { this->type = type; }

	// Data modification
	void	clearData();

	// Data import
	bool	importMem(void* data, DWORD size);
	bool	importMemChunk(MemChunk& mc);
	bool	importFile(string filename, DWORD offset, DWORD size);
	bool	importEntry(ArchiveEntry* entry);

	// Type detection
	void	detectType(bool force = false);
	string	getTypeString();

	// Extra properties stuff
	bool	hasExProp(string key);
	string	getExProp(string key);
	bool	setExProp(string key, string value);
};


#endif //__ARCHIVEENTRY_H__
