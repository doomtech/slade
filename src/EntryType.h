
#ifndef __ENTRYTYPE_H__
#define __ENTRYTYPE_H__

#include "PropertyList.h"
class ArchiveEntry;

/*
enum {
#define xa(id, name, val)			id = val,
#define xb(id, name)				id,
#define xx(id, name, func)			id,
#define xy(id, name, func, load)	id,
#include "EntryTypeList.h"
	EDF_UNKNOWN,
};

// Quick'n'dirty for now
class EntryDataFormat {
private:

public:
	EntryDataFormat(){}
	~EntryDataFormat(){}

	static bool isFormat(MemChunk& mc, uint16_t format);

	static bool detectPng(MemChunk& mc);
	static bool detectBmp(MemChunk& mc);
	static bool detectGif(MemChunk& mc);
	static bool detectPcx(MemChunk& mc);
	static bool detectTga(MemChunk& mc);
	static bool detectTiff(MemChunk& mc);
	static bool detectJpeg(MemChunk& mc);
	static bool detectDoomGfx(MemChunk& mc);
	static bool detectDoomGfxAlpha(MemChunk& mc);
	static bool detectDoomGfxBeta(MemChunk& mc);
	static bool detectDoomGfxArah(MemChunk& mc);
	static bool detectDoomGfxSnea(MemChunk& mc);
	static bool detectDoomFlat(MemChunk& mc);
	static bool detectDoomLegacy(MemChunk& mc);
	static bool detectPlanar(MemChunk& mc);
	static bool detect4bitChunk(MemChunk& mc);
	static bool detectImgz(MemChunk& mc);
	static bool detectPalette(MemChunk& mc);
	static bool detectTranslationTable(MemChunk& mc);
	static bool detectWad(MemChunk& mc);
	static bool detectZip(MemChunk& mc);
	static bool detectLib(MemChunk& mc);
	static bool detectDat(MemChunk& mc);
	static bool detectSCSprite(MemChunk& mc);
	static bool detectMus(MemChunk& mc);
	static bool detectMidi(MemChunk& mc);
	static bool detectModIt(MemChunk& mc);
	static bool detectModXm(MemChunk& mc);
	static bool detectModS3m(MemChunk& mc);
	static bool detectModMod(MemChunk& mc);
	static bool detectSndDoom(MemChunk& mc);
	static bool detectSndWav(MemChunk& mc);
	static bool detectSndOgg(MemChunk& mc);
	static bool detectSndFlac(MemChunk& mc);
	static bool detectSndSpeaker(MemChunk& mc);
	static bool detectTextureX(MemChunk& mc);
	static bool detectPnames(MemChunk& mc);
	static bool detectAnimated(MemChunk& mc);
	static bool detectSwitches(MemChunk& mc);
	static bool detectFontM(MemChunk& mc);
	static bool detectFont0(MemChunk& mc);
	static bool detectFont1(MemChunk& mc);
	static bool detectFont2(MemChunk& mc);
	static bool detectBMF(MemChunk& mc);
	static bool detectAnsi(MemChunk& mc);
	static bool detectZGLNodes(MemChunk& mc);
	static bool detectZGLNodes2(MemChunk& mc);
	static bool detectDMD(MemChunk& mc);
	static bool detectMDL(MemChunk& mc);
	static bool detectMD2(MemChunk& mc);
	static bool detectMD3(MemChunk& mc);
};
*/

class EntryType {
private:
	// Type info
	string		id;
	string		name;
	string		extension;		// File extension to use when exporting entries of this type
	string		icon;			// Icon to use in entry list
	string		editor;			// The in-program editor to use (hardcoded ids, see *EntryPanel constructors)
	int			index;
	bool		detectable;		// False only for special types that should be set not detected
	uint8_t		reliability;	// How 'reliable' this type's detection is. A higher value means it's less
								// likely this type can be detected erroneously. 0-255 (default is 255)
	PropertyList	extra;		// Extra properties for types with certain special behaviours
								// Current recognised properties listed below:
								// bool "image": Can be loaded into a SImage, therefore can be converted to other supported image formats
								// bool "patch": Can be used as a TEXTUREx patch

	// Type matching criteria
	string			format;				// To be of this type, the entry data must match the specified format
	vector<string>	match_extension;
	vector<string>	match_name;
	vector<int>		match_size;
	int				size_limit[2];		// Minimum/maximum size
	vector<int>		size_multiple;		// Entry size must be a multiple of this
	string			section;			// The 'section' of the archive the entry is in, eg "sprites" for entries
										// between SS_START/SS_END in a wad, or the 'sprites' folder in a zip
	vector<string>	match_archive;		// The types of archive the entry can be found in (e.g., wad or zip)

public:
	EntryType(string id = "Unknown");
	~EntryType();

	// Setters
	/*
	void setName(string name)			{ this->name = name; }
	void setExtension(string extension)	{ this->extension = extension; }
	void setIcon(string icon)			{ this->icon = icon; }
	void setFormat(string format)		{ this->format = format; }
	void setEditor(string editor)		{ this->editor = editor; }
	void addMatchExtension(string ext) 	{ this->match_extension.push_back(ext); }
	void addMatchName(string name) 		{ this->match_name.push_back(name); }
	void setMinSize(int size) 			{ this->size_limit[0] = size; }
	void setMaxSize(int size)			{ this->size_limit[1] = size; }
	void addSizeMultiple(int size) 		{ this->size_multiple.push_back(size); }
	void addMatchSize(int size)			{ this->match_size.push_back(size); }
	void setDetectable(bool detect)		{ this->detectable = detect; }
	void setSection(string section)		{ this->section = section; }
	void setReliability(uint8_t val)	{ this->reliability = val; }
	//void setGroup(string group)			{ this->group = group; }
	 */

	// Getters
	string			getId()				{ return id; }
	string			getName()			{ return name; }
	string			getExtension()		{ return extension; }
	string			getFormat()			{ return format; }
	string			getEditor()			{ return editor; }
	string			getIcon()			{ return icon; }
	//string			getGroup()			{ return group; }
	int				getIndex()			{ return index; }
	uint8_t			getReliability()	{ return reliability; }
	PropertyList&	extraProps()		{ return extra; }

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
	static void				cleanupEntryTypes();
};

#endif//__ENTRYTYPE_H__
