
#ifndef __CTEXTURE_H__
#define __CTEXTURE_H__

#include "TextureXList.h"
#include "PatchTable.h"
#include "Tokenizer.h"
#include "PropertyList.h"
#include "ListenerAnnouncer.h"

class CTPatch {
private:
	patch_t		patch;
	int16_t		offset_x;
	int16_t		offset_y;

	PropertyList	ex_props;

public:
	CTPatch();
	CTPatch(patch_t& patch, int16_t offset_x = 0, int16_t offset_y = 0);
	~CTPatch();

	string			getPatchName() { return patch.name; }
	ArchiveEntry*	getPatchEntry() { return patch.entry; }
	int16_t			xOffset() { return offset_x; }
	int16_t			yOffset() { return offset_y; }

	void			setPatchName(string name) { patch.name = name; }
	void			setPatchEntry(ArchiveEntry* entry) { patch.entry = entry; }
	void			setOffsetX(int16_t offset) { offset_x = offset; }
	void			setOffsetY(int16_t offset) { offset_y = offset; }
};

class CTexture : public Announcer {
private:
	string			name;
	uint16_t		width;
	uint16_t		height;
	double			scale_x;
	double			scale_y;
	vector<CTPatch>	patches;
	PropertyList	ex_props;

public:
	CTexture();
	~CTexture();

	string		getName() { return name; }
	uint16_t	getWidth() { return width; }
	uint16_t	getHeight() { return height; }
	double		getScaleX() { return scale_x; }
	double		getScaleY() { return scale_y; }
	size_t		nPatches() { return patches.size(); }
	CTPatch*	getPatch(size_t index);

	void	setName(string name) { this->name = name; announce(_T("modified")); }
	void	setWidth(uint16_t width) { this->width = width; announce(_T("modified")); }
	void	setHeight(uint16_t height) { this->height = height; announce(_T("modified")); }
	void	setScaleX(double scale) { this->scale_x = scale; announce(_T("modified")); }
	void	setScaleY(double scale) { this->scale_y = scale; announce(_T("modified")); }

	void	clear();

	bool	addPatch(patch_t& patch, int16_t offset_x = 0, int16_t offset_y = 0, int index = -1);
	bool	removePatch(size_t index);
	bool	replacePatch(size_t index, patch_t newpatch);
	bool	duplicatePatch(size_t index, int16_t offset_x = 8, int16_t offset_y = 8);
	bool	swapPatches(size_t p1, size_t p2);

	bool	fromTX(tx_texture_t& info, PatchTable& ptable);
	bool	toTX(tx_texture_t& info, PatchTable& ptable);
	bool	fromZD(Tokenizer& tz);
	string	toZD();
};

#endif//__CTEXTURE_H__
