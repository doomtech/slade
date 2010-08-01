/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    TextureXList.cpp
 * Description: Handles a collection of Composite Textures (ie,
 *              encapsulates a TEXTUREx entry)
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
#include "TextureXList.h"
#include "Archive.h"
#include "ArchiveManager.h"


/*******************************************************************
 * TEXTUREXLIST CLASS FUNCTIONS
 *******************************************************************/

/* TextureXList::TextureXList
 * TextureXList class constructor
 *******************************************************************/
TextureXList::TextureXList() {
	// Setup 'invalid' texture
	tex_invalid.setName("INVALID_TEXTURE");	// Deliberately set the name to >8 characters
}

/* TextureXList::~TextureXList
 * TextureXList class destructor
 *******************************************************************/
TextureXList::~TextureXList() {
}

/* TextureXList::getTexture
 * Returns the texture at [index], or the 'invalid' texture if
 * [index] is out of range
 *******************************************************************/
CTexture* TextureXList::getTexture(size_t index) {
	// Check index
	if (index >= textures.size())
		return &tex_invalid;

	// Return texture at index
	return textures[index];
}

/* TextureXList::getTexture
 * Returns the texture matching [name], or the 'invalid' texture if
 * no match is found
 *******************************************************************/
CTexture* TextureXList::getTexture(string name) {
	// Search for texture by name
	for (size_t a = 0; a < textures.size(); a++) {
		if (s_cmpnocase(textures[a]->getName(), name))
			return textures[a];
	}

	// Not found
	return &tex_invalid;
}

void TextureXList::addTexture(CTexture* tex, int position) {
	// Add it to the list at position if valid
	if (position >= 0 && (unsigned)position < textures.size())
		textures.insert(textures.begin() + position, tex);
	else
		textures.push_back(tex);
}

void TextureXList::removeTexture(unsigned index) {
	// Check index
	if (index > textures.size())
		return;

	// Remove the texture from the list
	textures.erase(textures.begin() + index);
}

/* TextureXList::clear
 * Clears all textures
 *******************************************************************/
void TextureXList::clear(bool clear_patches) {
	// Clear textures list
	for (unsigned a = 0; a < textures.size(); a++)
		delete textures[a];
	textures.clear();
}

// Some structs for reading TEXTUREx data
struct tdef_t
{ // Just the data relevant to SLADE3
	char		name[8];
	uint16_t	flags;
	uint8_t		scale[2];
	int16_t		width;
	int16_t		height;
};

struct nltdef_t
{ // The nameless version used by Doom Alpha 0.4
	uint16_t	flags;
	uint8_t		scale[2];
	int16_t		width;
	int16_t		height;
	int16_t		columndir[2];
	int16_t		patchcount;
};

struct ftdef_t
{ // The full version with some useless data
	char		name[8];
	uint16_t	flags;
	uint8_t		scale[2];
	int16_t		width;
	int16_t		height;
	int16_t		columndir[2];
	int16_t		patchcount;
};

struct stdef_t
{ // The Strife version with less useless data
	char		name[8];
	uint16_t	flags;
	uint8_t		scale[2];
	int16_t		width;
	int16_t		height;
	int16_t		patchcount;
};

/* TextureXList::removePatch
 * Updates all textures in the list to 'remove' [patch]
 *******************************************************************/
void TextureXList::removePatch(string patch) {
	// Go through all textures
	for (unsigned a = 0; a < textures.size(); a++)
		textures[a]->removePatch(patch);	// Remove patch from texture
}

/* TextureXList::readTEXTUREXData
 * Reads in a doom-format TEXTUREx entry. Returns true on success,
 * false otherwise
 *******************************************************************/
