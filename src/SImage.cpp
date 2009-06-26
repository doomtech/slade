
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    SImage.cpp
 * Description: SImage class: Encapsulates a 32bit image, handles
 *              loading/saving different formats, palette conversions,
 *              offsets, and a bunch of other stuff
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "SImage.h"
#include "FreeImage.h"


/* SImage::SImage
 * SImage class constructor
 *******************************************************************/
SImage::SImage() {
	width = 0;
	height = 0;
	data = NULL;
	offset_x = 0;
	offset_y = 0;

	for (int a = 0; a < 256; a++)
		palette.colours[a].set(a, a, a, 255);
}

/* SImage::~SImage
 * SImage class destructor
 *******************************************************************/
SImage::~SImage() {
	if (data)
		delete[] data;
}

/* SImage::loadImage
 * Loads an image (if it's format is supported by FreeImage)
 * Returns false if the given image data was invalid, true otherwise
 *******************************************************************/
bool SImage::loadImage(uint8_t* img_data, int size) {
	// Check data
	if (!img_data) {
		Global::error = _T("Invalid data given");
		return false;
	}

	// Check size
	if (size < 8) {
		Global::error = _T("Invalid data size");
		return false;
	}

	// Create FreeImage bitmap from entry data
	FIMEMORY* mem = FreeImage_OpenMemory(img_data, size);
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(mem, 0);
	FIBITMAP *bm = FreeImage_LoadFromMemory(fif, mem, 0);
	FreeImage_CloseMemory(mem);

	// Check it created/read ok
	if (!bm) {
		Global::error = _T("Unable to read image data (unsupported format?)");
		return false;
	}

	// Get image palette if it exists
	RGBQUAD* bm_pal = FreeImage_GetPalette(bm);
	if (bm_pal) {
		palette.valid = true;
		for (int a = 0; a < 256; a++)
			palette.colours[a] = rgba_t(bm_pal[a].rgbRed, bm_pal[a].rgbGreen, bm_pal[a].rgbBlue, 255);
	}
	else
		palette.valid = false;

	// Get width & height
	width = FreeImage_GetWidth(bm);
	height = FreeImage_GetHeight(bm);

	// Convert to 32bpp & flip vertically
	FIBITMAP *rgb = FreeImage_ConvertTo32Bits(bm);
	FreeImage_FlipVertical(rgb);

	// Clear current data if it exists
	if (data)
		delete[] data;

	// Load raw RGBA data
	data = new uint8_t[width * height * 4];
	uint8_t* bits_rgb = FreeImage_GetBits(rgb);
	int c = 0;
	for (int a = 0; a < width * height; a++) {
		data[c++] = bits_rgb[a * 4 + 2];	// Red
		data[c++] = bits_rgb[a * 4 + 1];	// Blue
		data[c++] = bits_rgb[a * 4];		// Green
		data[c++] = bits_rgb[a * 4 + 3];	// Alpha
	}

	// If the image was a PNG
	if (fif == FIF_PNG) {
		// Find offsets if present
		long xoff = 0;
		long yoff = 0;
		for (int a = 0; a < size; a++) {
			// Check for 'grAb' header
			if (img_data[a] == 'g' && img_data[a + 1] == 'r' &&
				img_data[a + 2] == 'A' && img_data[a + 3] == 'b') {
				memcpy(&xoff, data + a + 4, 4);
				memcpy(&yoff, data + a + 8, 4);
				xoff = wxINT32_SWAP_ON_LE(xoff);
				yoff = wxINT32_SWAP_ON_LE(yoff);
				break;
			}

			// Stop when we get to the 'IDAT' chunk
			if (img_data[a] == 'I' && img_data[a + 1] == 'D' &&
				img_data[a + 2] == 'A' && img_data[a + 3] == 'T')
				break;
		}

		offset_x = xoff;
		offset_y = yoff;
	}

	// Free memory
	FreeImage_Unload(rgb);
	FreeImage_Unload(bm);

	// Return success
	return true;
}

/* SImage::loadDoomGfx
 * Loads a Doom Gfx format image, using the given palette.
 * Returns false if the image data was invalid, true otherwise
 *******************************************************************/
bool SImage::loadDoomGfx(uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check size
	if (size < sizeof(patch_header_t))
		return false;

	// Get header & offsets
	patch_header_t* header = (patch_header_t*)gfx_data;
	long* col_offsets = (long*)((uint8_t*)gfx_data + sizeof(patch_header_t));

	// Setup variables
	width = header->width;
	height = header->height;
	offset_x = header->left;
	offset_y = header->top;
	palette.valid = true;

	// Clear current data if it exists
	if (data)
		delete[] data;

	// Load data
	data = new uint8_t[width * height * 4];
	memset(data, 0, width * height * 4);	// Set image to black/transparent by default
	for (int c = 0; c < width; c++) {
		// Go to start of column
		uint8_t* bits = gfx_data;
		bits += col_offsets[c];

		// Read posts
		while (1) {
			// Get row offset
			uint8_t row = *bits;

			if (row == 255) // End of column?
				break;

			// Get no. of pixels
			bits++;
			BYTE n_pix = *bits;

			bits++; // Skip buffer
			for (BYTE p = 0; p < n_pix; p++) {
				bits++;
				int pos = ((row + p)*width + c) * 4;
				rgba_t col = palette.colours[*bits];
				data[pos] = col.r;
				data[pos+1] = col.g;
				data[pos+2] = col.b;
				data[pos+3] = 255;
			}
			bits += 2; // Skip buffer & go to next row offset
		}
	}

	// Return success
	return true;
}

/* SImage::loadDoomFlat
 * Loads a Doom Flat format image, using the given palette.
 * Returns false if the image data was invalid, true otherwise
 *******************************************************************/
bool SImage::loadDoomFlat(uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check size
	if (size != 64*64)
		return false;

	// Setup variables
	width = 64;
	height = 64;
	palette.valid = true;

	// Clear current data if it exists
	if (data)
		delete[] data;

	// Read raw pixel data
	data = new uint8_t[64*64*4];
	int c = 0;
	for (int a = 0; a < 64*64; a ++) {
		rgba_t col = palette.colours[gfx_data[a]];
		data[c++] = col.r;	// Red
		data[c++] = col.g;	// Green
		data[c++] = col.b;	// Blue
		data[c++] = 255;	// Alpha
	}

	return true;
}
