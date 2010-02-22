
#ifndef __ENTRYTYPE_H__
#define __ENTRYTYPE_H__

enum {
	EDF_ANY = 0,
	EDF_PNG,
	EDF_BMP,
	EDF_JPEG,
	EDF_GFX_DOOM,
	EDF_GFX_FLAT,
	EDF_WAD,
	EDF_MUS,
	EDF_MIDI,
	EDF_MOD_IT,
	EDF_MOD_XM,
	EDF_MOD_S3M,
	EDF_MOD_MOD,
	EDF_SND_DOOM,
	EDF_SND_WAV,

	EDF_UNKNOWN,
};

class EntryType {
private:
	// Type info
	string		id;
	string		name;
	string		extension;			// File extension to use when exporting entries of this type

	// Type matching criteria
	uint16_t		format;				// To be of this type, the entry data must match the specified format
	vector<string>	match_extension;
	vector<string>	match_name;
	vector<int>		match_size;
	int				size_limit[2];		// Minimum/maximum size
	vector<int>		size_multiple;		// Entry size must be a multiple of this

public:
	EntryType(string id = _T("Unknown"));
	~EntryType();

	// Setters
	void setName(string name)			{ this->name = name; }
	void setExtension(string extension)	{ this->extension = extension; }
	void setFormat(uint16_t format)		{ this->format = format; }
	void addMatchExtension(string ext) 	{ this->match_extension.push_back(ext); }
	void addMatchName(string name) 		{ this->match_name.push_back(name); }
	void setMinSize(int size) 			{ this->size_limit[0] = size; }
	void setMaxSize(int size)			{ this->size_limit[1] = size; }
	void addSizeMultiple(int size) 		{ this->size_multiple.push_back(size); }
	void addMatchSize(int size)			{ this->match_size.push_back(size); }

	// Getters
	string		getId()			{ return id; }
	string		getName()		{ return name; }
	string		getExtension()	{ return extension; }
	uint16_t	getFormat()		{ return format; }
	/*
	string		getMatchExtension()	{ return match_extension; }
	string		getMatchName()		{ return match_name; }
	int			getMinSize()		{ return size_limit[0]; }
	int			getMaxSize()		{ return size_limit[1]; }
	int			getSizeMultiple()	{ return size_multiple; }
	*/

	// Misc
	void	addToList();
	void	dump();

	// Static functions
	static bool loadEntryTypes();
};

#endif//__ENTRYTYPE_H__
