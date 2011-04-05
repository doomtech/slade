
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
#include "ArchiveManager.h"
#include "ResourceManager.h"
#include "Misc.h"
#include "SImage.h"
#include <wx/colour.h>


/*******************************************************************
 * CTPATCH CLASS FUNCTIONS
 *******************************************************************/

/* CTPatch::CTPatch
 * CTPatch class default constructor
 *******************************************************************/
CTPatch::CTPatch() {
	this->offset_x = 0;
	this->offset_y = 0;
}

/* CTPatch::CTPatch
 * CTPatch class constructor w/initial values
 *******************************************************************/
CTPatch::CTPatch(string name, int16_t offset_x, int16_t offset_y) {
	this->name = name;
	this->offset_x = offset_x;
	this->offset_y = offset_y;
}

/* CTPatch::CTPatch
 * CTPatch class constructor copying another CTPatch
 *******************************************************************/
CTPatch::CTPatch(CTPatch* copy) {
	name = copy->name;
	offset_x = copy->offset_x;
	offset_y = copy->offset_y;
}

/* CTPatch::~CTPatch
 * CTPatch class destructor
 *******************************************************************/
CTPatch::~CTPatch() {
}

/* CTPatch::searchEntry
 * Search for a patch's entry in the given archive or its parents
 *******************************************************************/
void CTPatch::searchEntry(Archive* parent) {
	/*
	// Attempt to find patch entry
	Archive::search_options_t options;	options.match_name = name;

	// First, search parent archive (patches namespace > global namespace > graphics namespace)
	// TODO 1: search order should be patches > global for treeless archives, patches > graphics for others.
	// TODO 2: For ZDoom textures, if the keyword "Graphic" is used instead of "Patch" in the texture
	// definition, then priority order should be graphics > patches instead.
	if (parent) {
		options.match_namespace = "patches";
		entry = parent->findLast(options);

		if (!entry) {
			options.match_namespace = "";
			entry = parent->findLast(options);
		}

		if (!entry) {
			options.match_namespace = "graphic";
			entry = parent->findLast(options);
		}
	}

	// Next, search open resource archives + base resource archive
	if (!entry) {
		options.match_namespace = "patches";
		entry = theArchiveManager->findResourceEntry(options, parent);
	}
	if (!entry) {
		options.match_namespace = "";
		entry = theArchiveManager->findResourceEntry(options, parent);
	}
	if (!entry) {
		options.match_namespace = "graphic";
		entry = theArchiveManager->findResourceEntry(options, parent);
	}
	*/
}

/* CTPatch::getPatchEntry
 * Returns the entry (if any) associated with this patch via the
 * resource manager. Entries in [parent] will be prioritised over
 * entries in any other open archive
 *******************************************************************/
ArchiveEntry* CTPatch::getPatchEntry(Archive* parent) {
	// Default patches should be in patches namespace
	ArchiveEntry* entry = theResourceManager->getPatchEntry(name, "patches", parent);

	// Not found in patches, check in graphics namespace
	if (!entry) entry = theResourceManager->getPatchEntry(name, "graphics", parent);

	return entry;
}

/*******************************************************************
 * CTPATCHEX CLASS FUNCTIONS
 *******************************************************************/

/* CTPatchEx::CTPatchEx
 * CTPatchEx class default constructor
 *******************************************************************/
CTPatchEx::CTPatchEx() {
	flip_x = false;
	flip_y = false;
	rotation = 0;
	alpha = 1.0f;
	style = "Copy";
	blendtype = 0;
	type = PTYPE_PATCH;
}

/* CTPatchEx::CTPatchEx
 * CTPatchEx class constructor w/basic initial values
 *******************************************************************/
CTPatchEx::CTPatchEx(string name, int16_t offset_x, int16_t offset_y, uint8_t type)
: CTPatch(name, offset_x, offset_y) {
	flip_x = false;
	flip_y = false;
	rotation = 0;
	alpha = 1.0f;
	style = "Copy";
	blendtype = 0;
	this->type = type;
}

