
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    SImage.cpp
 * Description: SImage class - Encapsulates a paletted or 32bit image.
 *              Handles loading/saving different formats, palette
 *              conversions, offsets, and a bunch of other stuff
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
#include "Misc.h"
#include <wx/filefn.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
// Define grAb chunk struct
struct grab_chunk_t {
	char name[4];
	int32_t xoff;
	int32_t yoff;
};

// Define valid flat sizes
uint32_t valid_flat_size[][2] = {
	{  10,  12 },	// gnum format
	{  16,  16 },	// \ 
	{  32,  64 },	// Strife startup sprite
	{  48,  48 },	// /
	{  64,  64 },	// standard flat size
	{  64, 128 },	// Hexen flat size variant
	{ 128, 128 },	// \ 
	{ 256, 256 },	// hires flat size
	{ 512, 512 },	// /
	{ 320, 200 },	// full screen format
};
uint32_t	n_valid_flat_sizes = 10;


/*******************************************************************
 * SIMAGE CLASS FUNCTIONS
 *******************************************************************/

/* SImage::SImage
 * SImage class constructor
 *******************************************************************/
SImage::SImage() {
	width = 0;
	height = 0;
	data = NULL;
	mask = NULL;
	offset_x = 0;
	offset_y = 0;
	//has_palette = false;
	//has_builtinpal = false;
	format = RGBA;
}

/* SImage::~SImage
 * SImage class destructor
 *******************************************************************/
SImage::~SImage() {
	if (data)
		delete[] data;
}

/* SImage::getRGBAData
 * Loads the image as RGBA data into <mc>. Returns false if image is
 * invalid, true otherwise
 *******************************************************************/
bool SImage::getRGBAData(MemChunk& mc, Palette8bit* pal) {
	// Check the image is valid
	if (!isValid())
		return false;

	// Init rgba data
	mc.reSize(width * height * 4, false);

	// If data is already in RGBA format just return a copy
	if (format == RGBA) {
		mc.importMem(data, width * height * 4);
		return true;
	}

	// Otherwise convert
	else if (format == PALMASK) {
		// Get palette to use
		Palette8bit usepal;
		if (has_palette)
			usepal.copyPalette(&palette);
		else if (pal)
			usepal.copyPalette(pal);

		uint8_t rgba[4];
		for (int a = 0; a < width * height; a++) {
			// Get colour
			rgba_t col = usepal.colour(data[a]);

			// Set alpha
			if (mask)
				col.a = mask[a];
			else
				col.a = 255;

			col.write(rgba);	// Write colour to array
			mc.write(rgba, 4);	// Write array to MemChunk
		}

		return true;
	}

	return false;	// Invalid image type
}

/* SImage::getRGBData
 * Loads the image as RGB data into <mc>. Returns false if image is
 * invalid, true otherwise
 *******************************************************************/
bool SImage::getRGBData(MemChunk& mc, Palette8bit* pal) {
	// Check the image is valid
	if (!isValid())
		return false;

	// Init rgb data
	mc.reSize(width * height * 3, false);

	if (format == RGBA) {
		// RGBA format, remove alpha information
		for (int a = 0; a < width * height * 4; a += 4)
			mc.write(&data[a], 3);

		return true;
	}
	else if (format == PALMASK) {
		// Paletted, convert to RGB

		// Get palette to use
		Palette8bit usepal;
		if (has_palette)
			usepal.copyPalette(&palette);
		else if (pal)
			usepal.copyPalette(pal);

		// Build RGB data
		uint8_t rgba[4];
		for (int a = 0; a < width * height; a ++) {
			usepal.colour(data[a]).write(rgba);
			mc.write(rgba, 3);
		}

		return true;
	}

	return false;	// Invalid image type
}

/* SImage::setXOffset
 * Changes the image X offset
 *******************************************************************/
void SImage::setXOffset(int offset) {
	// Change the X offset
	offset_x = offset;

	// Announce change
	announce(_T("offsets_changed"));
}

/* SImage::setYOffset
 * Changes the image Y offset
 *******************************************************************/
void SImage::setYOffset(int offset) {
	// Change the Y offset
	offset_y = offset;

	// Announce change
	announce(_T("offsets_changed"));
}

/* SImage::clearData
 * Deletes/clears any existing image data
 *******************************************************************/
void SImage::clearData(bool clear_mask) {
	// Delete data if it exists
	if (data) {
		delete[] data;
		data = NULL;
	}

	// Delete mask if it exists
	if (mask && clear_mask) {
		delete[] mask;
		mask = NULL;
	}
}

void SImage::clear() {
	// Clear image data
	clearData(true);

	// Reset variables
	width = 0;
	height = 0;
	offset_x = 0;
	offset_y = 0;

	// Announce change
	announce(_T("image_changed"));
}

/* SImage::fillAlpha
 * 'Fills' the alpha channel or mask with the given <alpha> value
 *******************************************************************/
void SImage::fillAlpha(uint8_t alpha) {
	// Check image is valid
	if (!isValid())
		return;

	if (format == RGBA) {
		// RGBA format, set alpha values to given one
		for (int a = 3; a < width * height * 4; a += 4)
			data[a] = alpha;
	}
	else if (format == PALMASK) {
		// Paletted masked format, fill mask with alpha value
		if (!mask)
			mask = new uint8_t[width * height];

		memset(mask, alpha, width * height);
	}

	// Announce change
	announce(_T("image_changed"));
}

/* SImage::findUnusedColour
 * Returns the first unused palette index, or -1 if the image is not
 * paletted or uses all 256 colours
 *******************************************************************/
short SImage::findUnusedColour() {
	// Only for paletted images
	if (format != PALMASK)
		return -1;

	// Init used colours list
	uint8_t used[256];
	memset(used, 0, 256);

	// Go through image data and mark used colours
	for (int a = 0; a < width * height; a++)
		used[data[a]] = 1;

	// Find first unused
	for (int a = 0; a < 256; a++) {
		if (!used[a])
			return a;
	}

	// No unused colours found
	return -1;
}

bool SImage::trim(int width, int height) {
	// Check new width/height are smaller than current
	if (width > this->width || height > this->height)
		return false;

	if (format == RGBA) {
		// Write image portion to new data
		uint8_t* new_data = new uint8_t[width * height * 4];
		uint32_t c = 0;
		for (int row = 0; row < height; row++) {
			memcpy(new_data + c, data + c, width * 4);
			c += this->width * 4;
		}

		// Update variables
		clearData(true);
		data = new_data;
		this->width = width;
		this->height = height;

		return true;
	}
	else if (format == PALMASK) {
		// Write image portion to new data
		uint8_t* new_data = new uint8_t[width * height];
		uint32_t c = 0;
		for (int row = 0; row < height; row++) {
			memcpy(new_data + c, data + c, width);
			c += this->width;
		}

		// Write mask portion to new mask
		uint8_t* new_mask = new uint8_t[width * height];
		c = 0;
		for (int row = 0; row < height; row++) {
			memcpy(new_mask + c, mask + c, width);
			c += this->width;
		}

		// Update variables
		clearData(true);
		data = new_data;
		mask = new_mask;
		this->width = width;
		this->height = height;

		return true;
	}

	// Announce change
	announce(_T("image_changed"));

	return false;
}

bool SImage::validFlatSize() {
	for (uint32_t a = 0; a < n_valid_flat_sizes; a++) {
		if (width == valid_flat_size[a][0] &&
			height == valid_flat_size[a][1])
			return true;
	}
	return (width == 320 || width == 256);
}

/* SImage::loadImage
 * Loads an image (if its format is supported by FreeImage)
 * Returns false if the given image data was invalid, true otherwise
 *******************************************************************/
