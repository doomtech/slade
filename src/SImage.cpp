
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
		mc.loadMem(data, width * height * 4);
		return true;
	}

	// Otherwise convert
	if (format == PALMASK) {
		uint8_t rgba[4];
		for (uint32_t a = 0; a < width * height; a ++) {
			// Get colour
			rgba_t col = palette.colour(data[a]);

			// Set alpha
			if (mask)
				col.a = mask[a];
			else
				col.a = 255;

			col.write(rgba);
			mc.write(rgba, 4);
		}

		return true;
	}
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
		for (uint32_t a = 0; a < width * height * 4; a += 4)
			mc.write(&data[a], 3);

		return true;
		//return rgb_data;
	}
	if (format == PALMASK) {
		// Paletted, convert to RGB
		uint8_t rgba[4];
		for (uint32_t a = 0; a < width * height; a ++) {
			palette.colour(data[a]).write(rgba);
			mc.write(rgba, 3);
		}

		return true;
	}
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

/* SImage::applyPalette
 * Converts the image to the given palette (using nearest-colour
 * matching)
 *******************************************************************/
void SImage::applyPalette(Palette8bit& pal) {
	// Check image is valid
	if (!isValid())
		return;

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
	FIBITMAP* bm32 = FreeImage_ConvertFromRawBits(rgba_data.getData(), width, height, width * 4, 32, 0, 0, 0, false);
	FIBITMAP* bm = FreeImage_ConvertTo24Bits(bm32);

	// Create FreeImage palette
	RGBQUAD fi_pal[256];
	for (int a = 0; a < 256; a++) {
		fi_pal[a].rgbRed = pal.colour(a).r;
		fi_pal[a].rgbGreen = pal.colour(a).g;
		fi_pal[a].rgbBlue = pal.colour(a).b;
	}

	// Convert image to palette
	FIBITMAP* pbm = FreeImage_ColorQuantizeEx(bm, FIQ_NNQUANT, 256, 256, fi_pal);

	// Load given palette
	palette.copyPalette(pal);

	// Clear current image data (but not mask)
	clearData(false);

	// Load pixel data from the FIBITMAP
	data = new uint8_t[width * height];
	for (int a = 0; a < height; a++)
		memcpy(data + a * width, FreeImage_GetScanLine(pbm, a), width);

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

	// Update variables
	format = PALMASK;

	// Clean up
	FreeImage_Unload(bm32);
	FreeImage_Unload(bm);
	FreeImage_Unload(pbm);
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

	// Reset offsets
	offset_x = 0;
	offset_y = 0;

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
		long xoff = 0;
		long yoff = 0;
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
				int pos = ((row + p)*width + c);
				data[pos] = *bits;
				mask[pos] = 255;
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
	has_palette = true;
	format = PALMASK;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[64*64];
	memcpy(data, gfx_data, 64 * 64);

	return true;
}

/* SImage::toPNG
 * Writes the image as PNG data to <out>, keeping palette information
 * if it exists. Returns true on success
 *******************************************************************/
bool SImage::toPNG(MemChunk& out) {
	return false;
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
		bool ispost = false;

		offset = c * width;
		for (int r = 0; r < height; r++) {
			// If the current pixel is not transparent, add it to the current post
			if (mask[offset] > alpha_threshold) {
				// If we're not currently building a post, begin one and set it's offset
				if (!ispost) {
					post.row_off = r;
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
		}

		// If the column ended with a post, add it
		if (post.pixels.size() > 0)
			col.posts.push_back(post);

		// Add the column data
		columns.push_back(col);

		// Go to next column
		offset++;
	}

	// Write doom gfx data to output
	out.clear();
	out.seek(0, 0);

	// Setup header
	patch_header_t header;
	header.top = offset_y;
	header.left = offset_x;
	header.width = getWidth();
	header.height = getHeight();

	// Write it
	out.write(&header.width, 2);
	out.write(&header.height, 2);
	out.write(&header.left, 2);
	out.write(&header.top, 2);

	// Write dummy column offsets for now
	long *col_offsets = new long[columns.size()];
	out.write(col_offsets, columns.size() * 4);

	// Write posts
	for (int c = 0; c < columns.size(); c++) {
		// Record column offset
		col_offsets[c] = out.currentPos();

		for (int p = 0; p < columns[c].posts.size(); p++) {
			// Write row offset
			out.write(&columns[c].posts[p].row_off, 1);

			// Write no. of pixels
			uint8_t npix = columns[c].posts[p].pixels.size();
			out.write(&npix, 1);

			// Write unused byte
			uint8_t temp = 0;
			out.write(&temp, 1);

			// Write pixels
			for (int a = 0; a < columns[c].posts[p].pixels.size(); a++)
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
	if (!(width == 64 && height == 64 ||
			width == 64 && height == 128 ||
			width == 128 && height == 128 ||
			width == 320 && height == 200)) {
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

	// Set new format
	format = RGBA;

	// Clear current data
	clearData(true);

	// Get 32bit data
	MemChunk rgba_data;
	getRGBAData(rgba_data);

	// Copy it
	data = new uint8_t[width * height * 4];
	memcpy(data, rgba_data.getData(), width * height * 4);

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
 *******************************************************************/
bool SImage::convertPaletted(Palette8bit& pal, uint8_t alpha_threshold, bool keep_trans, rgba_t col_trans) {
	// Apply the palette
	applyPalette(pal);

	if (keep_trans) {
		// If we are keeping the existing transparency info, apply the
		// alpha threshold given (anything above is completely opaque,
		// anything equal or below is completely transparent)
		for (int a = 0; a < width * height; a++) {
			if (mask[a] > alpha_threshold)
				mask[a] = 255;
			else
				mask[a] = 0;
		}
	}
	else {
		// If we aren't keeping the existing transparency info, set all
		// pixels to opaque except for pixels of the given transparent colour
		for (int a = 0; a < width * height; a++) {
			if (pal.colour(data[a]).equals(col_trans))
				mask[a] = 0;
			else
				mask[a] = 255;
		}
	}

	// Success
	return true;
}
