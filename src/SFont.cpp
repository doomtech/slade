
#include "Main.h"
#include "SFont.h"

SFont::SFont() {
	// Init character map
	for (unsigned a = 0; a < 256; a++)
		characters[a] = NULL;
}

SFont::~SFont() {
}

bool SFont::loadFont0(MemChunk& mc) {
	return false;
}

bool SFont::loadFont1(MemChunk& mc) {
	return false;
}

bool SFont::loadFont2(MemChunk& mc) {
	return false;
}

bool SFont::loadFontM(MemChunk& mc) {
	/*
	// Check data
	if (!gfx_data || size % 256)
		return false;

	// Setup variables
	offset_x = offset_y = 0;
	has_palette = false;
	format = PALMASK;

	size_t charwidth = 8;
	size_t charheight = size>>8;
	width = charwidth;
	height = charheight << 8;

	if (width * height != size * 8)
		return false;

	// reset data
	clearData();
	data = new uint8_t[width*height];
	memset(data, 0xFF, width*height);
	mask = new uint8_t[width*height];
	memset(mask, 0x00, width*height);

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

	//Each pixel is described as a single bit, either on or off
	for (size_t i = 0; i < (unsigned)size; ++i) {
		for (size_t p = 0; p < 8; ++p)
			mask[(i*8)+p] = ((gfx_data[i]>>(7-p)) & 1) * 255;
	}
	return true;
	*/

	return false;
}

bool SFont::loadBMF(MemChunk& mc) {
	return false;
}

void SFont::drawCharacter(char c, rgba_t colour) {
	
}

void SFont::drawString(string str, rgba_t colour) {
	
}