bool SImage::loadImage(const uint8_t* img_data, int size) {
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

	// Reset offsets
	offset_x = 0;
	offset_y = 0;

	// Create FreeImage bitmap from entry data
	FIMEMORY* mem = FreeImage_OpenMemory((BYTE*)img_data, size);
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
		has_palette = true;
		for (int a = 0; a < 256; a++)
			palette.setColour(a, rgba_t(bm_pal[a].rgbRed, bm_pal[a].rgbGreen, bm_pal[a].rgbBlue, 255));
	}
	else
		has_palette = false;

	// Get width & height
	width = FreeImage_GetWidth(bm);
	height = FreeImage_GetHeight(bm);

	// Set format
	format = RGBA;

	// Convert to 32bpp & flip vertically
	FIBITMAP *rgb = FreeImage_ConvertTo32Bits(bm);
	FreeImage_FlipVertical(rgb);

	// Clear current data if it exists
	clearData();

	// Load raw RGBA data
	data = new uint8_t[width * height * 4];
	uint8_t* bits_rgb = FreeImage_GetBits(rgb);
	int c = 0;
	for (int a = 0; a < width * height; a++) {
		data[c++] = bits_rgb[a * 4 + 2];	// Red
		data[c++] = bits_rgb[a * 4 + 1];	// Green
		data[c++] = bits_rgb[a * 4];		// Blue
		data[c++] = bits_rgb[a * 4 + 3];	// Alpha
	}

	// If the image was a PNG
	if (fif == FIF_PNG) {
		bool alPh_chunk = false;
		bool grAb_chunk = false;

		// Find offsets if present
		int32_t xoff = 0;
		int32_t yoff = 0;
		for (int a = 0; a < size; a++) {
			// Check for 'grAb' header
			if (!grAb_chunk &&
				img_data[a] == 'g' && img_data[a + 1] == 'r' &&
				img_data[a + 2] == 'A' && img_data[a + 3] == 'b') {
				memcpy(&xoff, img_data + a + 4, 4);
				memcpy(&yoff, img_data + a + 8, 4);
				xoff = wxINT32_SWAP_ON_LE(xoff);
				yoff = wxINT32_SWAP_ON_LE(yoff);
				grAb_chunk = true;
			}

			// Check for 'alPh' header
			if (!alPh_chunk &&
				img_data[a] == 'a' && img_data[a + 1] == 'l' &&
				img_data[a + 2] == 'P' && img_data[a + 3] == 'h') {
				alPh_chunk = true;
			}

			// Found both lumps so no need to search further
			if (grAb_chunk && alPh_chunk)
				break;

			// Stop when we get to the 'IDAT' chunk
			if (img_data[a] == 'I' && img_data[a + 1] == 'D' &&
				img_data[a + 2] == 'A' && img_data[a + 3] == 'T')
				break;
		}

		offset_x = xoff;
		offset_y = yoff;

		// If the picture is nothing more than a ZDoom-style alpha map
		if (alPh_chunk) {
			for (int a = 0; a < width * height; a++)
				data[a * 4 + 3] = (data[a * 4 + 2] | data[a * 4 + 1] | data[a * 4]);
		}
	}

	// Free memory
	FreeImage_Unload(rgb);
	FreeImage_Unload(bm);

	// Announce change
	announce(_T("image_changed"));

	// Return success
	return true;
}

/* SImage::loadDoomGfx
 * Loads a Doom Gfx format image, using the image palette. [version]
 * is the gfx 'version' to load as (0=normal, 1=press beta, 2=alpha).
 * Returns false if the image data was invalid, true otherwise
 *******************************************************************/
bool SImage::loadDoomGfx(const uint8_t* gfx_data, int size, uint8_t version) {
	// Check data
	if (!gfx_data)
		return false;

	// Check size
	if ((unsigned)size < (version > 1 ? sizeof(oldpatch_header_t) : sizeof(patch_header_t)))
		return false;

	// Init variables
	uint32_t* col_offsets = NULL;
	format = PALMASK;
	has_palette = false;

	// Read header
	uint8_t hdr_size = 0;
	if (version > 1) {
		width = gfx_data[0];
		height = gfx_data[1];
		offset_x = (int8_t)gfx_data[2];
		offset_y = (int8_t)gfx_data[3];
		hdr_size = 4;
	}
	else {
		patch_header_t* header = (patch_header_t*)gfx_data;
		width = wxINT16_SWAP_ON_BE(header->width);
		height = wxINT16_SWAP_ON_BE(header->height);
		offset_x = wxINT16_SWAP_ON_BE(header->left);
		offset_y = wxINT16_SWAP_ON_BE(header->top);
		hdr_size = 8;
	}

	// Read column offsets
	col_offsets = new uint32_t[width];
	if (version > 0) {
		uint16_t* c_ofs = (uint16_t*)((uint8_t*)gfx_data + hdr_size);
		for (int a = 0; a < width; a++)
			col_offsets[a] = wxUINT16_SWAP_ON_BE(c_ofs[a]);
	}
	else {
		uint32_t* c_ofs = (uint32_t*)((uint8_t*)gfx_data + hdr_size);
		for (int a = 0; a < width; a++)
			col_offsets[a] = wxUINT32_SWAP_ON_BE(c_ofs[a]);
	}

	// Clear current data if it exists
	clearData();

	// Load data
	data = new uint8_t[width * height];
	memset(data, 0, width * height);	// Set colour to palette index 0
	mask = new uint8_t[width * height];
	memset(mask, 0, width * height);	// Set mask to fully transparent
	for (int c = 0; c < width; c++) {
		// Get current column offset (byteswap if needed)
		uint32_t col_offset = col_offsets[c];//wxUINT32_SWAP_ON_BE(col_offsets[c]);

		// Check column offset is valid
		if (col_offset >= (unsigned)size) {
			clearData();
			return false;
		}

		// Go to start of column
		const uint8_t* bits = gfx_data;
		bits += col_offset;

		// Read posts
		int top = -1;
		while (1) {
			// Get row offset
			uint8_t row = *bits;

			if (row == 0xFF) // End of column?
				break;

			// Tall patches support
			if (row <= top && version == 0)
				top += row;
			else
				top = row;

			// Get no. of pixels
			bits++;
			uint8_t n_pix = *bits;

			if (version == 0) bits++; // Skip buffer
			for (uint8_t p = 0; p < n_pix; p++) {
				// Get pixel position
				bits++;
				int pos = ((top + p)*width + c);

				// Stop if we're outside the image
				if (pos > width*height)
					break;

				// Stop if for some reason we're outside the gfx data
				if (bits > gfx_data + size)
					break;

				// Write pixel data
				data[pos] = *bits;
				mask[pos] = 255;
			}
			if (version == 0) bits++; // Skip buffer
			bits++; // Go to next row offset
		}
	}

	// Announce change
	announce(_T("image_changed"));

	// Return success
	return true;
}

/* SImage::loadDoomFlat
 * Loads a Doom Flat format image, using the given palette.
 * Also loads about any raw image format that's the same as flats,
 * but with a different dimension.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadDoomFlat(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check/setup size
	bool valid_size = false;
	for (uint32_t a = 0; a < n_valid_flat_sizes; a++) {
		uint32_t w = valid_flat_size[a][0];
		uint32_t h = valid_flat_size[a][1];

		if (size == w * h || size - 4 == w * h) {
			width = w;
			height = h;
			valid_size = true;
			break;
		}
	}

	if (size == 8776) { // Inkworks and its signature at the end of COLORMAPS
		size = 8704;
	}
	// Check valid size
	if (!valid_size)
	{
		if (size % 320 == 0)	{	// This should handle any custom AUTOPAGE
			width = 320;
			height = size/320;
		} else if (size % 256 == 0) { // This allows display of COLORMAPS
			width = 256;
			height = size/256;
		} else {
			return false;
		}
	}

	// Setup variables
	format = PALMASK;
	has_palette = false;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];
	memcpy(data, gfx_data, width * height);

	// Create mask (all opaque)
	mask = new uint8_t[width*height];
	memset(mask, 255, width*height);

	// Announce change
	announce(_T("image_changed"));

	return true;
}


/* SImage::loadDoomArah
 * Loads a Doom Alpha Raw And Header image.
 * Returns false if the image data was invalid, true otherwise.
 * It seems index 255 is treated as transparent in this format.
 *******************************************************************/
