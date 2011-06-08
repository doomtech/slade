
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
	int					filter;

	// Some generic/global textures
	static GLTexture	tex_background;	// Checkerboard background texture

	// Stuff used internally
	bool	loadData(const uint8_t* data, uint32_t width, uint32_t height, bool add = false);
	bool	loadImagePortion(SImage* image, rect_t rect, Palette8bit* pal = NULL, bool add = false);

public:
	enum {
		// Filter types
		NEAREST,
		LINEAR,
		MIPMAP,
	};
	
	GLTexture(bool allow_split = true);
	~GLTexture();

	bool		isLoaded() { return loaded; }
	uint32_t	getWidth() { return width; }
	uint32_t	getHeight() { return height; }
	int			getFilter() { return filter; }
	
	void		setFilter(int filter) { this->filter = filter; }

	bool	loadImage(SImage* image, Palette8bit* pal = NULL);
	bool	loadRawData(const uint8_t* data, uint32_t width, uint32_t height);

	bool	clear();
	bool	genChequeredTexture(uint8_t block_size, rgba_t col1, rgba_t col2);

	bool	bind();
	bool	draw2d(double x = 0, double y = 0, bool flipx = false, bool flipy = false);
	bool	draw2dTiled(uint32_t width, uint32_t height);

	static GLTexture&	bgTex();
	static void			resetBgTex();
};

#endif//__GLTEXTURE_H__
