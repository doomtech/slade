
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
#include "WxStuff.h"
#include "EntryOperations.h"
#include "Misc.h"
#include "Console.h"
#include "ArchiveManager.h"
#include "TextureXEditor.h"
#include "EntryDataFormat.h"
#include "ExtMessageDialog.h"
#include "MainWindow.h"
#include <wx/filename.h>
#include <wx/utils.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(String, path_acc, "", CVAR_SAVE);


/*******************************************************************
 * STRUCTS
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
		wxLogMessage(S_FMT("Entry \"%s\" is of type \"%s\" which does not support offsets", CHR(entry->getName()), CHR(entry->getType()->getName())));
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

		// Set its type back to png
		entry->setType(type);
	}
	else
		return false;

	return true;
}

/* EntryOperations::openExternal
 * Opens [entry] in the default OS program for its data type
 *******************************************************************/
bool EntryOperations::openExternal(ArchiveEntry* entry) {
	if (!entry)
		return false;

	// Build entry filename
	wxFileName fn;
	fn.SetFullName(entry->getName(false));
	if (entry->getType() != EntryType::unknownType())
		fn.SetExt(entry->getType()->getExtension());

	// Export to file
	string path = appPath(fn.GetFullName(), DIR_TEMP);
	entry->exportFile(path);

	// Open the file externally
	wxLaunchDefaultApplication(path);

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
		wxLogMessage(S_FMT("Entry \"%s\" is of type \"%s\" rather than PNG", CHR(entry->getName()), CHR(entry->getType()->getName())));
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
		wxLogMessage(S_FMT("Entry \"%s\" is of type \"%s\" rather than PNG", entry->getName().c_str(), entry->getTypeString().c_str()));
		return false;
	}

	// Read width and height from IHDR chunk
	const uint8_t* data = entry->getData(true);
	const ihdr_t* ihdr = (ihdr_t*)(data + 12);

	// tRNS chunks are only valid for paletted PNGs, and must be before the first IDAT.
	// Specs say they must be after PLTE chunk as well, so to play it safe, we'll insert
	// them just before the first IDAT.
	uint32_t trns_start = 0;
	uint32_t trns_size	= 0;
	uint32_t idat_start = 0;
	for (uint32_t a = 0; a < entry->getSize(); a++) {

		// Check for 'tRNS' header
		if (data[a] == 't' && data[a + 1] == 'R' &&
			data[a + 2] == 'N' && data[a + 3] == 'S') {
			trns_start = a - 4;
			trans_chunk_t* trns = (trans_chunk_t*)(data + a);
			trns_size = 12 + READ_B32(data, a-4);
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
		wxLogMessage(S_FMT("Entry \"%s\" is of type \"%s\" rather than PNG", entry->getName().c_str(), entry->getTypeString().c_str()));
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
		wxLogMessage(S_FMT("Entry \"%s\" is of type \"%s\" rather than PNG", entry->getName().c_str(), entry->getTypeString().c_str()));
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

/* EntryOperations::addToPatchTable
 * Adds all [entries] to their parent archive's patch table, if it
 * exists. If not, the user is prompted to create or import texturex
 * entries
 *******************************************************************/
bool EntryOperations::addToPatchTable(vector<ArchiveEntry*> entries) {
	// Check any entries were given
	if (entries.size() == 0)
		return true;

	// Get parent archive
	Archive* parent = entries[0]->getParent();

	// Find patch table in parent archive
	Archive::search_options_t opt;
	opt.match_type = EntryType::getType("pnames");
	ArchiveEntry* pnames = parent->findLast(opt);

	// Check it exists
	if (!pnames) {
		// Create texture entries
		if (!TextureXEditor::setupTextureEntries(parent))
			return false;

		pnames = parent->findLast(opt);
	}

	// Check it isn't locked (texturex editor open or iwad)
	if (pnames->isLocked()) {
		if (parent->isReadOnly())
			wxMessageBox("Cannot perform this action on an IWAD", "Error", wxICON_ERROR);
		else
			wxMessageBox("Cannot perform this action because one or more texture related entries is locked. Please close the archive's texture editor if it is open.", "Error", wxICON_ERROR);

		return false;
	}

	// Load to patch table
	PatchTable ptable;
	ptable.loadPNAMES(pnames);

	// Add entry names to patch table
	for (unsigned a = 0; a < entries.size(); a++) {
		// Check entry type
		if (!(entries[a]->getType()->extraProps().propertyExists("image"))) {
			wxLogMessage("Entry %s is not a valid image", entries[a]->getName());
			continue;
		}

		// Check entry name
		if (entries[a]->getName(true).Length() > 8) {
			wxLogMessage("Entry %s has too long a name to add to the patch table (name must be 8 characters max)", entries[a]->getName());
			continue;
		}

		ptable.addPatch(entries[a]->getName(true));
	}

	// Write patch table data back to pnames entry
	return ptable.writePNAMES(pnames);
}

/* EntryOperations::createTexture
 * Same as addToPatchTable, but also creates a single-patch texture
 * from each added patch
 *******************************************************************/
bool EntryOperations::createTexture(vector<ArchiveEntry*> entries) {
	// Check any entries were given
	if (entries.size() == 0)
		return true;

	// Get parent archive
	Archive* parent = entries[0]->getParent();

	// Create texture entries if needed
	if (!TextureXEditor::setupTextureEntries(parent))
		return false;

	// Find patch table in parent archive
	Archive::search_options_t opt;
	opt.match_type = EntryType::getType("pnames");
	ArchiveEntry* pnames = parent->findLast(opt);

	// Check it exists
	if (!pnames)
		return false;

	// Find texturex entry to add to
	opt.match_type = EntryType::getType("texturex");
	ArchiveEntry* texturex = parent->findFirst(opt);

	// Check it exists
	if (!texturex)
		return false;

	// Check entries aren't locked (texture editor open or iwad)
	if (pnames->isLocked() || texturex->isLocked()) {
		if (parent->isReadOnly())
			wxMessageBox("Cannot perform this action on an IWAD", "Error", wxICON_ERROR);
		else
			wxMessageBox("Cannot perform this action because one or more texture related entries is locked. Please close the archive's texture editor if it is open.", "Error", wxICON_ERROR);

		return false;
	}

	// Load patch table
	PatchTable ptable;
	ptable.loadPNAMES(pnames);

	// Load texture list
	TextureXList tx;
	tx.readTEXTUREXData(texturex, ptable);

	// Create textures from entries
	SImage image;
	for (unsigned a = 0; a < entries.size(); a++) {
		// Check entry type
		if (!(entries[a]->getType()->extraProps().propertyExists("image"))) {
			wxLogMessage("Entry %s is not a valid image", entries[a]->getName());
			continue;
		}

		// Check entry name
		string name = entries[a]->getName(true);
		if (name.Length() > 8) {
			wxLogMessage("Entry %s has too long a name to add to the patch table (name must be 8 characters max)", entries[a]->getName());
			continue;
		}

		// Add to patch table
		ptable.addPatch(name);

		// Load patch to temp image
		Misc::loadImageFromEntry(&image, entries[a]);

		// Create texture
		CTexture* ntex = new CTexture();
		ntex->setName(name);
		ntex->addPatch(name, 0, 0);
		ntex->setWidth(image.getWidth());
		ntex->setHeight(image.getHeight());

		// Setup texture scale
		if (tx.getFormat() == TXF_TEXTURES)
			ntex->setScale(1, 1);
		else
			ntex->setScale(0, 0);

		// Add to texture list
		tx.addTexture(ntex);
	}

	// Write patch table data back to pnames entry
	ptable.writePNAMES(pnames);

	// Write texture data back to texturex entry
	tx.writeTEXTUREXData(texturex, ptable);

	return true;
}

/* EntryOperations::convertTextures
 * Converts multiple TEXTURE1/2 entries to a single ZDoom text-based
 * TEXTURES entry
 *******************************************************************/
bool EntryOperations::convertTextures(vector<ArchiveEntry*> entries) {
	// Check any entries were given
	if (entries.size() == 0)
		return false;

	// Get parent archive of entries
	Archive* parent = entries[0]->getParent();

	// Can't do anything if entry isn't in an archive
	if (!parent)
		return false;

	// Find patch table in parent archive
	Archive::search_options_t opt;
	opt.match_type = EntryType::getType("pnames");
	ArchiveEntry* pnames = parent->findLast(opt);

	// Check it exists
	if (!pnames)
		return false;

	// Load patch table
	PatchTable ptable;
	ptable.loadPNAMES(pnames);

	// Read all texture entries to a single list
	TextureXList tx;
	for (unsigned a = 0; a < entries.size(); a++)
		tx.readTEXTUREXData(entries[a], ptable, true);

	// Convert to extended (TEXTURES) format
	tx.convertToTEXTURES();

	// Create new TEXTURES entry and write to it
	ArchiveEntry* textures = parent->addNewEntry("TEXTURES", parent->entryIndex(entries[0]));
	if (textures) {
		bool ok = tx.writeTEXTURESData(textures);
		EntryType::detectEntryType(textures);
		textures->setExtensionByType();
		return ok;
	}
	else
		return false;
}

/* EntryOperations::compileACS
 * Attempts to compile [entry] as an ACS script. If the entry is
 * named SCRIPTS, the compiled data is imported to the BEHAVIOR
 * entry previous to it, otherwise it is imported to a same-name
 * compiled library entry in the acs namespace
 *******************************************************************/
bool EntryOperations::compileACS(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Check entry has a parent (this is useless otherwise)
	if (!entry->getParent())
		return false;

	// Check entry is text
	if (!EntryDataFormat::getFormat("text")->isThisFormat(entry->getMCData())) {
		wxMessageBox("Error: Entry does not appear to be text", "Error", wxOK|wxCENTRE|wxICON_ERROR);
		return false;
	}

	// Check if the ACC path is set up
	string accpath = path_acc;
	if (accpath.IsEmpty() || !wxFileExists(accpath)) {
		wxMessageBox("Error: ACC path not defined, please configure in SLADE preferences", "Error", wxOK|wxCENTRE|wxICON_ERROR);
		return false;
	}

	// Setup some path strings
	wxFileName fn(accpath);
	fn.SetFullName(entry->getName());
	fn.SetExt("acs");
	string srcfile = fn.GetFullPath();
	fn.SetExt("o");
	string ofile = fn.GetFullPath();

	// Export script to file
	entry->exportFile(srcfile);

	// Execute acc
	string command = path_acc + " \"" + srcfile + "\" \"" + ofile + "\"";
	wxExecute(command, wxEXEC_SYNC);

	// Delete source file
	wxRemoveFile(srcfile);

	// Check it compiled successfully
	if (wxFileExists(ofile)) {
		// Check if the script is a map script (BEHAVIOR)
		if (S_CMPNOCASE(entry->getName(), "SCRIPTS")) {
			// Get entry before SCRIPTS
			ArchiveEntry* prev = entry->prevEntry();

			// Create a new entry there if it isn't BEHAVIOR
			if (!prev || !(S_CMPNOCASE(prev->getName(), "BEHAVIOR")))
				prev = entry->getParent()->addNewEntry("BEHAVIOR", entry->getParent()->entryIndex(entry));

			// Import compiled script
			prev->importFile(ofile);
		}
		else {
			// Otherwise, treat it as a library

			// See if the compiled library already exists as an entry
			Archive::search_options_t opt;
			opt.match_namespace = "acs";
			opt.match_name = entry->getName(true);
			ArchiveEntry* lib = entry->getParent()->findLast(opt);

			// If it doesn't exist, create it
			if (!lib)
				lib = entry->getParent()->addEntry(new ArchiveEntry(entry->getName(true) + ".o"), "acs");

			// Import compiled script
			lib->importFile(ofile);
		}

		// Delete compiled script file
		wxRemoveFile(ofile);
	}
	else {
		// Read acs.err to string
		fn.SetFullName("acs.err");
		wxFile file(fn.GetFullPath());
		char* buf = new char[file.Length()];
		file.Read(buf, file.Length());
		string errors = wxString::From8BitData(buf, file.Length());
		delete[] buf;

		ExtMessageDialog dlg(NULL, "Error Compiling");
		dlg.setMessage("The following errors were encountered while compiling, please fix them and recompile:");
		dlg.setExt(errors);
		dlg.ShowModal();

		return false;
	}

	return true;
}

/* EntryOperations::exportAsPNG
 * Converts [entry] to a PNG image (if possible) and saves the PNG
 * data to a file [filename]. Does not alter the entry data itself
 *******************************************************************/
bool EntryOperations::exportAsPNG(ArchiveEntry* entry, string filename) {
	// Check entry was given
	if (!entry)
		return false;

	// Create image from entry
	SImage image;
	if (!Misc::loadImageFromEntry(&image, entry)) {
		wxLogMessage("Error converting %s: %s", CHR(entry->getName()), CHR(Global::error));
		return false;
	}

	// Write png data
	MemChunk png;
	if (!image.toPNG(png, theMainWindow->getPaletteChooser()->getSelectedPalette(entry))) {
		wxLogMessage("Error converting %s", CHR(entry->getName()));
		return false;
	}

	// Export file
	return png.exportFile(filename);
}


/*******************************************************************
 * CONSOLE COMMANDS
 *******************************************************************/

/*
CONSOLE_COMMAND (test_ee, 1) {
	if (theArchiveManager->numArchives() > 0) {
		for (size_t a = 0; a < args.size(); a++) {
			ArchiveEntry* entry = theArchiveManager->getArchive(0)->entryAtPath(args[a]);
			if (entry)
				EntryOperations::openExternal(entry);
			else
				wxLogMessage("Entry %s not found", CHR(args[a]));
		}
	}
}
*/
