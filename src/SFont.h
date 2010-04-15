
#ifndef __SFONT_H__
#define __SFONT_H__

#include <wx/hashmap.h>

class SFontChar {
private:
	uint16_t	width;
	uint16_t	height;
	uint8_t*	data;	// RGBA

public:
	SFontChar();
	~SFontChar();
};

// Declare hash map class to hold font characters
WX_DECLARE_INT_HASH_MAP(SFontChar, FontCharMap);

class SFont {
private:
	FontCharMap	characters;

public:
	SFont();
	~SFont();
};

#endif//__SFONT_H__
