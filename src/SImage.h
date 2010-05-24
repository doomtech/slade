
#ifndef __SIMAGE_H__
#define	__SIMAGE_H__

#include "Palette.h"
#include "ListenerAnnouncer.h"

enum SIFormat {
	PALMASK,	// 2 bytes per pixel: palette index and alpha value
	RGBA,		// 4 bytes per pixel: RGBA
};

class SImage : public Announcer {
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

	// Internal functions
	void	clearData(bool clear_mask = true);

public:
	SImage();
	virtual ~SImage();

	bool			isValid() { return (width > 0 && height > 0 && data); }

	SIFormat		getFormat() { return format; }
	bool			getRGBAData(MemChunk& mc, Palette8bit* pal = NULL);
	bool			getRGBData(MemChunk& mc, Palette8bit* pal = NULL);
	int				getWidth() { return width; }
	int				getHeight() { return height; }
	bool			hasPalette() { return has_palette; }
	point2_t		offset() { return point2_t(offset_x, offset_y); }

	void			setXOffset(int offset);
	void			setYOffset(int offset);

	// Misc
	void	clear();
	void	fillAlpha(uint8_t alpha = 0);
	short	findUnusedColour();
	bool	trim(int width, int height);
	bool	validFlatSize();
	size_t	countColours();
	void	shrinkPalette(Palette8bit* pal = NULL);

	// Image format reading
	bool	loadImage(const uint8_t* data, int size);
	bool	loadDoomGfx(const uint8_t* data, int size, uint8_t version = 0);
	bool	loadDoomGfxA(const uint8_t* data, int size) {return loadDoomGfx(data, size, 2);}
	bool	loadDoomGfxB(const uint8_t* data, int size) {return loadDoomGfx(data, size, 1);}
	bool	loadDoomSnea(const uint8_t* data, int size);
	bool	loadDoomFlat(const uint8_t* data, int size);
	bool	loadDoomArah(const uint8_t* gfx_data, int size);
	bool	loadDoomLegacy(const uint8_t* gfx_data, int size);
	bool	loadPlanar(const uint8_t* gfx_data, int size);
	bool	load4bitChunk(const uint8_t* gfx_data, int size);
	bool	loadImgz(const uint8_t* gfx_data, int size);
	bool	loadFont0(const uint8_t* gfx_data, int size);
	bool	loadFont1(const uint8_t* gfx_data, int size);
	bool	loadFont2(const uint8_t* gfx_data, int size);
	bool	loadFontM(const uint8_t* gfx_data, int size);
	bool	loadBMF(const uint8_t* gfx_data, int size);
	bool	loadSCSprite(const uint8_t* data, int size);

	// Image format writing
	bool	toPNG(MemChunk& out, Palette8bit* pal = NULL);
	bool	toDoomGfx(MemChunk& out, uint8_t alpha_threshold = 0);
	bool	toDoomFlat(MemChunk& out);
	bool	toPlanar(MemChunk& out, Palette8bit* pal = NULL);
	bool	to4bitChunk(MemChunk& out, Palette8bit* pal = NULL);

	// Conversion stuff
	bool	convertRGBA(Palette8bit* pal = NULL);
	bool	convertPaletted(Palette8bit* pal_target, Palette8bit* pal_current = NULL);
	bool	maskFromColour(rgba_t colour, Palette8bit* pal = NULL, bool force_mask = false);
	bool	cutoffMask(uint8_t threshold, bool force_mask = false);
};

#endif //__SIMAGE_H__
