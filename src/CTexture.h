
#ifndef __CTEXTURE_H__
#define __CTEXTURE_H__

#include "Tokenizer.h"
#include "ArchiveEntry.h"
#include "ListenerAnnouncer.h"

// Basic patch
class CTPatch {
protected:
	string			name;
	ArchiveEntry*	entry;
	int16_t			offset_x;
	int16_t			offset_y;

public:
	CTPatch();
	CTPatch(string name, int16_t offset_x = 0, int16_t offset_y = 0, ArchiveEntry* entry = NULL);
	~CTPatch();

	string			getName() { return name; }
	ArchiveEntry*	getEntry() { return entry; }
	int16_t			xOffset() { return offset_x; }
	int16_t			yOffset() { return offset_y; }

	void	setName(string name) { this->name = name; }
	void	setEntry(ArchiveEntry* entry) { this->entry = entry; }
	void	setOffsetX(int16_t offset) { offset_x = offset; }
	void	setOffsetY(int16_t offset) { offset_y = offset; }
};

// Extended patch (for TEXTURES)
class CTPatchEx : public CTPatch {
private:
	bool			flip_x;
	bool			flip_y;
	int16_t			rotation;
	vector<string>	translation;
	string			blend;
	rgba_t			blend_col;
	float			alpha;
	string			style;
	uint8_t			blendtype;	// 0=none, 1=translation, 2=blend(string), 3=blend(rgba)

public:
	CTPatchEx();
	CTPatchEx(CTPatch* copy);
	~CTPatchEx();

	bool	parse(Tokenizer& tz);
	string	asText();
};

class TextureXList;

class CTexture : public Announcer {
friend class TextureXList;
private:
	// Basic info
	string				name;
	uint16_t			width;
	uint16_t			height;
	double				scale_x;
	double				scale_y;
	bool				world_panning;
	vector<CTPatch*>	patches;

	// Extended (TEXTURES) info
	bool	extended;
	string	type;
	bool	optional;
	bool	no_decals;
	bool	null_texture;
	int16_t	offset_x;
	int16_t	offset_y;

public:
	CTexture();
	~CTexture();

	void	copyTexture(CTexture* copy);

	string		getName() { return name; }
	uint16_t	getWidth() { return width; }
	uint16_t	getHeight() { return height; }
	double		getScaleX() { return scale_x; }
	double		getScaleY() { return scale_y; }
	bool		worldPanning() { return world_panning; }
	bool		isExtended() { return extended; }
	bool		isOptional() { return optional; }
	bool		noDecals() { return no_decals; }
	bool		nullTexture() { return null_texture; }
	size_t		nPatches() { return patches.size(); }
	CTPatch*	getPatch(size_t index);

	void	setName(string name) { this->name = name; announce("modified"); }
	void	setWidth(uint16_t width) { this->width = width; announce("modified"); }
	void	setHeight(uint16_t height) { this->height = height; announce("modified"); }
	void	setScaleX(double scale) { this->scale_x = scale; announce("modified"); }
	void	setScaleY(double scale) { this->scale_y = scale; announce("modified"); }
	void	setScale(double x, double y) { this->scale_x = x; this->scale_y = y; announce("modified"); }

	void	clear();

	bool	addPatch(string patch, int16_t offset_x = 0, int16_t offset_y = 0, ArchiveEntry* entry = NULL, int index = -1);
	bool	removePatch(size_t index);
	bool	removePatch(string patch);
	bool	replacePatch(size_t index, string newpatch, ArchiveEntry* newentry = NULL);
	bool	duplicatePatch(size_t index, int16_t offset_x = 8, int16_t offset_y = 8);
	bool	swapPatches(size_t p1, size_t p2);

	bool	parse(Tokenizer& tz, string type);
	string	asText();

	bool	convertExtended();
};

#endif//__CTEXTURE_H__
