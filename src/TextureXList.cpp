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
	tex_invalid.name = "INVALID_TEXTURE";	// Deliberately set the name to >8 characters
	tex_invalid.width = -1;
	tex_invalid.height = -1;
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
tx_texture_t TextureXList::getTexture(size_t index) {
	// Check index
	if (index >= textures.size())
		return tex_invalid;

	// Return texture at index
	return textures[index];
}

/* TextureXList::getTexture
 * Returns the texture matching [name], or the 'invalid' texture if
 * no match is found
 *******************************************************************/
tx_texture_t TextureXList::getTexture(string name) {
	// Search for texture by name
	for (size_t a = 0; a < textures.size(); a++) {
		if (textures[a].name.CmpNoCase(name) == 0)
			return textures[a];
	}

	// Not found
	return tex_invalid;
}

void TextureXList::addTexture(tx_texture_t& tex, int position) {
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
	textures.clear();
}

// Some structs for reading TEXTUREx data
struct tdef_t
{ // The normal version with just the relevant data
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


/* TextureXList::removePatch
 * Updates all textures in the list to 'remove' the patch [index]
 *******************************************************************/
void TextureXList::removePatch(unsigned index) {
	// Go through all textures
	for (unsigned a = 0; a < textures.size(); a++) {
		tx_texture_t& tex = textures[a];

		// Go through texture's patches
		for (int p = 0; p < (int)tex.patches.size(); p++) {
			tx_patch_t& patch = tex.patches[p];

			// Remove if it matches [index]
			if (patch.patch == index) {
				tex.patches.erase(tex.patches.begin() + p);
				p--;
			}

			// Reduce patch index # if it's greater than the patch being removed
			if (patch.patch > index)
				patch.patch--;
		}
	}
}

/* TextureXList::readTEXTUREXData
 * Reads in a doom-format TEXTUREx entry. Returns true on success,
 * false otherwise
 *******************************************************************/
bool TextureXList::readTEXTUREXData(ArchiveEntry* texturex) {
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
			if (!texturex->read(&nameless, 8)) {
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
		tx_texture_t tex;
		tex.name = wxString::From8BitData(tdef.name, 8);
		tex.width = tdef.width;
		tex.height = tdef.height;
		tex.scale_x = tdef.scale[0];
		tex.scale_y = tdef.scale[1];
		tex.flags = tdef.flags;

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
			tex.patches.push_back(pdef);
		}

		// Add texture to list
		textures.push_back(tex);
	}

	// Clean up
	delete[] offsets;

	return true;
}

bool TextureXList::writeTEXTUREXData(ArchiveEntry* texturex) {
	// Check entry was given
	if (!texturex)
		return false;


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