/* CTPatchEx::CTPatchEx
 * CTPatchEx class constructor copying a regular CTPatch
 *******************************************************************/
CTPatchEx::CTPatchEx(CTPatch* copy) {
	flip_x = false;
	flip_y = false;
	rotation = 0;
	alpha = 1.0f;
	style = "Copy";
	blendtype = 0;
	offset_x = copy->xOffset();
	offset_y = copy->yOffset();
	name = copy->getName();
	type = PTYPE_PATCH;
}

/* CTPatchEx::CTPatchEx
 * CTPatchEx class constructor copying another CTPatchEx
 *******************************************************************/
CTPatchEx::CTPatchEx(CTPatchEx* copy) {
	flip_x = copy->flip_x;
	flip_y = copy->flip_y;
	rotation = copy->rotation;
	alpha = copy->alpha;
	style = copy->style;
	blendtype = copy->blendtype;
	colour = copy->colour;
	offset_x = copy->offset_x;
	offset_y = copy->offset_y;
	name = copy->name;
	type = copy->type;
	translation.copy(copy->translation);
}

/* CTPatchEx::~CTPatchEx
 * CTPatchEx class destructor
 *******************************************************************/
CTPatchEx::~CTPatchEx() {
}

/* CTPatchEx::getPatchEntry
 * Returns the entry (if any) associated with this patch via the
 * resource manager. Entries in [parent] will be prioritised over
 * entries in any other open archive
 *******************************************************************/
ArchiveEntry* CTPatchEx::getPatchEntry(Archive* parent) {
	// 'Patch' type: patches > graphics
	if (type == PTYPE_PATCH) {
		ArchiveEntry* entry = theResourceManager->getPatchEntry(name, "patches", parent);
		if (!entry) entry = theResourceManager->getPatchEntry(name, "graphics", parent);
		return entry;
	}

	// 'Graphic' type: graphics > patches
	if (type == PTYPE_GRAPHIC) {
		ArchiveEntry* entry = theResourceManager->getPatchEntry(name, "graphics", parent);
		if (!entry) entry = theResourceManager->getPatchEntry(name, "patches", parent);
		return entry;
	}
	// Silence warnings
	return NULL;
}

/* CTPatchEx::parse
 * Parses a ZDoom TEXTURES format patch definition
 *******************************************************************/
bool CTPatchEx::parse(Tokenizer& tz, uint8_t type) {
	// Read basic info
	this->type = type;
	name = tz.getToken().Upper();
	tz.getToken();	// Skip ,
	offset_x = tz.getInteger();
	tz.getToken();	// Skip ,
	offset_y = tz.getInteger();

	// Check if there is any extended info
	if (tz.peekToken() == "{") {
		// Skip {
		tz.getToken();

		// Parse extended info
		string property = tz.getToken();
		while (property != "}") {
			// FlipX
			if (S_CMPNOCASE(property, "FlipX"))
				flip_x = true;

			// FlipY
			if (S_CMPNOCASE(property, "FlipY"))
				flip_y = true;

			// Rotate
			if (S_CMPNOCASE(property, "Rotate"))
				rotation = tz.getInteger();

			// Translation
			if (S_CMPNOCASE(property, "Translation")) {
				// Add first translation string
				translation.parse(tz.getToken());

				// Add any subsequent translations (separated by commas)
				while (tz.peekToken() == ",") {
					tz.getToken();	// Skip ,
					translation.parse(tz.getToken());
				}

				blendtype = 1;
			}

			// Blend
			if (S_CMPNOCASE(property, "Blend")) {
				double val;
				wxColour col;
				blendtype = 2;

				// Read first value
				string first = tz.getToken();

				// If no second value, it's just a colour string
				if (tz.peekToken() != ",") {
					col.Set(first);
					colour.set(col.Red(), col.Green(), col.Blue());
				}
				else {
					// Second value could be alpha or green
					tz.getToken();	// Skip ,
					double second = tz.getDouble();

					// If no third value, it's an alpha value
					if (tz.peekToken() != ",") {
						col.Set(first);
						colour.set(col.Red(), col.Green(), col.Blue(), second*255);
						blendtype = 3;
					}
					else {
						// Third value exists, must be R,G,B,A format
						tz.getToken();	// Skip ,
						first.ToDouble(&val);
						colour.r = val*255;
						colour.g = second*255;
						colour.b = tz.getDouble()*255;
						tz.getToken();	// Skip ,
						colour.a = tz.getDouble()*255;
						blendtype = 3;
					}
				}
			}

			// Alpha
			if (S_CMPNOCASE(property, "Alpha"))
				alpha = tz.getFloat();

			// Style
			if (S_CMPNOCASE(property, "Style"))
				style = tz.getToken();

			// Read next property name
			property = tz.getToken();
		}
	}

	return true;
}

