
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

	// For multi-image files
	int			imgindex;
	int			numimages;

	// Internal functions
	void	clearData(bool clear_mask = true);

public:
	SImage();
	virtual ~SImage();

	bool			isValid() { return (width > 0 && height > 0 && data); }

	SIFormat		getFormat() { return format; }
	bool			getRGBAData(MemChunk& mc, Palette8bit* pal = NULL);
	bool			getRGBData(MemChunk& mc, Palette8bit* pal = NULL);
	bool			getPalData(MemChunk& mc);
	int				getWidth() { return width; }
	int				getHeight() { return height; }
	int				getIndex() { return imgindex; }
	int				getSize() { return numimages; }
	bool			hasPalette() { return has_palette; }
	point2_t		offset() { return point2_t(offset_x, offset_y); }

	void			setXOffset(int offset);
	void			setYOffset(int offset);

	// Misc
	void	clear();
	void	fillAlpha(uint8_t alpha = 0);
	short	findUnusedColour();
	bool	validFlatSize();
	size_t	countColours();
	void	shrinkPalette(Palette8bit* pal = NULL);

	// Image format reading
	bool	loadImage(const uint8_t* data, int size);
	bool	loadDoomGfx(const uint8_t* data, int size, uint8_t version = 0);
	bool	loadDoomGfxA(const uint8_t* data, int size) {return loadDoomGfx(data, size, 2);}
	bool	loadDoomGfxB(const uint8_t* data, int size) {return loadDoomGfx(data, size, 1);}
	bool	loadDoomSnea(const uint8_t* data, int size);
	bool	loadDoomFlat(const uint8_t* data, int size, bool columnmajor = false);
	bool	loadDoomArah(const uint8_t* gfx_data, int size, int transindex = 255);
	bool	loadQuake(const uint8_t* gfx_data, int size);
	bool	loadQuakeIIWal(const uint8_t* gfx_data, int size, int index);
	bool	loadRottGfx(const uint8_t* gfx_data, int size, bool transparent);
	bool	loadRottLbm(const uint8_t* gfx_data, int size);
	bool	loadRottRaw(const uint8_t* gfx_data, int size);
	bool	loadRottPic(const uint8_t* gfx_data, int size);
	bool	loadWolfPic(const uint8_t* gfx_data, int size);
	bool	loadWolfSprite(const uint8_t* gfx_data, int size);
	bool	loadPlanar(const uint8_t* gfx_data, int size);
	bool	load4bitChunk(const uint8_t* gfx_data, int size);
	bool	loadImgz(const uint8_t* gfx_data, int size);
	bool	loadFont0(const uint8_t* gfx_data, int size);
	bool	loadFont1(const uint8_t* gfx_data, int size);
	bool	loadFont2(const uint8_t* gfx_data, int size);
	bool	loadFontM(const uint8_t* gfx_data, int size);
	bool	loadBMF(const uint8_t* gfx_data, int size);
	bool	loadWolfFont(const uint8_t* gfx_data, int size);
	bool	loadSCSprite(const uint8_t* data, int size);
	bool	loadSCWall(const uint8_t* data, int size);
	bool	loadAnaMip(const uint8_t* data, int size);
	bool	loadBuildTile(const uint8_t* gfx_data, int size, int index);
	bool	loadHeretic2M8(const uint8_t* gfx_data, int size, int index);
	bool	loadHeretic2M32(const uint8_t* gfx_data, int size, int index);
	bool	loadHalfLifeTex(const uint8_t* gfx_data, int size, int index);
	bool	loadJediBM(const uint8_t* gfx_data, int size, int index);
	bool	JediFrame(const uint8_t* gfx_data, uint32_t hdroffs);
	bool	loadJediFME(const uint8_t* gfx_data, int size);
	bool	loadJediWAX(const uint8_t* gfx_data, int size, int index);
	bool	loadJediFNT(const uint8_t* gfx_data, int size);
	bool	loadJediFONT(const uint8_t* gfx_data, int size);

	// Image format writing
	bool	safeConvert(MemChunk& out, Palette8bit* pal = NULL);
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

	// Image modification
	bool	setPixel(int x, int y, rgba_t colour);
	bool	setPixel(int x, int y, uint8_t pal_index, uint8_t alpha = 255);
	bool	imgconv();
	bool	rotate(int angle);
	bool	mirror(bool vert);
	bool	crop(long x1, long y1, long x2, long y2);
	bool	resize(int nwidth, int nheight);
	bool	setImageData(uint8_t *ndata, int nwidth, int nheight, SIFormat nformat);
};

#endif //__SIMAGE_H__
