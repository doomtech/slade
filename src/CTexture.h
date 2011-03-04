
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
	void	searchEntry(Archive* parent);
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
	rgba_t			colour;
	float			alpha;
	string			style;
	uint8_t			blendtype;	// 0=none, 1=translation, 2=blend, 3=tint

public:
	CTPatchEx();
	CTPatchEx(string name, int16_t offset_x = 0, int16_t offset_y = 0, ArchiveEntry* entry = NULL);
	CTPatchEx(CTPatch* copy);
	CTPatchEx(CTPatchEx* copy);
	~CTPatchEx();

	bool	flipX() { return flip_x; }
	bool	flipY() { return flip_y; }
	int16_t	getRotation() { return rotation; }
	rgba_t	getColour() { return colour; }
	float	getAlpha() { return alpha; }
	string	getStyle() { return style; }
	uint8_t	getBlendType() { return blendtype; }

	void	flipX(bool flip) { flip_x = flip; }
	void	flipY(bool flip) { flip_y = flip; }
	void	setRotation(int16_t rot) { rotation = rot; }
	void	setColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { colour.set(r, g, b, a); }
	void	setAlpha(float a) { alpha = a; }
	void	setStyle(string s) { style = s; }
	void	setBlendType(uint8_t type) { blendtype = type; }

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
	int16_t		getOffsetX() { return offset_x; }
	int16_t		getOffsetY() { return offset_y; }
	bool		worldPanning() { return world_panning; }
	string		getType() { return type; }
	bool		isExtended() { return extended; }
	bool		isOptional() { return optional; }
	bool		noDecals() { return no_decals; }
	bool		nullTexture() { return null_texture; }
	size_t		nPatches() { return patches.size(); }
	CTPatch*	getPatch(size_t index);

	void	setName(string name) { this->name = name; }
	void	setWidth(uint16_t width) { this->width = width; }
	void	setHeight(uint16_t height) { this->height = height; }
	void	setScaleX(double scale) { this->scale_x = scale; }
	void	setScaleY(double scale) { this->scale_y = scale; }
	void	setScale(double x, double y) { this->scale_x = x; this->scale_y = y; }
	void	setOffsetX(int16_t offset) { this->offset_x = offset; }
	void	setOffsetY(int16_t offset) { this->offset_y = offset; }
	void	setWorldPanning(bool wp) { this->world_panning = wp; }
	void	setType(string type) { this->type = type; }
	void	setExtended(bool ext) { this->extended = ext; }
	void	setOptional(bool opt) { this->optional = opt; }
	void	setNoDecals(bool nd) { this->no_decals = nd; }
	void	setNullTexture(bool nt) { this->null_texture = nt; }

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
