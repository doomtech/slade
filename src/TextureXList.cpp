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
}

/* TextureXList::~TextureXList
 * TextureXList class destructor
 *******************************************************************/
TextureXList::~TextureXList() {
}

/* TextureXList::getCTexture
 * Returns the CTexture at [index], or NULL if [index] is out of
 * range
 *******************************************************************/
CTexture* TextureXList::getCTexture(size_t index) {
	// Check index range
	if (index >= nTextures())
		return NULL;

	// Delete current composite texture
	tx_texture_t& td = textures[index];
	if (td.tex)
		delete td.tex;

	// Build composite texture
	CTexture* tex = new CTexture(td.name, td.width, td.height);

	// Add patches
	for (size_t a = 0; a < td.patches.size(); a++) {
		tx_patch_t pd = td.patches[a];

		// Create patch
		CTPatch* ctp = new CTPatch();
		ctp->setOffsets(pd.left, pd.top);

		// Load patch image entry
		ctp->loadImage(patches[pd.patch].entry);

		// Add it to the texture
		tex->addPatch(ctp);
	}

	// Set tex
	td.tex = tex;

	return td.tex;
}

/* TextureXList::getCTexture
 * Returns a CTexture matching [name], or NULL if no match found
 *******************************************************************/
CTexture* TextureXList::getCTexture(string name) {
	for (size_t a = 0; a < nTextures(); a++) {
		if (textures[a].name.CmpNoCase(name) == 0)
			return getCTexture(a);
	}

	// No match found
	return NULL;
}

tx_texture_t TextureXList::getTexInfo(size_t index) {
	if (index >= nTextures())
		return tx_texture_t();

	return textures[index];
}

tx_texture_t TextureXList::getTexInfo(string name) {
	for (size_t a = 0; a < nTextures(); a++) {
		if (textures[a].name.CmpNoCase(name) == 0)
			return getTexInfo(a);
	}

	// No match found
	return tx_texture_t();
}

/* TextureXList::getPatchName
 * Returns the patch name at [index], or empty if [index] is out of
 * range
 *******************************************************************/
string TextureXList::getPatchName(size_t index) {
	// Check index range
	if (index >= nPatches())
		return _T("");

	return patches[index].name;
}

/* TextureXList::getPatchEntry
 * Returns the patch name at [index], or NULL if [index] is out of
 * range
 *******************************************************************/
ArchiveEntry* TextureXList::getPatchEntry(size_t index) {
	// Check index range
	if (index >= nPatches())
		return NULL;

	return patches[index].entry;
}

/* TextureXList::clear
 * Clears all textures and patches
 *******************************************************************/
void TextureXList::clear() {
	// Clear textures
	//for (size_t a = 0; a < textures.size(); a++)
	//	delete textures[a];
	textures.clear();

	// Clear patches
	patches.clear();
}

/* TextureXList::updatePatches
 * Updates all patch entries
 *******************************************************************/
