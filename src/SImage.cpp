
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
}

/* SImage::~SImage
 * SImage class destructor
 *******************************************************************/
SImage::~SImage() {
	if (data)
		delete[] data;
}

/* SImage::loadPNG
 * Loads a PNG image
 * Returns false if the given image data was invalid, true otherwise
 *******************************************************************/
bool SImage::loadPNG(uint8_t* img_data, int size) {
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
	// Get header & offsets
	patch_header_t* header = (patch_header_t*)gfx_data;
	long* col_offsets = (long*)((uint8_t*)gfx_data + sizeof(patch_header_t));

	// Setup variables
	width = header->width;
	height = header->height;

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
				data[pos] = *bits;
				data[pos+1] = *bits;
				data[pos+2] = *bits;
				data[pos+3] = 255;
			}
			bits += 2; // Skip buffer & go to next row offset
		}
	}

	// Return success
	return true;
}
