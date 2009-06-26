
#ifndef __SIMAGE_H__
#define	__SIMAGE_H__

struct palette_t {
	rgba_t	colours[255];
	short	index_trans;
	bool	valid;
	palette_t() { index_trans = -1; valid = false; }
};

class SImage {
private:
	int			width;
	int			height;
	uint8_t*	data;
	palette_t	palette;
	int			offset_x;
	int			offset_y;

public:
	SImage();
	virtual ~SImage();

	uint8_t*	getRGBAData() { return data; }
	int			getWidth() { return width; }
	int			getHeight() { return height; }

	bool	loadImage(uint8_t* data, int size);
	bool	loadDoomGfx(uint8_t* data, int size);
	bool	loadDoomFlat(uint8_t* data, int size);
};

#endif //__SIMAGE_H__