bool TextureXList::updatePatches(Archive* archive) {
	bool okay = true;

	for (size_t a = 0; a < patches.size(); a++) {
		// Attempt to find patch entry
		ArchiveEntry* entry = NULL;

		if (archive)
			entry = archive->getEntry(patches[a].name);						// Search given archive first
		if (!entry)
			entry = theArchiveManager->getResourceEntry(patches[a].name);	// Next search open resource archives + base resource archive

		// Set patch entry
		patches[a].entry = entry;

		if (!entry) {
			wxLogMessage(_T("Patch \"%s\" not found"), patches[a].name.c_str());
			okay = false;
		}
	}

	return okay;
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


/* TextureXList::readTEXTUREXData
 * Reads in a doom-format TEXTUREx entry. Returns true on success,
 * false otherwise
 *******************************************************************/
bool TextureXList::readTEXTUREXData(ArchiveEntry* texturex, ArchiveEntry* pnames) {
	// Check entries were actually given
	if (!texturex || !pnames)
		return false;

	// Clear current textures/patches
	clear();

	// Read PNAMES

	// Read number of pnames
	uint32_t n_pnames = 0;
	pnames->seek(0, SEEK_SET);
	if (!pnames->read(&n_pnames, 4)) {
		wxLogMessage(_T("Error: PNAMES entry is corrupt"));
		return false;
	}

	// Read pnames content
	for (uint32_t a = 0; a < n_pnames; a++) {
		char pname[9] = "";
		pname[8] = 0;

		// Try to read pname
		if (!pnames->read(&pname, 8)) {
			wxLogMessage(_T("Error: PNAMES entry is corrupt"));
			return false;
		}

		// Create patch info
		pnames_entry_t patch;
		patch.name = wxString(pname).Upper();

		// Attempt to find patch entry
		ArchiveEntry* entry = pnames->getParent()->getEntry(patch.name);	// Search parent archive first
		if (!entry)
			entry = theArchiveManager->getResourceEntry(patch.name);		// Next search open resource archives + base resource archive

		if (entry)
			patch.entry = entry;
		else
			wxLogMessage(_T("Patch \"%s\" not found"), pname);

		// Add patch to list
		patches.push_back(patch);
	}


	// Read TEXTUREx

	// Read header
	texturex->seek(0, SEEK_SET);
	int32_t		n_tex = 0;
	int32_t*	offsets = NULL;

	// Number of textures
	if (!texturex->read(&n_tex, 4)) {
		wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
		return false;
	}

	// Texture definition offsets
	offsets = new int32_t[n_tex];
	if (!texturex->read(offsets, n_tex * 4)) {
		wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
		return false;
	}
	
	// Read the first texture definition to try to identify the format
	if (!texturex->seek(offsets[0], SEEK_SET)) {
		wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
		return false;
	}
	// Look at the name field. Is it present or not?
	char tempname[8];
	if (!texturex->read(&tempname, 8)) {
		wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
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
			wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
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
			wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
			return false;
		}
		
		// Read definition
		tdef_t tdef;
		if (txformat == TXF_NAMELESS) {
			nltdef_t nameless;
			// Auto-naming mechanism taken from DeuTex
			if (a > 99999) {
				wxLogMessage(_T("Error: More than 100000 nameless textures"));
				return false;
			}
			sprintf (tdef.name, "TEX%05d", a);
			if (!texturex->read(&nameless, 8)) {
				wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
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
			wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
			return false;
		}

		// Skip unused
		if (txformat != TXF_STRIFE11) {
			if (!texturex->seek(4, SEEK_CUR)) {
				wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
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
		//CTexture* tex = new CTexture(wxString::From8BitData(tdef.name, 8), tdef.width, tdef.height);

		// Read patches
		int16_t n_patches = 0;
		if (!texturex->read(&n_patches, 2)) {
			wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
			return false;
		}

		for (uint16_t p = 0; p < n_patches; p++) {
			// Read patch definition
			tx_patch_t pdef;
			if (!texturex->read(&pdef, 6)) {
				wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
				return false;
			}

			// Skip unused
			if (txformat != TXF_STRIFE11) {
				if (!texturex->seek(4, SEEK_CUR)) {
					wxLogMessage(_T("Error: TEXTUREx entry is corrupt"));
					return false;
				}
			}

			// Check patch id
			if (pdef.patch >= patches.size()) {
				wxLogMessage(_T("Texture %s contains non-existant patch id %d"), tex.name.c_str(), pdef.patch);
				continue;
			}

			// Create patch
			//CTPatch* ctp = new CTPatch();
			//ctp->setOffsets(pdef.left, pdef.top);

			// Load patch image entry
			//ctp->loadImage(patches[pdef.patch].entry);

			// Add it to the texture
			tex.patches.push_back(pdef);
			//tex->addPatch(ctp);
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
