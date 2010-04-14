
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    CompositeTexture.cpp
 * Description: C(omposite)Texture class, represents a composite
 *              texture as described in TEXTUREx entries
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
#include "CTexture.h"


/*******************************************************************
 * CTPATCH CLASS FUNCTIONS
 *******************************************************************/

/* CTPatch::CTPatch
 * CTPatch class constructor
 *******************************************************************/
CTPatch::CTPatch() {
}

/* CTPatch::~CTPatch
 * CTPatch class destructor
 *******************************************************************/
CTPatch::~CTPatch() {
}


/*******************************************************************
 * CTEXTURE CLASS FUNCTIONS
 *******************************************************************/

/* CTexture::CTexture
 * CTexture class constructor
 *******************************************************************/
CTexture::CTexture() {
}

/* CTexture::~CTexture
 * CTexture class destructor
 *******************************************************************/
CTexture::~CTexture() {
}

/* CTexture::getPatch
 * Returns the patch at [index], or NULL if [index] is out of bounds
 *******************************************************************/
CTPatch* CTexture::getPatch(size_t index) {
	// Check index
	if (index >= patches.size())
		return NULL;

	// Return patch at index
	return &patches[index];
}

/* CTexture::clear
 * Clears all texture data
 *******************************************************************/
void CTexture::clear() {
	this->name = _T("");
	this->width = 0;
	this->height = 0;
	this->scale_x = 1.0;
	this->scale_y = 1.0;
	this->patches.clear();
	this->ex_props.clear();
}

/* CTexture::fromTX
 * Reads in texture information from a TEXTUREx format texture
 *******************************************************************/
bool CTexture::fromTX(tx_texture_t& info, PatchTable& ptable) {
	// Clear any current texture data
	clear();

	// Setup texture properties
	name = info.name;
	width = info.width;
	height = info.height;
	scale_x = 1.0;
	scale_y = 1.0;

	// Convert from TEXTUREx scale
	if (info.scale_x != 0)
		scale_x = (double)info.scale_x / 8.0;
	if (info.scale_y != 0)
		scale_y = (double)info.scale_y / 8.0;

	// Flags
	if (info.flags & TX_WORLDPANNING)
		ex_props.addFlag(_T("WorldPanning"));

	// Setup patches
	for (size_t a = 0; a < info.patches.size(); a++) {
		// Get patch index
		uint16_t index = info.patches[a].patch;

		// Check patch index is in range
		if (index >= ptable.nPatches()) {
			wxLogMessage(_T("Texture %s contains invalid patch index %d"), name.c_str(), index);
			continue;
		}

		// Create patch
		CTPatch patch;
		patch.setPatchName(ptable.patchName(index));
		patch.setPatchEntry(ptable.patchEntry(index));
		patch.setOffsetX(info.patches[a].left);
		patch.setOffsetY(info.patches[a].top);

		// Add it to the texture
		patches.push_back(patch);
	}

	// Done
	return true;
}

/* CTexture::toTX
 * Writes texture information to a TEXTUREx format texture
 *******************************************************************/
bool CTexture::toTX(tx_texture_t& info, PatchTable& ptable) {
	// Write texture properties
	info.name = name;
	info.width = width;
	info.height = height;
	info.scale_x = uint8_t(scale_x * 8);
	info.scale_y = uint8_t(scale_y * 8);

	// Write flags
	uint16_t flags = 0;
	if (ex_props.propertyExists(_T("WorldPanning")))
		flags |= TX_WORLDPANNING;
	info.flags = flags;

	// Write patches info
	info.patches.clear();
	for (size_t a = 0; a < patches.size(); a++) {
		// Get patch index
		int16_t index = ptable.patchIndex(patches[a].getPatchName());
		if (index < 0)
			index = ptable.patchIndex(patches[a].getPatchEntry());
		if (index < 0) {
			wxLogMessage(_T("Could not find patch %s in patch table"), patches[a].getPatchName().c_str());
			continue;
		}

		// Create patch
		tx_patch_t patch;
		patch.left = patches[a].xOffset();
		patch.top = patches[a].yOffset();
		patch.patch = index;

		// Write patch info
		info.patches.push_back(patch);
	}

	// Done
	return true;
}

/* CTexture::fromZD
 * Reads in texture information from a ZDoom TEXTURES format texture
 *******************************************************************/
bool CTexture::fromZD(Tokenizer& tz) {
	// TODO: Implement reading TEXTURES definition
	return false;
}

/* CTexture::toZD
 * Writes texture information to a ZDoom TEXTURES format texture
 *******************************************************************/
string CTexture::toZD() {
	// TODO: Implement writing TEXUTRES definition
	return wxEmptyString;
}
