
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Misc.cpp
 * Description: Misc functions that don't necessarily belong
 *              anywhere else (generally stuff that involves multiple
 *              unrelated classes)
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
#include "Misc.h"
#include "EntryDataFormat.h"


/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

/* Misc::loadImageFromEntry
 * Loads an image from <entry> into <image>. Returns false if the
 * given entry wasn't a valid image, true otherwise
 *******************************************************************/
bool Misc::loadImageFromEntry(SImage* image, ArchiveEntry* entry) {
	if (!entry)
		return false;

	// Detect entry type if it isn't already
	if (entry->getType() == EntryType::unknownType())
		EntryType::detectEntryType(entry);

	/*
	switch (entry->getType()->getFormat()) {
#define xa(id, name, val)
#define xb(id, name)
#define xx(id, name, func)
#define xy(id, name, func, load)	case id: return image->load(entry->getData(true), entry->getSize());
#define xz(id, name)
#include "EntryTypeList.h"

		// General image formats (that FreeImage supports at least)
		default:
			if (!image->loadImage(entry->getData(true), entry->getSize())) {
				Global::error = _T("Image format not supported by FreeImage");
				return false;
			}
			else
				return true;
	}
	*/

	// Check for format "image" property
	if (!entry->getType()->extraProps().propertyExists("image")) {
		Global::error = "Entry type is not a valid image";
		return false;
	}

	// Load depending on format
	string format = entry->getType()->getFormat();
	if (s_cmpnocase(format, _T("img_doom")))
		return image->loadDoomGfx(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("img_doom_alpha")))
		return image->loadDoomGfxA(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("img_doom_beta")))
		return image->loadDoomGfxB(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("img_doom_snea")))
		return image->loadDoomSnea(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("img_doom_arah")))
		return image->loadDoomArah(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("img_imgz")))
		return image->loadImgz(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("img_legacy")))
		return image->loadDoomLegacy(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("img_planar")))
		return image->loadPlanar(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("img_4bitchunk")))
		return image->load4bitChunk(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, "img_raw"))
		return image->loadDoomFlat(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("font_doom_alpha")))
		return image->loadFont0(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("font_zd_console")))
		return image->loadFont1(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("font_zd_big")))
		return image->loadFont2(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("font_bmf")))
		return image->loadBMF(entry->getData(), entry->getSize());
	else if (s_cmpnocase(format, _T("font_mono")))
		return image->loadFontM(entry->getData(), entry->getSize());
	else {
		if (!image->loadImage(entry->getData(true), entry->getSize())) {
			Global::error = _T("Image format not supported by FreeImage");
			return false;
		}
		else
			return true;
	}

	// Unknown image type
	Global::error = _T("Entry is not a known image format");
	return false;
}

/* Misc::detectPaletteHack
 * Detects the few known cases where a picture does not use PLAYPAL
 * as its default palette.
 *******************************************************************/
int	Misc::detectPaletteHack(ArchiveEntry* entry)
{
	if (entry == NULL || entry->getType() == NULL)
		return PAL_NOHACK;
	else if (entry->getType()->getFormat() == "img_doom_arah"	&& entry->getName() == "TITLEPIC")
		return PAL_ALPHAHACK;	// Doom Alpha 0.2
	else if (entry->getType()->getFormat() == "img_doom_snea"	&& entry->getName() == "TITLEPIC")
		return PAL_ALPHAHACK;	// Doom Alpha 0.4 and 0.5
	else if (entry->getType()->getFormat() == "img_raw"			&& entry->getName() == "E2END")
		return PAL_HERETICHACK;	// Heretic

	// Default:
	return PAL_NOHACK;
}


/* Misc::loadPaletteFromArchive
 * Writes palette information from the PLAYPAL entry in <archive> to
 * <pal>. Returns false if PLAYPAL entry was missing or invalid,
 * true otherwise
 *******************************************************************/
bool Misc::loadPaletteFromArchive(Palette8bit* pal, Archive* archive, int lump) {
	// Check parameters
	if (!pal || !archive)
		return false;

	// Find PLAYPAL entry
	ArchiveEntry* playpal = NULL;
	if (lump == PAL_ALPHAHACK)
		playpal = archive->getEntry(_T("TITLEPAL"));
	else if (lump == PAL_HERETICHACK)
		playpal = archive->getEntry(_T("E2PAL"));
	if (!playpal)
		playpal = archive->getEntry(_T("PLAYPAL"));

	// Check it was found
	if (!playpal)
		return false;

	// Check it is the correct size
	if (playpal->getSize() < 256*3)
		return false;

	// Read palette colours
	const uint8_t* playpal_dat = playpal->getData(true);
	int c = 0;
	for (int a = 0; a < 256; a++) {
		uint8_t r = playpal_dat[c++];
		uint8_t g = playpal_dat[c++];
		uint8_t b = playpal_dat[c++];
		pal->setColour(a, rgba_t(r, g,  b, 255));
	}

	return true;
}

/* Misc::sizeAsString
 * Converts <size> to a string representing it as a 'bytes' size, ie
 * "1.24kb", "4.00mb". Sizes under 1kb aren't given an appendage
 *******************************************************************/
string Misc::sizeAsString(uint32_t size) {
	if (size < 1024) {
		return s_fmt(_T("%d"), size);
	}
	else if (size < 1024*1024) {
		double kb = (double)size / 1024;
		return s_fmt(_T("%1.2fkb"), kb);
	}
	else {
		double mb = (double)size / (1024*1024);
		return s_fmt(_T("%1.2fmb"), mb);
	}
}



// CRC stuff

/* Table of CRCs of all 8-bit messages. */
uint32_t crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void) {
	uint32_t c;
	int n, k;

	for (n = 0; n < 256; n++) {
		c = (uint32_t) n;

		for (k = 0; k < 8; k++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}

		crc_table[n] = c;
	}

	crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below)). */
uint32_t update_crc(uint32_t crc, uint8_t *buf, uint32_t len) {
	uint32_t c = crc;

	if (!crc_table_computed)
		make_crc_table();

	for (uint32_t n = 0; n < len; n++)
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);

	return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
uint32_t Misc::crc(uint8_t *buf, uint32_t len) {
	return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}
