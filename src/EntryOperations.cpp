
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

struct alph_chunk_t {
	char name[4];
};

struct trns_chunk_t {
	char name[4];
	uint8_t entries[256];
};

struct trans_chunk_t {
	char name[5];
};

struct chunk_size_t {
	uint32_t size;
};

/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

/* EntryOperations::modifyGfxOffsets
 * Changes the offsets of the given gfx entry. Returns false if the
 * entry is invalid or not an offset-supported format, true otherwise
 *******************************************************************/
bool EntryOperations::modifyGfxOffsets(ArchiveEntry* entry, int auto_type, point2_t offsets, bool xc, bool yc, bool relative) {
	if (entry == NULL || entry->getType() == NULL)
		return false;

	// Check entry type
	EntryType* type = entry->getType();
	string entryformat = type->getFormat();
	if (!(entryformat == "img_doom" || entryformat == "img_doom_arah" ||
		entryformat == "img_doom_alpha" || "img_doom_beta" || entryformat == "img_png")) {
		wxLogMessage(s_fmt("Entry \"%s\" is of type \"%s\" which does not support offsets", chr(entry->getName()), chr(entry->getType()->getName())));
		return false;
	}

	// Doom gfx format, normal and beta version.
	// Also arah format from alpha 0.2 because it uses the same header format.
	if (entryformat == "img_doom" || entryformat == "img_doom_beta" || entryformat == "image_doom_arah")
	{
		// Get patch header
		patch_header_t header;
		entry->seek(0, SEEK_SET);
		entry->read(&header, 8);
		//patch_header_t* header = (patch_header_t*)entry->getData(true);

		// Apply new offsets
		if (auto_type >= 0) {
			// Auto Offsets selected
			int w = wxINT16_SWAP_ON_BE(header.width);
			int h = wxINT16_SWAP_ON_BE(header.height);

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
				header.left = wxINT16_SWAP_ON_BE((int16_t) offsets.x);

			if (yc)
				header.top = wxINT16_SWAP_ON_BE((int16_t) offsets.y);
		}

		// Write new header to entry
		entry->seek(0, SEEK_SET);
		entry->write(&header, 8);
	}

	// Doom alpha gfx format
	else if (entryformat == "img_doom_alpha") {
		// Get patch header
		entry->seek(0, SEEK_SET);
		oldpatch_header_t header;
		entry->read(&header, 4);

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
				header.left = (int8_t) offsets.x;

			if (yc)
				header.top = (int8_t) offsets.y;
		}

		// Write new header to entry
		entry->seek(0, SEEK_SET);
		entry->write(&header, 4);
	}

	// PNG format
	else if (entryformat == "img_png") {
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
		grab_chunk_t gc = { { 'g', 'r', 'A', 'b' }, wxINT32_SWAP_ON_LE(xoff), wxINT32_SWAP_ON_LE(yoff) };
		uint32_t dcrc = wxUINT32_SWAP_ON_LE(Misc::crc((uint8_t*)&gc, 12));

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

		// Set it's type back to png
		entry->setType(type);
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

/* EntryOperations::modifyalPhChunk
 * Add or remove the alPh chunk from a PNG entry
 *******************************************************************/
bool EntryOperations::modifyalPhChunk(ArchiveEntry* entry, bool value) {
	if (!entry || !entry->getType())
		return false;

	// Don't bother if the entry is locked.
	if (entry->isLocked())
		return false;

	// Check entry type
	if (!(entry->getType()->getFormat() == "img_png")) {
		wxLogMessage(s_fmt("Entry \"%s\" is of type \"%s\" rather than PNG", chr(entry->getName()), chr(entry->getType()->getName())));
		return false;
	}

	// Read width and height from IHDR chunk
	const uint8_t* data = entry->getData(true);
	const ihdr_t* ihdr = (ihdr_t*)(data + 12);

	// Find existing alPh chunk
	uint32_t alph_start = 0;
	for (uint32_t a = 0; a < entry->getSize(); a++) {
		// Check for 'alPh' header
		if (data[a] == 'a' && data[a + 1] == 'l' &&
			data[a + 2] == 'P' && data[a + 3] == 'h') {
			alph_start = a - 4;
			break;
		}

		// Stop when we get to the 'IDAT' chunk
		if (data[a] == 'I' && data[a + 1] == 'D' &&
			data[a + 2] == 'A' && data[a + 3] == 'T')
			break;
	}

	// We want to set alPh, and it is already there: nothing to do.
	if (value && alph_start > 0)
		return false;

	// We want to unset alPh, and it is already not there: nothing to do either.
	else if (!value && alph_start == 0)
		return false;

	// We want to set alPh, which is missing: create it.
	else if (value && alph_start == 0)
	{
		// Build new PNG from the original w/ the new alPh chunk
		MemChunk npng;

		// Init new png data size
		npng.reSize(entry->getSize() + 12);

		// Write PNG header and IHDR chunk
		npng.write(data, 33);

		// Create new alPh chunk
		uint32_t csize = wxUINT32_SWAP_ON_LE(0);
		alph_chunk_t gc = { 'a', 'l', 'P', 'h' };
		uint32_t dcrc = wxUINT32_SWAP_ON_LE(Misc::crc((uint8_t*)&gc, 4));

		// Create alPh chunk
		npng.write(&csize, 4);
		npng.write(&gc, 4);
		npng.write(&dcrc, 4);

		// Write the rest of the PNG data
		uint32_t to_write = entry->getSize() - 33;
		npng.write(data + 33, to_write);

		// Load new png data to the entry
		entry->importMemChunk(npng);
	}

	// We want to unset alPh, which is present: delete it.
	else if (!value && alph_start > 0)
	{
		// Build new PNG from the original without the alPh chunk
		MemChunk npng;
		uint32_t rest_start = alph_start + 12;

		// Init new png data size
		npng.reSize(entry->getSize() - 12);

		// Write PNG info before alPh chunk
		npng.write(data, alph_start);

		// Write the rest of the PNG data
		uint32_t to_write = entry->getSize() - rest_start;
		npng.write(data + rest_start, to_write);

		// Load new png data to the entry
		entry->importMemChunk(npng);
	}

	// We don't know what we want, but it can't be good, so we do nothing.
	else
		return false;

	return true;
}

/* EntryOperations::modifytRNSChunk
 * Add or remove the tRNS chunk from a PNG entry
 * Returns true if the entry was altered
 *******************************************************************/
bool EntryOperations::modifytRNSChunk(ArchiveEntry* entry, bool value) {
	// Avoid NULL pointers, they're annoying.
	if (!entry || !entry->getType())
		return false;

	// Don't bother if the entry is locked.
	if (entry->isLocked())
		return false;

	// Check entry type
	if (!(entry->getType()->getFormat() == "img_png")) {
		wxLogMessage(s_fmt("Entry \"%s\" is of type \"%s\" rather than PNG", entry->getName().c_str(), entry->getTypeString().c_str()));
		return false;
	}

	// Read width and height from IHDR chunk
	const uint8_t* data = entry->getData(true);
	const ihdr_t* ihdr = (ihdr_t*)(data + 12);

	// tRNS chunks are only valid for paletted PNGs, and must be before the first IDAT.
	// Specs say they must be after PLTE chunk as well, so to play it safe, we'll insert
	// them just before the first IDAT.
	uint32_t trns_start = 0;
	uint8_t  trns_size	= 0;
	uint32_t idat_start = 0;
	for (uint32_t a = 0; a < entry->getSize(); a++) {

		// Check for 'tRNS' header
		if (data[a] == 't' && data[a + 1] == 'R' &&
			data[a + 2] == 'N' && data[a + 3] == 'S') {
			trns_start = a - 4;
			trans_chunk_t* trns = (trans_chunk_t*)(data + a);
			trns_size = 12 + (uint8_t)data[a-1];
		}

		// Stop when we get to the 'IDAT' chunk
		if (data[a] == 'I' && data[a + 1] == 'D' &&
			data[a + 2] == 'A' && data[a + 3] == 'T')
		{
			idat_start = a - 4;
			break;
		}
	}

	// The IDAT chunk starts before the header is finished, this doesn't make sense, abort.
	if (idat_start < 33)
		return false;

	// We want to set tRNS, and it is already there: nothing to do.
	if (value && trns_start > 0)
		return false;

	// We want to unset tRNS, and it is already not there: nothing to do either.
	else if (!value && trns_start == 0)
		return false;

	// We want to set tRNS, which is missing: create it. We're just going to set index 0 to 0,
	// and leave the rest of the palette indices alone.
	else if (value && trns_start == 0)
	{
		// Build new PNG from the original w/ the new tRNS chunk
		MemChunk npng;

		// Init new png data size
		npng.reSize(entry->getSize() + 13);

		// Write PNG header stuff up to the first IDAT chunk
		npng.write(data, idat_start);

		// Create new tRNS chunk
		uint32_t csize = wxUINT32_SWAP_ON_LE(1);
		trans_chunk_t gc = { 't', 'R', 'N', 'S', '\0' };
		uint32_t dcrc = wxUINT32_SWAP_ON_LE(Misc::crc((uint8_t*)&gc, 5));

		// Write tRNS chunk
		npng.write(&csize, 4);
		npng.write(&gc, 5);
		npng.write(&dcrc, 4);

		// Write the rest of the PNG data
		uint32_t to_write = entry->getSize() - idat_start;
		npng.write(data + idat_start, to_write);

		// Load new png data to the entry
		entry->importMemChunk(npng);
	}

	// We want to unset tRNS, which is present: delete it.
	else if (!value && trns_start > 0)
	{
		// Build new PNG from the original without the tRNS chunk
		MemChunk npng;
		uint32_t rest_start = trns_start + trns_size;

		// Init new png data size
		npng.reSize(entry->getSize() - trns_size);

		// Write PNG header and stuff up to tRNS start
		npng.write(data, trns_start);

		// Write the rest of the PNG data
		uint32_t to_write = entry->getSize() - rest_start;
		npng.write(data + rest_start, to_write);

		// Load new png data to the entry
		entry->importMemChunk(npng);
	}

	// We don't know what we want, but it can't be good, so we do nothing.
	else
		return false;

	return true;
}

/* EntryOperations::getalPhChunk
 * Tell whether a PNG entry has an alPh chunk or not
 *******************************************************************/
bool EntryOperations::getalPhChunk(ArchiveEntry* entry) {
	if (!entry || !entry->getType())
		return false;

	// Check entry type
	if (entry->getType()->getFormat() != "img_png") {
		wxLogMessage(s_fmt("Entry \"%s\" is of type \"%s\" rather than PNG", entry->getName().c_str(), entry->getTypeString().c_str()));
		return false;
	}

	// Find existing alPh chunk
	const uint8_t* data = entry->getData(true);
	for (uint32_t a = 0; a < entry->getSize(); a++) {
		// Check for 'alPh' header
		if (data[a] == 'a' && data[a + 1] == 'l' &&
			data[a + 2] == 'P' && data[a + 3] == 'h') {
			return true;
		}

		// Stop when we get to the 'IDAT' chunk
		if (data[a] == 'I' && data[a + 1] == 'D' &&
			data[a + 2] == 'A' && data[a + 3] == 'T')
			break;
	}
	return false;
}

/* EntryOperations::gettRNSChunk
 * Add or remove the tRNS chunk from a PNG entry
 *******************************************************************/
bool EntryOperations::gettRNSChunk(ArchiveEntry* entry) {
	if (!entry || !entry->getType())
		return false;

	// Check entry type
	if (entry->getType()->getFormat() != "img_png") {
		wxLogMessage(s_fmt("Entry \"%s\" is of type \"%s\" rather than PNG", entry->getName().c_str(), entry->getTypeString().c_str()));
		return false;
	}

	// tRNS chunks are only valid for paletted PNGs, and the chunk must before the first IDAT.
	// Specs say it should be after a PLTE chunk, but that's not always the case (e.g., sgrna7a3.png).
	const uint8_t* data = entry->getData(true);
	for (uint32_t a = 0; a < entry->getSize(); a++) {

		// Check for 'tRNS' header
		if (data[a] == 't' && data[a + 1] == 'R' &&
			data[a + 2] == 'N' && data[a + 3] == 'S') {
			return true;
		}

		// Stop when we get to the 'IDAT' chunk
		if (data[a] == 'I' && data[a + 1] == 'D' &&
			data[a + 2] == 'A' && data[a + 3] == 'T')
			break;
	}
	return false;
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
				wxLogMessage(s_fmt("Entry %s not found"), args[a].c_str());
		}
	}
}
ConsoleCommand test_ee("test_ee", &c_test_ee, 1);