/* CTPatchEx::asText
 * Returns a text representation of the patch in ZDoom TEXTURES
 * format
 *******************************************************************/
string CTPatchEx::asText() {
	// Init text string
	string typestring = "Patch";
	if (type == PTYPE_GRAPHIC) typestring = "Graphic";
	string text = S_FMT("\t%s %s, %d, %d\n", CHR(typestring), CHR(name), offset_x, offset_y);

	// Check if we need to write any extra properties
	if (!flip_x && !flip_y && rotation == 0 && blendtype == 0 && alpha == 1.0f && S_CMPNOCASE(style, "Copy"))
		return text;
	else
		text += "\t{\n";

	// Write patch properties
	if (flip_x)
		text += "\t\tFlipX\n";
	if (flip_y)
		text += "\t\tFlipY\n";
	if (rotation != 0)
		text += S_FMT("\t\tRotate %d\n", rotation);
	if (blendtype == 1 && !translation.isEmpty()) {
		text += "\t\tTranslation ";
		text += translation.asText();
		text += "\n";
	}
	if (blendtype >= 2) {
		wxColour col(colour.r, colour.g, colour.b);
		text += S_FMT("\t\tBlend \"%s\"", CHR(col.GetAsString(wxC2S_HTML_SYNTAX)));

		if (blendtype == 3)
			text += S_FMT(", %1.1f\n", (double)colour.a / 255.0);
		else
			text += "\n";
	}
	if (alpha < 1.0f)
		text += S_FMT("\t\tAlpha %1.2f\n", alpha);
	if (!(S_CMPNOCASE(style, "Copy")))
		text += S_FMT("\t\tStyle %s\n", CHR(style));

	// Write ending
	text += "\t}\n";

	return text;
}


/*******************************************************************
 * CTEXTURE CLASS FUNCTIONS
 *******************************************************************/

/* CTexture::CTexture
 * CTexture class constructor
 *******************************************************************/
CTexture::CTexture() {
	this->width = 0;
	this->height = 0;
	this->name = "";
	this->scale_x = 1.0;
	this->scale_y = 1.0;
	this->world_panning = false;
	this->extended = false;
	this->optional = false;
	this->no_decals = false;
	this->null_texture = false;
	this->offset_x = 0;
	this->offset_y = 0;
	this->type = "Texture";
}

/* CTexture::~CTexture
 * CTexture class destructor
 *******************************************************************/
CTexture::~CTexture() {
	for (unsigned a = 0; a < patches.size(); a++)
		delete patches[a];
}

/* CTexture::copyTexture
 * Copies the texture [tex] to this texture
 *******************************************************************/
