
#ifndef __SIMAGE_H__
#define	__SIMAGE_H__

class SImage {
private:
	int			width;
	int			height;
	uint8_t*	data;

public:
	SImage();
	virtual ~SImage();

	uint8_t*	getRGBAData() { return data; }
	int			getWidth() { return width; }
	int			getHeight() { return height; }

	bool	loadPNG(uint8_t* data, int size);
	bool	loadDoomGfx(uint8_t* data, int size);
};

#endif //__SIMAGE_H__
