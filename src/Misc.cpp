
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Misc.cpp
 * Description: Misc functions that don't necessarily belong
 *              anywhere else (generally stuff that involves multiple
 *              unrelated classes)
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
#include "Misc.h"


/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

/* Misc::loadImageFromEntry
 * Loads an image from <entry> into <image>. Returns false if the
 * given entry wasn't a valid image, true otherwise
 *******************************************************************/
bool Misc::loadImageFromEntry(SImage* image, ArchiveEntry* entry) {
	if (!entry)
		return false;

	switch (entry->getType()->getFormat()) {
		// Doom gfx format
		case EDF_GFX_DOOM:
			return image->loadDoomGfx(entry->getData(true), entry->getSize());
		// Alpha Doom gfx format
		case EDF_GFX_DOOM_ALPHA:
			return image->loadDoomGfx(entry->getData(true), entry->getSize(), 2);
		// Beta Doom gfx format
		case EDF_GFX_DOOM_BETA:
			return image->loadDoomGfx(entry->getData(true), entry->getSize(), 1);
		// Doom flat format
		case EDF_GFX_FLAT:
			return image->loadDoomFlat(entry->getData(true), entry->getSize());
		// General image formats (that FreeImage supports at least)
		default:
			if (!image->loadImage(entry->getData(true), entry->getSize())) {
				Global::error = _T("Image format not supported by FreeImage");
				return false;
			}
			else
				return true;
	}

	// Unknown image type
	Global::error = _T("Entry is not a known image format");
	return false;
}

/* Misc::loadPaletteFromArchive
 * Writes palette information from the PLAYPAL entry in <archive> to
 * <pal>. Returns false if PLAYPAL entry was missing or invalid,
 * true otherwise
 *******************************************************************/
bool Misc::loadPaletteFromArchive(Palette8bit* pal, Archive* archive) {
	// Check parameters
	if (!pal || !archive)
		return false;

	// Find PLAYPAL entry
	ArchiveEntry* playpal = archive->getEntry(_T("PLAYPAL"));

	// Check it was found
	if (!playpal)
		return false;

	// Check it is the correct size
	if (playpal->getSize() < 256*3)
		return false;

	// Read palette colours
	const uint8_t* playpal_dat = playpal->getData(true);
	int c = 0;
	for (int a = 0; a < 256; a++) {
		uint8_t r = playpal_dat[c++];
		uint8_t g = playpal_dat[c++];
		uint8_t b = playpal_dat[c++];
		pal->setColour(a, rgba_t(r, g,  b, 255));
	}

	return true;
}

/* Misc::sizeAsString
 * Converts <size> to a string representing it as a 'bytes' size, ie
 * "1.24kb", "4.00mb". Sizes under 1kb aren't given an appendage
 *******************************************************************/
string Misc::sizeAsString(uint32_t size) {
	if (size < 1024) {
		return s_fmt(_T("%d"), size);
	}
	else if (size < 1024*1024) {
		double kb = (double)size / 1024;
		return s_fmt(_T("%1.2fkb"), kb);
	}
	else {
		double mb = (double)size / (1024*1024);
		return s_fmt(_T("%1.2fmb"), mb);
	}

	return _T("");
}