bool SImage::loadDoomArah(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Setup variables
	patch_header_t* header = (patch_header_t*)gfx_data;
	width = wxINT16_SWAP_ON_BE(header->width);
	height = wxINT16_SWAP_ON_BE(header->height);
	offset_x = wxINT16_SWAP_ON_BE(header->left);
	offset_y = wxINT16_SWAP_ON_BE(header->top);
	format = PALMASK;
	has_palette = false;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];
	memcpy(data, gfx_data+8, width * height);

	// Create mask (all opaque)
	mask = new uint8_t[width*height];
	memset(mask, 255, width*height);

	// Mark as transparent all pixels that are set to FF
	for (size_t  i = 0; i < (unsigned)(width*height); ++i)
		if (data[i] == 255) mask[i] = 0;

	// Announce change
	announce(_T("image_changed"));

	return true;
}

/* SImage::loadDoomSnea
 * Loads a Doom alpha "snea" format image, using the given palette.
 * I have no idea if "snea" if the official name of the format, or
 * even what it is supposed to mean, but that's how DeuTex calls 
 * them, and it's the only available tool that can read them as far 
 * as I know, so this name will do.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadDoomSnea(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check/setup size
	offset_x = offset_y = 0;
	uint8_t qwidth = gfx_data[0];
	width = qwidth * 4;
	height = gfx_data[1];

	// The TITLEPIC in the Doom Press-Release Beta has 
	// two extraneous null bytes at the end for padding.
	if (size == 64004 && qwidth == 80 && height == 200)
		size = 64002;

	if (size != 2 + width * height)
		return false;

	// Setup variables
	format = PALMASK;
	has_palette = false;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];

	// Since gfx_data is a const pointer, we can't work on it.
	uint8_t * tempdata = new uint8_t[size];
	memcpy(tempdata, gfx_data, size);

	uint8_t* entryend = tempdata + size;
	uint8_t* dataend = data + size - 2;
	uint8_t* pixel = tempdata + 2;
	uint8_t* brush = data;

	// Algorithm taken from DeuTex.
	// I do not pretend to understand it, 
	// but my own attempt didn't work.
	while (pixel < entryend) {
		*brush = *pixel++;
		brush += 4;
		if (brush >= dataend)
			brush -= size - 3;
	}

	delete[] tempdata;

	// Create mask (all opaque)
	mask = new uint8_t[width*height];
	memset(mask, 255, width*height);

	// Announce change
	announce(_T("image_changed"));

	return true;
}

/* SImage::loadPlanar
 * Loads a planar graphic such as those used by Hexen's startup screen.
 * This was a convenient format back then as it could be sent directly
 * to the VGA memory; however this is now both obscure and obsolete.
 * The code is taken from ZDoom.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadPlanar(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check size
	if (size != 153648)
		return false;

	// Init some variables
	width = 640;
	height = 480;
	format = PALMASK;
	has_palette = true;

	union {
		RGBQUAD		color;
		uint32_t	quad;
	};
	color.rgbReserved = 0;
	rgba_t colour(0, 0, 0, 0, -1);

	// Initialize the bitmap palette.
	for (int i = 0; i < 16; ++i) {
		color.rgbRed   = gfx_data[i*3+0]; 
		color.rgbGreen = gfx_data[i*3+1];
		color.rgbBlue  = gfx_data[i*3+2];
		// Convert from 6-bit per component to 8-bit per component.
		quad = (quad << 2) | ((quad >> 4) & 0x03030303);
		colour.r = color.rgbRed;
		colour.g = color.rgbGreen;
		colour.b = color.rgbBlue;
		palette.setColour(i, colour);
	}
	// Fill the rest of the palette with clones of index 0
	for (int i = 16; i<256; ++i) {
		palette.setColour(i, palette.colour(0));
	}

	// Prepare data and mask (opaque)
	clearData();
	data = new uint8_t[width*height];
	mask = new uint8_t[width*height];
	memset(mask, 0xFF, width*height);

	uint8_t * dest = data;
	int y, x;
	const uint8_t *src1, *src2, *src3, *src4;
	size_t plane_size = width / 8 * height;

	src1 = gfx_data + 48;		// 80: 10000000	08: 00001000
	src2 = src1 + plane_size;	// 40: 01000000 04: 00000100
	src3 = src2 + plane_size;	// 20: 00100000 02: 00000010
	src4 = src3 + plane_size;	// 10: 00010000 01: 00000001

	for (y = height; y > 0; --y) {
		for (x = width; x > 0; x -= 8) {
			dest[0] = ((*src4 & 0x80) >> 4)	| ((*src3 & 0x80) >> 5) | ((*src2 & 0x80) >> 6) | ((*src1 & 0x80) >> 7);
			dest[1] = ((*src4 & 0x40) >> 3) | ((*src3 & 0x40) >> 4) | ((*src2 & 0x40) >> 5) | ((*src1 & 0x40) >> 6);
			dest[2] = ((*src4 & 0x20) >> 2) | ((*src3 & 0x20) >> 3) | ((*src2 & 0x20) >> 4) | ((*src1 & 0x20) >> 5);
			dest[3] = ((*src4 & 0x10) >> 1) | ((*src3 & 0x10) >> 2) | ((*src2 & 0x10) >> 3) | ((*src1 & 0x10) >> 4);
			dest[4] = ((*src4 & 0x08)     ) | ((*src3 & 0x08) >> 1) | ((*src2 & 0x08) >> 2) | ((*src1 & 0x08) >> 3);
			dest[5] = ((*src4 & 0x04) << 1) | ((*src3 & 0x04)     ) | ((*src2 & 0x04) >> 1) | ((*src1 & 0x04) >> 2);
			dest[6] = ((*src4 & 0x02) << 2) | ((*src3 & 0x02) << 1) | ((*src2 & 0x02)     ) | ((*src1 & 0x02) >> 1);
			dest[7] = ((*src4 & 0x01) << 3) | ((*src3 & 0x01) << 2) | ((*src2 & 0x01) << 1) | ((*src1 & 0x01)     );
			dest += 8;
			src1 += 1;
			src2 += 1;
			src3 += 1;
			src4 += 1;
		}
	}

	// Success
	return true;
}

/* SImage::load4bitChunk
 * Loads a 4bit graphic such as those used by Hexen's startup screen.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::load4bitChunk(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check size
	if (size == 32) {
		width = 4;
		height = 16;
	} else if (size == 184) {
		width = 16;
		height = 23;
	} else return false;

	format = PALMASK;
	has_palette = false;
	clearData();
	data = new uint8_t[width*height];
	mask = new uint8_t[width*height];
	memset(mask, 0xFF, width*height);

	for (int i = 0; i < size; ++i) {
		data[i*2  ] = ((gfx_data[i] & 0xF0)>>4);
		data[i*2+1] =  (gfx_data[i] & 0x0F);
	}
	return true;
}
/* SImage::loadImgz
 * Loads a picture in ZDoom's imgz format, using code adapted from it.
 * This format is special in that the info given is only for the alpha
 * channel, there is no color information at all.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadImgz(const uint8_t* gfx_data, int size) {
	if (size < sizeof(imgz_header_t))
		return false;

	// Setup variables
	imgz_header_t *header = (imgz_header_t *)gfx_data;
	width = wxINT16_SWAP_ON_BE(header->width);
	height = wxINT16_SWAP_ON_BE(header->height);
	offset_x = wxINT16_SWAP_ON_BE(header->left);
	offset_y = wxINT16_SWAP_ON_BE(header->top);
	has_palette = false;
	format = PALMASK;

	// Create data (all white) and mask
	clearData();
	data = new uint8_t[width*height];
	mask = new uint8_t[width*height];
	memset(data, 0xFF, width*height);

	if (!header->compression) {
		memcpy(mask, gfx_data + sizeof(imgz_header_t), size - sizeof(imgz_header_t));
		return true;
	}
	else {
		// Since gfx_data is a const pointer, we can't work on it.
		uint8_t * tempdata = new uint8_t[size]; 
		memcpy(tempdata, gfx_data, size);

		// We'll use wandering pointers. The original pointer is kept for cleanup.
		uint8_t * read = tempdata + sizeof(imgz_header_t);
		uint8_t * readend = read + size - 1;
		uint8_t * dest  = mask;
		uint8_t * destend = dest + width*height;

		uint8_t code = 0; size_t length = 0;

		while (read < readend && dest < destend) {
			code = *read++;
			if (code < 0x80) {
				length = code + 1;
				memcpy(dest, read, length);
				dest+=length; 
				read+=length;
			}
			else if (code > 0x80) {
				length = 0x101 - code;
				code = *read++;
				memset(dest, code, length);
				dest+=length;
			}
		}
		delete[] tempdata;
	}

	return true;
};

/* SImage::loadFont0
 * Loads a Doom alpha HUFONT lump and displays it as a picture.
 * Why "font0" when it has no FON0 header? Because alpha. ;)
 * The format used is simple: 
 * Offset | Length | Type | Name
 *  0x000 |      2 | ui16 | image height (one value for all chars)
 *  0x002 |  256*1 | ui08 | characterwidth (one value per char)
 *  0x102 |  256*2 | ui16 | characteroffset (one value per char)
 *  0x302 |    x*1 | ui08 | pixel color index (one value per pixel)
 * So, total size - 302 % value @ 0x00 must be null.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadFont0(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	if (size <= 0x302)
		return false;

	offset_x = offset_y = 0;
	height = gfx_data[0] + (gfx_data[1]<<8);

	size_t datasize = size - 0x302;
	if (datasize % height)
		return false;

	width = datasize / height;

	clearData();
	has_palette = false;
	format = PALMASK;

	// Create new picture and mask
	const uint8_t* r = gfx_data + 0x302;
	data = new uint8_t[datasize];
	mask = new uint8_t[datasize];
	memset(mask, 0xFF, datasize);

	// Data is in column-major format, convert to row-major
	size_t p = 0;
	for (size_t i = 0; i < datasize; ++i) {
		data[p] = r[i];

		// Index 0 is transparent
		if (data[p] == 0)
			mask[p] = 0;

		// Move to next column
		p+=width;

		// Move to next row
		if (p >= datasize) {
			p -= datasize;
			++p;
		}
	}
	return true;
}

/* SImage::loadFont1
 * Loads a ZDoom FON1 lump and displays it as a picture.
 * Graphically-speaking, a FON1 lump is a column of 256 characters,
 * each width*height as indicated by the header. Of course,
 * it would be better to convert that into a 16x16 grid, which
 * would be a lot more legible...
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadFont1(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check/setup size
	size_t charwidth = gfx_data[4]+256*gfx_data[5];
	width = charwidth;
	size_t charheight = gfx_data[6]+256*gfx_data[7];
	height = charheight<<8;
	offset_x = offset_y = 0;

	// Setup variables
	offset_x = offset_y = 0;
	has_palette = false;
	format = PALMASK;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];
	mask = new uint8_t[width*height];
	memset(mask, 0xFF, width*height);

	// Since gfx_data is a const pointer, we can't work on it.
	uint8_t * tempdata = new uint8_t[size];
	memcpy(tempdata, gfx_data, size);


	// We'll use wandering pointers. The original pointer is kept for cleanup.
	uint8_t * read = tempdata + 8;
	uint8_t * readend = tempdata + size - 1;
	uint8_t * dest  = data;
	uint8_t * destend = dest + width*height;

	uint8_t code = 0; size_t length = 0;

	// This uses the same RLE algorithm as compressed IMGZ.
	while (read < readend && dest < destend) {
		code = *read++;
		if (code < 0x80) {
			length = code + 1;
			memcpy(dest, read, length);
			dest+=length; 
			read+=length;
		}
		else if (code > 0x80) {
			length = 0x101 - code;
			code = *read++;
			memset(dest, code, length);
			dest+=length;
		}
	}
	delete[] tempdata;
	// Add transparency to mask
	for (size_t i = 0; i < (unsigned)(width*height); ++i)
		if (data[i] == 0)
			mask[i] = 0x00;

	// Announce change
	announce(_T("image_changed"));

	return true;
}

/* SImage::loadFont2
 * Loads a ZDoom FON2 lump and displays it as a picture.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
struct Font2Char {
	uint16_t width;
	uint8_t * data;
	Font2Char() { width = 0; data = NULL; }
};
struct Font2Header {
	uint8_t headr[4];
	uint16_t charheight;
	uint8_t firstc;
	uint8_t lastc;
	uint8_t constantw;
	uint8_t shading;
	uint8_t palsize;
	uint8_t kerning;	// Theoretically a flag field, but with only one flag...
};
bool SImage::loadFont2(const uint8_t* gfx_data, int size) {
	// Clear current data if it exists
	clearData();

	// Initializes some stuff
	offset_x = offset_y = 0;
	has_palette = true;
	format = PALMASK;

	if (size < sizeof(Font2Header))
		return false;

	const Font2Header * header = (Font2Header *) gfx_data;
	width = 0;
	height = wxUINT16_SWAP_ON_BE(header->charheight);

	// We can't deal with a null height, of course
	if (height == 0)
		return false;

	const uint8_t * p = gfx_data + sizeof(Font2Header);

	// Skip kerning information which do not concern us.
	if (header->kerning)
		p+=2;

	// Start building the character table.
	size_t numchars = wxUINT16_SWAP_ON_BE(header->lastc) - wxUINT16_SWAP_ON_BE(header->firstc) +1;
	Font2Char * chars = new Font2Char[numchars];
	for (size_t i = 0; i < numchars; ++i) {
		chars[i].width = wxUINT16_SWAP_ON_BE(*(uint16_t *)p);
		// Let's increase the total width
		width+=chars[i].width;
		// The width information is enumerated for each character only if they are
		// not constant width. Regardless, move the read pointer away after the last.
		if (!(header->constantw) || (i == numchars - 1))
			p+=2;
	}

	// Let's build the palette now. 
	for (size_t i = 0; i < header->palsize + 1u; ++i) {
		rgba_t color;
		color.r = *p++;
		color.g = *p++;
		color.b = *p++;
		palette.setColour(i, color);
	}

	// 0 is transparent, last is border color, the rest of the palette entries should
	// be increasingly bright
	palette.setTransIndex(0);

	// Clear current data if it exists
	clearData();

	// The picture data follows, using the same RLE as FON1 and IMGZ.
	for (size_t i = 0; i < numchars; ++i) {
		// A big font is not necessarily continuous; several characters
		// may be skipped; they are given a width of 0.
		if (chars[i].width) {
			size_t numpixels = chars[i].width * height;
			chars[i].data = new uint8_t[numpixels];
			uint8_t * d = chars[i].data;
			uint8_t code = 0; size_t length = 0;

			while (numpixels) {
				code = *p++;
				if (code < 0x80) {
					length = code + 1;
					// Overflows shouldn't happen
					if (length > numpixels) {
						return false;
					}
					memcpy(d, p, length);
					d+=length; 
					p+=length;
					numpixels -= length;
				}
				else if (code > 0x80) {
					length = 0x101 - code;
					// Overflows shouldn't happen
					if (length > numpixels) {
						return false;
					}
					code = *p++;
					memset(d, code, length);
					d+=length;
					numpixels -= length;
				}
			}
		}
	}

	// Now let's assemble all characters together in a single picture.
	if (!width)
		return false;

	// Clear current data if it exists
	clearData();

	data = new uint8_t[width*height];
	uint8_t * d = data;
	for (size_t i = 0; i < (unsigned)height; ++i) {
		for (size_t j = 0; j < numchars; ++j) {
			if (chars[j].width) {
				memcpy(d, chars[j].data+(i*chars[j].width), chars[j].width);
				d+=chars[j].width;
			}
		}
	}

	// Clean up the characters once the big picture is finished
	for (size_t i = 0; i < numchars; ++i) {
		if (chars[i].data)
			delete[] chars[i].data;
	}
	delete[] chars;

	// Now transparency for the mask
	mask = new uint8_t[width*height];
	memset(mask, 0xFF, width*height);
	for (size_t i = 0; i < (unsigned)(width*height); ++i)
		if (data[i] == 0)
			mask[i] = 0;

	// Yay!
	return true;
}

/* SImage::loadBMF
 * Loads a byte map font lump and displays it as a picture.
 * Specs for the format are here: http://bmf.wz.cz/bmf-format.htm
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
struct BMFChar {
	uint8_t which; // 0
	uint8_t width; // 1
	uint8_t height;// 2
	 int8_t offsx; // 3
	 int8_t offsy; // 4
	uint8_t shift; // 5
	// Rest is not part of the header proper
	const uint8_t * cdata;
	BMFChar() { which = width = height = offsx = offsy = shift = 0; cdata = NULL; }
};
struct BMFFont {
	uint8_t headr[4];	//  0
	uint8_t version;	//  4
	uint8_t lineheight;	//  5
	 int8_t size_over;	//  6
	 int8_t size_under;	//  7
	 int8_t add_space;	//  8
	 int8_t size_inner;	//  9
	uint8_t num_colors;	// 10
	uint8_t top_color;	// 11
	uint8_t reserved[4];// 12
	uint8_t pal_size;	// 16
	// Rest is not part of the header proper
	uint8_t info_size;
	uint16_t num_chars;
	const char * info;
	BMFChar * chars;
	BMFFont() {
		lineheight = size_over = size_under = add_space = size_inner =
			num_colors = top_color = pal_size = info_size = num_chars = 0;
		info = NULL; chars = NULL;
	}
	BMFFont(BMFFont * other) {
		lineheight = other->lineheight;
		size_over = other->size_over;
		size_under = other->size_under;
		add_space = other->add_space;
		size_inner = other->size_inner;
		num_colors = other->num_colors;
		top_color = other->top_color;
		pal_size = other->pal_size;
		info_size = other->info_size;
		num_chars = other->num_chars;
		info = NULL; chars = NULL;
	}
	~BMFFont() { if (chars) delete[] chars; }
};
bool SImage::loadBMF(const uint8_t* gfx_data, int size) {
	if (!gfx_data || size < 24)
		return false;
	const uint8_t * eod = gfx_data + size;

	BMFFont mf((BMFFont *)gfx_data);

	// Check for invalid data, we need at least one visible color
	if (mf.pal_size == 0)
		return false;

	// Clean up old data and set up variables
	clearData();
	format = PALMASK;
	has_palette = true;

	const uint8_t * ofs = gfx_data+17;

	// Setup palette -- it's a 6-bit palette (63 max) so we have to convert it to 8-bit.
	// Palette index 0 is used as the transparent color and not described at all.
	palette.setColour(0, rgba_t(0, 0, 0, 0));
	for (size_t i = 0; i < mf.pal_size; ++i) {
		palette.setColour(i+1, rgba_t((ofs[(i*3)]<<2)+(ofs[(i*3)]>>4), 
			(ofs[(i*3)+1]<<2)+(ofs[(i*3)]>>4), (ofs[(i*3)+2]<<2)+(ofs[(i*3)+2]>>4), 255));
	}

	// Move to after the palette and get amount of characters
	ofs += mf.pal_size*3;
	if (ofs >= eod) {
		wxLogMessage(_T("BMF aborted: no data after palette"));
		return false;
	}
	mf.info_size = ofs[0];
	if (mf.info_size > 0) {
		mf.info = (char *) ofs+1;
	}
	ofs+=mf.info_size+1;
	mf.num_chars = ofs[0] + (ofs[1]<<8);
	if (mf.num_chars == 0)
		return false;

	// Now we are at the beginning of character data
	ofs+=2;
	if (ofs >= eod) {
		wxLogMessage(_T("BMF aborted: no data after char size"));
		return false;
	}
	// Let's create each character's data and compute the total size
	mf.chars = new BMFChar[mf.num_chars];
	int miny = ofs[4], maxy = ofs[2];
	width = ofs[5] + abs(ofs[3]);
	for (size_t i = 0; i < mf.num_chars; ++i) {
		mf.chars[i].which = ofs[0];
		mf.chars[i].width = ofs[1];
		mf.chars[i].height= ofs[2];
		mf.chars[i].offsx = ofs[3];
		mf.chars[i].offsy = ofs[4];
		mf.chars[i].shift = ofs[5];
		mf.chars[i].cdata = ofs+6;
		ofs+=(6+ (mf.chars[i].width * mf.chars[i].height));
		// Sanity check, some supposedly-valid fonts do not have all the
		// characters they pretend to have (e.g., 274.bmf). Being truncated
		// does not prevent them from being considered valid, so cut them off
		if (ofs >= eod && (i+1) < mf.num_chars) {
			mf.num_chars = i;
		}
		// Zap empty characters, no need to waste space displaying their void.
		if (mf.chars[i].width == 0 && mf.chars[i].height == 0) {
			--i;
			--mf.num_chars;
		} else {
			if (miny > mf.chars[i].offsy)
				miny = mf.chars[i].offsy;
			if (maxy < mf.chars[i].height)
				maxy = mf.chars[i].height;
			width += mf.add_space + mf.chars[i].shift;
		}
	}
	height = maxy - miny;

	// Create new fully transparent image
	size_t pixela = 0, pixelb = 0, pixels = width * height;
	data = new uint8_t[pixels];
	mask = new uint8_t[pixels];
	memset(data, 0x00, pixels);
	memset(mask, 0x00, pixels);

	// Start processing each character, painting it on the empty canvas
	int startx = (mf.chars[0].offsy < 0 ? 0 : mf.chars[0].offsy);
	int starty = (miny < 0 ? 0 - miny : 0);
	for (int i = 0; i < mf.num_chars; ++i) {
		BMFChar * mc = &mf.chars[i];
		if (mc->width && mc->height) {
			for (int v = 0; v < mc->height; ++v) {
				for (int u = 0; u < mc->width; ++u) {
					// Source pixel
					pixela = v*mc->width + u;
					// Destination pixel
					pixelb = (starty+v+mc->offsy)*width+startx+u+mc->offsx;
					// Only paint if appropriate
					if ((mc->cdata + pixela < eod) && (mc->cdata + pixela < mf.chars[i+1].cdata - 6) &&
						mc->cdata[pixela] && pixelb < pixels) {
						data[pixelb] = mc->cdata[pixela];
						mask[pixelb] = 0xFF;
					}
				}
			}
			startx+=(mf.add_space + mc->shift);
		}
	}
	return true;
}

/* SImage::loadFontM
 * Loads a monochrome, monospaced font and displays it as a picture.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadFontM(const uint8_t* gfx_data, int size) {
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

	//Each pixel is described as a single bit, either on or off
	for (size_t i = 0; i < (unsigned)size; ++i) {
		for (size_t p = 0; p < 8; ++p)
			mask[(i*8)+p] = ((gfx_data[i]>>(7-p)) & 1) * 255;
	}
	return true;
}


/* SImage::countColours
 * Returns the number of unique colors in a paletted image
 *******************************************************************/
