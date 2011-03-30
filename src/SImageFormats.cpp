
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    SImageFormats.cpp
 * Description: SImage class - Encapsulates a paletted or 32bit image.
 *              Handles loading/saving different formats, palette
 *              conversions, offsets, and a bunch of other stuff
 *
 *				This file contains the load/save functions.
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

/* SETUP:
 * 1. STANDARD FORMATS (PNG, JPG, etc.)
 * 2. DOOM FORMATS (Gfx, flat.)
 * 3. OBSOLETE DOOM FORMATS (Alpha formats)
 * 4. HEXEN FORMATS (Planar-related)
 * 5. SOURCE PORT FORMATS (Legacy, ZDoom)
 * 6. OTHER GAMES FORMATS (Shadowcaster, Amulets & Armor, etc.)
 * 7. FONT FORMATS (including standard, Alpha and ZDoom fonts)
 *******************************************************************/


/*******************************************************************
 * STANDARD FORMATS
 *******************************************************************/
// Define grAb chunk struct
struct grab_chunk_t {
	char name[4];
	int32_t xoff;
	int32_t yoff;
};


/* SImage::loadImage
 * Loads an image (if its format is supported by FreeImage)
 * Returns false if the given image data was invalid, true otherwise
 *******************************************************************/
bool SImage::loadImage(const uint8_t* img_data, int size) {
	// Check data
	if (!img_data) {
		Global::error = "Invalid data given";
		return false;
	}

	// Check size
	if (size < 8) {
		Global::error = "Invalid data size";
		return false;
	}

	// Reset offsets
	offset_x = 0;
	offset_y = 0;

	// Reset index
	numimages = 1;
	imgindex = 0;

	// Create FreeImage bitmap from entry data
	FIMEMORY* mem = FreeImage_OpenMemory((BYTE*)img_data, size);
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(mem, 0);
	FIBITMAP *bm = FreeImage_LoadFromMemory(fif, mem, 0);
	FreeImage_CloseMemory(mem);

	// Check it created/read ok
	if (!bm) {
		Global::error = "Unable to read image data (unsupported format?)";
		return false;
	}

	// Get image palette if it exists
	RGBQUAD* bm_pal = FreeImage_GetPalette(bm);
	if (bm_pal) {
		has_palette = true;
		int a = 0;
		int b = FreeImage_GetColorsUsed(bm);
		if (b > 256)
			b = 256;
		for (; a < b; a++)
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

	// Announce change and return success
	announce("image_changed");
	return true;
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
			bm_pal[a].rgbRed = usepal.colour(a).r;
			bm_pal[a].rgbGreen = usepal.colour(a).g;
			bm_pal[a].rgbBlue = usepal.colour(a).b;
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
	FreeImage_Save(FIF_PNG, bm, CHR(appPath("temp.png", DIR_TEMP)));

	// Load it into a memchunk
	MemChunk png;
	png.importFile(appPath("temp.png", DIR_TEMP));

	// Check it loaded ok
	if (png.getSize() == 0) {
		wxLogMessage("Error reading temporary file");
		return false;
	}

	// Write PNG header and IHDR
	const uint8_t* png_data = png.getData();
	out.clear();
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
	wxRemoveFile(appPath("temp.png", DIR_TEMP));

	// Success
	return true;
}

/*******************************************************************
 * DOOM FORMATS
 *******************************************************************/

// Doom Gfx format structs
struct post_t {
	uint8_t			row_off;
	vector<uint8_t>	pixels;
};

struct column_t {
	vector<post_t> posts;
};

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
	numimages = 1;
	imgindex = 0;


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

				// Fail if bogus data gives a negative pos (this corrupts the heap!)
				if (pos < 0) {
					clearData();
					return false;
				}

				// Write pixel data
				data[pos] = *bits;
				mask[pos] = 255;
			}
			if (version == 0) bits++; // Skip buffer
			bits++; // Go to next row offset
		}
	}

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::toDoomGfx
 * Writes the image as Doom Gfx data to <out>. Returns false if the
 * image is not paletted, true otherwise
 *******************************************************************/
