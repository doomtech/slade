
#ifndef __SFONT_H__
#define __SFONT_H__

#include "GLTexture.h"

class SFontChar {
private:
	uint16_t	width;
	uint16_t	height;
	rect_t		tex_bounds;

public:
	SFontChar();
	~SFontChar();
};

class SFont {
private:
	SFontChar*	characters[256];
	GLTexture	texture;

public:
	SFont();
	~SFont();
	
	// Font reading
	bool	loadFont0(MemChunk& mc);
	bool	loadFont1(MemChunk& mc);
	bool	loadFont2(MemChunk& mc);
	bool	loadFontM(MemChunk& mc);
	bool	loadBMF(MemChunk& mc);
	
	// Rendering
	void	drawCharacter(char c, rgba_t colour = COL_WHITE);
	void	drawString(string str, rgba_t colour = COL_WHITE);
};

#endif//__SFONT_H__