size_t SImage::countColours() {
	// If the picture is not paletted, return 0.
	if (format != PALMASK)
		return 0;

	bool * usedcolours = new bool[256];
	memset(usedcolours, 0, 256);
	size_t used = 0;

	for (int a = 0; a < width*height; ++a) {
		usedcolours[data[a]] = true;
	}
	for (size_t b = 0; b < 256; ++b) {
		if (usedcolours[b])
			++used;
	}

	delete[] usedcolours;
	return used;
}

/* SImage::shrinkPalette
 * Shifts all the used colours to the beginning of the palette
 *******************************************************************/
void SImage::shrinkPalette(Palette8bit* pal) {
	// If the picture is not paletted, stop.
	if (format != PALMASK)
		return;

	// Check if a palette was given (if not, process the image's palette)
	if (!pal)
		pal = &palette;

	// Init variables
	Palette8bit newpal;
	bool * usedcolours = new bool[256];
	int * remap = new int[256];
	memset(usedcolours, 0, 256);
	size_t used = 0;

	// Count all color indices actually used on the picture
	for (int a = 0; a < width*height; ++a) {
		usedcolours[data[a]] = true;
	}

	// Create palette remapping information
	for (size_t b = 0; b < 256; ++b) {
		if (usedcolours[b]) {
			newpal.setColour(used, pal->colour(b));
			remap[b] = used;
			++used;
		}
	}

	// Remap image to new palette indices
	for (int c = 0; c < width*height; ++c) {
		data[c] = remap[data[c]];
	}
	pal->copyPalette(&newpal);

	// Cleanup
	delete[] usedcolours;
	delete[] remap;
}