bool TextureXList::readTEXTUREXData(ArchiveEntry* texturex, PatchTable& patch_table) {
	// Check entries were actually given
	if (!texturex)
		return false;

	// Clear current textures
	clear();

	// Read TEXTUREx

	// Read header
	texturex->seek(0, SEEK_SET);
	int32_t		n_tex = 0;
	int32_t*	offsets = NULL;

	// Number of textures
	if (!texturex->read(&n_tex, 4)) {
		wxLogMessage("Error: TEXTUREx entry is corrupt");
		return false;
	}

	// Texture definition offsets
	offsets = new int32_t[n_tex];
	if (!texturex->read(offsets, n_tex * 4)) {
		wxLogMessage("Error: TEXTUREx entry is corrupt");
		return false;
	}

	// Read the first texture definition to try to identify the format
	if (!texturex->seek(offsets[0], SEEK_SET)) {
		wxLogMessage("Error: TEXTUREx entry is corrupt");
		return false;
	}
	// Look at the name field. Is it present or not?
	char tempname[8];
	if (!texturex->read(&tempname, 8)) {
		wxLogMessage("Error: TEXTUREx entry is corrupt");
		return false;
	}
	// Let's pretend it is and see what happens.
	txformat = TXF_NORMAL;

	// Only the characters A-Z (uppercase), 0-9, and [ ] - _ should be used in texture names.
	for (uint8_t a = 0; a < 8; ++a) {
		if (a > 0 && tempname[a] == 0)
			// We found a null-terminator for the string, so we can assume it's okay.
			break;
		if (!((tempname[a] >= 'A' && tempname[a] <= '[') ||
			  (tempname[a] >= '0' && tempname[a] <= '9') ||
			   tempname[a] == ']' || tempname[a] == '-' || tempname[a] == '_'))
			// We're out of character range, so this is probably not a texture name.
			txformat = TXF_NAMELESS;
	}

	// Now let's see if it is the abridged Strife format or not.
	if (txformat == TXF_NORMAL) {
		// No need to test this again since it was already tested before.
		texturex->seek(offsets[0], SEEK_SET);
		ftdef_t temp;
		if (!texturex->read(&temp, 22)) {
			wxLogMessage("Error: TEXTUREx entry is corrupt");
			return false;
		}
		// Test condition adapted from ZDoom; apparently the first two bytes of columndir
		// may be set to garbage values by some editors and are therefore unreliable.
		if (wxINT16_SWAP_ON_BE(temp.patchcount <= 0) || (temp.columndir[1] != 0))
			txformat = TXF_STRIFE11;
	}

	// Read all texture definitions
	for (int32_t a = 0; a < n_tex; a++) {
		// Skip to texture definition
		if (!texturex->seek(offsets[a], SEEK_SET)) {
			wxLogMessage("Error: TEXTUREx entry is corrupt");
			return false;
		}

		// Read definition
		tdef_t tdef;
		if (txformat == TXF_NAMELESS) {
			nltdef_t nameless;
			// Auto-naming mechanism taken from DeuTex
			if (a > 99999) {
				wxLogMessage("Error: More than 100000 nameless textures");
				return false;
			}
			char temp[9] = "";
			sprintf (temp, "TEX%05d", a);
			memcpy(tdef.name, temp, 8);

			// Read texture info
			if (!texturex->read(&nameless, 14)) {
				wxLogMessage("Error: TEXTUREx entry is corrupt");
				return false;
			}

			// Copy data to permanent structure
			tdef.flags = nameless.flags;
			tdef.scale[0] = nameless.scale[0];
			tdef.scale[1] = nameless.scale[1];
			tdef.width = nameless.width;
			tdef.height = nameless.height;
		}
		else if (!texturex->read(&tdef, 16)) {
			wxLogMessage("Error: TEXTUREx entry is corrupt");
			return false;
		}

		// Skip unused
		if (txformat != TXF_STRIFE11) {
			if (!texturex->seek(4, SEEK_CUR)) {
				wxLogMessage("Error: TEXTUREx entry is corrupt");
				return false;
			}
		}

		// Create texture
		CTexture* tex = new CTexture();
		tex->setName(wxString::From8BitData(tdef.name, 8));
		tex->setWidth(tdef.width);
		tex->setHeight(tdef.height);
		tex->setScale(tdef.scale[0], tdef.scale[1], true);

		// Set flags
		if (tdef.flags & TX_WORLDPANNING)
			tex->exProps().addFlag("WorldPanning");

		// Read patches
		int16_t n_patches = 0;
		if (!texturex->read(&n_patches, 2)) {
			wxLogMessage("Error: TEXTUREx entry is corrupt");
			return false;
		}

		for (uint16_t p = 0; p < n_patches; p++) {
			// Read patch definition
			tx_patch_t pdef;
			if (!texturex->read(&pdef, 6)) {
				wxLogMessage("Error: TEXTUREx entry is corrupt");
				return false;
			}

			// Skip unused
			if (txformat != TXF_STRIFE11) {
				if (!texturex->seek(4, SEEK_CUR)) {
					wxLogMessage("Error: TEXTUREx entry is corrupt");
					return false;
				}
			}

			// Add it to the texture
			string patch = patch_table.patchName(pdef.patch);
			if (patch.IsEmpty()) {
				wxLogMessage("Warning: Texture %s contains patch %d which is invalid - may be incorrect PNAMES entry", chr(tex->getName()), pdef.patch);
				patch = s_fmt("INVPATCH%04d", pdef.patch);
			}
			
			tex->addPatch(patch, pdef.left, pdef.top);
		}

		// Add texture to list
		textures.push_back(tex);
	}

	// Clean up
	delete[] offsets;

	return true;
}

#define SAFEFUNC(x) if(!x) return false;