void CTexture::copyTexture(CTexture* tex) {
	// Clear current texture
	clear();

	// Copy texture info
	this->name = tex->name;
	this->width = tex->width;
	this->height = tex->height;
	this->scale_x = tex->scale_x;
	this->scale_y = tex->scale_y;
	this->world_panning = tex->world_panning;
	this->extended = tex->extended;
	this->optional = tex->optional;
	this->no_decals = tex->no_decals;
	this->null_texture = tex->null_texture;
	this->offset_x = tex->offset_x;
	this->offset_y = tex->offset_y;
	this->type = tex->type;

	// Copy patches
	for (unsigned a = 0; a < tex->nPatches(); a++) {
		if (tex->extended) {
			CTPatchEx* patch = new CTPatchEx((CTPatchEx*)tex->getPatch(a));
			patches.push_back(patch);
		}
		else {
			CTPatch* patch = tex->getPatch(a);
			addPatch(patch->getName(), patch->xOffset(), patch->yOffset());//, patch->getEntry());
		}
	}
}

/* CTexture::getPatch
 * Returns the patch at [index], or NULL if [index] is out of bounds
 *******************************************************************/
CTPatch* CTexture::getPatch(size_t index) {
	// Check index
	if (index >= patches.size())
		return NULL;

	// Return patch at index
	return patches[index];
}

/* CTexture::clear
 * Clears all texture data
 *******************************************************************/
void CTexture::clear() {
	this->name = "";
	this->width = 0;
	this->height = 0;
	this->scale_x = 1.0;
	this->scale_y = 1.0;
	this->world_panning = false;
	this->extended = false;
	this->optional = false;
	this->no_decals = false;
	this->null_texture = false;
	this->offset_x = 0;
	this->offset_y = 0;

	// Clear patches
	this->patches.clear();
	for (unsigned a = 0; a < patches.size(); a++)
		delete patches[a];
}

/* CTexture::addPatch
 * Adds a patch to the texture with the given attributes, at [index].
 * If [index] is -1, the patch is added to the end of the list.
 *******************************************************************/
bool CTexture::addPatch(string patch, int16_t offset_x, int16_t offset_y, int index) {
	// Create new patch
	CTPatch* np;
	if (extended)
		np = new CTPatchEx(patch, offset_x, offset_y);
	else
		np = new CTPatch(patch, offset_x, offset_y);

	// Add it either after [index] or at the end
	if (index >= 0 && (unsigned) index < patches.size())
		patches.insert(patches.begin() + index, np);
	else
		patches.push_back(np);

	// Announce
	announce("patches_modified");

	return true;
}

/* CTexture::removePatch
 * Removes the patch at [index]. Returns false if [index] is invalid,
 * true otherwise
 *******************************************************************/
bool CTexture::removePatch(size_t index) {
	// Check index
	if (index >= patches.size())
		return false;

	// Remove the patch
	delete patches[index];
	patches.erase(patches.begin() + index);

	// Announce
	announce("patches_modified");

	return true;
}

/* CTexture::removePatch
 * Removes all instances of [patch] from the texture. Returns true if
 * any were removed, false otherwise
 *******************************************************************/
bool CTexture::removePatch(string patch) {
	// Go through patches
	bool removed = false;
	vector<CTPatch*>::iterator i = patches.begin();
	while (i != patches.end()) {
		if (S_CMP((*i)->getName(), patch)) {
			delete (*i);
			patches.erase(i);
			removed = true;
		}
		else
			i++;
	}

	if (removed)
		announce("patches_modified");

	return removed;
}

/* CTexture::replacePatch
 * Replaces the patch at [index] with [newpatch], and updates its
 * associated ArchiveEntry with [newentry]. Returns false if [index]
 * is out of bounds, true otherwise
 *******************************************************************/
bool CTexture::replacePatch(size_t index, string newpatch) {
	// Check index
	if (index >= patches.size())
		return false;

	// Replace patch at [index] with new
	patches[index]->setName(newpatch);

	// Announce
	announce("patches_modified");

	return true;
}

/* CTexture::duplicatePatch
 * Duplicates the patch at [index], placing the duplicated patch
 * at [offset_x],[offset_y] from the original. Returns false if
 * [index] is out of bounds, true otherwise
 *******************************************************************/