/* SImage::toPNG
 * Writes the image as PNG data to <out>, keeping palette information
 * if it exists. Returns true on success
 *******************************************************************/
bool SImage::toPNG(MemChunk& out, Palette8bit* pal) {
	FIBITMAP* bm;

	if (format == RGBA) {
		// Init 32bpp FIBITMAP
		bm = FreeImage_Allocate(width, height, 32, 0x0000FF00, 0x00FF0000, 0x000000FF);

		// Write image data
		uint8_t* bits = FreeImage_GetBits(bm);
		uint32_t c = 0;
		for (int a = 0; a < width * height * 4; a += 4) {
			bits[c++] = data[a+2];
			bits[c++] = data[a+1];
			bits[c++] = data[a];
			bits[c++] = data[a+3];
		}
	}
	else if (format == PALMASK) {
		// Init 8bpp FIBITMAP
		bm = FreeImage_Allocate(width, height, 8);

		// Get palette to use
		Palette8bit usepal;
		if (has_palette)
			usepal.copyPalette(&palette);
		else if (pal)
			usepal.copyPalette(pal);

		// Set palette
		RGBQUAD* bm_pal = FreeImage_GetPalette(bm);
		for (int a = 0; a < 256; a++) {
			bm_pal[a].rgbRed = palette.colour(a).r;
			bm_pal[a].rgbGreen = palette.colour(a).g;
			bm_pal[a].rgbBlue = palette.colour(a).b;
		}

		// Find unused colour (for transparency)
		short unused = findUnusedColour();

		// Set any transparent pixels to this colour (if we found an unused colour)
		if (unused >= 0) {
			for (int a = 0; a < width * height; a++) {
				if (mask[a] == 0)
					data[a] = unused;
			}

			// Set palette transparency
			usepal.setTransIndex(unused);
		}

		// Set freeimage palette transparency if needed
		if (usepal.transIndex() >= 0)
			FreeImage_SetTransparentIndex(bm, usepal.transIndex());

		// Write image data
		for (int row = 0; row < height; row++) {
			uint8_t* scanline = FreeImage_GetScanLine(bm, row);
			memcpy(scanline, data + (row * width), width);
		}
	}
	else
		return false;

	// Flip the image
	FreeImage_FlipVertical(bm);

	// Write the image to a temp file
	FreeImage_Save(FIF_PNG, bm, chr(appPath(_T("temp.png"), DIR_TEMP)));

	// Load it into a memchunk
	MemChunk png;
	png.importFile(appPath(_T("temp.png"), DIR_TEMP));
	const uint8_t* png_data = png.getData();

	// Write PNG header and IHDR
	out.write(png_data, 33);

	// Create grAb chunk with offsets
	int32_t xoff = offset_x;
	int32_t yoff = offset_y;
	uint32_t csize = wxUINT32_SWAP_ON_LE(8);
	grab_chunk_t gc = { { 'g', 'r', 'A', 'b' }, wxINT32_SWAP_ON_LE(xoff), wxINT32_SWAP_ON_LE(yoff) };
	uint32_t dcrc = wxUINT32_SWAP_ON_LE(Misc::crc((uint8_t*)&gc, 12));

	// Write grAb chunk
	out.write(&csize, 4);
	out.write(&gc, 12);
	out.write(&dcrc, 4);

	// Write remaining PNG data
	out.write(png_data + 33, png.getSize() - 33);

	// Clean up
	wxRemoveFile(appPath(_T("temp.png"), DIR_TEMP));

	// Success
	return true;
}

