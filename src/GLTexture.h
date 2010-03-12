
#ifndef __GLTEXTURE_H__
#define	__GLTEXTURE_H__

#include "SImage.h"

class GLTexture {
private:
	int		width;
	int		height;
	GLuint	id;
	bool	loaded;

public:
	GLTexture();
	~GLTexture();

	bool	isLoaded() { return loaded; }
	int		getWidth() { return width; }
	int		getHeight() { return height; }

	bool	loadData(const uint8_t* data, uint32_t width, uint32_t height);
	bool	loadImage(SImage* image);
	bool	loadImagePortion(SImage* image, rect_t rect);

	bool	genChequeredTexture(uint8_t block_size, rgba_t col1, rgba_t col2);

	bool	bind();
};

#endif//__GLTEXTURE_H__
