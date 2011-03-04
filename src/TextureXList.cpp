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
#include "MainWindow.h"
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

/* TextureXList::addTexture
 * Adds [tex] to the texture list at [position]
 *******************************************************************/
void TextureXList::addTexture(CTexture* tex, int position) {
	// Add it to the list at position if valid
	if (position >= 0 && (unsigned)position < textures.size())
		textures.insert(textures.begin() + position, tex);
	else
		textures.push_back(tex);
}

/* TextureXList::removeTexture
 * Removes the texture at [index] from the list
 *******************************************************************/
void TextureXList::removeTexture(unsigned index) {
	// Check index
	if (index >= textures.size())
		return;

	// Delete the texture
	delete textures[index];

	// Remove the texture from the list
	textures.erase(textures.begin() + index);
}

/* TextureXList::swapTextures
 * Swaps the texture at [index1] with the texture at [index2]
 *******************************************************************/
void TextureXList::swapTextures(unsigned index1, unsigned index2) {
	// Check indices
	if (index1 >= textures.size() || index2 >= textures.size())
		return;

	// Swap them
	CTexture* temp = textures[index1];
	textures[index1] = textures[index2];
	textures[index2] = temp;
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
bool TextureXList::readTEXTUREXData(ArchiveEntry* texturex, PatchTable& patch_table, bool add) {
	// Check entries were actually given
	if (!texturex)
		return false;

	// Clear current textures if needed
	if (!add) clear();

	// Update palette
	theMainWindow->getPaletteChooser()->setGlobalFromArchive(texturex->getParent());

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

	// If it's an empty TEXTUREx entry, stop here
	if (n_tex == 0)
		return true;

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
		tex->name = wxString::From8BitData(tdef.name, 8);
		tex->width = tdef.width;
		tex->height = tdef.height;
		tex->scale_x = tdef.scale[0]/8.0;
		tex->scale_y = tdef.scale[1]/8.0;

		// Set flags
		if (tdef.flags & TX_WORLDPANNING)
			tex->world_panning = true;

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

/* TextureXList::writeTEXTUREXData
 * Writes the texture list in TEXTUREX format to [texturex], using
 * [patch_table] for patch information. Returns true on success,
 * false otherwise
 *******************************************************************/
bool TextureXList::writeTEXTUREXData(ArchiveEntry* texturex, PatchTable& patch_table) {
	// Check entry was given
	if (!texturex)
		return false;

	if (texturex->isLocked())
		return false;

	wxLogMessage("Writing " + getTextureXFormatString() + " format TEXTUREx entry");

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
	int32_t* offsets = new int32_t[numtextures];
	int32_t foo = wxINT32_SWAP_ON_BE((signed) numtextures);

	// Write header
	txdata.seek(0, SEEK_SET);
	SAFEFUNC(txdata.write(&foo, 4));

	// Go to beginning of texture definitions
	SAFEFUNC(txdata.seek(4 + (numtextures*4), SEEK_SET));

	// Write texture entries
	for (size_t i = 0; i < numtextures; ++i) {
		// Get texture to write
		CTexture* tex = textures[i];

		// Set offset
		offsets[i] = (signed)txdata.currentPos();

		// Write texture entry
		switch (txformat) {
			case TXF_NORMAL:
			{
				// Create 'normal' doom format texture definition
				ftdef_t txdef;
				memset(txdef.name, 0, 8); // Set texture name to all 0's (to ensure compatibility with XWE)
				strncpy(txdef.name, chr(tex->getName().Upper()), tex->getName().Len());
				txdef.flags			= 0;
				txdef.scale[0]		= (tex->getScaleX()*8);
				txdef.scale[1]		= (tex->getScaleY()*8);
				txdef.width			= tex->getWidth();
				txdef.height		= tex->getHeight();
				txdef.columndir[0]	= 0;
				txdef.columndir[1]	= 0;
				txdef.patchcount	= tex->nPatches();

				// Check for WorldPanning flag
				if (tex->world_panning)
					txdef.flags |= TX_WORLDPANNING;

				// Write texture definition
				SAFEFUNC(txdata.write(&txdef, 22));

				break;
			}
			case TXF_NAMELESS:
			{
				// Create nameless texture definition
				nltdef_t txdef;
				txdef.flags			= 0;
				txdef.scale[0]		= (tex->getScaleX()*8);
				txdef.scale[1]		= (tex->getScaleY()*8);
				txdef.width			= tex->getWidth();
				txdef.height		= tex->getHeight();
				txdef.columndir[0]	= 0;
				txdef.columndir[1]	= 0;
				txdef.patchcount	= tex->nPatches();

				// Write texture definition
				SAFEFUNC(txdata.write(&txdef, 14));

				break;
			}
			case TXF_STRIFE11:
			{
				// Create strife format texture definition
				stdef_t txdef;
				memset(txdef.name, 0, 8); // Set texture name to all 0's (to ensure compatibility with XWE)
				strncpy(txdef.name, chr(tex->getName().Upper()), tex->getName().Len());
				txdef.flags			= 0;
				txdef.scale[0]		= (tex->getScaleX()*8);
				txdef.scale[1]		= (tex->getScaleY()*8);
				txdef.width			= tex->getWidth();
				txdef.height		= tex->getHeight();
				txdef.patchcount	= tex->nPatches();

				// Check for WorldPanning flag
				if (tex->world_panning)
					txdef.flags |= TX_WORLDPANNING;

				// Write texture definition
				SAFEFUNC(txdata.write(&txdef, 18));

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
			pdef.patch = patch_table.patchIndex(patch->getName());	// Note this will be -1 if the patch doesn't exist in the patch table. This should never happen with the texture editor, though.

			// Write common data
			SAFEFUNC(txdata.write(&pdef, 6));

			// In non-Strife formats, there's some added rubbish
			if (txformat != TXF_STRIFE11) {
				foo = 0;
				SAFEFUNC(txdata.write(&foo, 4));
			}
		}
	}

	// Write offsets
	SAFEFUNC(txdata.seek(4, SEEK_SET));
	SAFEFUNC(txdata.write(offsets, 4*numtextures));

	// Write data to the TEXTUREx entry
	texturex->importMemChunk(txdata);

	// Update entry type
	EntryType::detectEntryType(texturex);

	// Clean up
	delete[] offsets;

	return true;
}

/* TextureXList::readTEXTURESData
 * Reads in a ZDoom-format TEXTURES entry. Returns true on success,
 * false otherwise
 *******************************************************************/
bool TextureXList::readTEXTURESData(ArchiveEntry* entry) {
	// Check for empty entry
	if (!entry) {
		Global::error = "Attempt to read texture data from NULL entry";
		return false;
	}
	if (entry->getSize() == 0) {
		txformat = TXF_TEXTURES;
		return true;
	}

	// Get text to parse
	Tokenizer tz;
	tz.openMem(&(entry->getMCData()), entry->getName());

	// Parsing gogo
	string token = tz.getToken();
	while (!token.IsEmpty()) {
		// Texture definition
		if (s_cmpnocase(token, "Texture")) {
			CTexture* tex = new CTexture();
			if (tex->parse(tz, "Texture"))
				textures.push_back(tex);
		}

		// Sprite definition
		if (s_cmpnocase(token, "Sprite")) {
			CTexture* tex = new CTexture();
			if (tex->parse(tz, "Sprite"))
				textures.push_back(tex);
		}

		// Graphic definition
		if (s_cmpnocase(token, "Graphic")) {
			CTexture* tex = new CTexture();
			if (tex->parse(tz, "Graphic"))
				textures.push_back(tex);
		}

		// WallTexture definition
		if (s_cmpnocase(token, "WallTexture")) {
			CTexture* tex = new CTexture();
			if (tex->parse(tz, "WallTexture"))
				textures.push_back(tex);
		}

		// Flat definition
		if (s_cmpnocase(token, "Flat")) {
			CTexture* tex = new CTexture();
			if (tex->parse(tz, "Flat"))
				textures.push_back(tex);
		}

		token = tz.getToken();
	}

	// Go through each patch of each texture and update them
	for (uint32_t t = 0; t < textures.size(); ++t) {
		for (uint32_t p = 0; p < textures[t]->nPatches(); p++) {
			textures[t]->getPatch(p)->searchEntry(entry->getParent());
		}
	}

	txformat = TXF_TEXTURES;

	return true;
}

/* TextureXList::writeTEXTURESData
 * Writes the texture list in TEXTURES format to [entry] Returns true
 * on success, false otherwise
 *******************************************************************/
bool TextureXList::writeTEXTURESData(ArchiveEntry* entry) {
	// Check format
	if (txformat != TXF_TEXTURES)
		return false;

	// Generate a big string :P
	string textures_data = "// Texture definitions generated by SLADE3\n// on " + wxNow() + "\n\n";
	for (unsigned a = 0; a < textures.size(); a++)
		textures_data += textures[a]->asText();
	textures_data += "// End of texture definitions\n";

	// Write it to the entry
	return entry->importMem(textures_data.ToAscii(), textures_data.length());
}

/* TextureXList::getTextureXFormatString
 * Returns a string representation of the texture list format
 *******************************************************************/
string TextureXList::getTextureXFormatString() {
	switch (txformat) {
		case TXF_NORMAL:
			return "Doom TEXTUREx";
			break;
		case TXF_STRIFE11:
			return "Strife TEXTUREx";
			break;
		case TXF_NAMELESS:
			return "Nameless (Doom Alpha)";
			break;
		case TXF_TEXTURES:
			return "ZDoom TEXTURES";
			break;
		default:
			return "Unknown";
	}
}

/* TextureXList::convertToTEXTURES
 * Converts all textures in the list to extended TEXTURES format
 *******************************************************************/
bool TextureXList::convertToTEXTURES() {
	// Check format is appropriate
	if (txformat == TXF_TEXTURES) {
		Global::error = "Already TEXTURES format";
		return false;
	}

	// Convert all textures to extended format
	for (unsigned a = 0; a < textures.size(); a++)
		textures[a]->convertExtended();

	// First texture is null texture
	textures[0]->null_texture = true;

	// Set new format
	txformat = TXF_TEXTURES;

	return true;
}