// Doom Gfx format structs
struct post_t {
	uint8_t			row_off;
	vector<uint8_t>	pixels;
};

struct column_t {
	vector<post_t> posts;
};

/* SImage::toDoomGfx
 * Writes the image as Doom Gfx data to <out>. Returns false if the
 * image is not paletted, true otherwise
 *******************************************************************/
bool SImage::toDoomGfx(MemChunk& out, uint8_t alpha_threshold) {
	// Check if data is paletted
	if (format != PALMASK) {
		wxLogMessage(_T("Cannot convert truecolour image to doom gfx format - convert to 256-colour first."));
		return false;
	}

	// Convert image to column/post structure
	vector<column_t> columns;

	// Go through columns
	uint32_t offset = 0;
	for (int c = 0; c < width; c++) {
		column_t col;
		post_t post;
		post.row_off = 0;
		bool ispost = false;
		bool first_254 = true;	// First 254 pixels should use absolute offsets

		offset = c;
		uint8_t row_off = 0;
		for (int r = 0; r < height; r++) {
			// If we're at offset 254, create a dummy post for tall doom gfx support
			if (row_off == 254) {
				// Finish current post if any
				if (ispost) {
					col.posts.push_back(post);
					post.pixels.clear();
					ispost = false;
				}

				// Begin relative offsets
				first_254 = false;

				// Create dummy post
				post.row_off = 254;
				col.posts.push_back(post);

				// Clear post
				row_off = 0;
				ispost = false;
			}

			// If the current pixel is not transparent, add it to the current post
			if (mask[offset] > alpha_threshold) {
				// If we're not currently building a post, begin one and set it's offset
				if (!ispost) {
					// Set offset
					post.row_off = row_off;

					// Reset offset if we're in relative offsets mode
					if (!first_254)
						row_off = 0;

					// Start post
					ispost = true;
				}

				// Add the pixel to the post
				post.pixels.push_back(data[offset]);
			}
			else if (ispost) {
				// If the current pixel is transparent and we are currently building
				// a post, add the current post to the list and clear it
				col.posts.push_back(post);
				post.pixels.clear();
				ispost = false;
			}

			// Go to next row
			offset += width;
			row_off++;
		}

		// If the column ended with a post, add it
		if (ispost)
			col.posts.push_back(post);

		// Add the column data
		columns.push_back(col);

		// Go to next column
		offset++;
	}

	// Write doom gfx data to output
	out.clear();
	out.seek(0, SEEK_SET);

	// Setup header
	patch_header_t header;
	header.top = offset_y;
	header.left = offset_x;
	header.width = getWidth();
	header.height = getHeight();
	
	// Byteswap header values if needed
	header.top = wxINT16_SWAP_ON_BE(header.top);
	header.left = wxINT16_SWAP_ON_BE(header.left);
	header.width = wxINT16_SWAP_ON_BE(header.width);
	header.height = wxINT16_SWAP_ON_BE(header.height);

	// Write it
	out.write(&header.width, 2);
	out.write(&header.height, 2);
	out.write(&header.left, 2);
	out.write(&header.top, 2);

	// Write dummy column offsets for now
	uint32_t *col_offsets = new uint32_t[columns.size()];
	out.write(col_offsets, columns.size() * 4);

	// Write columns
	for (size_t c = 0; c < columns.size(); c++) {
		// Record column offset
		col_offsets[c] = wxUINT32_SWAP_ON_BE(out.currentPos());

		// Determine column size (in bytes)
		uint32_t col_size = 0;
		for (size_t p = 0; p < columns[c].posts.size(); p++)
			col_size += columns[c].posts[p].pixels.size() + 4;

		// Allocate memory to write the column data
		out.reSize(out.getSize() + col_size, true);

		// Write column posts
		for (size_t p = 0; p < columns[c].posts.size(); p++) {
			// Write row offset
			out.write(&columns[c].posts[p].row_off, 1);

			// Write no. of pixels
			uint8_t npix = columns[c].posts[p].pixels.size();
			out.write(&npix, 1);

			// Write unused byte
			uint8_t temp = 0;
			out.write(&temp, 1);

			// Write pixels
			for (size_t a = 0; a < columns[c].posts[p].pixels.size(); a++)
				out.write(&columns[c].posts[p].pixels[a], 1);

			// Write unused byte
			out.write(&temp, 1);
		}

		// Write '255' row to signal end of column
		uint8_t temp = 255;
		out.write(&temp, 1);
	}

	// Now we write column offsets
	out.seek(8, SEEK_SET);
	out.write(col_offsets, columns.size() * 4);

	return true;
}