bool CTexture::duplicatePatch(size_t index, int16_t offset_x, int16_t offset_y) {
	// Check index
	if (index >= patches.size())
		return false;

	// Get patch info
	CTPatch* dp = patches[index];

	// Add duplicate patch
	if (extended)
		patches.insert(patches.begin() + index, new CTPatchEx((CTPatchEx*)patches[index]));
	else
		patches.insert(patches.begin() + index, new CTPatch(patches[index]));

	// Offset patch by given amount
	patches[index+1]->setOffsetX(dp->xOffset() + offset_x);
	patches[index+1]->setOffsetY(dp->yOffset() + offset_y);

	// Announce
	announce("patches_modified");

	return true;
}

/* CTexture::swapPatches
 * Swaps the patches at [p1] and [p2]. Returns false if either index
 * is invalid, true otherwise
 *******************************************************************/
bool CTexture::swapPatches(size_t p1, size_t p2) {
	// Check patch indices are correct
	if (p1 >= patches.size() || p2 >= patches.size())
		return false;

	// Swap the patches
	CTPatch* temp = patches[p1];
	patches[p1] = patches[p2];
	patches[p2] = temp;

	// Announce
	announce("patches_modified");

	return true;
}

/* CTexture::parse
 * Parses a TEXTURES format texture definition
 *******************************************************************/
bool CTexture::parse(Tokenizer& tz, string type) {
	// Check if optional
	if (S_CMPNOCASE(tz.peekToken(), "optional")) {
		tz.getToken();	// Skip it
		optional = true;
	}

	// Read basic info
	this->type = type;
	this->extended = true;
	name = tz.getToken().Upper();
	tz.getToken();	// Skip ,
	width = tz.getInteger();
	tz.getToken();	// Skip ,
	height = tz.getInteger();

	// Check for extended info
	if (tz.peekToken() == "{") {
		tz.getToken();	// Skip {

		// Read properties
		string property = tz.getToken();
		while (property != "}") {
			// Check if end of text is reached (error)
			if (property.IsEmpty()) {
				wxLogMessage("Error parsing texture %s: End of text found, missing } perhaps?", CHR(name));
				return false;
			}

			// XScale
			if (S_CMPNOCASE(property, "XScale"))
				scale_x = tz.getFloat();

			// YScale
			if (S_CMPNOCASE(property, "YScale"))
				scale_y = tz.getFloat();

			// Offset
			if (S_CMPNOCASE(property, "Offset")) {
				offset_x = tz.getInteger();
				tz.getToken();	// Skip ,
				offset_y = tz.getInteger();
			}

			// WorldPanning
			if (S_CMPNOCASE(property, "WorldPanning"))
				world_panning = true;

			// NoDecals
			if (S_CMPNOCASE(property, "NoDecals"))
				no_decals = true;

			// NullTexture
			if (S_CMPNOCASE(property, "NullTexture"))
				null_texture = true;

			// Patch
			if (S_CMPNOCASE(property, "Patch")) {
				CTPatchEx* patch = new CTPatchEx();
				patch->parse(tz);
				patches.push_back(patch);
			}

			// Graphic
			if (S_CMPNOCASE(property, "Graphic")) {
				CTPatchEx* patch = new CTPatchEx();
				patch->parse(tz, PTYPE_GRAPHIC);
				patches.push_back(patch);
			}

			// Read next property
			property = tz.getToken();
		}
	}

	return true;
}

/* CTexture::asText
 * Returns a string representation of the texture, in ZDoom TEXTURES
 * format
 *******************************************************************/
string CTexture::asText() {
	// Can't write non-extended texture as text
	if (!extended)
		return "";

	// Init text string
	string text = S_FMT("%s %s, %d, %d\n{\n", CHR(type), CHR(name), width, height);

	// Write texture properties
	if (scale_x != 1.0)
		text += S_FMT("\tXScale %1.1f\n", scale_x);
	if (scale_y != 1.0)
		text += S_FMT("\tYScale %1.1f\n", scale_y);
	if (offset_x != 0 || offset_y != 0)
		text += S_FMT("\tOffset %d, %d\n", offset_x, offset_y);
	if (world_panning)
		text += "\tWorldPanning\n";
	if (no_decals)
		text += "\tNoDecals\n";
	if (null_texture)
		text += "\tNullTexture\n";

	// Write patches
	for (unsigned a = 0; a < patches.size(); a++)
		text += ((CTPatchEx*)patches[a])->asText();

	// Write ending
	text += "}\n\n";

	return text;
}

