
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


bool Misc::loadImageFromEntry(SImage* image, ArchiveEntry* entry) {
	switch (entry->getType()) {
		// General image formats
		case ETYPE_PNG:
			return image->loadImage(entry->getData(true), entry->getSize());
		case ETYPE_IMAGE:
			return image->loadImage(entry->getData(true), entry->getSize());
		// Doom gfx format
		case ETYPE_GFX:
			return image->loadDoomGfx(entry->getData(true), entry->getSize());
		case ETYPE_PATCH:
			return image->loadDoomGfx(entry->getData(true), entry->getSize());
		case ETYPE_SPRITE:
			return image->loadDoomGfx(entry->getData(true), entry->getSize());
		// Doom flat format
		case ETYPE_FLAT:
			return image->loadDoomFlat(entry->getData(true), entry->getSize());
		case ETYPE_GFX2:
			return image->loadDoomFlat(entry->getData(true), entry->getSize());
	}

	// Unknown image type
	Global::error = _T("Entry is not a known image format");
	return false;
}

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
	uint8_t* playpal_dat = playpal->getData(true);
	int c = 0;
	for (int a = 0; a < 256; a++) {
		uint8_t r = playpal_dat[c++];
		uint8_t g = playpal_dat[c++];
		uint8_t b = playpal_dat[c++];
		pal->setColour(a, rgba_t(r, g,  b, 255));
	}

	return true;
}

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
