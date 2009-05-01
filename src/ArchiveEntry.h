
#ifndef __ARCHIVEENTRY_H__
#define	__ARCHIVEENTRY_H__

class Archive;

#include <wx/hashmap.h>
WX_DECLARE_STRING_HASH_MAP(wxString, PropertyList);

class ArchiveEntry {
private:
	Archive*		parent;
	string			name;
	BYTE*			data;
	DWORD			size;
	bool			data_loaded;
	PropertyList	ex_props;

public:
	ArchiveEntry(string name = _T(""), Archive* parent = NULL);
	virtual ~ArchiveEntry();

	// Accessors
	Archive*	getParent() { return parent; }
	string		getName() { return name; }
	DWORD		getSize() { return size; }
	BYTE*		getData(bool allow_load = true);
	bool		isLoaded() { return data_loaded; }

	// Modifiers
	void		setParent(Archive* parent) { this->parent = parent; }
	void		setName(string name) { this->name = name; }
	void		setLoaded(bool loaded = true) { data_loaded = loaded; }
	void		setSize(DWORD size) { this->size = size; }
	void		setData(BYTE* data) { this->data = data; }

	// Data modification
	void	clearData();

	// Data import
	bool	importMem(void* data, DWORD size);
	bool	importMemChunk(MemChunk& mc);
	bool	importFile(string filename, DWORD offset, DWORD size);
	bool	importEntry(ArchiveEntry* entry);

	// Extra properties stuff
	bool	hasExProp(string key);
	string	getExProp(string key);
	bool	setExProp(string key, string value);
};


#endif //__ARCHIVEENTRY_H__