/* CTexture::convertExtended
 * Converts the texture to 'extended' (ZDoom TEXTURES) format
 *******************************************************************/
bool CTexture::convertExtended() {
	// Don't convert if already extended
	if (extended)
		return true;

	// Convert scale if needed
	if (scale_x == 0) scale_x = 1;
	if (scale_y == 0) scale_y = 1;

	// Convert all patches over to extended format
	for (unsigned a = 0; a < patches.size(); a++) {
		CTPatchEx* expatch = new CTPatchEx(patches[a]);
		delete patches[a];
		patches[a] = expatch;
	}

	// Set extended flag
	extended = true;

	return true;
}

/* CTexture::toImage
 * Generates a SImage representation of this texture, using patches
 * from [parent] primarily, and the palette [pal]
 *******************************************************************/
bool CTexture::toImage(SImage& image, Archive* parent, Palette8bit* pal, bool force_rgba) {
	// Init image
	image.clear();
	image.resize(width, height);

	// Add patches
	SImage p_img;
	si_drawprops_t dp;
	dp.src_alpha = false;
	if (extended) {
		// Extended texture

		// Add each patch to image
		for (unsigned a = 0; a < patches.size(); a++) {
			CTPatchEx* patch = (CTPatchEx*)patches[a];

			// Load patch entry
			if (!Misc::loadImageFromEntry(&p_img, patch->getPatchEntry(parent)))
				continue;

			// Convert to RGBA if forced
			if (force_rgba)
				p_img.convertRGBA(pal);

			// Flip/rotate if needed
			if (patch->flipX())
				p_img.mirror(false);
			if (patch->flipY())
				p_img.mirror(true);
			if (patch->getRotation() != 0)
				p_img.rotate(patch->getRotation());

			// Setup transparency blending
			dp.blend = NORMAL;
			dp.alpha = 1.0f;
			dp.src_alpha = false;
			if (patch->getStyle() == "CopyAlpha")
				dp.src_alpha = true;
			else if (patch->getStyle() == "Translucent")
				dp.alpha = patch->getAlpha();
			else if (patch->getStyle() == "Add") {
				dp.blend = ADD;
				dp.alpha = patch->getAlpha();
			}
			else if (patch->getStyle() == "Subtract") {
				dp.blend = SUBTRACT;
				dp.alpha = patch->getAlpha();
			}
			else if (patch->getStyle() == "ReverseSubtract") {
				dp.blend = REVERSE_SUBTRACT;
				dp.alpha = patch->getAlpha();
			}
			else if (patch->getStyle() == "Modulate") {
				dp.blend = MODULATE;
				dp.alpha = patch->getAlpha();
			}

			// Setup patch colour
			if (patch->getBlendType() == 1)
				p_img.applyTranslation(&(patch->getTranslation()), pal);
			else if (patch->getBlendType() == 2)
				p_img.colourise(patch->getColour(), pal);
			else if (patch->getBlendType() == 3)
				p_img.tint(patch->getColour(), patch->getColour().fa(), pal);


			// Add patch to texture image
			image.drawImage(p_img, patch->xOffset(), patch->yOffset(), dp, pal, pal);
		}
	}
	else {
		// Normal texture

		// Add each patch to image
		for (unsigned a = 0; a < patches.size(); a++) {
			CTPatch* patch = patches[a];
			if (Misc::loadImageFromEntry(&p_img, patch->getPatchEntry(parent)))
				image.drawImage(p_img, patch->xOffset(), patch->yOffset(), dp, pal, pal);
		}
	}

	return true;
}
