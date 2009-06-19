
#include "Main.h"
#include "SImage.h"
#include "FreeImage.h"

SImage::SImage() {
	width = 0;
	height = 0;
	data = NULL;
}

SImage::~SImage() {
	if (data)
		delete[] data;
}

bool SImage::loadPNG(uint8_t* data, int size) {
	// Check data
	if (!data) {
		Global::error = _T("Invalid data given");
		return false;
	}

	// Check size
	if (size < 8) {
		Global::error = _T("Invalid data size");
		return false;
	}

	// Create FreeImage bitmap from entry data
	FIMEMORY* mem = FreeImage_OpenMemory(data, size);
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(mem, 0);
	FIBITMAP *bm = FreeImage_LoadFromMemory(fif, mem, 0);
	FreeImage_CloseMemory(mem);

	// Check it created/read ok
	if (!bm) {
		Global::error = _T("Unable to read PNG data");
		return false;
	}

	// Get width & height
	width = FreeImage_GetWidth(bm);
	height = FreeImage_GetHeight(bm);

	// Convert to 32bpp & flip vertically
	FIBITMAP *rgb = FreeImage_ConvertTo32Bits(bm);
	FreeImage_FlipVertical(rgb);

	// Clear current data if it exists
	if (this->data)
		delete[] this->data;

	// Load raw RGBA data
	this->data = new uint8_t[width * height * 4];
	uint8_t* bits_rgb = FreeImage_GetBits(rgb);
	int c = 0;
	for (int a = 0; a < width * height; a++) {
		this->data[c++] = bits_rgb[a * 4 + 2];	// Red
		this->data[c++] = bits_rgb[a * 4 + 1];	// Blue
		this->data[c++] = bits_rgb[a * 4];		// Green
		this->data[c++] = bits_rgb[a * 4 + 3];	// Alpha
	}

	// Free memory
	FreeImage_Unload(rgb);
	FreeImage_Unload(bm);

	// Return success
	return true;
}
