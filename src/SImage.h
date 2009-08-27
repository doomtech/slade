
#ifndef __SIMAGE_H__
#define	__SIMAGE_H__

#include "Palette.h"

enum SIFormat {
	PALMASK,	// 2 bytes per pixel: palette index and alpha value
	RGBA,		// 4 bytes per pixel: RGBA
};

class SImage {
private:
	int			width;
	int			height;
	uint8_t*	data;
	uint8_t*	mask;
	SIFormat	format;
	Palette8bit	palette;
	bool		has_palette;
	int			offset_x;
	int			offset_y;

public:
	SImage();
	virtual ~SImage();

	uint8_t*		getRGBAData();
	int				getWidth() { return width; }
	int				getHeight() { return height; }
	Palette8bit&	getPalette() { return palette; }
	bool			hasPalette() { return has_palette; }
	point2_t		offset() { return point2_t(offset_x, offset_y); }

	void	clearData(bool clear_mask = true);

	void	applyPalette(Palette8bit& pal);

	bool	loadImage(uint8_t* data, int size);
	bool	loadDoomGfx(uint8_t* data, int size);
	bool	loadDoomFlat(uint8_t* data, int size);

	bool	toPNG(MemChunk& out);
	bool	toDoomGfx(MemChunk& out, uint8_t alpha_threshold = 0);
	bool	toDoomFlat(MemChunk& out);
};

#endif //__SIMAGE_H__
