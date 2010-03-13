
#ifndef __GLTEXTURE_H__
#define	__GLTEXTURE_H__

#include "SImage.h"

struct gl_tex_t {
	GLuint		id;
	uint32_t	width;
	uint32_t	height;
};

class GLTexture {
private:
	uint32_t			width;
	uint32_t			height;
	vector<gl_tex_t>	tex;
	bool				loaded;
	bool				allow_split;

public:
	GLTexture(bool allow_split = true);
	~GLTexture();

	bool		isLoaded() { return loaded; }
	uint32_t	getWidth() { return width; }
	uint32_t	getHeight() { return height; }

	bool	loadData(const uint8_t* data, uint32_t width, uint32_t height, bool add = false);
	bool	loadImage(SImage* image);
	bool	loadImagePortion(SImage* image, rect_t rect, bool add = false);

	bool	clear();
	bool	genChequeredTexture(uint8_t block_size, rgba_t col1, rgba_t col2);

	bool	bind();
	bool	draw2d(int32_t x = 0, int32_t y = 0);
	bool	draw2dTiled(uint32_t width, uint32_t height);
};

#endif//__GLTEXTURE_H__
