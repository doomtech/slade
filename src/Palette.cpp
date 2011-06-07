
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Palette8bit.cpp
 * Description: Palette8bit class, handles a 256-colour palette and
 *              performs various colour transformations etc
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
#include "Palette.h"
#include "Misc.h"
#include "Translation.h"
#include "SIFormat.h"


/*******************************************************************
 * PALETTE8BIT CLASS FUNCTIONS
 *******************************************************************/

/* Palette8bit::Palette8bit
 * Palette8bit class constructor
 *******************************************************************/
Palette8bit::Palette8bit() {
	index_trans = -1;

	// Init palette (to greyscale)
	for (int a = 0; a < 256; a++) {
		colours[a].set(a, a, a, 255);
		colours_hsl[a].l = (double)a / 255.0;
	}
}

/* Palette8bit::~Palette8bit
 * Palette8bit class destructor
 *******************************************************************/
Palette8bit::~Palette8bit() {
}

/* Palette8bit::loadMem
 * Reads colour information from raw data (MemChunk)
 *******************************************************************/
bool Palette8bit::loadMem(MemChunk& mc) {
	// Check that the given data has at least 1 colour (3 bytes)
	if (mc.getSize() < 3)
		return false;

	// Read in colours
	int c = 0;
	for (size_t a = 0; a < mc.getSize(); a += 3) {
		uint8_t rgb[3] = {0, 0, 0};

		// Read RGB value
		if (mc.read(rgb, 3)) {
			// Set colour in palette
			colours[c].set(rgb[0], rgb[1], rgb[2], 255);
			colours_hsl[c++] = Misc::rgbToHsl((double)rgb[0]/255.0, (double)rgb[1]/255.0, (double)rgb[2]/255.0);
		}

		// If we have read 256 colours, finish
		if (c == 256)
			break;
	}

	return true;
}

/* Palette8bit::loadMem
 * Reads colour information from raw data
 *******************************************************************/
bool Palette8bit::loadMem(const uint8_t* data, uint32_t size) {
	// Check that the given data has at least 1 colour (3 bytes)
	if (size < 3)
		return false;

	// Read in colours
	int c = 0;
	for (size_t a = 0; a < size; a += 3) {
		// Set colour in palette
		colours[c].set(data[a], data[a+1], data[a+2], 255);
		colours_hsl[c++] = Misc::rgbToHsl((double)data[a]/255.0, (double)data[a+1]/255.0, (double)data[a+2]/255.0);

		// If we have read 256 colours, finish
		if (c == 256)
			break;
	}

	return true;
}

/* Palette8bit::saveMem
 * Writes colour information to a MemChunk
 *******************************************************************/
bool Palette8bit::saveMem(MemChunk& mc, int format) {
	// Clear memchunk
	mc.clear();

	// Write to requested format

	// Raw data
	if (format == FORMAT_RAW) {
		mc.reSize(768);
		for (unsigned a = 0; a < 256; a++)
			mc.write(&colours[a], 3);
	}

	// CSV
	else if (format == FORMAT_CSV) {
		string csv;
		for (unsigned a = 0; a < 256; a++)
			csv += S_FMT("%d, %d, %d\n", colours[a].r, colours[a].g, colours[a].b);
		mc.importMem((const uint8_t*)((const char*)csv.ToAscii()), csv.Length());
	}

	// JASC palette
	else if (format == FORMAT_JASC) {
		string jasc = "JASC-PAL\n0100\n256\n";
		for (unsigned a = 0; a < 256; a++)
			jasc += S_FMT("%d %d %d\n", colours[a].r, colours[a].g, colours[a].b);
		mc.importMem((const uint8_t*)((const char*)jasc.ToAscii()), jasc.Length());
	}

	// Image
	else if (format == FORMAT_IMAGE) {
		SImage image;

		// Generate palette image
		image.create(128, 128, PALMASK, this);
		unsigned xoff = 0;
		unsigned yoff = 0;
		for (unsigned a = 0; a < 256; a++) {
			// Draw colour square
			for (unsigned y = 0; y < 8; y++) {
				for (unsigned x = 0; x < 8; x++)
					image.setPixel(xoff+x, yoff+y, a, 255);
			}
			
			// Go to next square
			xoff += 8;
			if (xoff >= 128) {
				xoff = 0;
				yoff += 8;
			}
		}

		// Write PNG format
		SIFormat::getFormat("png")->saveImage(image, mc, this);
	}

	// Invalid format
	else
		return false;

	return true;
}

/* Palette8bit::saveFile
 * Writes colour information to a file at [filename]. Returns false
 * if the file could not be opened/created, true otherwise
 *******************************************************************/
bool Palette8bit::saveFile(string filename, int format) {
	// Write data to MemChunk
	MemChunk mc;
	if (!saveMem(mc, format))
		return false;

	// Write MemChunk to file
	return mc.exportFile(filename);
}

/* Palette8bit::setColour
 * Sets the colour at [index]
 *******************************************************************/
void Palette8bit::setColour(uint8_t index, rgba_t col) {
	colours[index].set(col);
	colours_hsl[index] = Misc::rgbToHsl(col.dr(), col.dg(), col.db());
}

/* Palette8bit::setColour
 * Sets the colour at [index]'s red component
 *******************************************************************/
void Palette8bit::setColourR(uint8_t index, uint8_t val) {
	colours[index].r = val;
	colours_hsl[index] = Misc::rgbToHsl(colours[index].dr(), colours[index].dg(), colours[index].db());
}

/* Palette8bit::setColour
 * Sets the colour at [index]'s green component
 *******************************************************************/
