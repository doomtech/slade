
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    SImage.cpp
 * Description: SImage class - Encapsulates a paletted or32bit image,
 *              handles loading/saving different formats,
 *              palette conversions, offsets, and a bunch of other stuff
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
	{ 64, 64 },
	{ 128, 128 },
	{ 256, 256 },
	{ 512, 512 },
	{ 320, 200 },
};
uint32_t	n_valid_flat_sizes = 5;


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
	has_palette = false;
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
bool SImage::getRGBAData(MemChunk& mc) {
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
		uint8_t rgba[4];
		for (int a = 0; a < width * height; a++) {
			// Get colour
			rgba_t col = palette.colour(data[a]);

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
bool SImage::getRGBData(MemChunk& mc) {
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
		uint8_t rgba[4];
		for (int a = 0; a < width * height; a ++) {
			palette.colour(data[a]).write(rgba);
			mc.write(rgba, 3);
		}

		return true;
	}

	return false;	// Invalid image type
}

/* SImage::setPalette
 * Copies the given palette to the image palette
 *******************************************************************/
void SImage::setPalette(Palette8bit* pal) {
	// Check palette
	if (!pal)
		return;

	// Copy the palette
	palette.copyPalette(pal);

	// Announce change
	announce(_T("image_changed"));
}

/* SImage::setPalette
 * Changes the image X offset
 *******************************************************************/
void SImage::setXOffset(int offset) {
	// Change the X offset
	offset_x = offset;

	// Announce change
	announce(_T("offsets_changed"));
}

/* SImage::setPalette
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

	return false;
}

/* SImage::loadImage
 * Loads an image (if it's format is supported by FreeImage)
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
		// Find offsets if present
		int32_t xoff = 0;
		int32_t yoff = 0;
		for (int a = 0; a < size; a++) {
			// Check for 'grAb' header
			if (img_data[a] == 'g' && img_data[a + 1] == 'r' &&
				img_data[a + 2] == 'A' && img_data[a + 3] == 'b') {
				memcpy(&xoff, img_data + a + 4, 4);
				memcpy(&yoff, img_data + a + 8, 4);
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
	if (size < sizeof(patch_header_t))
		return false;

	// Init variables
	uint32_t* col_offsets = NULL;
	has_palette = true;
	format = PALMASK;

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

	/*
	// Get header & offsets
	patch_header_t* header = (patch_header_t*)gfx_data;
	col_offsets = (uint32_t*)((uint8_t*)gfx_data + sizeof(patch_header_t));

	// Setup variables
	width = wxINT16_SWAP_ON_BE(header->width);
	height = wxINT16_SWAP_ON_BE(header->height);
	offset_x = wxINT16_SWAP_ON_BE(header->left);
	offset_y = wxINT16_SWAP_ON_BE(header->top);
	has_palette = true;
	format = PALMASK;
	 */

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
		if (col_offset >= size) {
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
 * Returns false if the image data was invalid, true otherwise
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

		if (size == w * h) {
			width = w;
			height = h;
			valid_size = true;
			break;
		}
	}

	// Check valid size
	if (!valid_size)
		return false;

	// Setup variables
	has_palette = true;
	format = PALMASK;

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

/* SImage::loadDoomGfxOld
 * Loads an Alpha/Beta Doom Gfx format image, using the given palette.
 * Returns false if the image data was invalid, true otherwise
 *******************************************************************/
 /*
bool SImage::loadDoomGfxOld(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check size
	if (size < 4)
		return false;

	// Get header & offsets
	patch_header_t* header = (patch_header_t*)gfx_data;
	uint16_t* col_offsets = (uint16_t*)((uint8_t*)gfx_data + 8);

	// Setup variables
	width = header->width;
	height = header->height;
	offset_x = header->left;
	offset_y = header->top;
	has_palette = true;
	format = PALMASK;

	// Clear current data if it exists
	clearData();

	// Load data
	data = new uint8_t[width * height];
	memset(data, 0, width * height);	// Set colour to palette index 0
	mask = new uint8_t[width * height];
	memset(mask, 0, width * height);	// Set mask to fully transparent
	for (int c = 0; c < width; c++) {
		// Get current column offset (byteswap if needed)
		uint32_t col_offset = wxUINT16_SWAP_ON_BE(col_offsets[c]);

		// Check column offset is valid
		if (col_offset >= size) {
			clearData();
			return false;
		}

		// Go to start of column
		const uint8_t* bits = gfx_data;
		bits += col_offset;

		// Read posts
		while (1) {
			// Get row offset
			uint8_t row = *bits;

			if (row == 0xFF) // End of column?
				break;

			// Get no. of pixels
			bits++;
			uint8_t n_pix = *bits;

			//bits++; // Skip buffer
			for (uint8_t p = 0; p < n_pix; p++) {
				// Get pixel position
				bits++;
				int pos = ((row + p)*width + c);

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
			bits++;
			//bits += 2; // Skip buffer & go to next row offset
		}
	}

	// Announce change
	announce(_T("image_changed"));

	// Return success
	return true;
}
 */

/* SImage::toPNG
 * Writes the image as PNG data to <out>, keeping palette information
 * if it exists. Returns true on success
 *******************************************************************/
bool SImage::toPNG(MemChunk& out) {
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

		// Set palette
		RGBQUAD* bm_pal = FreeImage_GetPalette(bm);
		for (int a = 0; a < 256; a++) {
			bm_pal[a].rgbRed = palette.colour(a).r;
			bm_pal[a].rgbGreen = palette.colour(a).g;
			bm_pal[a].rgbBlue = palette.colour(a).b;
		}

		// Find unused colour (for transparency)
		short unused = findUnusedColour();

		// Set any transparent pixels to this colour (if we found an unused colour
		if (unused >= 0) {
			for (int a = 0; a < width * height; a++) {
				if (mask[a] == 0)
					data[a] = unused;
			}

			// Set palette transparency
			palette.setTransIndex(unused);
		}

		// Set freeimage palette transparency if needed
		if (palette.transIndex() >= 0)
			FreeImage_SetTransparentIndex(bm, palette.transIndex());

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
	uint32_t dcrc = wxUINT32_SWAP_ON_LE(crc((uint8_t*)&gc, 12));

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
	if (!((width == 64 && height == 64) ||
			(width == 64 && height == 128) ||
			(width == 128 && height == 128) ||
			(width == 320 && height == 200))) {
		wxLogMessage(_T("Cannot convert to doom flat format, invalid size (must be either 64x64, 64x128, 128x128 or 320x200)"));
		return false;
	}

	// If it's paletted simply write out the data straight
	out.write(data, width * height);

	return true;
}

/* SImage::convertRGBA
 * Converts the image to 32bpp (RGBA). Returns false if the image was
 * already 32bpp, true otherwise.
 *******************************************************************/
bool SImage::convertRGBA() {
	// If it's already 32bpp do nothing
	if (format == RGBA)
		return false;

	// Get 32bit data
	MemChunk rgba_data;
	getRGBAData(rgba_data);

	// Clear current data
	clearData(true);

	// Copy it
	data = new uint8_t[width * height * 4];
	memcpy(data, rgba_data.getData(), width * height * 4);

	// Set new format
	format = RGBA;

	// Announce change
	announce(_T("image_changed"));

	// Done
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
bool SImage::maskFromColour(rgba_t colour, bool force_mask) {
	if (format == PALMASK) {
		// Palette+Mask format, go through the mask
		for (int a = 0; a < width * height; a++) {
			if (palette.colour(data[a]).equals(colour))
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
	if (format == PALMASK) {
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




// CRC stuff for PNG

/* Table of CRCs of all 8-bit messages. */
uint32_t crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void) {
	uint32_t c;
	int n, k;

	for (n = 0; n < 256; n++) {
		c = (uint32_t) n;

		for (k = 0; k < 8; k++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}

		crc_table[n] = c;
	}

	crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below)). */
uint32_t update_crc(uint32_t crc, uint8_t *buf, uint32_t len) {
	uint32_t c = crc;

	if (!crc_table_computed)
		make_crc_table();

	for (uint32_t n = 0; n < len; n++)
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);

	return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
uint32_t crc(uint8_t *buf, uint32_t len) {
	return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}
