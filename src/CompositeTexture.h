
#ifndef __COMPOSITETEXTURE_H__
#define __COMPOSITETEXTURE_H__

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

	bool	loadImage(ArchiveEntry* gfx_entry);
	void	setOffsets(int x, int y) { offsets.set(x, y); }
	void	setScale(double x, double y) { scale.set(x, y); }
};


class CompositeTexture {
private:
	string				name;
	vector<CTPatch*>	patches;
	uint32_t			width;
	uint32_t			height;
	fpoint2_t			scale;

public:
	CompositeTexture(string name, uint32_t width = 0, uint32_t height = 0);
	~CompositeTexture();

	bool	addPatch(CTPatch* patch);
};

#endif//__COMPOSITETEXTURE_H__
