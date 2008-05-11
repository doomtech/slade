
#ifndef __LUMP_H__
#define __LUMP_H__

class Wad;

#include <wx/hashmap.h>
WX_DECLARE_STRING_HASH_MAP(string, PropertyList);

class Lump
{
private:
	Wad*			parent;
	string			name;
	DWORD			offset;
	BYTE*			data;
	DWORD			size;
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

	// Modifiers
	void		setParent(Wad* parent_) { parent = parent_; }
	void		setName(string name_) { name = name_; }
	void		setOffset(DWORD offset_) { offset = offset_; }
	void		setSize(DWORD size_) { size = size_; }
	void		setData(BYTE* data_) { data = data_; }


	// Extra properties stuff
	bool	hasExProp(string key);
	string	getExProp(string key);
	bool	setExProp(string key, string value);
};

#endif //__LUMP_H__
