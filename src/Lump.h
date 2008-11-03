
#ifndef __LUMP_H__
#define __LUMP_H__

class Wad;

#include <wx/hashmap.h>
WX_DECLARE_STRING_HASH_MAP(wxString, PropertyList);

class Lump
{
private:
	Wad*			parent;
	string			name;
	DWORD			offset; // In a zip/pk3 file, this will be the entry index. -1 means undefined
	BYTE*			data;
	DWORD			size;

	// This should only ever be false if the lump
	// has been opened along with a wad or zip file
	bool			data_loaded;

	PropertyList	ex_props;

public:
	Lump(Wad* parent = NULL);
	~Lump();

	// Accessors
	Wad*		getParent() { return parent; }
	string		getName() { return name; }
	DWORD		getOffset() { return offset; }
	DWORD		getSize() { return size; }
	BYTE*		getData(bool allow_load = true);
	bool		isLoaded() { return data_loaded; }

	// Modifiers
	void		setParent(Wad* parent_) { parent = parent_; }
	void		setName(string name_) { name = name_; }
	void		setOffset(DWORD offset_) { offset = offset_; }
	void		setSize(DWORD size_) { size = size_; }
	void		setData(BYTE* data_) { data = data_; }
	void		setLoaded(bool loaded = true) { data_loaded = loaded; }

	// Data modification
	void	clearData();

	// Data import
	bool	importMem(void* data, DWORD size);
	bool	importMemChunk(MemChunk& mc);
	bool	importFile(string filename, DWORD offset, DWORD size);
	bool	importLump(Lump* lump);

	// Extra properties stuff
	bool	hasExProp(string key);
	string	getExProp(string key);
	bool	setExProp(string key, string value);
};

#endif //__LUMP_H__
