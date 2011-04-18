
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
#include "SIFormat.h"
#include "Translation.h"
#include "MathStuff.h"
#include <wx/filefn.h>


/*******************************************************************
 * SIMAGE CLASS FUNCTIONS
 *******************************************************************/

/* SImage::SImage
 * SImage class constructor
 *******************************************************************/
SImage::SImage(SIType type) {
	width = 0;
	height = 0;
	data = NULL;
	mask = NULL;
	offset_x = 0;
	offset_y = 0;
	this->type = type;
	numimages = 1;
	imgindex = 0;
	has_palette = false;
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
	if (type == RGBA) {
		mc.importMem(data, width * height * 4);
		return true;
	}

	// Otherwise convert
	else if (type == PALMASK) {
		// Get palette to use
		if (has_palette || !pal)
			pal = &palette;

		uint8_t rgba[4];
		for (int a = 0; a < width * height; a++) {
			// Get colour
			rgba_t col = pal->colour(data[a]);

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

	if (type == RGBA) {
		// RGBA format, remove alpha information
		for (int a = 0; a < width * height * 4; a += 4)
			mc.write(&data[a], 3);

		return true;
	}
	else if (type == PALMASK) {
		// Paletted, convert to RGB

		// Get palette to use
		if (has_palette || !pal)
			pal = &palette;

		// Build RGB data
		uint8_t rgba[4];
		for (int a = 0; a < width * height; a ++) {
			pal->colour(data[a]).write(rgba);
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
	if (type == RGBA)
		return false;

	else if (type == PALMASK) {
		mc.write(data, width * height);
		return true;
	}

	return false;	// Invalid image type
}

/* SImage::getStride
 * Returns the number of bytes per image row
 *******************************************************************/
unsigned SImage::getStride() {
	if (type == RGBA)
		return width*4;
	else
		return width;
}

/* SImage::getBpp
 * Returns the number of bytes per image pixel
 *******************************************************************/
uint8_t SImage::getBpp() {
	if (type == RGBA)
		return 4;
	else
		return 1;
}

/* SImage::getPixel
 * Returns the colour of the pixel at [x,y] in the image, or black+
 * invisible if out of range
 *******************************************************************/
rgba_t SImage::getPixel(unsigned x, unsigned y, Palette8bit* pal) {
	// Get pixel index
	unsigned index = y * getStride() + x * getBpp();

	// Check it
	if (index >= unsigned(width*height*getBpp()))
		return rgba_t(0, 0, 0, 0);

	// Get colour at pixel
	rgba_t col;
	if (type == RGBA) {
		col.r = data[index];
		col.g = data[index+1];
		col.b = data[index+2];
		col.a = data[index+3];
	}
	else {
		// Get palette to use
		if (has_palette || !pal)
			pal = &palette;

		col.set(pal->colour(data[index]));
	}

	return col;
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

void SImage::create(int width, int height, SIType type, Palette8bit* pal) {
	// Check valid width/height
	if (width < 0 || height < 0)
		return;

	// Clear current image
	clearData();

	// Create blank image
	if (type == PALMASK) {
		data = new uint8_t[width*height];
		memset(data, 0, width*height);
		mask = new uint8_t[width*height];
		memset(mask, 0, width*height);
	}
	else {
		data = new uint8_t[width*height*4];
		memset(data, 0, width*height*4);
	}

	// Set image properties
	this->width = width;
	this->height = height;
	this->type = type;
	offset_x = 0;
	offset_y = 0;
	numimages = 1;
	imgindex = 0;
	if (pal) {
		palette.copyPalette(pal);
		has_palette = true;
	}
	else
		has_palette = false;
}

/* SImage::clear
 * Deletes/clears any existing image data, and resets the image to
 * zero-sized
 *******************************************************************/
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

	if (type == RGBA) {
		// RGBA format, set alpha values to given one
		for (int a = 3; a < width * height * 4; a += 4)
			data[a] = alpha;
	}
	else if (type == PALMASK) {
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
	if (type != PALMASK)
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

/* SImage::countColours
 * Returns the number of unique colors in a paletted image
 *******************************************************************/
size_t SImage::countColours() {
	// If the picture is not paletted, return 0.
	if (type != PALMASK)
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
	if (type != PALMASK)
		return;

	// Get palette to use
	if (has_palette || !pal)
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

/* SImage::copyImage
 * Copies all data and properties from [image]
 *******************************************************************/
bool SImage::copyImage(SImage* image) {
	// Check image was given
	if (!image)
		return false;

	// Clear current data
	clearData();

	// Copy image properties
	width = image->width;
	height = image->height;
	type = image->type;
	palette.copyPalette(&image->palette);
	has_palette = image->has_palette;
	offset_x = image->offset_x;
	offset_y = image->offset_y;
	imgindex = image->imgindex;
	numimages = image->numimages;

	// Copy image data
	if (image->data) {
		data = new uint8_t[width*height*getBpp()];
		memcpy(data, image->data, width*height*getBpp());
	}
	if (image->mask) {
		mask = new uint8_t[width*height];
		memcpy(mask, image->mask, width*height);
	}

	return true;
}

/* SImage::convertRGBA
 * Converts the image to 32bpp (RGBA). Returns false if the image was
 * already 32bpp, true otherwise.
 *******************************************************************/
bool SImage::convertRGBA(Palette8bit* pal) {
	// If it's already 32bpp do nothing
	if (type == RGBA)
		return false;

	// Get 32bit data
	MemChunk rgba_data;
	getRGBAData(rgba_data, pal);

	// Clear current data
	clearData(true);

	// Copy it
	data = new uint8_t[width * height * 4];
	memcpy(data, rgba_data.getData(), width * height * 4);

	// Set new type & update variables
	type = RGBA;
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

	// Create mask from alpha info (if converting from RGBA)
	if (type == RGBA) {
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

	// Load given palette
	palette.copyPalette(pal_target);

	// Clear current image data (but not mask)
	clearData(false);

	// Do conversion
	data = new uint8_t[width * height];
	unsigned i = 0;
	rgba_t col;
	for (int a = 0; a < width*height; a++) {
		col.r = rgba_data[i++];
		col.g = rgba_data[i++];
		col.b = rgba_data[i++];
		data[a] = palette.nearestColour(col);
		i++;	// Skip alpha
	}

	// Update variables
	type = PALMASK;
	has_palette = true;

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
	if (type == PALMASK && mask != NULL) {
		// Get palette to use
		if (has_palette || !pal)
			pal = &palette;

		// Palette+Mask type, go through the mask
		for (int a = 0; a < width * height; a++) {
			if (pal->colour(data[a]).equals(colour))
				mask[a] = 0;
			else
				mask[a] = 255;
		}
	}
	else if (type == RGBA) {
		// If we're forcing generation of mask data, create it if it doesn't exist
		if (force_mask && !mask)
			mask = new uint8_t[width * height];

		// RGBA type, go through alpha channel
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
	if (type == PALMASK && mask != NULL) {
		for (int a = 0; a < width * height; a++) {
			if (mask[a] > threshold)
				mask[a] = 255;
			else
				mask[a] = 0;
		}
	}
	else if (type == RGBA) {
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
	if (type == PALMASK && !has_palette)
		return toDoomGfx(out);
	else return toPNG(out, pal);
}

/* SImage::setPixel
 * Sets the pixel at [x],[y] to [colour]. Returns false if the
 * position is out of range, true otherwise
 *******************************************************************/
bool SImage::setPixel(int x, int y, rgba_t colour, Palette8bit* pal) {
	// Check position
	if (x < 0 || x >= width || y < 0 || y >= height)
		return false;

	// Set the pixel
	if (type == RGBA)
		colour.write(data + (y * (width*4) + (x*4)));
	else {
		// Get palette to use
		if (has_palette || !pal)
			pal = &palette;

		data[y * width + x] = pal->nearestColour(colour);
	}

	// Announce
	announce("image_changed");

	return true;
}

/* SImage::setPixel
 * Sets the pixel at [x],[y] to the palette colour at [pal_index],
 * and the transparency of the pixel to [alpha] (if possible).
 * Returns false if the position is out of bounds, true otherwise
 *******************************************************************/
bool SImage::setPixel(int x, int y, uint8_t pal_index, uint8_t alpha) {
	// Check position
	if (x < 0 || x >= width || y < 0 || y >= height)
		return false;

	// RGBA (use palette colour, probably don't want this, but it's here anyway :P)
	if (type == RGBA) {
		// Set the pixel
		rgba_t col = palette.colour(pal_index);
		col.a = alpha;
		col.write(data + (y * (width*4) + (x*4)));

		return true;
	}

	// Paletted
	else if (type == PALMASK) {
		// Set the pixel
		data[y*width+x] = pal_index;
		if (mask) mask[y*width+x] = alpha;

		return true;
	}

	// Announce
	announce("image_changed");

	// Invalid type
	return false;
}

/* SImage::rotate
 * Rotates the image with an angle of 90°, 180° or 270°. Why not
 * use FreeImage_Rotate instead? So as not to bother converting
 * to and fro a FIBITMAP...
 *******************************************************************/
bool SImage::rotate(int angle) {
	if (!data)
		return false;

	if (angle == 0)	return true;	// Nothing to do
	if (angle % 90) return false;	// Unsupported angle
	while (angle < 0) angle += 360;
	angle %= 360;

	uint8_t * nd, * nm;
	int nw, nh;

	// Compute new dimensions and numbers of pixels and bytes
	if (angle % 180) { nw = height; nh = width; }
	else {	nw = width; nh = height; }
	int numpixels = width*height; int numbpp = 0;
	if (type==PALMASK)	numbpp = 1;
	else if (type==RGBA)	numbpp = 4;
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
	if (type==PALMASK)	numbpp = 1;
	else if (type==RGBA)	numbpp = 4;
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

/* SImage::imgconv
 * Converts from column-major to row-major
 *******************************************************************/
bool SImage::imgconv() {
	int oldwidth = width;
	width = height;
	height = oldwidth;
	rotate(90);
	mirror(true);
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
	if (type==PALMASK)	numbpp = 1;
	else if (type==RGBA)	numbpp = 4;
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

/* SImage::resize
 * Resizes the image, conserving current data (will be cropped if
 * new size is smaller)
 *******************************************************************/
bool SImage::resize(int nwidth, int nheight) {
	// Check values
	if (nwidth < 0 || nheight < 0)
		return false;

	// If either dimension is zero, just clear the image
	if (nwidth == 0 || nheight == 0) {
		clear();
		return true;
	}

	// Init new image data
	uint8_t* newdata, *newmask;
	uint8_t bpp = 1;
	if (type == RGBA) bpp = 4;
	// Create new image data
	newdata = new uint8_t[nwidth * nheight * bpp];
	memset(newdata, 0, nwidth*nheight*bpp);
	// Create new mask if needed
	if (type == PALMASK) {
		newmask = new uint8_t[nwidth * nheight];
		memset(newmask, 0, nwidth*nheight);
	}
	else
		newmask = NULL;

	// Write new image data
	unsigned offset = 0;
	unsigned rowlen = MIN(width, nwidth)*bpp;
	unsigned nrows = MIN(height, nheight);
	for (unsigned y = 0; y < nrows; y++) {
		// Copy data row
		memcpy(newdata + offset, data + (y * width * bpp), rowlen);

		// Copy mask row
		if (newmask)
			memcpy(newmask + offset, mask + (y * width), rowlen);

		// Go to next row
		offset += nwidth*bpp;
	}

	// Update variables
	width = nwidth;
	height = nheight;
	clearData();
	data = newdata;
	mask = newmask;

	// Announce change
	announce("image_changed");

	return true;
}

/* SImage::setImageData
 * Sets the image data, size, and type from raw data
 *******************************************************************/
bool SImage::setImageData(uint8_t *ndata, int nwidth, int nheight, SIType ntype) {
	if (ndata) {
		clearData();
		type = ntype;
		width = nwidth;
		height = nheight;
		data = ndata;

		// Announce change
		announce("image_changed");

		return true;
	}
	return false;
}

/* SImage::applyTranslation
 * Applies a palette translation to the image
 *******************************************************************/
bool SImage::applyTranslation(Translation* tr, Palette8bit* pal) {
	// Check image is ok
	if (!data)
		return false;

	// Can't apply a translation to a non-paletted image
	if (type != PALMASK)
		return false;

	// Get palette to use
	if (has_palette || !pal)
		pal = &palette;

	// Go through pixels
	for (int p = 0; p < width*height; p++) {
		uint8_t i = data[p];

		// No need to process transparent pixels
		if (mask && mask[p] == 0)
			continue;

		// Go through each translation component
		for (unsigned a = 0; a < tr->nRanges(); a++) {
			TransRange* r = tr->getRange(a);

			// Palette range translation
			if (r->getType() == TRANS_PALETTE) {
				TransRangePalette* tp = (TransRangePalette*)r;

				// Check pixel is within translation range
				if (i >= tp->oStart() && i <= tp->oEnd()) {
					// Figure out how far along the range this colour is
					double range_frac = 0;
					if (tp->oStart() != tp->oEnd())
						range_frac = double(i - tp->oStart()) / double(tp->oEnd() - tp->oStart());

					// Determine destination palette index
					uint8_t di = tp->dStart() + range_frac * (tp->dEnd() - tp->dStart());

					// Apply new colour
					data[p] = di;
				}
			}

			// Colour range
			else if (r->getType() == TRANS_COLOUR) {
				TransRangeColour* tc = (TransRangeColour*)r;

				// Check pixel is within translation range
				if (i >= tc->oStart() && i <= tc->oEnd()) {
					// Figure out how far along the range this colour is
					double range_frac = 0;
					if (tc->oStart() != tc->oEnd())
						range_frac = double(i - tc->oStart()) / double(tc->oEnd() - tc->oStart());

					// Determine destination colour
					uint8_t r = tc->dStart().r + range_frac * (tc->dEnd().r - tc->dStart().r);
					uint8_t g = tc->dStart().g + range_frac * (tc->dEnd().g - tc->dStart().g);
					uint8_t b = tc->dStart().b + range_frac * (tc->dEnd().b - tc->dStart().b);

					// Find nearest colour in palette
					uint8_t di = pal->nearestColour(rgba_t(r, g, b));

					// Apply new colour
					data[p] = di;
				}
			}

			// Desaturated colour range
			else if (r->getType() == TRANS_DESAT) {
				TransRangeDesat* td = (TransRangeDesat*)r;

				// Check pixel is within translation range
				if (i >= td->oStart() && i <= td->oEnd()) {
					// Get greyscale colour
					rgba_t col = pal->colour(i);
					float grey = (col.r*0.3f + col.g*0.59f + col.b*0.11f) / 255.0f;

					// Determine destination colour
					uint8_t r = MIN(255, int((td->dSr() + grey*(td->dEr() - td->dSr()))*255.0f));
					uint8_t g = MIN(255, int((td->dSg() + grey*(td->dEg() - td->dSg()))*255.0f));
					uint8_t b = MIN(255, int((td->dSb() + grey*(td->dEb() - td->dSb()))*255.0f));

					// Find nearest colour in palette
					uint8_t di = pal->nearestColour(rgba_t(r, g, b));

					// Apply new colour
					data[p] = di;
				}
			}
		}
	}

	return true;
}

/* SImage::drawPixel
 * Draws a pixel of [colour] at [x],[y], blending it according to
 * the options set in [properties]. If the image is paletted, the
 * resulting pixel colour is converted to its nearest match in [pal]
 *******************************************************************/
bool SImage::drawPixel(int x, int y, rgba_t colour, si_drawprops_t& properties, Palette8bit* pal) {
	// Check valid coords
	if (x < 0 || y < 0 || x >= width || y >= height)
		return false;

	// Get palette to use
	if (has_palette || !pal)
		pal = &palette;

	// Setup alpha
	if (properties.src_alpha)
		colour.a *= properties.alpha;
	else
		colour.a = 255*properties.alpha;

	// Do nothing if completely transparent
	if (colour.a == 0)
		return true;

	// Get pixel index
	unsigned p = y * getStride() + x * getBpp();

	// Check for simple case (normal blending, no transparency involved)
	if (colour.a == 255 && properties.blend == NORMAL) {
		if (type == RGBA)
			colour.write(data+p);
		else {
			data[p] = pal->nearestColour(colour);
			mask[p] = colour.a;
		}

		return true;
	}

	// Not-so-simple case, do full processing
	rgba_t d_colour;
	if (type == PALMASK)
		d_colour = pal->colour(data[p]);
	else
		d_colour.set(data[p], data[p+1], data[p+2], data[p+3]);

	// Additive blending
	if (properties.blend == ADD) {
		d_colour.set(	MathStuff::clamp(d_colour.r+colour.r*properties.alpha, 0, 255),
						MathStuff::clamp(d_colour.g+colour.g*properties.alpha, 0, 255),
						MathStuff::clamp(d_colour.b+colour.b*properties.alpha, 0, 255),
						MathStuff::clamp(d_colour.a + colour.a, 0, 255));
	}

	// Subtractive blending
	else if (properties.blend == SUBTRACT) {
		d_colour.set(	MathStuff::clamp(d_colour.r-colour.r*properties.alpha, 0, 255),
						MathStuff::clamp(d_colour.g-colour.g*properties.alpha, 0, 255),
						MathStuff::clamp(d_colour.b-colour.b*properties.alpha, 0, 255),
						MathStuff::clamp(d_colour.a + colour.a, 0, 255));
	}

	// Reverse-Subtractive blending
	else if (properties.blend == REVERSE_SUBTRACT) {
		d_colour.set(	MathStuff::clamp((-d_colour.r)+colour.r*properties.alpha, 0, 255),
						MathStuff::clamp((-d_colour.g)+colour.g*properties.alpha, 0, 255),
						MathStuff::clamp((-d_colour.b)+colour.b*properties.alpha, 0, 255),
						MathStuff::clamp(d_colour.a + colour.a, 0, 255));
	}

	// 'Modulate' blending
	else if (properties.blend == MODULATE) {
		d_colour.set(	MathStuff::clamp(colour.r*d_colour.r / 255, 0, 255),
						MathStuff::clamp(colour.g*d_colour.g / 255, 0, 255),
						MathStuff::clamp(colour.b*d_colour.b / 255, 0, 255),
						MathStuff::clamp(d_colour.a + colour.a, 0, 255));
	}

	// Normal blending (or unknown blend type)
	else {
		float inv_alpha = 1.0f - properties.alpha;
		d_colour.set(	d_colour.r*inv_alpha + colour.r*properties.alpha,
						d_colour.g*inv_alpha + colour.g*properties.alpha,
						d_colour.b*inv_alpha + colour.b*properties.alpha,
						MathStuff::clamp(d_colour.a + colour.a, 0, 255));
	}

	// Apply new colour
	if (type == PALMASK) {
		data[p] = pal->nearestColour(d_colour);
		mask[p] = d_colour.a;
	}
	else
		d_colour.write(data+p);

	return true;
}

/* SImage::drawImage
 * Draws an image on to this image at [x],[y], with blending options
 * set in [properties]. [pal_src] is used for the source image, and
 * [pal_dest] is used for the destination image, if either is
 * paletted
 *******************************************************************/
bool SImage::drawImage(SImage& img, int x_pos, int y_pos, si_drawprops_t& properties, Palette8bit* pal_src, Palette8bit* pal_dest) {
	// Check images
	if (!data || !img.data)
		return false;

	// Setup palettes
	if (img.has_palette || !pal_src)
		pal_src = &(img.palette);
	if (has_palette || !pal_dest)
		pal_dest = &palette;

	// Go through pixels
	unsigned s_stride = img.getStride();
	uint8_t s_bpp = img.getBpp();
	unsigned sp = 0;
	for (int y = y_pos; y < y_pos + img.height; y++) {		// Rows
		// Skip out-of-bounds rows
		if (y < 0 || y >= height) {
			sp += s_stride;
			continue;
		}

		for (int x = x_pos; x < x_pos + img.width; x++) {	// Columns
			// Skip out-of-bounds columns
			if (x < 0 || x >= width) {
				sp += s_bpp;
				continue;
			}

			// Skip if source pixel is fully transparent
			if ((img.type == PALMASK && img.mask[sp] == 0) ||
				(img.type == RGBA && img.data[sp+3] == 0)) {
				sp += s_bpp;
				continue;
			}

			// Draw pixel
			if (img.type == PALMASK) {
				rgba_t col = pal_src->colour(img.data[sp]);
				col.a = img.mask[sp];
				drawPixel(x, y, col, properties, pal_dest);
			}
			else
				drawPixel(x, y, rgba_t(img.data[sp], img.data[sp+1], img.data[sp+2], img.data[sp+3]), properties, pal_dest);

			// Go to next source pixel
			sp += s_bpp;
		}
	}

	return true;
}

/* SImage::colourise
 * Colourises the image to [colour]. If the image is paletted, each
 * pixel will be set to its nearest matching colour in [pal]
 *******************************************************************/
bool SImage::colourise(rgba_t colour, Palette8bit* pal) {
	// Get palette to use
	if (has_palette || !pal)
		pal = &palette;

	// Go through all pixels
	uint8_t bpp = getBpp();
	rgba_t col;
	for (int a = 0; a < width*height*bpp; a+= bpp) {
		// Get current pixel colour
		if (type == RGBA)
			col.set(data[a], data[a+1], data[a+2], data[a+3]);
		else
			col.set(pal->colour(data[a]));

		// Colourise it
		float grey = (col.r*0.3f + col.g*0.59f + col.b*0.11f) / 255.0f;
		col.r = colour.r*grey;
		col.g = colour.g*grey;
		col.b = colour.b*grey;

		// Set pixel colour
		if (type == RGBA)
			col.write(data+a);
		else
			data[a] = pal->nearestColour(col);
	}

	return true;
}

/* SImage::tint
 * Tints the image to [colour] by [amount]. If the image is paletted,
 * each pixel will be set to its nearest matching colour in [pal]
 *******************************************************************/
bool SImage::tint(rgba_t colour, float amount, Palette8bit* pal) {
	// Get palette to use
	if (has_palette || !pal)
		pal = &palette;

	// Go through all pixels
	uint8_t bpp = getBpp();
	rgba_t col;
	for (int a = 0; a < width*height*bpp; a+= bpp) {
		// Get current pixel colour
		if (type == RGBA)
			col.set(data[a], data[a+1], data[a+2], data[a+3]);
		else
			col.set(pal->colour(data[a]));

		// Tint it
		float inv_amt = 1.0f - amount;
		col.set(col.r*inv_amt + colour.r*amount,
				col.g*inv_amt + colour.g*amount,
				col.b*inv_amt + colour.b*amount, col.a);

		// Set pixel colour
		if (type == RGBA)
			col.write(data+a);
		else
			data[a] = pal->nearestColour(col);
	}

	return true;
}
