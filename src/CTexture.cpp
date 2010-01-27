
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
#include "Misc.h"


/*******************************************************************
 * CTPATCH CLASS FUNCTIONS
 *******************************************************************/

/* CTPatch::CTPatch
 * CTPatch class constructor
 *******************************************************************/
CTPatch::CTPatch() {
	// Init variables
	offsets.set(0, 0);
	scale.set(1, 1);
}

/* CTPatch::~CTPatch
 * CTPatch class destructor
 *******************************************************************/
CTPatch::~CTPatch() {
}

bool CTPatch::loadImage(ArchiveEntry *gfx_entry) {
	return Misc::loadImageFromEntry(&image, gfx_entry);
}


/*******************************************************************
 * CTEXTURE CLASS FUNCTIONS
 *******************************************************************/

/* CTexture::CTexture
 * CTexture class constructor
 *******************************************************************/
CTexture::CTexture(string name, uint32_t width, uint32_t height) {
	// Init variables
	this->name = name;
	this->width = width;
	this->height = height;
	scale.set(1, 1);
}

/* CTexture::~CTexture
 * CTexture class destructor
 *******************************************************************/
CTexture::~CTexture() {
	// Cleanup patches
	for (size_t a = 0; a < patches.size(); a++)
		delete patches[a];
}

bool CTexture::addPatch(CTPatch* patch) {
	if (!patch)
		return false;

	patches.push_back(patch);

	return true;
}

CTPatch* CTexture::getPatch(size_t index) {
	if (index >= patches.size())
		return NULL;
	else
		return patches[index];
}