bool SImage::toDoomGfx(MemChunk& out, uint8_t alpha_threshold) {
	// Check if data is paletted
	if (format != PALMASK) {
		wxLogMessage("Cannot convert truecolour image to doom gfx format - convert to 256-colour first.");
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
				// If we're not currently building a post, begin one and set its offset
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

// Define valid flat sizes
uint32_t valid_flat_size[][2] = {
	{	2,	 2 },	// lol Heretic F_SKY1
	{  10,  12 },	// gnum format
	{  16,  16 },	// \ 
	{  32,  64 },	// Strife startup sprite
	{  48,  48 },	// /
	{  64,  64 },	// standard flat size
	{  64,	65 },	// Heretic flat size variant
	{  64, 128 },	// Hexen flat size variant
	{ 128, 128 },	// \ 
	{ 256, 256 },	// hires flat size
	{ 512, 512 },	// /
	{ 256, 200 },	// Rise of the Triad sky
	{ 320, 200 },	// full screen format
};
uint32_t	n_valid_flat_sizes = 14;

/* SImage::validFlatSize
 * Returns whether the image has dimensions appropriate for a flat
 * or another raw-type image (including things such as AUTOPAGE).
 *******************************************************************/
bool SImage::validFlatSize() {
	for (uint32_t a = 0; a < n_valid_flat_sizes; a++) {
		if (width == valid_flat_size[a][0] &&
			height == valid_flat_size[a][1])
			return true;
	}
	return (width == 320 || width == 256);
} // FIXME: width == 256 is a temporary hack to handle translation tables.

/* SImage::loadDoomFlat
 * Loads a Doom Flat format image, using the given palette.
 * Also loads about any raw image format that's the same as flats,
 * but with a different dimension.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadDoomFlat(const uint8_t* gfx_data, int size, bool columnmajor) {
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

	// Swap height and width for column-major formats
	if (columnmajor) {
		int tmp = width;
		width = height;
		height = tmp;
	}


	// Setup variables
	format = PALMASK;
	has_palette = false;
	numimages = 1;
	imgindex = 0;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];
	memcpy(data, gfx_data, width * height);

	// Create mask (all opaque)
	mask = new uint8_t[width*height];
	memset(mask, 255, width*height);

	// Handle column-major flat formats
	if (columnmajor) {
		rotate(90);
		mirror(true);
	}

	// Announce change and return success
	announce("image_changed");
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
		wxLogMessage("Cannot convert truecolour image to doom flat format - convert to 256-colour first.");
		return false;
	}

	// Check image size
	if (!validFlatSize()) {
		wxLogMessage("Cannot convert to doom flat format, invalid size (must be either 64x64, 64x128, 128x128 or 320x200)");
		return false;
	}

	// If it's paletted simply write out the data straight
	out.write(data, width * height);

	return true;
}

/*******************************************************************
 * OBSOLETE DOOM FORMATS
 *******************************************************************/

/* SImage::loadDoomArah
 * Loads a Doom Alpha Raw And Header image.
 * Returns false if the image data was invalid, true otherwise.
 * It seems index 255 is treated as transparent in this format.
 *******************************************************************/
bool SImage::loadDoomArah(const uint8_t* gfx_data, int size, int transindex) {
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
	numimages = 1;
	imgindex = 0;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];
	memcpy(data, gfx_data+8, width * height);

	// Create mask (all opaque)
	mask = new uint8_t[width*height];
	memset(mask, 255, width*height);

	// Mark as transparent all pixels that are set to 'transindex'
	for (size_t  i = 0; i < (unsigned)(width*height); ++i)
		if (data[i] == transindex) mask[i] = 0;

	// Announce change and return success
	announce("image_changed");
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
	if (size == width * height + 4)
		size -= 2;

	if (size != 2 + width * height)
		return false;

	// Setup variables
	format = PALMASK;
	has_palette = false;
	numimages = 1;
	imgindex = 0;

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

	// Announce change and return success
	announce("image_changed");
	return true;
}


/*******************************************************************
 * HEXEN FORMATS
 *******************************************************************/

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
	numimages = 1;
	imgindex = 0;

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

	// Announce change and return success
	announce("image_changed");
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
		wxLogMessage("Cannot convert truecolour image to planar format - convert to 16-colour first.");
		return false;
	}

	if (countColours() > 16) {
		wxLogMessage(S_FMT("Cannot convert to planar format, too many colors (%d)"), countColours());
		return false;
	}

	// Check image size
	if (!(width == 640 && height == 480)) {
		wxLogMessage("Cannot convert to planar format, invalid size (must be 640x480)");
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
	numimages = 1;
	imgindex = 0;
	clearData();
	data = new uint8_t[width*height];
	mask = new uint8_t[width*height];
	memset(mask, 0xFF, width*height);

	for (int i = 0; i < size; ++i) {
		data[i*2  ] = ((gfx_data[i] & 0xF0)>>4);
		data[i*2+1] =  (gfx_data[i] & 0x0F);
	}
	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::to4bitChunk
 * Writes the image as 4-bit chunky data to <out>. Returns false if the
 * image is not a valid candidate, true otherwise
 *******************************************************************/
bool SImage::to4bitChunk(MemChunk& out, Palette8bit* pal) {
	// Check if data is paletted
	if (format != PALMASK) {
		wxLogMessage("Cannot convert truecolour image to 4-bit format - convert to 16-colour first.");
		return false;
	}

	if (countColours() > 16) {
		wxLogMessage(S_FMT("Cannot convert to 4-bit format, too many colors (%d)"), countColours());
		return false;
	}

	// Check image size
	if (!((width == 4 && height == 16) || (width == 16 && height == 23))) {
		wxLogMessage("No point in converting to 4-bit format, image isn't a valid Hexen size (4x16 or 16x23)");
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

/*******************************************************************
 * SOURCE PORT FORMATS
 *******************************************************************/

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
	numimages = 1;
	imgindex = 0;

	// Create data (all white) and mask
	clearData();
	data = new uint8_t[width*height];
	mask = new uint8_t[width*height];
	memset(data, 0xFF, width*height);

	if (!header->compression) {
		memcpy(mask, gfx_data + sizeof(imgz_header_t), size - sizeof(imgz_header_t));

		// Announce change and return success
		announce("image_changed");
		return true;
	} else {
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

	// Announce change and return success
	announce("image_changed");
	return true;
};

/*******************************************************************
 * OTHER GAMES FORMATS
 *******************************************************************/

/* SImage::loadQuake
 * Loads a graphic in the Quake HUD picture format.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
enum QuakePicType
{
	QUAKE_PALETTE	= 0,
	QUAKE_INTENSITY	= 1,
	QUAKE_ALPHA		= 2,
	QUAKE_RGB24		= 3,
	QUAKE_RGB32		= 4
};
bool SImage::loadQuake(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Setup variables
	width = wxINT16_SWAP_ON_BE(*(const uint16_t*)(gfx_data));
	height = wxINT16_SWAP_ON_BE(*(const uint16_t*)(gfx_data+4));
	uint8_t mode = gfx_data[3];
	offset_x = 0;
	offset_y = 0;
	numimages = 1;
	imgindex = 0;
	if (mode < QUAKE_RGB24)
		format = PALMASK;
	else format = RGBA;
	if (mode == 0 || mode > QUAKE_ALPHA)
		has_palette = false;
	else {
		// Picture is an intensity map, so use greyscale palette
		has_palette = true;
		for (size_t i = 0; i < 256; ++i) {
			palette.setColour(i, rgba_t(i, i, i, 255));
		}
	}

	// Clear current data if it exists
	clearData();

	if (mode == QUAKE_PALETTE) {
		// Read raw pixel data
		data = new uint8_t[width*height];
		memcpy(data, gfx_data+8, width * height);

		// Create mask (all opaque)
		mask = new uint8_t[width*height];
		memset(mask, 255, width*height);

		// Index 255 is transparent
		for (int i = 0; i < width*height; ++i)
			if (data[i] == 255)
				mask[i] = 0;
	} else if (mode == QUAKE_INTENSITY) {
		// Read raw pixel data
		data = mask = new uint8_t[width*height];
		memcpy(data, gfx_data+8, width * height);
	} else if (mode == QUAKE_ALPHA) {
		// Read raw pixel data
		data = new uint8_t[width*height];
		mask = new uint8_t[width*height];
		for (int i = 0; i < size - 8; i+=2) {
			data[i/2] = gfx_data[i+8];
			mask[i/2] = gfx_data[i+9];
		}
	} else if (mode == QUAKE_RGB24) {
		// Read raw pixel data
		data = new uint8_t[width*height*4];
		for (int i = 0; i < size - 8; i+=3) {
			data[(i/3)*4  ] = gfx_data[i+ 8];
			data[(i/3)*4+1] = gfx_data[i+ 9];
			data[(i/3)*4+2] = gfx_data[i+10];
			data[(i/3)*4+3] = 255;
		}
	} else if (mode == QUAKE_RGB32) {
		// Read raw pixel data
		data = new uint8_t[width*height*4];
		memcpy(data, gfx_data+8, width * height * 4);
	}

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadQuakeSprite
 * Loads a picture in Quake's sprite format.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadQuakeSprite(const uint8_t* gfx_data, int size, int index) {
	if (size < 64)
		return false;

	uint32_t maxheight = READ_L32(gfx_data, 16);
	uint32_t maxwidth  = READ_L32(gfx_data, 20);
	uint32_t nframes   = READ_L32(gfx_data, 24);

	// Setup variables
	numimages   = nframes;
	has_palette = false;
	format		= PALMASK;

	// Makes sum of frame pictures, not just frames
	// Also gather offsets in a data structure
	vector<size_t> pics;
	uint32_t imgofs = 36;
	for (size_t a = 0; a < nframes; ++a) {
		if (READ_L32(gfx_data, imgofs) != 0) {
			// We have a frame with a group of picture
			uint32_t grpsz = READ_L32(gfx_data, imgofs + 4);
			if (grpsz == 0) {
				Global::error = "Quake sprite data contains empty group";
				return false;
			}
			// Move to end of group header
			imgofs += (grpsz+2)<<2;
			for (size_t b = 0; b < grpsz; ++b) {
				uint32_t pw = READ_L32(gfx_data, imgofs + 8);
				uint32_t ph = READ_L32(gfx_data, imgofs + 12);
				// Store image offset
				pics.push_back(imgofs);
				// Move to end of picture data
				imgofs += 16 + pw * ph;
				if (imgofs > (unsigned) size) {
					Global::error = "Quake sprite data too short";
					return false;
				}
			}
			// We must have made it through all the group's picture,
			// so replace the count for the group (1) by the group's size.
			numimages += (grpsz - 1);
		} else {
			// We have a frame with a single picture
			imgofs += 4;
			uint32_t pw = READ_L32(gfx_data, imgofs + 8);
			uint32_t ph = READ_L32(gfx_data, imgofs + 12);
			// Store image offset
			pics.push_back(imgofs);
			// Move to end of picture data
			imgofs += 16 + pw * ph;
		}
		if (imgofs > (unsigned) size) {
			Global::error = "Quake sprite data too short";
			return false;
		}
	}

	// Sanitize index if needed
	index %= numimages;
	if (index < 0)
		index = numimages + index;
	imgindex = index;

	// Setup variables using appropriate image data
	imgofs = pics[index];
	offset_x = READ_L32(gfx_data, imgofs + 0);
	offset_y = READ_L32(gfx_data, imgofs + 4);
	width	 = READ_L32(gfx_data, imgofs + 8);
	height	 = READ_L32(gfx_data, imgofs +12);
	// Horizontal offsets seem computed differently from Doom, so translate them
	offset_x += width;

	// Clear current data if it exists
	clearData();
	pics.clear();

	// Load image data
	data = new uint8_t[width * height];
	mask = new uint8_t[width * height];
	memcpy(data, gfx_data + imgofs + 16, width * height);
	memset(mask, 0xFF, width * height);
	for (int a = 0; a < width * height; ++a)
		if (data[a] == 0xFF)
			mask[a] = 0;

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadQuakeTex
 * Loads a picture in Quake's texture format.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadQuakeTex(const uint8_t* gfx_data, int size, int index) {
	if (size < 125)
		return false;

	// Setup variables
	numimages   = 4;
	has_palette = false;
	format		= PALMASK;
	width		= READ_L32(gfx_data, 16);
	height		= READ_L32(gfx_data, 20);

	// Sanitize index if needed
	index %= numimages;
	if (index < 0)
		index = numimages + index;
	imgindex = index;

	// Adjust variables according to mip level
	if (index) {
		width  >>= index;
		height >>= index;
	}

	// Find offset
	uint32_t imgofs = READ_L32(gfx_data, 24+(index<<2));

	// Clear current data if it exists
	clearData();

	// Load image data
	data = new uint8_t[width * height];
	memcpy(data, gfx_data + imgofs, width * height);

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadSCSprite
 * Loads a sprite in ShadowCaster's format.
 * Returns false if the image data was invalid, true otherwise.
*******************************************************************/
bool SImage::loadSCSprite(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;
	if (size < 3)
		return false;

	width = READ_L16(gfx_data, 2);
	if (width == 0)
		return false;

	height = 0;
	for (int j = 0; j < width; ++j)
	{
		int colstart = READ_L16(gfx_data, ((j<<1)+4));
		// Columns with a null offset are skipped
		if (colstart == 0) continue;
		if (colstart < 0 || size < colstart+2 || colstart < (width*2+4))
		{
			return false;
		}
		int start		= gfx_data[colstart];
		int stop		= gfx_data[colstart+1];
		int colheight	= start - stop;
		if (colheight < 0 || size < colstart+colheight+1)
		{
			return false;
		}
		if (start > height)
			height = start;
	}

	if (height == 0)
		return false;

	// Clear current data if it exists
	clearData();

	// Setup variables
	format = PALMASK;
	has_palette = false;
	numimages = 1;
	imgindex = 0;

	// Format has no offsets, so just set them automatically
	offset_x = width/2;
	offset_y = height;

	// Create raw pixel data
	int bound = width*height;
	data = new uint8_t[bound];
	memset(data, 0, bound);
	mask = new uint8_t[bound];
	memset(mask, 0, bound);

	// Read pixel data
	for (int h = 0, i = 4; h < width; ++h, i+=2)
	{
		int colstart = READ_L16(gfx_data, i);
		if (colstart)
		{
			int start		= gfx_data[colstart];
			int stop		= gfx_data[colstart+1];
			int colheight	= start - stop;
			int startheight = height - start;
			for (int z = 0; z < colheight; ++z)
			{
				int mypixel = ((z+startheight)*width)+h;
				if (mypixel >= bound || (colstart+2+z) >= size)
					return false;
				if (mypixel < 0)
					return false;
				data[mypixel] = gfx_data[colstart+2+z];
				if (data[mypixel])
					mask[mypixel] = 0xFF;
			}
		}
	}

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadSCWall
 * Loads a wall in ShadowCaster's format.
 * Returns false if the image data was invalid, true otherwise.
*******************************************************************/
#define SCWALLOFFSET 130 // Headers contain 129 bytes of junk.
bool SImage::loadSCWall(const uint8_t* gfx_data, int size) {
	if (!size)
		return false;

	height = gfx_data[0]*4;
	width = 64;
	if (size != width * height + SCWALLOFFSET)
		return false;

	// Clear current data if it exists
	clearData();
	data = new uint8_t[width*height];
	memset(data, 0xFF, width*height);
	mask = new uint8_t[width*height];
	memset(mask, 0xFF, width*height);

	// Setup variables
	format = PALMASK;
	has_palette = false;
	numimages = 1;
	imgindex = 0;

	// Read pixel data
	int pixelreader = SCWALLOFFSET;
	int brush = 0;
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y, ++pixelreader) {
			brush = (y*64)+x;
			data[brush] = gfx_data[pixelreader];
			if (data[brush] == 0)
				mask[brush] = 0;
		}
	}

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadAnaMip
 * Loads a picture in Amulets & Armor's format.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadAnaMip(const uint8_t* gfx_data, int size) {
	if (size < 4)
		return false;

	// Setup variables
	width = READ_L16(gfx_data, 0);
	height = READ_L16(gfx_data, 2);
	has_palette = false;
	format = PALMASK;

	// Technically false, as there are multiple mipmap levels.
	// May implement them later. Not in any hurry about them.
	numimages = 1;
	imgindex = 0;

	if (size < (4 + (width * height)))
		return false;

	// Create data (all white) and mask
	clearData();
	data = new uint8_t[width*height];
	mask = new uint8_t[width*height];
	memset(mask, 0xFF, width*height);

	memcpy(data, gfx_data + 4, width*height);

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadBuildTile
 * Loads a picture from an ART tile collection.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadBuildTile(const uint8_t* gfx_data, int size, int index) {
	if (size < 16)
		return false;

	uint32_t firsttile = wxUINT32_SWAP_ON_BE(((uint32_t*)gfx_data)[2]);
	uint32_t lasttile  = wxUINT32_SWAP_ON_BE(((uint32_t*)gfx_data)[3]);

	// Setup variables
	numimages = 1 + lasttile - firsttile;
	has_palette = false;
	format = PALMASK;

	// Sanitize index if needed
	index %= numimages;
	if (index < 0)
		index = numimages + index;

	// Each tile has a 2-byte width, a 2-byte height, and a 4-byte
	// picanm struct. The header itself is 16 bytes.
	size_t x_offs = 16;
	size_t y_offs = x_offs + (numimages<<1);
	size_t o_offs = y_offs + (numimages<<1);

	// Compute the address where our tile's graphic data starts
	size_t datastart = (numimages * 8) + 16;
	if (index > 0)
	{
		// We can skip these steps if looking at the first tile in the ART file.
		for (int i = 0; i < index; ++i) {
			width = READ_L16(gfx_data, (x_offs+(i<<1)));
			height = READ_L16(gfx_data, (y_offs+(i<<1)));
			datastart += (width * height);
		}

		// Increment values to that of the tile we want
		x_offs += (index<<1);
		y_offs += (index<<1);
		o_offs += (index<<2);
	}
	if ((unsigned)size < datastart)
		return false;

	// Setup variables

	// Little cheat here as they are defined in column-major format,
	// not row-major. So we'll just call the rotate function.
	height = READ_L16(gfx_data, x_offs);
	width = READ_L16(gfx_data, y_offs);
	if ((unsigned)size < (datastart+(width*height)))
		return false;

	// Offsets are signed bytes, so they need a cast
	offset_x = (int8_t)gfx_data[o_offs+1];
	offset_y = (int8_t)gfx_data[o_offs+2];

	imgindex = index;

	// Create data
	clearData();
	data = new uint8_t[width*height];

	// Fill data with pixel data
	memcpy(data, gfx_data + datastart, width * height);

	// Convert from column-major to row-major
	rotate(90);
	mirror(true);

	// Create mask
	mask = new uint8_t[width*height];
	for (int i = 0; i < width*height; ++i) {
		if (data[i] == 0xFF)	mask[i] = 0;
		else					mask[i] = 0xFF;
	}

	// Offsets are not computed from the same reference point, so convert them
	offset_x += (width>>1);
	offset_y += height;

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadHeretic2M8
 * Loads a Heretic II mipmapped paletted image.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadHeretic2M8(const uint8_t* gfx_data, int size, int index) {
	if (size < 1040)
		return false;

	const uint32_t * g_data = (uint32_t *) gfx_data;
	int i = 9;
	while (i < 25 && g_data[i] != 0) ++i;
	numimages = i - 9;
	has_palette = true;
	format = PALMASK;

	// Sanitize index if needed
	index %= numimages;
	if (index < 0)
		index = numimages + index;


	// Setup variables
	imgindex = index;
	width = wxUINT32_SWAP_ON_BE(g_data[index+9]);
	height = wxUINT32_SWAP_ON_BE(g_data[index+25]);
	size_t data_offset = wxUINT32_SWAP_ON_BE(g_data[index+41]);
	if (!width || !height || ! data_offset || size < (int)data_offset + (width*height)) {
		Global::error = "M8 file: invalid data for mip level";
		return false;
	}

	// Let's build the palette now.
	for (size_t c = 0; c < 256; ++c) {
		rgba_t color;
		color.r = gfx_data[(c*3)+0x104];
		color.g = gfx_data[(c*3)+0x105];
		color.b = gfx_data[(c*3)+0x106];
		palette.setColour(c, color);
	}

	// Create data
	clearData();
	data = new uint8_t[width*height];

	// Fill data with pixel data
	memcpy(data, gfx_data + data_offset, width * height);

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadHeretic2M32
 * Loads a Heretic II mipmapped truecolor image.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadHeretic2M32(const uint8_t* gfx_data, int size, int index) {
	if (size < 968)
		return false;

	const uint32_t * g_data = (uint32_t *) gfx_data;
	int i = 129;
	while (i < 145 && g_data[i] != 0) ++i;
	numimages = i - 129;
	has_palette = false;
	format = RGBA;

	// Sanitize index if needed
	index %= numimages;
	if (index < 0)
		index = numimages + index;


	// Setup variables
	imgindex = index;
	width = wxUINT32_SWAP_ON_BE(g_data[index+129]);
	height = wxUINT32_SWAP_ON_BE(g_data[index+145]);
	size_t data_offset = wxUINT32_SWAP_ON_BE(g_data[index+161]);
	if (!width || !height || ! data_offset || size < (int)data_offset + (4*width*height)) {
		Global::error = "M32 file: invalid data for mip level";
		return false;
	}

	// Create data
	clearData();
	data = new uint8_t[width*height*4];

	// Fill data with pixel data
	memcpy(data, gfx_data + data_offset, width * height * 4);

	// Announce change and return success
	announce("image_changed");
	return true;
}


/* SImage::loadQuakeIIWal
 * Loads a Quake II wal-format texture.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadQuakeIIWal(const uint8_t* gfx_data, int size, int index) {
	if (size < 101)
		return false;

	numimages = 4;
	has_palette = false;
	format = PALMASK;

	// Sanitize index if needed
	index %= numimages;
	if (index < 0)
		index = numimages + index;


	// Setup variables
	imgindex = index;
	width = READ_L32(gfx_data, 32) >> index;
	height = READ_L32(gfx_data, 36) >> index;
	size_t data_offset = READ_L32(gfx_data, 40+(index<<2));
	if (!width || !height || ! data_offset || size < (int)data_offset + (width*height)) {
		Global::error = "WAL file: invalid data for mip level";
		return false;
	}

	// Create data
	clearData();
	data = new uint8_t[width*height];

	// Fill data with pixel data
	memcpy(data, gfx_data + data_offset, width * height);

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadHalfLifeTex
 * Loads a Half-Life mipmapped paletted image.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadHalfLifeTex(const uint8_t* gfx_data, int size, int index) {
	if (size < 812)
		return false;

	numimages = 4;
	has_palette = true;
	format = PALMASK;

	// Sanitize index if needed
	index %= numimages;
	if (index < 0)
		index = numimages + index;


	// Setup variables
	imgindex = index;
	width = READ_L32(gfx_data, 16) >> index;
	height = READ_L32(gfx_data, 20) >> index;
	size_t data_offset = READ_L32(gfx_data, 24+(index<<2));
	if (!width || !height || ! data_offset || size < (int)data_offset + (width*height)) {
		Global::error = "HLT file: invalid data for mip level";
		return false;
	}

	// Let's build the palette now.
	size_t pal_offset = READ_L32(gfx_data, 36) + ((READ_L32(gfx_data, 16) >> 3) * (READ_L32(gfx_data, 20) >> 3));
	if (size < (int)pal_offset + 5) {
		Global::error = "HLT file: invalid palette offset";
		return false;
	}
	size_t palsize = READ_L16(gfx_data, pal_offset);
	if (palsize == 0 || palsize > 256 || size < (int)(pal_offset + 2 + (palsize*3))) {
		wxLogMessage("palsize %d, paloffset %d, entry size %d", palsize, pal_offset, size);
		Global::error = "HLT file: invalid palette size";
		return false;
	}
	for (size_t c = 0; c < palsize; ++c) {
		rgba_t color;
		color.r = gfx_data[(c*3)+pal_offset+2];
		color.g = gfx_data[(c*3)+pal_offset+3];
		color.b = gfx_data[(c*3)+pal_offset+4];
		palette.setColour(c, color);
	}

	// Create data
	clearData();
	data = new uint8_t[width*height];

	// Fill data with pixel data
	memcpy(data, gfx_data + data_offset, width * height);

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadRottGfx
 * Loads a Rise of the Triad Gfx format image, using the image palette.
 * This format, descibed in hacker.txt from the ROTT source code pack,
 * is "extremely similar to the patches used in another game".
 * Returns false if the image data was invalid, true otherwise
 *******************************************************************/
bool SImage::loadRottGfx(const uint8_t* gfx_data, int size, bool transparent) {
	// Check data
	if (!gfx_data)
		return false;

	// Check size
	if ((unsigned)size < sizeof(rottpatch_header_t))
		return false;

	// Init variables
	uint16_t* col_offsets = NULL;
	format = PALMASK;
	has_palette = false;
	numimages = 1;
	imgindex = 0;


	// Read header
	rottpatch_header_t* header = (rottpatch_header_t*)gfx_data;
	width = wxINT16_SWAP_ON_BE(header->width);
	height = wxINT16_SWAP_ON_BE(header->height);
	offset_x = wxINT16_SWAP_ON_BE(header->left);
	offset_y = wxINT16_SWAP_ON_BE(header->top);
	size_t hdr_size = sizeof(rottpatch_header_t);
	uint8_t alpha = 0xFF;
	if (transparent) {
		//wxLogMessage("Transparent");
		//alpha = gfx_data[sizeof(rottpatch_header_t)];
		hdr_size +=2;
	}

	// Read column offsets
	col_offsets = new uint16_t[width];
	uint16_t* c_ofs = (uint16_t*)((uint8_t*)gfx_data + hdr_size);
	for (int a = 0; a < width; a++)
		col_offsets[a] = wxUINT16_SWAP_ON_BE(c_ofs[a]);

	// Clear current data if it exists
	clearData();

	// Load data
	data = new uint8_t[width * height];
	memset(data, 0, width * height);	// Set colour to palette index 0
	mask = new uint8_t[width * height];
	memset(mask, 0, width * height);	// Set mask to fully transparent
	for (int c = 0; c < width; c++) {
		// Get current column offset
		uint16_t col_offset = col_offsets[c];
		//if (transparent) wxLogMessage("Column %i, offset at %i, offset %i", c, (c<<1) + hdr_size, col_offset);

		// Check column offset is valid
		if (col_offset >= (unsigned)size) {
			clearData();
			return false;
		}

		// Go to start of column
		const uint8_t* bits = gfx_data + col_offset;

		// Read posts
		int counter = 0;
		while (1) {
			// Get row offset
			//if (transparent) wxLogMessage("Row number %i at offset %i", *bits, bits - gfx_data);
			uint8_t row = *bits++;

			if (row == 0xFF/* || row > height*/) // End of column?
				break;

			// This apparently triggers translucency mode, but I'm not sure how it works
			if (transparent && *bits == 254) {
				clearData();
				Global::error = "Sorry, masked ROTT patches with translucency are not yet supported";
				return false;
			} else {

				// Get no. of pixels
				uint8_t n_pix = *bits++;
				//if (transparent) wxLogMessage("Post %i, row = %i, numpixels = %i, offset %i", counter++, row, n_pix, bits - gfx_data);

				if (n_pix >= height && row != 0 && row < height) {
					wxLogMessage("Hack triggered!");
					bits+=2;
					continue;
				}

				for (uint8_t p = 0; p < n_pix; p++) {
					// Get pixel position
					int pos = ((row + p)*width + c);
					//if (transparent) wxLogMessage("Pixel %i: pos %i, offset %i", p, pos, bits - gfx_data);

						// Stop if we're outside the image
					if (pos > width*height)
						break;

					// Stop if for some reason we're outside the gfx data
					if (bits > gfx_data + size)
						break;

					// Fail if bogus data gives a negative pos (this corrupts the heap!)
					if (pos < 0) {
						clearData();
						return false;
					}

					// Write pixel data
					data[pos] = *bits++;
					mask[pos] = alpha;
				}
			}
		}
	}

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadRottLbm
 * Loads a Rise of the Triad LBM format image.
 * Returns false if the image data was invalid, true otherwise
 *******************************************************************/
bool SImage::loadRottLbm(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Setup variables
	offset_x = offset_y = 0;
	width = READ_L16(gfx_data, 0);
	height = READ_L16(gfx_data, 2);
	has_palette = true;
	format = PALMASK;
	numimages = 1;
	imgindex = 0;

	// ROTT source code says: "LIMITATIONS - Only works with 320x200!!!"
	if (width != 320 || height != 200)
		return false;

	// Clear current data if it exists
	clearData();

	// Let's build the palette now.
	for (size_t c = 0; c < 256; ++c) {
		rgba_t color;
		color.r = gfx_data[(c*3)+4];
		color.g = gfx_data[(c*3)+5];
		color.b = gfx_data[(c*3)+6];
		palette.setColour(c, color);
	}

	// Read raw pixel data
	data = new uint8_t[width*height];

	// Create some variables needed for LBM decompression
	const uint8_t * read = gfx_data + 768 +4;
	const uint8_t * readend = gfx_data + size;
	uint8_t * dest = data;
	uint8_t * destend = data + (width * height);
	uint8_t code = 0;
	uint8_t length = 0;
	uint8_t count = 0;

	while (read < readend && dest < destend && count < width) {
		code = *read++;
		if (code < 0x80) {
			length = code + 1;
			memcpy(dest, read, length);
			dest+=length;
			read+=length;
		} else if (code > 0x80) {
			length = (code^0xFF)+2;;
			code = *read++;
			memset(dest, code, length);
			dest+=length;
		} else length = 0;
		count += length;
	}

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadRottRaw
 * Loads a ROTT raw data + header image.
 *******************************************************************/
bool SImage::loadRottRaw(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Setup variables
	patch_header_t* header = (patch_header_t*)gfx_data;
	height = wxINT16_SWAP_ON_BE(header->width); // Cheat a bit since
	width = wxINT16_SWAP_ON_BE(header->height); // it's column-major
	offset_x = wxINT16_SWAP_ON_BE(header->left);
	offset_y = wxINT16_SWAP_ON_BE(header->top);
	format = PALMASK;
	has_palette = false;
	numimages = 1;
	imgindex = 0;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];
	memcpy(data, gfx_data+8, width * height);

	// Convert from column-major to row-major
	rotate(90);
	mirror(true);

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadRottPic
 * Loads a ROTT picture: same as Doom snea, but with transparency.
 *******************************************************************/
bool SImage::loadRottPic(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check/setup size
	offset_x = offset_y = 0;
	uint8_t qwidth = gfx_data[0];
	width = qwidth * 4;
	height = gfx_data[1];

	if (size != 4 + width * height)
		return false;

	// Setup variables
	format = PALMASK;
	has_palette = false;
	numimages = 1;
	imgindex = 0;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];

	// Since gfx_data is a const pointer, we can't work on it.
	uint8_t * tempdata = new uint8_t[size];
	memcpy(tempdata, gfx_data, size);

	uint8_t* entryend = tempdata + size - 2;
	uint8_t* dataend = data + size - 4;
	uint8_t* pixel = tempdata + 2;
	uint8_t* brush = data;
	while (pixel < entryend) {
		*brush = *pixel++;
		brush += 4;
		if (brush >= dataend)
			brush -= size - 5;
	}

	delete[] tempdata;

	// Create mask (index 255 is transparent)
	mask = new uint8_t[width*height];
	memset(mask, 255, width*height);
	for (int i = 0; i < width*height; ++i)
		if (data[i] == 0xFF) mask[i] = 0;

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadWolfPic
 * Loads a Wolfenstein 3D picture.
 *******************************************************************/
bool SImage::loadWolfPic(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check/setup size
	offset_x = offset_y = 0;
	width = READ_L16(gfx_data, 0);
	height = READ_L16(gfx_data, 2);

	if (size != 4 + width * height)
		return false;

	// Setup variables
	format = PALMASK;
	has_palette = false;
	numimages = 1;
	imgindex = 0;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];

	const uint8_t* pixel = gfx_data + 4;
	const uint8_t* entryend = gfx_data + size;
	uint8_t* brush = data;
	uint8_t* dataend = data + size - 4;

	while (pixel < entryend) {
		*brush = *pixel++;
		brush += 4;
		if (brush >= dataend)
			brush -= size - 5;
	}

	// Create mask (index 255 is transparent)
	/*mask = new uint8_t[width*height];
	memset(mask, 255, width*height);
	for (int i = 0; i < width*height; ++i)
		if (data[i] == 0xFF) mask[i] = 0;*/

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadWolfSprite
 * Loads a Wolfenstein 3D sprite.
 *******************************************************************/
bool SImage::loadWolfSprite(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	// Check/setup size
	uint8_t leftpix, rightpix;
	leftpix = gfx_data[0];
	rightpix = gfx_data[2];
	width = 1 + rightpix - leftpix;
	height = 64;
	offset_x = 32 - leftpix;
	offset_y = height;

	// Setup variables
	format = PALMASK;
	has_palette = false;
	numimages = 1;
	imgindex = 0;

	// Clear current data if it exists
	clearData();

	// Read raw pixel data
	data = new uint8_t[width*height];
	mask = new uint8_t[width*height];
	memset(mask, 0, (width*height));

	uint16_t * cmdptr = (uint16_t *)(gfx_data + 4);
	uint32_t i, x, y;
	for (x = 0 ; x < (unsigned)width ; ++x )
	{
        int16_t * linecmds = (int16_t *)(gfx_data + wxINT16_SWAP_ON_BE( *cmdptr ));
		cmdptr++;
		for (; wxINT16_SWAP_ON_BE(*linecmds); linecmds += 3)
		{
			i = (wxINT16_SWAP_ON_BE(linecmds[2])>>1) + wxINT16_SWAP_ON_BE(linecmds[1]);
			for (y = (uint32_t)(wxINT16_SWAP_ON_BE(linecmds[2])>>1); y < (uint32_t)(wxINT16_SWAP_ON_BE(linecmds[0]) / 2); ++y, ++i)
			{
				data[y * width + x] = gfx_data[i];
				mask[y * width + x] = 255;
			}
		}
	}

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* JediRLE0
 * Used by several Jedi Engine formats
 *******************************************************************/
bool JediRLE0(const uint8_t* src, int coloffs, int width, int height, uint8_t * data) {
	for (int x = 0; x < width; ++x) {
		int p = READ_L32(src, (coloffs + (x<<2)));
		uint8_t * endcol = data + height;
		while (data < endcol) {
			int val = src[p++];
			if (val < 0x80) {
				memcpy(data, src + p, val);
				data += val;
				p += val;
			} else {
				memset(data, 0, val - 0x80);
				data += (val - 0x80);
			}
		}
	}
	return true;
}

/* JediRLE
 * Used by several Jedi Engine formats
 *******************************************************************/
bool JediRLE(const uint8_t* src, int coloffs, int width, int height, uint8_t * data) {
	for (int x = 0; x < width; ++x) {
		int p = READ_L32(src, (coloffs + (x<<2)));
		uint8_t * endcol = data + height;
		while (data < endcol) {
			int val = src[p++];
			if (val < 0x80) {
				memcpy(data, src + p, val);
				data += val;
				p += val;
			} else {
				memset(data, src[p++], val - 0x80);
				data += (val - 0x80);
			}
		}
	}
	return true;
}

/* Jedi Engine bitmap header struct
 *******************************************************************/
struct JediBMHeader
{
	char magic[4];		// "BM "0x1E
	uint16_t width;		// If 1 and height not 1, multiple BM
	uint16_t height;	// If height and width both 1, 1x1 bitmap
	uint16_t wid2;		// Same as width except for multiple BM, then it's 65534
	uint16_t hei2;		// Same as height except for multiple BM, then it's number of images
	uint8_t flag;		// Determines transparency among other stuff
	uint8_t log;		// Worth either log2(height) or 0
	uint16_t rle;		// 0: not compressed, 1: compressed with RLE, 2: compressed with RLE0
	uint32_t coloffs;	// Column offsets in compressed bitmaps
	uint32_t pad[3];	// Padding, should be zero
};
struct JediBMSubheader
{
	uint16_t width;		// If 1 and height not 1, multiple BM
	uint16_t height;	// If height and width both 1, 1x1 bitmap
	uint16_t wid2;		// Unused
	uint16_t hei2;		// Unused
	uint32_t size;		// Unused
	uint8_t log;		// Worth log2(height)
	uint8_t pad1[11];	// No known use
	uint8_t flag;		// Determines transparency among other stuff
	uint8_t pad2[3];	// No known use
};

/* SImage::loadJediBM
 * Loads a Jedi Engine bitmap
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadJediBM(const uint8_t* gfx_data, int size, int index) {
	if (size < 32)
		return false;

	const JediBMHeader * header = (JediBMHeader *) gfx_data;
	bool multibm = false;
	bool transparent = !!(header->flag & 8);
	size_t data_offset = 32;

	// Check for multiple images
	if (wxINT16_SWAP_ON_BE(header->width) == 1 && wxINT16_SWAP_ON_BE(header->height) != 1) {
		if (header->rle == 0 && wxINT16_SWAP_ON_BE(header->wid2) == 65534) {
			numimages = wxINT16_SWAP_ON_BE(header->hei2);
		} else {
			Global::error = "Jedi BM file: invalid multi-BM file";
			return false;
		}
		multibm = true;
	} else numimages = 1;

	// Sanitize index if needed
	index %= numimages;
	if (index < 0)
		index = numimages + index;

	// Setup variables
	has_palette = false;
	format = PALMASK;
	imgindex = index;

	if (multibm) {
		// 32 for the header, 2 for control data, then four bytes per subimage
		int offset_offset = 34 + (imgindex<<2);
		if (offset_offset + 4 > size) {
			Global::error = "Jedi BM file: invalid multi-BM file";
			return false;
		}
		const uint32_t * header_poffset = (uint32_t *)(gfx_data + offset_offset);
		uint32_t header_offset = wxINT32_SWAP_ON_BE(*header_poffset)+34;
		const JediBMSubheader * subheader = (JediBMSubheader *) (gfx_data + header_offset);
		transparent = !!(subheader->flag & 8);
		// Little cheat here as they are defined in column-major format,
		// not row-major. So we'll just call the rotate function.
		height = wxINT16_SWAP_ON_BE(subheader->width);
		width = wxINT16_SWAP_ON_BE(subheader->height);
		data_offset = header_offset + 28;
	} else {
		// Same little cheat.
		height = wxINT16_SWAP_ON_BE(header->width);
		width = wxINT16_SWAP_ON_BE(header->height);
	}

	// Create data
	clearData();
	data = new uint8_t[width*height];

	// Fill data with pixel data
	if (header->rle == 0) {
		memcpy(data, gfx_data + data_offset, width * height);
	} else if (header->rle == 1) {
		JediRLE(gfx_data + data_offset, header->coloffs, height, width, data);
	} else if (header->rle == 2) {
		JediRLE0(gfx_data + data_offset, header->coloffs, height, width, data);
	} else {
		Global::error = "Jedi BM file: Invalid compression scheme";
		clearData();
		return false;
	}

	// Convert from column-major to row-major
	rotate(90);

	// Manage transparency
	if (transparent) {
		mask = new uint8_t[width*height];
		memset(mask, 0xFF, width*height);
		for (int i = 0; i < width*height; ++i)
			if (data[i] == 0)
				mask[i] = 0;
	}

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* Jedi Engine frame header struct
 *******************************************************************/
struct JediFMEHeader1
{
	int32_t offsx;		// X offset (right is positive)
	int32_t offsy;		// Y offset (down is positive)
	uint32_t flag;		// Only one flag used: 1 for flipped horizontally
	uint32_t head2;		// Offset to secondary header
	uint32_t width;		// Unused; the data from the secondary header is used instead
	uint32_t height;	// Unused; the data from the secondary header is used instead
	uint32_t pad[2];	// Padding, should be zero
};
struct JediFMEHeader2
{
	uint32_t width;		// Used instead of the value in the primary header
	uint32_t height;	// Used instead of the value in the primary header
	uint32_t flag;		// Only one flag used: 1 for RLE0 compression
	uint32_t size;		// Data size for compressed FME, same as file size - 32
	uint32_t coloffs;	// Offset to column data, practically always 0
	uint32_t padding;	// No known use
};

/* SImage::JediFrame
 * Used by several Jedi Engine formats
 *******************************************************************/
bool SImage::JediFrame(const uint8_t* gfx_data, uint32_t hdroffs) {
	const JediFMEHeader1 * header1 = (JediFMEHeader1 *) (gfx_data + hdroffs);
	const JediFMEHeader2 * header2 = (JediFMEHeader2 *) (gfx_data + wxINT32_SWAP_ON_BE(header1->head2));
	bool flip = !!(header1->flag & 1);
	size_t data_offset = header1->head2;

	// Setup variables
	offset_x = 0 - wxINT32_SWAP_ON_BE(header1->offsx);
	offset_y = 0 - wxINT32_SWAP_ON_BE(header1->offsy);

	// Little cheat here as they are defined in column-major format,
	// not row-major. So we'll just call the rotate function.
	height = wxINT32_SWAP_ON_BE(header2->width);
	width = wxINT32_SWAP_ON_BE(header2->height);

	// Create data
	clearData();
	data = new uint8_t[width*height];

	// Fill data with pixel data
	if (header2->flag == 0)
		memcpy(data, gfx_data + header1->head2 + 24, width * height);
	else JediRLE0(gfx_data + wxINT32_SWAP_ON_BE(header1->head2), 24,
		wxINT32_SWAP_ON_BE(header2->width), wxINT32_SWAP_ON_BE(header2->height), data);

	// Convert from column-major to row-major
	rotate(90);

	// Mirror if needed
	if (flip) mirror(false);

	// Manage transparency
	mask = new uint8_t[width*height];
	memset(mask, 0xFF, width*height);
	for (int i = 0; i < width*height; ++i)
		if (data[i] == 0)
			mask[i] = 0;

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadJediFME
 * Loads a Jedi Engine frame
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadJediFME(const uint8_t* gfx_data, int size) {
	if (size < 32)
		return false;

	// Setup variables
	has_palette = false;
	format = PALMASK;
	imgindex = 0;
	numimages = 1;

	// The image loading code is in a different function
	// so that it can be used by WAXes too.
	return JediFrame(gfx_data, 0);
}

/* Jedi Engine wax header struct
 *******************************************************************/
struct JediWAXHeader
{
	uint32_t version;	// Constant worth 0x00100100
	uint32_t numseqs;	// Number of sequences
	uint32_t numframes;	// Number of frames
	uint32_t numcells;	// Number of cells
	uint32_t pad[4];	// Unused values
	uint32_t waxes[32];	// Offsets to the WAX subheaders
};
struct JediWAXSubheader
{
	uint32_t width;		// Scaled width, in world units
	uint32_t height;	// Scaled height, in world units
	uint32_t fps;		// Frames per second of animation
	uint32_t pad[4];	// Unused values, must be zero
	uint32_t seqs[32];	// Offsets to sequences
};

struct JediWAXSequence
{
	uint32_t pad[4];	// Unused values, must be zero
	uint32_t frames[32];// Offsets to frames
};

/* SImage::loadJediWAX
 * Loads a Jedi Engine frame
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadJediWAX(const uint8_t* gfx_data, int size, int index) {
	if (size < 460)
		return false;

	const JediWAXHeader * header = (JediWAXHeader *) gfx_data;
	vector<size_t> frameoffs;
	// This is a recursive nightmare. What were the LucasArt devs smoking when they specced this format?
	for (int i = 0; i < 32; ++i) {
		uint32_t waxi = wxINT32_SWAP_ON_BE(header->waxes[i]);
		if (waxi > sizeof(header) && waxi < size - sizeof(JediWAXSubheader)) {
			const JediWAXSubheader * subhdr = (JediWAXSubheader *) (gfx_data + waxi);
			for (int j = 0; j < 32; ++j) {
				uint32_t seqj = wxINT32_SWAP_ON_BE(subhdr->seqs[j]);
				if (seqj > sizeof(header) && seqj < size - sizeof(JediWAXSequence)) {
					const JediWAXSequence * seq = (JediWAXSequence *) (gfx_data + seqj);
					for (int k = 0; k < 32; ++k) {
						uint32_t framk = wxINT32_SWAP_ON_BE(seq->frames[k]);
						if (framk > sizeof(header) && framk < size - sizeof(JediWAXSequence)) {
							const JediFMEHeader1 * frame = (JediFMEHeader1 *) (gfx_data + framk);
							uint32_t cell = wxINT32_SWAP_ON_BE(frame->head2);
							if (cell > sizeof(header) && cell < size - sizeof(JediFMEHeader2)) {
								bool notfound = true;
								for (size_t l = 0; l < frameoffs.size(); ++l) {
									if (frameoffs[l] == framk) {
										notfound = false;
										break;
									}
								}
								if (notfound) frameoffs.push_back(framk);
							}
						}
					}
				}
			}
		}
	}
	// Urgh. At least it's over now.

	// Sanitize index if needed
	numimages = frameoffs.size();
	if (frameoffs.empty()) {
		Global::error = "Jedi WAX error: No cell found in wax!"; // What a surreal error message ;)
		return false;
	}
	index %= numimages;
	if (index < 0)
		index = numimages + index;

	// Setup variables
	has_palette = false;
	format = PALMASK;
	imgindex = index;

	// The one good thing about this format is that we can reuse the FME code for loading the actual image
	return JediFrame(gfx_data, frameoffs[imgindex]);
}

/*******************************************************************
 * FONT FORMATS
 *******************************************************************/

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
	height = READ_L16(gfx_data, 0);

	size_t datasize = size - 0x302;
	if (datasize % height)
		return false;

	width = datasize / height;

	clearData();
	has_palette = false;
	format = PALMASK;

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

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
	// Announce change and return success
	announce("image_changed");
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

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

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

	// Announce change and return success
	announce("image_changed");
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

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

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

	// Announce change and return success
	announce("image_changed");
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

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

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
		wxLogMessage("BMF aborted: no data after palette");
		return false;
	}
	mf.info_size = ofs[0];
	if (mf.info_size > 0) {
		mf.info = (char *) ofs+1;
	}
	ofs+=mf.info_size+1;
	mf.num_chars = READ_L16(ofs, 0);
	if (mf.num_chars == 0)
		return false;

	// Now we are at the beginning of character data
	ofs+=2;
	if (ofs >= eod) {
		wxLogMessage("BMF aborted: no data after char size");
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
	// Announce change and return success
	announce("image_changed");
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

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

	//Each pixel is described as a single bit, either on or off
	for (size_t i = 0; i < (unsigned)size; ++i) {
		for (size_t p = 0; p < 8; ++p)
			mask[(i*8)+p] = ((gfx_data[i]>>(7-p)) & 1) * 255;
	}
	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadWolfFont
 * Loads a Wolf3D-format font.
 * The format used is simple, basically like the Doom alpha HUFONT,
 * except not in the same order:
 * Offset | Length | Type | Name
 *  0x000 |      2 | ui16 | image height (one value for all chars)
 *  0x002 |  256*2 | ui16 | characteroffset (one value per char)
 *  0x202 |  256*1 | ui08 | characterwidth (one value per char)
 *  0x302 |    x*1 | ui08 | pixel color index (one value per pixel)
 * So, total size - 302 % value @ 0x00 must be null.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadWolfFont(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	if (size <= 0x302)
		return false;

	offset_x = offset_y = 0;
	height = READ_L16(gfx_data, 0);

	size_t datasize = size - 0x302;
	if (datasize % height)
		return false;

	width = datasize / height;

	clearData();
	has_palette = false;
	format = PALMASK;

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

	// Create new picture and mask
	const uint8_t* r = gfx_data + 0x302;
	data = new uint8_t[datasize];
	mask = new uint8_t[datasize];
	memset(mask, 0xFF, datasize);
	memcpy(data, r, datasize);

	size_t p = 0; // Previous width
	size_t w = 0; // This character's width
	// Run through each character
	for (size_t c = 0; c < 256; ++c) {
		p += w; // Add previous character width to total
		w = gfx_data[c + 0x202]; // Get this character's width
		if (!w) continue;
		size_t o = READ_L16(gfx_data, ((c<<1)+2));
		for (size_t i = 0; i < w * height; ++i) {
			// Compute source and destination offsets
			size_t s = o + i;
			size_t d = ((i/w) * width) + (i%w) + p;
			data[d] = gfx_data[s];
			// Index 0 is transparent
			if (data[d] == 0)
				mask[d] = 0;
		}
	}
	// Announce change and return success
	announce("image_changed");
	return true;
}


/* SImage::loadJediFNT
 * Loads a Jedi Engine-format bitmap font.
 * The header tells the height and which are the first and last
 * characters described. Then the character data consists of a
 * single byte of data for the width of that character, followed
 * by a list of columns. The characters are listed in order.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadJediFNT(const uint8_t* gfx_data, int size) {
	// Check data
	if (!gfx_data)
		return false;

	if (size <= 35)
		return false;

	// The character data is presented as a list of columns
	// preceded by a byte for
	offset_x = offset_y = 0;

	// Since the format is column-major, we'll use our usual cheat of
	// inverting height and width to build the picture, then rotating it.
	width = gfx_data[4];

	// First and last characters
	uint8_t firstc = gfx_data[8];
	uint8_t lastc  = gfx_data[9];
	uint8_t numchr = 1 + lastc - firstc;

	// Go through each character to compute the total width (pre-rotation height)
	height = 0;
	size_t wo = 32; // Offset to width of next character
	for (uint8_t i = 0; i < numchr; ++i) {
		height += gfx_data[wo];
		wo += 1 + (width * gfx_data[wo]);
	}

	clearData();
	has_palette = false;
	format = PALMASK;

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

	// Create new picture and mask
	data = new uint8_t[width * height];
	mask = new uint8_t[width * height];
	memset(mask, 0xFF, width * height);

	// Run through each character and add the pixel data
	wo = 32;
	size_t col = 0;
	for (uint8_t i = 0; i < numchr; ++i) {
		uint8_t numcols = gfx_data[wo++];
		memcpy(data + (col * width), gfx_data + wo, numcols * width);
		col += numcols;
		wo += width * numcols;
	}

	// Make index 0 transparent
	for (int i = 0; i < width * height; ++i)
		if (data[i] == 0)
			mask[i] = 0;

	// Convert from column-major to row-major
	rotate(90);

	// Announce change and return success
	announce("image_changed");
	return true;
}

/* SImage::loadJediFONT
 * Loads a Jedi Engine-format monochrome font.
 * Contrarily to what the DF specs claim, the first two int16
 * values are not the first and last characters as in the FNT
 * format; instead, they are the first character and the number
 * of characters! They're also mistaken about character width.
 * Returns false if the image data was invalid, true otherwise.
 *******************************************************************/
bool SImage::loadJediFONT(const uint8_t* gfx_data, int size) {
	// Check data
	if (size < 16)
		return false;

	int numchr = READ_L16(gfx_data, 2);

	// Setup variables
	offset_x = offset_y = 0;
	height = READ_L16(gfx_data, 6) * numchr;
	width = READ_L16(gfx_data, 4);
	has_palette = false;
	format = PALMASK;

	// reset data
	clearData();
	data = new uint8_t[width*height];
	memset(data, 0xFF, width*height);
	mask = new uint8_t[width*height];
	memset(mask, 0x00, width*height);

	// Technically each character is its own image, though.
	numimages = 1;
	imgindex = 0;

	// We don't care about the character widths since
	// technically it's always eight anyway. The offset
	// to graphic data corresponds to 12 (header size)
	// plus one byte by character for width.
	int o = 12 + numchr;
	int bpc = width / 8;

	//Each pixel is described as a single bit, either on or off
	for (int i = 0; i < height; ++i) {
		for (int p = 0; p < width; ++p) {
			switch (bpc) {
				case 1: mask[(i*width)+p] = ((gfx_data[o+i]>>(7-p)) & 1) * 255; break;
				case 2: mask[(i*width)+p] = ((READ_B16(gfx_data, o+(i*2))>>(15-p)) & 1) * 255; break;
				case 3: mask[(i*width)+p] = ((READ_B24(gfx_data, o+(i*3))>>(23-p)) & 1) * 255; break;
				case 4: mask[(i*width)+p] = ((READ_B32(gfx_data, o+(i*4))>>(31-p)) & 1) * 255; break;
				default: clearData(); Global::error = "Jedi FONT: Weird word width"; return false;
			}
		}
	}
	// Announce change and return success
	announce("image_changed");
	return true;
}

