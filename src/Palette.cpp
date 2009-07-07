
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Palette.cpp
 * Description: Palette class, handles a palette and performs various
 *              colour transformations etc
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


/* Palette::Palette
 * Palette class constructor
 *******************************************************************/
Palette::Palette() {
	index_trans = -1;

	// Init palette (to greyscale)
	for (int a = 0; a < 256; a++)
		colours[a].set(a, a, a, 255);
}

/* Palette::~Palette
 * Palette class destructor
 *******************************************************************/
Palette::~Palette() {
}

/* Palette::copyPalette
 * Copies the given palette into this one
 *******************************************************************/
void Palette::copyPalette(Palette& copy) {
	for (int a = 0; a < 256; a++)
		setColour(a, copy.colour(a));
	index_trans = copy.transIndex();
}

/* Palette::loadFromArchive
 * Loads colour information from an existing palette in the given
 * archive. Returns false if the archive didn't contain a palette,
 * true otherwise
 *******************************************************************/
bool Palette::loadFromArchive(Archive* archive) {
	ArchiveEntry* playpal = archive->getEntry(_T("PLAYPAL"));

	if (!playpal)
		return false;

	if (playpal->getSize() < 256*3)
		return false;

	uint8_t* playpal_dat = playpal->getData(true);
	int c = 0;
	for (int a = 0; a < 256; a++) {
		uint8_t r = playpal_dat[c++];
		uint8_t g = playpal_dat[c++];
		uint8_t b = playpal_dat[c++];
		colours[a].set(r, g, b, 255);
		//colours[a].set(playpal_dat[c++], playpal_dat[c++], playpal_dat[c++], 255);
	}

	return true;
}