bool TextureXList::writeTEXTUREXData(ArchiveEntry* texturex, PatchTable& patch_table) {
	// Check entry was given
	if (!texturex)
		return false;

	if (texturex->isLocked())
		return false;

	/* Total size of a TEXTUREx lump, in bytes:
		Header: 4 + (4 * numtextures)
		Textures:
			22 * numtextures (normal format)
			14 * numtextures (nameless format)
			18 * numtextures (Strife 1.1 format)
		Patches:
			10 * sum of patchcounts (normal and nameless formats)
			 6 * sum of patchcounts (Strife 1.1 format)
	*/
	size_t numpatchrefs = 0;
	size_t numtextures = textures.size();
	for (size_t i = 0; i < numtextures; ++i) {
		numpatchrefs += textures[i]->nPatches();
	}
	wxLogMessage("%i patch references in %i textures", numpatchrefs, numtextures);

	size_t datasize = 0;
	size_t headersize = 4 + (4 * numtextures);
	switch (txformat) {
		case TXF_NORMAL:	datasize = 4 + (26 * numtextures) + (10 * numpatchrefs); break;
		case TXF_NAMELESS:	datasize = 4 + (18 * numtextures) + (10 * numpatchrefs); break;
		case TXF_STRIFE11:	datasize = 4 + (22 * numtextures) + ( 6 * numpatchrefs); break;
		// Some compilers insist on having default cases.
		default: return false;
	}
	
	MemChunk txdata(datasize);
	size_t txoffset = headersize;
	int32_t foo = wxINT32_SWAP_ON_BE((signed) numtextures);

	// Write header
	txdata.seek(0, SEEK_SET);
	SAFEFUNC(txdata.write(&foo, 4));

	// Write texture entries
	for (size_t i = 0; i < numtextures; ++i) {
		// Get texture to write
		CTexture* tex = textures[i];

		// Write offset
		foo = wxINT32_SWAP_ON_BE((signed) txoffset);
		SAFEFUNC(txdata.write(&foo, 4));
		SAFEFUNC(txdata.seek(txoffset, SEEK_SET));

		// Write texture entry
		switch (txformat) {
			case TXF_NORMAL:
			{
				// Create 'normal' doom format texture definition
				ftdef_t txdef;
				memset(txdef.name, 0, 8); // Set texture name to all 0's (to ensure compatibility with XWE)
				strncpy(txdef.name, chr(tex->getName()), tex->getName().Len());
				txdef.flags			= 0;
				txdef.scale[0]		= tex->getScaleX();
				txdef.scale[1]		= tex->getScaleY();
				txdef.width			= tex->getWidth();
				txdef.height		= tex->getHeight();
				txdef.columndir[0]	= 0;
				txdef.columndir[1]	= 0;
				txdef.patchcount	= tex->nPatches();

				// Check for WorldPanning flag
				if (tex->exProps().propertyExists("WorldPanning"))
					txdef.flags |= TX_WORLDPANNING;

				// Write texture definition
				SAFEFUNC(txdata.write(&tex, 22));

				// Increment offset
				txoffset += (22 + (10 * txdef.patchcount));

				break;
			}
			case TXF_NAMELESS:
			{
				// Create nameless texture definition
				nltdef_t txdef;
				txdef.flags			= 0;
				txdef.scale[0]		= tex->getScaleX();
				txdef.scale[1]		= tex->getScaleY();
				txdef.width			= tex->getWidth();
				txdef.height		= tex->getHeight();
				txdef.columndir[0]	= 0;
				txdef.columndir[1]	= 0;
				txdef.patchcount	= tex->nPatches();

				// Write texture definition
				SAFEFUNC(txdata.write(&txdef, 14));

				// Increment offset
				txoffset += (14 + (10 * txdef.patchcount));

				break;
			}
			case TXF_STRIFE11:
			{
				// Create strife format texture definition
				stdef_t txdef;
				memset(txdef.name, 0, 8); // Set texture name to all 0's (to ensure compatibility with XWE)
				strncpy(txdef.name, chr(tex->getName()), tex->getName().Len());
				txdef.flags			= 0;
				txdef.scale[0]		= tex->getScaleX();
				txdef.scale[1]		= tex->getScaleY();
				txdef.width			= tex->getWidth();
				txdef.height		= tex->getHeight();
				txdef.patchcount	= tex->nPatches();

				// Check for WorldPanning flag
				if (tex->exProps().propertyExists("WorldPanning"))
					txdef.flags |= TX_WORLDPANNING;

				// Write texture definition
				SAFEFUNC(txdata.write(&txdef, 18));

				// Increment offset
				txoffset += (18 + (6 * txdef.patchcount));

				break;
			}
			default: return false;
		}

		// Write patch references
		for (size_t k = 0; k < tex->nPatches(); ++k) {
			// Get patch to write
			CTPatch* patch = tex->getPatch(k);

			// Create patch definition
			tx_patch_t pdef;
			pdef.left = patch->xOffset();
			pdef.top = patch->yOffset();
			pdef.patch = patch_table.patchIndex(patch->patchName());	// Note this will be -1 if the patch doesn't exist in the patch table. This should never happen with the texture editor, though.

			// Write common data
			SAFEFUNC(txdata.write(&pdef, 6));

			// In non-Strife formats, there's some added rubbish
			if (txformat != TXF_STRIFE11) {
				foo = 0;
				SAFEFUNC(txdata.write(&foo, 4));
			}
		}

		// Return to offset list
		SAFEFUNC(txdata.seek(4*(i+1), SEEK_SET));
	}

	// Write data to the TEXTUREx entry
	texturex->importMemChunk(txdata);

	return true;
}

string TextureXList::getTextureXFormatString() {
	switch (txformat) {
		case TXF_NORMAL:
			return "Doom";
			break;
		case TXF_STRIFE11:
			return "Strife";
			break;
		case TXF_NAMELESS:
			return "Doom Alpha Nameless";
			break;
		default:
			return "Unknown";
	}
}