void Palette8bit::setColourG(uint8_t index, uint8_t val) {
	colours[index].g = val;
	colours_hsl[index] = Misc::rgbToHsl(colours[index].dr(), colours[index].dg(), colours[index].db());
}

/* Palette8bit::setColour
 * Sets the colour at [index]'s blue component
 *******************************************************************/
void Palette8bit::setColourB(uint8_t index, uint8_t val) {
	colours[index].b = val;
	colours_hsl[index] = Misc::rgbToHsl(colours[index].dr(), colours[index].dg(), colours[index].db());
}

/* Palette8bit::copyPalette8bit
 * Copies the given palette into this one
 *******************************************************************/
void Palette8bit::copyPalette(Palette8bit* copy) {
	if (!copy)
		return;

	for (int a = 0; a < 256; a++)
		setColour(a, copy->colour(a));

	index_trans = copy->transIndex();
}

/* Palette8bit::findColour
 * Returns the index of the colour in the palette matching [colour],
 * or -1 if no match is found
 *******************************************************************/
short Palette8bit::findColour(rgba_t colour) {
	for (int a = 0; a < 256; a++) {
		if (colours[a].equals(colour))
			return a;
	}

	return -1;
}

/* Palette8bit::nearestColour
 * Returns the index of the closest colour in the palette to [colour]
 *******************************************************************/
short Palette8bit::nearestColour(rgba_t colour) {
	int min_d = 999999;
	short index = 0;

	int d_r, d_g, d_b, diff;
	for (short a = 0; a < 256; a++) {
		d_r = colour.r - colours[a].r;
		d_g = colour.g - colours[a].g;
		d_b = colour.b - colours[a].b;
		diff = (d_r*d_r)+(d_g*d_g)+(d_b*d_b);

		// Exact match?
		if (diff == 0)
			return a;
		else if (diff < min_d) {
			min_d = diff;
			index = a;
		}
	}

	return index;
}

/* Palette8bit::countColours
 * Returns the number of unique colors in a palette
 *******************************************************************/
size_t Palette8bit::countColours() {
	rgba_t * usedcolours = new rgba_t[256];
	memset(usedcolours, 0, 256*sizeof(rgba_t));
	size_t used = 0;

	for (int a = 0; a < 256; a++) {
		bool found = false;
		for (size_t b = 0; b < used; b++) {
			if (colours[a].equals(usedcolours[b])) {
				found = true;
				break;
			}
		}
		if (!found)
			usedcolours[used++].set(colours[a]);
	}
	delete[] usedcolours;
	return used;
}

/* Palette8bit::applyTranslation
 * Applies the translation [trans] to this palette
 *******************************************************************/
void Palette8bit::applyTranslation(Translation* trans) {
	// Check translation was given
	if (!trans)
		return;

	// Duplicate palette (so translation ranges don't interfere with eachother)
	Palette8bit temp;
	temp.copyPalette(this);

	// Go through each translation component
	for (unsigned a = 0; a < trans->nRanges(); a++) {
		TransRange* r = trans->getRange(a);

		// Palette range translation
		if (r->getType() == TRANS_PALETTE) {
			TransRangePalette* tp = (TransRangePalette*)r;

			// Do remap
			for (unsigned i = tp->oStart(); i <= tp->oEnd(); i++) {
				// Figure out how far along the range this colour is
				double range_frac = 0;
				if (tp->oStart() != tp->oEnd())
					range_frac = double(i - tp->oStart()) / double(tp->oEnd() - tp->oStart());

				// Determine destination palette index
				uint8_t di = tp->dStart() + range_frac * (tp->dEnd() - tp->dStart());

				// Apply new colour
				temp.setColour(i, colours[di]);
			}
		}

		// Colour range
		else if (r->getType() == TRANS_COLOUR) {
			TransRangeColour* tc = (TransRangeColour*)r;

			// Do remap
			for (unsigned i = tc->oStart(); i <= tc->oEnd(); i++) {
				// Figure out how far along the range this colour is
				double range_frac = 0;
				if (tc->oStart() != tc->oEnd())
					range_frac = double(i - tc->oStart()) / double(tc->oEnd() - tc->oStart());

				// Determine destination colour
				uint8_t r = tc->dStart().r + range_frac * (tc->dEnd().r - tc->dStart().r);
				uint8_t g = tc->dStart().g + range_frac * (tc->dEnd().g - tc->dStart().g);
				uint8_t b = tc->dStart().b + range_frac * (tc->dEnd().b - tc->dStart().b);

				// Find nearest colour in palette
				uint8_t di = nearestColour(rgba_t(r, g, b));

				// Apply new colour
				temp.setColour(i, colours[di]);
			}
		}

		// Desaturated colour range
		else if (r->getType() == TRANS_DESAT) {
			TransRangeDesat* td = (TransRangeDesat*)r;

			// Do remap
			for (unsigned i = td->oStart(); i <= td->oEnd(); i++) {
				// Get greyscale colour
				float grey = (colours[i].r*0.3f + colours[i].g*0.59f + colours[i].b*0.11f) / 255.0f;

				// Determine destination colour
				uint8_t r = MIN(255, int((td->dSr() + grey*(td->dEr() - td->dSr()))*255.0f));
				uint8_t g = MIN(255, int((td->dSg() + grey*(td->dEg() - td->dSg()))*255.0f));
				uint8_t b = MIN(255, int((td->dSb() + grey*(td->dEb() - td->dSb()))*255.0f));

				// Find nearest colour in palette
				uint8_t di = nearestColour(rgba_t(r, g, b));

				// Apply new colour
				temp.setColour(i, colours[di]);
			}
		}
	}

	// Load translated palette
	copyPalette(&temp);
}