/* SImage::toDoomFlat
 * Writes the image as Doom Flat data to <out>. Returns false if the
 * image is not paletted or the image is not a valid size (64x64,
 * 64x128, 128x128, 256x256), true otherwise
 *******************************************************************/
bool SImage::toDoomFlat(MemChunk& out) {
	// Check if data is paletted
	if (format != PALMASK) {
		wxLogMessage(_T("Cannot convert truecolour image to doom flat format - convert to 256-colour first."));
		return false;
	}

	// Check image size
	if (!validFlatSize()) {
		wxLogMessage(_T("Cannot convert to doom flat format, invalid size (must be either 64x64, 64x128, 128x128 or 320x200)"));
		return false;
	}

	// If it's paletted simply write out the data straight
	out.write(data, width * height);

	return true;
}

/* SImage::toPlanar
 * Writes the image as planar data to <out>. Returns false if the
 * image is not a valid candidate (16 colors max, 640x480 dimension),
 * true otherwise
 *******************************************************************/
bool SImage::toPlanar(MemChunk& out, Palette8bit* pal) {
	// Check if data is paletted
	if (format != PALMASK) {
		wxLogMessage(_T("Cannot convert truecolour image to planar format - convert to 16-colour first."));
		return false;
	}

	if (countColours() > 16) {
		wxLogMessage(s_fmt(_T("Cannot convert to planar format, too many colors (%d)"), countColours()));
		return false;
	}

	// Check image size
	if (!(width == 640 && height == 480)) {
		wxLogMessage(_T("Cannot convert to planar format, invalid size (must be 640x480)"));
		return false;
	}

	// Get palette to use
	Palette8bit usepal;
	if (has_palette)
		usepal.copyPalette(&palette);
	else if (pal)
		usepal.copyPalette(pal);

	// Backup current image data (since shrinpPalette remaps the image colours)
	MemChunk backup(width*height);
	backup.write(data, width*height);

	// Make sure all used colors are in the first 16 entries of the palette
	shrinkPalette(&usepal);

	// Create planar palette
	uint8_t * mycolors = new uint8_t[3];
	for (size_t i = 0; i < 16; ++i) {
		mycolors[0] = usepal.colour(i).r>>2;
		mycolors[1] = usepal.colour(i).g>>2;
		mycolors[2] = usepal.colour(i).b>>2;
		out.write(mycolors, 3);
	}
	delete[] mycolors;

	// Create bitplanes
	uint8_t * planes = new uint8_t[153600];

	uint8_t *pln1, *pln2, *pln3, *pln4, *read;
	size_t plane_size = 153600/4;

	read = data;
	pln1 = planes;				// 80: 10000000	08: 00001000
	pln2 = pln1 + plane_size;	// 40: 01000000 04: 00000100
	pln3 = pln2 + plane_size;	// 20: 00100000 02: 00000010
	pln4 = pln3 + plane_size;	// 10: 00010000 01: 00000001

	for (int y = height; y > 0; --y) {
		for (int x = width; x > 0; x -= 8) {
			*pln1 = ((read[0] & 0x01) << 7 | (read[1] & 0x01) << 6 | (read[2] & 0x01) << 5 | (read[3] & 0x01) << 4
					|(read[4] & 0x01) << 3 | (read[5] & 0x01) << 2 | (read[6] & 0x01) << 1 | (read[7] & 0x01)     );
			*pln2 = ((read[0] & 0x02) << 6 | (read[1] & 0x02) << 5 | (read[2] & 0x02) << 4 | (read[3] & 0x02) << 3
					|(read[4] & 0x02) << 2 | (read[5] & 0x02) << 1 | (read[6] & 0x02)      | (read[7] & 0x02) >> 1);
			*pln3 = ((read[0] & 0x04) << 5 | (read[1] & 0x04) << 4 | (read[2] & 0x04) << 3 | (read[3] & 0x04) << 2
					|(read[4] & 0x04) << 1 | (read[5] & 0x04)      | (read[6] & 0x04) >> 1 | (read[7] & 0x04) >> 2);
			*pln4 = ((read[0] & 0x08) << 4 | (read[1] & 0x08) << 3 | (read[2] & 0x08) << 2 | (read[3] & 0x08) << 1
					|(read[4] & 0x08)      | (read[5] & 0x08) >> 1 | (read[6] & 0x08) >> 2 | (read[7] & 0x08) >> 3);
			read += 8;
			pln1 += 1;
			pln2 += 1;
			pln3 += 1;
			pln4 += 1;
		}
	}

	// Write image and cleanup
	out.write(planes, 153600);
	delete[] planes;
	backup.seek(0, SEEK_SET);
	backup.read(data, width*height);

	return true;
}

/* SImage::to4bitChunk
 * Writes the image as 4-bit chunky data to <out>. Returns false if the
 * image is not a valid candidate, true otherwise
 *******************************************************************/
bool SImage::to4bitChunk(MemChunk& out, Palette8bit* pal) {
	// Check if data is paletted
	if (format != PALMASK) {
		wxLogMessage(_T("Cannot convert truecolour image to 4-bit format - convert to 16-colour first."));
		return false;
	}

	if (countColours() > 16) {
		wxLogMessage(s_fmt(_T("Cannot convert to 4-bit format, too many colors (%d)"), countColours()));
		return false;
	}

	// Check image size
	if (!((width == 4 && height == 16) || (width == 16 && height == 23))) {
		wxLogMessage(_T("No point in converting to 4-bit format, image isn't a valid Hexen size (4x16 or 16x23)"));
		return false;
	}

	// Get palette to use
	Palette8bit usepal;
	if (has_palette)
		usepal.copyPalette(&palette);
	else if (pal)
		usepal.copyPalette(pal);

	// Backup current image data (since shrinpPalette remaps the image colours)
	MemChunk backup(width*height);
	backup.write(data, width*height);

	// Make sure all used colors are in the first 16 entries of the palette
	shrinkPalette();

	size_t filesize = width * height / 2;
	uint8_t * temp = new uint8_t[filesize];

	for (int i = 0; i < width * height; i+=2) {
		temp[i/2] = data[i]<<4 | data[i+1];
	}

	// Write image and cleanup
	out.write(temp, filesize);
	delete[] temp;
	backup.seek(0, SEEK_SET);
	backup.read(data, width*height);

	return true;
}

/* SImage::convertRGBA
 * Converts the image to 32bpp (RGBA). Returns false if the image was
 * already 32bpp, true otherwise.
 *******************************************************************/
bool SImage::convertRGBA(Palette8bit* pal) {
	// If it's already 32bpp do nothing
	if (format == RGBA)
		return false;

	// Get 32bit data
	MemChunk rgba_data;
	getRGBAData(rgba_data, pal);

	// Clear current data
	clearData(true);

	// Copy it
	data = new uint8_t[width * height * 4];
	memcpy(data, rgba_data.getData(), width * height * 4);

	// Set new format & update variables
	format = RGBA;
	has_palette = false;

	// Announce change
	announce(_T("image_changed"));

	// Done
	return true;
}

/* SImage::convertPaletted
 * Converts the image to paletted + mask. [pal_target] is the new
 * palette to convert to (the image's palette will also be set to
 * this). [pal_current] will be used as the image's current palette
 * if it doesn't already have one
 *******************************************************************/
