
#ifndef __CTEXTURE_H__
#define __CTEXTURE_H__

#include "SImage.h"
#include "ArchiveEntry.h"

class CTPatch {
private:
	SImage		image;
	point2_t	offsets;
	fpoint2_t	scale;

public:
	CTPatch();
	~CTPatch();

	SImage*	getImage() { return &image; }
	int		xOffset() { return offsets.x; }
	int		yOffset() { return offsets.y; }

	bool	loadImage(ArchiveEntry* gfx_entry);
	void	setOffsets(int x, int y) { offsets.set(x, y); }
	void	setScale(double x, double y) { scale.set(x, y); }
};


class CTexture {
private:
	string				name;
	vector<CTPatch*>	patches;
	uint32_t			width;
	uint32_t			height;
	fpoint2_t			scale;

public:
	CTexture(string name, uint32_t width = 0, uint32_t height = 0);
	~CTexture();

	string		getName() { return name; }
	uint32_t	getWidth() { return width; }
	uint32_t	getHeight() { return height; }
	fpoint2_t	getScale() { return scale; }
	uint32_t	nPatches() { return patches.size(); }
	CTPatch*	getPatch(size_t index);

	bool	addPatch(CTPatch* patch);
};

#endif//__CTEXTURE_H__