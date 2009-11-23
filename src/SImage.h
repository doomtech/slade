
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
	bool			getRGBAData(MemChunk& mc);
	bool			getRGBData(MemChunk& mc);
	int				getWidth() { return width; }
	int				getHeight() { return height; }
	Palette8bit*	getPalette() { return &palette; }
	bool			hasPalette() { return has_palette; }
	point2_t		offset() { return point2_t(offset_x, offset_y); }

	void			setPalette(Palette8bit* pal);
	void			setXOffset(int offset);
	void			setYOffset(int offset);

	// Misc
	void	fillAlpha(uint8_t alpha = 0);
	short	findUnusedColour();
	bool	trim(int width, int height);
	bool	validFlatSize();

	// Image format reading
	bool	loadImage(uint8_t* data, int size);
	bool	loadDoomGfx(uint8_t* data, int size);
	bool	loadDoomFlat(uint8_t* data, int size);

	// Image format writing
	bool	toPNG(MemChunk& out);
	bool	toDoomGfx(MemChunk& out, uint8_t alpha_threshold = 0);
	bool	toDoomFlat(MemChunk& out);

	// Conversion stuff
	bool	convertRGBA();
	bool	convertPaletted(Palette8bit* pal, uint8_t alpha_threshold = 0, bool keep_trans = true, rgba_t col_trans = COL_CYAN, int quant_type = 0);
	bool	maskFromColour(rgba_t colour, bool force_mask = false);
	bool	cutoffMask(uint8_t threshold, bool force_mask = false);
};


// CRC function needed for writing PNG grAb chunk
uint32_t crc(uint8_t *buf, uint32_t len);

#endif //__SIMAGE_H__
