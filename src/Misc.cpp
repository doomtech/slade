
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Misc.cpp
 * Description: Misc functions that don't necessarily belong
 *              anywhere else
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
