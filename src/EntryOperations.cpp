
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    EntryOperations.cpp
 * Description: Functions that perform specific operations on entries
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
#include "EntryOperations.h"
#include "Misc.h"
#include "Console.h"
#include "ArchiveManager.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/

// Define some png chunk structures
struct ihdr_t {
	uint32_t id;
	uint32_t width;
	uint32_t height;
	uint8_t cinfo[5];
};

struct grab_chunk_t {
	char name[4];
	int32_t xoff;
	int32_t yoff;
};


/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

/* EntryOperations::modifyGfxOffsets
 * Changes the offsets of the given gfx entry. Returns false if the
 * entry is invalid or not an offset-supported format, true otherwise
 *******************************************************************/
bool EntryOperations::modifyGfxOffsets(ArchiveEntry* entry, int auto_type, point2_t offsets, bool xc, bool yc, bool relative) {
	if (!entry)
		return false;

	// Check entry type
	int type = entry->getType();
	if (!(type == ETYPE_GFX || type == ETYPE_PATCH ||
		type == ETYPE_SPRITE || type == ETYPE_PNG)) {
		wxLogMessage(s_fmt(_T("Entry \"%s\" is of type \"%s\" which does not support offsets"), entry->getName().c_str(), entry->getTypeString().c_str()));
		return false;
	}

	// Doom gfx format
	if (type == ETYPE_GFX || type == ETYPE_PATCH || type == ETYPE_SPRITE) {
		// Get patch header
		patch_header_t header;
		entry->read(&header, 8);
		//patch_header_t* header = (patch_header_t*)entry->getData(true);

		// Apply new offsets
		if (auto_type >= 0) {
			// Auto Offsets selected
			int w = header.width;
			int h = header.height;

			if (auto_type == 0) {			// Monster
				header.left = w * 0.5;
				header.top = h - 4;
			}
			else if (auto_type == 1) {		// Projectile
				header.left = w * 0.5;
				header.top = h * 0.5;
			}
			else if (auto_type == 2) {		// Weapon
				header.left = -160 + (w * 0.5);
				header.top = -200 + h;
			}
		}
		else {
			// Set Offsets selected
			if (relative) {
				offsets.x += header.left;
				offsets.y += header.top;
			}

			if (xc)
				header.left = offsets.x;

			if (yc)
				header.top = offsets.y;
		}

		// Write new header to entry
		entry->seek(0, SEEK_SET);
		entry->write(&header, 8);
	}

	// PNG format
	else if (type == ETYPE_PNG) {
		// Read width and height from IHDR chunk
		const uint8_t* data = entry->getData(true);
		const ihdr_t* ihdr = (ihdr_t*)(data + 12);
		uint32_t w = wxINT32_SWAP_ON_LE(ihdr->width);
		uint32_t h = wxINT32_SWAP_ON_LE(ihdr->height);

		// Find existing grAb chunk
		uint32_t grab_start = 0;
		int32_t xoff = 0;
		int32_t yoff = 0;
		for (uint32_t a = 0; a < entry->getSize(); a++) {
			// Check for 'grAb' header
			if (data[a] == 'g' && data[a + 1] == 'r' &&
				data[a + 2] == 'A' && data[a + 3] == 'b') {
				grab_start = a - 4;
				grab_chunk_t* grab = (grab_chunk_t*)(data + a);
				xoff = wxINT32_SWAP_ON_LE(grab->xoff);
				yoff = wxINT32_SWAP_ON_LE(grab->yoff);
				break;
			}

			// Stop when we get to the 'IDAT' chunk
			if (data[a] == 'I' && data[a + 1] == 'D' &&
				data[a + 2] == 'A' && data[a + 3] == 'T')
				break;
		}

		// Apply new offsets
		if (auto_type >= 0) {
			// Auto Offsets selected
			if (auto_type == 0) {			// Monster
				xoff = w * 0.5;
				yoff = h - 4;
			}
			else if (auto_type == 1) {		// Projectile
				xoff = w * 0.5;
				yoff = h * 0.5;
			}
			else if (auto_type == 2) {		// Weapon
				xoff = -160 + (w * 0.5);
				yoff = -200 + h;
			}
		}
		else {
			// Set Offsets selected
			if (relative) {
				offsets.x += xoff;
				offsets.y += yoff;
			}

			if (xc)
				xoff = offsets.x;

			if (yc)
				yoff = offsets.y;
		}

		// Create new grAb chunk
		uint32_t csize = wxUINT32_SWAP_ON_LE(8);
		grab_chunk_t gc = { 'g', 'r', 'A', 'b', wxINT32_SWAP_ON_LE(xoff), wxINT32_SWAP_ON_LE(yoff) };
		uint32_t dcrc = wxUINT32_SWAP_ON_LE(crc((uint8_t*)&gc, 12));

		// Build new PNG from the original w/ the new grAb chunk
		MemChunk npng;
		uint32_t rest_start = 33;

		// Init new png data size
		if (grab_start == 0)
			npng.reSize(entry->getSize() + 20);
		else
			npng.reSize(entry->getSize());

		// Write PNG header and IHDR chunk
		npng.write(data, 33);

		// If no existing grAb chunk was found, write new one here
		if (grab_start == 0) {
			npng.write(&csize, 4);
			npng.write(&gc, 12);
			npng.write(&dcrc, 4);
		}
		else {
			// Otherwise write any other data before the existing grAb chunk
			uint32_t to_write = grab_start - 33;
			npng.write(data + 33, to_write);
			rest_start = grab_start + 20;

			// And now write the new grAb chunk
			npng.write(&csize, 4);
			npng.write(&gc, 12);
			npng.write(&dcrc, 4);
		}

		// Write the rest of the PNG data
		uint32_t to_write = entry->getSize() - rest_start;
		npng.write(data + rest_start, to_write);

		// Load new png data to the entry
		entry->importMemChunk(npng);
	}
	else
		return false;

	return true;
}

bool EntryOperations::openExternal(ArchiveEntry* entry) {
	if (!entry)
		return false;

	return true;
}


/*******************************************************************
 * CONSOLE COMMANDS
 *******************************************************************/

void c_test_ee(vector<string> args) {
	if (theArchiveManager->numArchives() > 0) {
		for (size_t a = 0; a < args.size(); a++) {
			ArchiveEntry* entry = theArchiveManager->getArchive(0)->getEntry(args[a]);
			if (entry)
				EntryOperations::openExternal(entry);
			else
				wxLogMessage(s_fmt(_T("Entry %s not found"), args[a].c_str()));
		}
	}
}
ConsoleCommand test_ee(_T("test_ee"), &c_test_ee, 1);
