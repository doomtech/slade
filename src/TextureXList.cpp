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


/*******************************************************************
 * TEXTUREXLIST CLASS FUNCTIONS
 *******************************************************************/

/* TextureXList::TextureXList
 * TextureXList class constructor
 *******************************************************************/
TextureXList::TextureXList() {
}

/* TextureXList::~TextureXList
 * TextureXList class destructor
 *******************************************************************/
TextureXList::~TextureXList() {
}

/* TextureXList::getTexture
 * Returns the CTexture at [index], or NULL if [index] is out of
 * range
 *******************************************************************/
CTexture* TextureXList::getTexture(int index) {
	// Check index range
	if (index < 0 || index > nTextures())
		return NULL;

	return textures[index];
}

/* TextureXList::getTexture
 * Returns a CTexture matching [name], or NULL if no match found
 *******************************************************************/
CTexture* TextureXList::getTexture(string name) {
	for (size_t a = 0; a < nTextures(); a++) {
		if (textures[a]->getName().CmpNoCase(name) == 0)
			return textures[a];
	}

	// No match found
	return NULL;
}

/* TextureXList::getPatch
 * Returns the patch entry at [index], or NULL if [index] is out of
 * range
 *******************************************************************/
ArchiveEntry* TextureXList::getPatch(int index) {
	// Check index range
	if (index < 0 || index > nPatches())
		return NULL;

	return patches[index];
}

/* TextureXList::getPatch
 * Returns the patch entry matching [name], or NULL if no match found
 *******************************************************************/
ArchiveEntry* TextureXList::getPatch(string name) {
	for (size_t a = 0; a < nPatches(); a++) {
		if (patches[a]->getName().CmpNoCase(name) == 0)
			return patches[a];
	}

	// No match found
	return NULL;
}

/* TextureXList::clear
 * Clears all textures and patches
 *******************************************************************/
void TextureXList::clear() {
	// Clear textures
	for (size_t a = 0; a < textures.size(); a++)
		delete textures[a];
	textures.clear();

	// Clear patches
	for (size_t a = 0; a < patches.size(); a++)
		delete patches[a];
	patches.clear();
}

/* TextureXList::readTEXTUREXData
 * Reads in a doom-format TEXTUREx entry. Returns true on success,
 * false otherwise
 *******************************************************************/
bool TextureXList::readTEXTUREXData(ArchiveEntry* texturex, ArchiveEntry* pnames) {
	// Check entries were actually given
	if (!texturex || !pnames)
		return false;

	// Get parent archive
	Archive* parent_archive = texturex->getParent();

	// Read PNAMES

	// Read number of pnames
	vector<string> patch_names;
	uint32_t n_pnames = 0;
	pnames->seek(0, SEEK_SET);
	pnames->read(&n_pnames, 4);

	// Read pnames content
	for (uint32_t a = 0; a < n_pnames; a++) {
		char pname[9] = "";
		pname[8] = 0;
		pnames->read(&pname, 8);
		patch_names.push_back(wxString(pname).Upper());
	}


	// Read TEXTUREx

	// Read header
	texturex->seek(0, SEEK_SET);
	int32_t		n_tex = 0;
	int32_t*	offsets = NULL;

	// Number of textures
	texturex->read(&n_tex, 4);

	// Texture definition offsets
	offsets = new int32_t[n_tex];
	texturex->read(offsets, n_tex * 4);

	// Read all texture definitions
	for (int32_t a = 0; a < n_tex; a++) {
		// Read name
		char name[9] = "";
		name[8] = 0;
		texturex->read(name, 8);

		// Read flags
		uint16_t flags;
		texturex->read(&flags, 2);

		// Read scale
		uint8_t scale[2];
		texturex->read(&scale, 2);

		// Read dimensions
		uint16_t width = 0;
		uint16_t height = 0;
		texturex->read(&width, 2);
		texturex->read(&height, 2);

		// Skip unused
		texturex->seek(4, SEEK_CUR);

		// Create texture
		CTexture* tex = new CTexture(name, width, height);

		// Read patches
		uint16_t n_patches = 0;
		texturex->read(&n_patches, 2);
		for (uint16_t p = 0; p < n_patches; p++) {
			// Read left offset
			int16_t left = 0;
			texturex->read(&left, 2);

			// Read top offset
			int16_t top = 0;
			texturex->read(&top, 2);

			// Read patch id
			uint16_t patch = 0;
			texturex->read(&patch, 2);

			// Create patch
			CTPatch* ctp = new CTPatch();
			ctp->setOffsets(left, top);

			// Find patch image entry
			ArchiveEntry* img = texturex->getParent()->getEntry(patch_names[patch]);
			if (!img) wxLogMessage(s_fmt(_T("Couldn't find patch %s"), patch_names[patch].c_str()));
			ctp->loadImage(img);

			// Add it to the texture
			tex->addPatch(ctp);

			// Skip unused
			texturex->seek(4, SEEK_CUR);
		}

		// Add texture to list
		textures.push_back(tex);
	}

	// Clean up
	delete[] offsets;

	return true;
}

/* TextureXList::readTEXTURESData
 * Reads in a zdoom-format TEXTURES entry. Returns true on success,
 * false otherwise
 *******************************************************************/
bool TextureXList::readTEXTURESData(ArchiveEntry* textures) {
	return false;
}
