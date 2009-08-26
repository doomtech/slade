
#ifndef __SIMAGE_H__
#define	__SIMAGE_H__

#include "Palette.h"

class SImage {
private:
	int			width;
	int			height;
	uint8_t*	data;
	Palette8bit	palette;
	bool		has_palette;
	int			offset_x;
	int			offset_y;

public:
	SImage();
	virtual ~SImage();

	uint8_t*		getRGBAData() { return data; }
	int				getWidth() { return width; }
	int				getHeight() { return height; }
	Palette8bit&	getPalette() { return palette; }
	bool			hasPalette() { return has_palette; }
	point2_t		offset() { return point2_t(offset_x, offset_y); }

	bool	loadImage(uint8_t* data, int size);
	bool	loadDoomGfx(uint8_t* data, int size);
	bool	loadDoomFlat(uint8_t* data, int size);

	bool	toPNG(MemChunk& out);
	bool	toDoomGfx(MemChunk& out);
	bool	toDoomFlat(MemChunk& out);
};

#endif //__SIMAGE_H__
