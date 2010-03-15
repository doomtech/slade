
#ifndef __ENTRYTYPE_H__
#define __ENTRYTYPE_H__

class ArchiveEntry;

enum {
	EDF_ANY = 0,
	EDF_PNG,
	EDF_BMP,
	EDF_JPEG,
	EDF_GFX_DOOM,
	EDF_GFX_DOOM_ARAH,
	EDF_GFX_DOOM_ALPHA,
	EDF_GFX_DOOM_BETA,
	EDF_GFX_DOOM_SNEA,
	EDF_GFX_DOOM_GNUM,
	EDF_GFX_FLAT,
	EDF_GFX_FULLSCREEN,
	EDF_GFX_AUTOPAGE,
	EDF_GFX_STRIFESU,
	EDF_GFX_PLANAR,
	EDF_GFX_IMGZ,
	EDF_WAD,
	EDF_ZIP,
	EDF_MUS,
	EDF_MIDI,
	EDF_MOD_IT,
	EDF_MOD_XM,
	EDF_MOD_S3M,
	EDF_MOD_MOD,
	EDF_SND_DOOM,
	EDF_SND_WAV,
	EDF_TEXT,

	EDF_UNKNOWN,
};

// Quick'n'dirty for now
class EntryDataFormat {
private:

public:
	EntryDataFormat(){}
	~EntryDataFormat(){}

	static uint16_t detectFormat(MemChunk& mc);
	static bool isFormat(MemChunk& mc, uint16_t format);

	static bool detectPng(MemChunk& mc);
	static bool detectBmp(MemChunk& mc);
	static bool detectJpeg(MemChunk& mc);
	static bool detectDoomGfx(MemChunk& mc);
	static bool detectDoomGfxAlpha(MemChunk& mc);
	static bool detectDoomGfxBeta(MemChunk& mc);
	static bool detectDoomGfxArah(MemChunk& mc);
	static bool detectDoomGfxSnea(MemChunk& mc);
	static bool detectDoomGfxGnum(MemChunk& mc);
	static bool detectDoomFlat(MemChunk& mc);
	static bool detectAutopage(MemChunk& mc);
	static bool detectStrifeStartup(MemChunk& mc);
	static bool detectPlanar(MemChunk& mc);
	static bool detectImgz(MemChunk& mc);
	static bool detectWad(MemChunk& mc);
	static bool detectZip(MemChunk& mc);
	static bool detectMus(MemChunk& mc);
	static bool detectMidi(MemChunk& mc);
	static bool detectModIt(MemChunk& mc);
	static bool detectModXm(MemChunk& mc);
	static bool detectModS3m(MemChunk& mc);
	static bool detectModMod(MemChunk& mc);
	static bool detectSndDoom(MemChunk& mc);
	static bool detectSndWav(MemChunk& mc);
};

class EntryType {
private:
	// Type info
	string		id;
	string		name;
	string		extension;	// File extension to use when exporting entries of this type
	string		icon;		// Icon to use in entry list
	string		editor;		// The in-program editor to use (hardcoded ids, see *EntryPanel constructors)
	int			index;
	bool		detectable;	// False only for special types that should be set not detected

	// Type matching criteria
	uint16_t		format;				// To be of this type, the entry data must match the specified format
	vector<string>	match_extension;
	vector<string>	match_name;
	vector<int>		match_size;
	int				size_limit[2];		// Minimum/maximum size
	vector<int>		size_multiple;		// Entry size must be a multiple of this
	string			section;			// The 'section' of the archive the entry is in, eg "sprites" for entries
										// between SS_START/SS_END in a wad, or the 'sprites' folder in a zip

public:
	EntryType(string id = _T("Unknown"));
	~EntryType();

	// Setters
	void setName(string name)			{ this->name = name; }
	void setExtension(string extension)	{ this->extension = extension; }
	void setIcon(string icon)			{ this->icon = icon; }
	void setFormat(uint16_t format)		{ this->format = format; }
	void setEditor(string editor)		{ this->editor = editor; }
	void addMatchExtension(string ext) 	{ this->match_extension.push_back(ext); }
	void addMatchName(string name) 		{ this->match_name.push_back(name); }
	void setMinSize(int size) 			{ this->size_limit[0] = size; }
	void setMaxSize(int size)			{ this->size_limit[1] = size; }
	void addSizeMultiple(int size) 		{ this->size_multiple.push_back(size); }
	void addMatchSize(int size)			{ this->match_size.push_back(size); }
	void setDetectable(bool detect)		{ this->detectable = detect; }
	void setSection(string section)		{ this->section = section; }

	// Getters
	string		getId()			{ return id; }
	string		getName()		{ return name; }
	string		getExtension()	{ return extension; }
	uint16_t	getFormat()		{ return format; }
	string		getEditor()		{ return editor; }
	string		getIcon()		{ return icon; }
	int			getIndex()		{ return index; }

	// Misc
	void	addToList();
	void	dump();
	void	copyToType(EntryType* target);

	// Magic goes here
	bool	isThisType(ArchiveEntry* entry);

	// Static functions
	static bool 			readEntryTypeDefinition(MemChunk& mc);
	static bool 			loadEntryTypes();
	static bool 			detectEntryType(ArchiveEntry* entry);
	static EntryType*		getType(string id);
	static EntryType*		unknownType();
	static EntryType*		folderType();
	static EntryType*		mapMarkerType();
	static wxArrayString	getIconList();
};

#endif//__ENTRYTYPE_H__