bool SImage::convertPaletted(Palette8bit* pal_target, Palette8bit* pal_current) {
	// Check image/parameters are valid
	if (!isValid() || !pal_target)
		return false;

	// Get image data as RGBA
	MemChunk rgba_data;
	getRGBAData(rgba_data, pal_current);

	// Swap red and blue colour information because FreeImage is retarded
	for (int a = 0; a < width * height * 4; a += 4) {
		uint8_t red = rgba_data[a];
		rgba_data[a] = rgba_data[a+2];
		rgba_data[a+2] = red;
	}

	// Build FIBITMAP from it
	FIBITMAP* bm32 = FreeImage_ConvertFromRawBits((uint8_t*)rgba_data.getData(), width, height, width * 4, 32, 0, 0, 0, false);
	FIBITMAP* bm = FreeImage_ConvertTo24Bits(bm32);

	// Create mask from alpha info (if converting from RGBA)
	if (format == RGBA) {
		// Clear current mask
		if (mask)
			delete[] mask;

		// Init mask
		mask = new uint8_t[width * height];

		// Get values from alpha channel
		int c = 0;
		for (int a = 3; a < width * height * 4; a += 4)
			mask[c++] = rgba_data[a];
	}

	// Create FreeImage palette
	RGBQUAD fi_pal[256];
	for (int a = 0; a < 256; a++) {
		fi_pal[a].rgbRed = pal_target->colour(a).r;
		fi_pal[a].rgbGreen = pal_target->colour(a).g;
		fi_pal[a].rgbBlue = pal_target->colour(a).b;
	}

	// Convert image to palette
	FIBITMAP* pbm = NULL;
	pbm = FreeImage_ColorQuantizeEx(bm, FIQ_NNQUANT, 256, 256, fi_pal);

	// Load given palette
	palette.copyPalette(pal_target);

	// Clear current image data (but not mask)
	clearData(false);

	// Load pixel data from the FIBITMAP
	data = new uint8_t[width * height];
	for (int a = 0; a < height; a++)
		memcpy(data + a * width, FreeImage_GetScanLine(pbm, a), width);

	// Update variables
	format = PALMASK;
	has_palette = true;

	// Clean up
	FreeImage_Unload(bm32);
	FreeImage_Unload(bm);
	FreeImage_Unload(pbm);

	// Announce change
	announce(_T("image_changed"));

	// Success
	return true;
}

/* SImage::convertPaletted
 * Converts the image to paletted + mask. Parameters:
 * <pal>: The palette to apply to the image
 * <alpha_threshold>: Specifies what alpha level is the cutoff for
 * the mask (equal or less is completely transparent, greater is
 * completely opaque)
 * <keep_trans>: If true, existing transparency information is kept
 * <col_trans>: If <keep_trans> is false, this colour will be
 * completely transparent, and everything else completely opaque
 * <quant_type>: Type of colour quantization (0 for Xiaolin Wu, 1
 * for NeuQuant)
 *******************************************************************/
/*
bool SImage::convertPaletted(Palette8bit* pal, uint8_t alpha_threshold, bool keep_trans, rgba_t col_trans, int quant_type) {
	// Check image is valid
	if (!isValid())
		return false;

	// Get image data as RGBA
	MemChunk rgba_data;
	getRGBAData(rgba_data);

	// Swap red and blue colour information because FreeImage is retarded
	for (int a = 0; a < width * height * 4; a += 4) {
		uint8_t red = rgba_data[a];
		rgba_data[a] = rgba_data[a+2];
		rgba_data[a+2] = red;
	}

	// Build FIBITMAP from it
	FIBITMAP* bm32 = FreeImage_ConvertFromRawBits((uint8_t*)rgba_data.getData(), width, height, width * 4, 32, 0, 0, 0, false);
	FIBITMAP* bm = FreeImage_ConvertTo24Bits(bm32);

	// Create mask from alpha info (if converting from RGBA)
	if (format == RGBA) {
		// Clear current mask
		if (mask)
			delete[] mask;

		// Init mask
		mask = new uint8_t[width * height];

		// Get values from alpha channel
		int c = 0;
		for (int a = 3; a < width * height * 4; a += 4)
			mask[c++] = rgba_data[a];
	}

	// If we are keeping the existing transparency, just apply the alpha threshold
	if (keep_trans)
		cutoffMask(alpha_threshold, true);
	else {
		// If we aren't keeping the existing transparency, apply the given transparency colour to the mask
		maskFromColour(col_trans, true);
	}

	// Create FreeImage palette
	RGBQUAD fi_pal[256];
	for (int a = 0; a < 256; a++) {
		fi_pal[a].rgbRed = pal->colour(a).r;
		fi_pal[a].rgbGreen = pal->colour(a).g;
		fi_pal[a].rgbBlue = pal->colour(a).b;
	}

	// Convert image to palette
	FIBITMAP* pbm = NULL;
	if (quant_type == 0)
		pbm = FreeImage_ColorQuantizeEx(bm, FIQ_NNQUANT, 256, 256, fi_pal);
	else
		pbm = FreeImage_ColorQuantizeEx(bm, FIQ_WUQUANT, 256, 256, fi_pal);

	// Load given palette
	palette.copyPalette(pal);

	// Clear current image data (but not mask)
	clearData(false);

	// Load pixel data from the FIBITMAP
	data = new uint8_t[width * height];
	for (int a = 0; a < height; a++)
		memcpy(data + a * width, FreeImage_GetScanLine(pbm, a), width);

	// Update variables
	format = PALMASK;
	has_palette = true;

	// Clean up
	FreeImage_Unload(bm32);
	FreeImage_Unload(bm);
	FreeImage_Unload(pbm);

	// Announce change
	announce(_T("image_changed"));

	// Success
	return true;
}

/* SImage::maskFromColour
 * Changes the mask/alpha channel so that pixels that match [colour]
 * are fully transparent, and all other pixels fully opaque. If
 * [force_mask] is true, a mask will be generated even if the image
 * is in RGBA format
 *******************************************************************/
bool SImage::maskFromColour(rgba_t colour, Palette8bit* pal, bool force_mask) {
	if (format == PALMASK && mask != NULL) {
		// Get palette to use
		Palette8bit usepal;
		if (has_palette)
			usepal.copyPalette(&palette);
		else if (pal)
			usepal.copyPalette(pal);

		// Palette+Mask format, go through the mask
		for (int a = 0; a < width * height; a++) {
			if (usepal.colour(data[a]).equals(colour))
				mask[a] = 0;
			else
				mask[a] = 255;
		}
	}
	else if (format == RGBA) {
		// If we're forcing generation of mask data, create it if it doesn't exist
		if (force_mask && !mask)
			mask = new uint8_t[width * height];

		// RGBA format, go through alpha channel
		uint32_t c = 0;
		for (int a = 0; a < width * height; a++) {
			rgba_t pix_col(data[c], data[c + 1], data[c + 2], 255);

			if (pix_col.equals(colour))
				data[c + 3] = 0;
			else
				data[c + 3] = 255;

			// Write the alpha value to the mask also if we're forcing generation of a mask
			if (force_mask)
				mask[a] = data[c + 3];

			// Skip to next pixel
			c += 4;
		}
	}
	else
		return false;

	// Announce change
	announce(_T("image_changed"));

	return true;
}

/* SImage::cutoffMask
 * Changes the mask/alpha channel so that any pixel alpha level
 * currently greater than [threshold] is fully opaque, and all
 * other pixels fully transparent. If [force_mask] is true, a mask
 * will be generated even if the image is in RGBA format
 *******************************************************************/
bool SImage::cutoffMask(uint8_t threshold, bool force_mask) {
	if (format == PALMASK && mask != NULL) {
		for (int a = 0; a < width * height; a++) {
			if (mask[a] > threshold)
				mask[a] = 255;
			else
				mask[a] = 0;
		}
	}
	else if (format == RGBA) {
		// If we're forcing generation of mask data, create it if it doesn't exist
		if (force_mask && !mask)
			mask = new uint8_t[width * height];

		// RGBA format, go through alpha channel
		uint32_t c = 0;
		for (int a = 3; a < width * height * 4; a += 4) {
			if (data[a] > threshold)
				data[a] = 255;
			else
				data[a] = 0;

			// Write the alpha value to the mask also if we're forcing generation of a mask
			if (force_mask)
				mask[c++] = data[a];
		}
	}
	else
		return false;

	// Announce change
	announce(_T("image_changed"));

	return true;
}
