
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
 *				This file contains all the generic operations.
 *				Format-dependent functions are in SImageFormats.cpp.
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
	format = RGBA;
	numimages = 1;
	imgindex = 0;
}

/* SImage::~SImage
 * SImage class destructor
 *******************************************************************/
SImage::~SImage() {
	clearData();
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

/* SImage::getPalData
 * Loads the image as index data into <mc>. Returns false if image is
 * invalid, true otherwise
 *******************************************************************/
bool SImage::getPalData(MemChunk& mc) {
	// Check the image is valid
	if (!isValid())
		return false;

	// Init rgb data
	mc.reSize(width * height, false);

	// Cannot do this for trucolor graphics.
	if (format == RGBA)
		return false;

	else if (format == PALMASK) {
		mc.write(data, width * height);
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
	announce("offsets_changed");
}

/* SImage::setYOffset
 * Changes the image Y offset
 *******************************************************************/
void SImage::setYOffset(int offset) {
	// Change the Y offset
	offset_y = offset;

	// Announce change
	announce("offsets_changed");
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
	announce("image_changed");
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
	announce("image_changed");
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
	announce("image_changed");

	return false;
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
	announce("image_changed");

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
	announce("image_changed");

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
	announce("image_changed");

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
	announce("image_changed");

	return true;
}

/* SImage::safeConvert
 * Converts the image to a format that can be written.
 * The rules are simple: use external palette -> Doom Gfx, and
 * otherwise -> PNG.
 *******************************************************************/
bool SImage::safeConvert(MemChunk& out, Palette8bit* pal) {
	if (format == PALMASK && !has_palette)
		return toDoomGfx(out);
	else return toPNG(out, pal);
}

/* SImage::rotate
 * Rotates the image with an angle of 90°, 180° or 270°. Why not
 * use FreeImage_Rotate instead? So as not to bother converting
 * to and fro a FIBITMAP...
 *******************************************************************/
bool SImage::rotate(int angle) {
	if (!data)
		return false;

	if (angle == 0)	return true;				// Nothing to do 
	if (angle < 0 || angle % 90) return false;	// Unsupported angle
	angle %= 360;

	uint8_t * nd, * nm;
	int nw, nh;

	// Compute new dimensions and numbers of pixels and bytes
	if (angle % 180) { nw = height; nh = width; }
	else {	nw = width; nh = height; }
	int numpixels = width*height; int numbpp = 0;
	if (format==PALMASK)	numbpp = 1;
	else if (format==RGBA)	numbpp = 4;
	else return false;

	// Create new data and mask
	nd = new uint8_t[numpixels*numbpp];
	if (mask) nm = new uint8_t[numpixels*numbpp];
	else nm = NULL;

	// Remapping loop
	int i, j, k;
	for (i = 0; i < numpixels; ++i) {
		switch (angle) {
			// Urgh maths...
			case 90:	j = (((nh - 1) - (i%width)) * nw) + (i/width);	break;
			case 180:	j = (numpixels - 1) - i;						break;
			case 270:	j = ((i%width) * nw) + ((nw - 1) - (i/width));	break;
			default: return false;
		}
		if (j >= numpixels) {
			wxLogMessage("Pixel %i remapped to %i, how did this even happen?", i, j);
			delete [] nd; if (mask) delete[] nm;
			return false;
		}
		for (k = 0; k < numbpp; ++k) {
			nd[(j*numbpp)+k] = data[(i*numbpp)+k];
			if (mask) nm[(j*numbpp)+k] = mask[(i*numbpp)+k];
		}
	}

	// It worked, yay
	clearData();
	data = nd; mask = nm; width = nw; height = nh;

	// Announce change
	announce("image_changed");
	return true;
}

/* SImage::mirror
 * Mirrors the image horizontally or vertically.
 *******************************************************************/
bool SImage::mirror(bool vertical) {
	uint8_t * nd, * nm;

	// Compute numbers of pixels and bytes
	int numpixels = width*height; int numbpp = 0;
	if (format==PALMASK)	numbpp = 1;
	else if (format==RGBA)	numbpp = 4;
	else return false;

	// Create new data and mask
	nd = new uint8_t[numpixels*numbpp];
	if (mask) nm = new uint8_t[numpixels*numbpp];
	else nm = NULL;

	// Remapping loop
	int i, j, k;
	for (i = 0; i < numpixels; ++i) {
		if (vertical)
			j = (((height - 1) - (i/width)) * width) + (i%width);
		else // horizontal
			j = ((i/width) * width) + ((width - 1) - (i%width));
		if (j >= numpixels) {
			wxLogMessage("Pixel %i remapped to %i, how did this even happen?", i, j);
			delete [] nd; if (mask) delete[] nm;
			return false;
		}
		for (k = 0; k < numbpp; ++k) {
			nd[(j*numbpp)+k] = data[(i*numbpp)+k];
			if (mask) nm[(j*numbpp)+k] = mask[(i*numbpp)+k];
		}
	}

	// It worked, yay
	clearData();
	data = nd; mask = nm;

	// Announce change
	announce("image_changed");
	return true;
}

/* SImage::crop
 * Crops a section of the image.
 *******************************************************************/
bool SImage::crop(long x1, long y1, long x2, long y2) {

	if (x2 == 0 || x2 > width) x2 = width;
	if (y2 == 0 || y2 > height) y2 = height;

	// No need to bother with incorrect values
	if (x2 <= x1 || y2 <= y1 || x1 > width || y1 > height)
		return false;

	uint8_t * nd, * nm;
	size_t nw, nh;
	nw = x2 - x1;
	nh = y2 - y1;

	// Compute numbers of pixels and bytes
	int numpixels = nw*nh; int numbpp = 0;
	if (format==PALMASK)	numbpp = 1;
	else if (format==RGBA)	numbpp = 4;
	else return false;

	// Create new data and mask
	nd = new uint8_t[numpixels*numbpp];
	if (mask) nm = new uint8_t[numpixels*numbpp];
	else nm = NULL;

	// Remapping loop
	size_t i, a, b;
	for (i = 0; i < nh; ++i) {
		a = i*nw*numbpp;
		b = (((i+y1)*width)+x1)*numbpp;
		memcpy(nd+a, data+b, nw*numbpp);
		if (mask) memcpy(nm+a, mask+b, nw*numbpp);
	}

	// It worked, yay
	clearData();
	data = nd; mask = nm;
	width = nw; height = nh;

	// Announce change
	announce("image_changed");
	return true;
}