
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


/*******************************************************************
 * PALETTE8BIT CLASS FUNCTIONS
 *******************************************************************/

/* Palette8bit::Palette8bit
 * Palette8bit class constructor
 *******************************************************************/
Palette8bit::Palette8bit() {
	index_trans = -1;

	// Init palette (to greyscale)
	for (int a = 0; a < 256; a++)
		colours[a].set(a, a, a, 255);
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
			colours[c++].set(rgb[0], rgb[1], rgb[2], 255);
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
		colours[c++].set(data[a], data[a+1], data[a+2], 255);

		// If we have read 256 colours, finish
		if (c == 256)
			break;
	}

	return true;
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
