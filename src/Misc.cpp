
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
#include "WadArchive.h"
#include "ZipArchive.h"
#include "Console.h"
#include "SIFormat.h"
#include <wx/filename.h>
#include "zlib/zlib.h"

/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, size_as_string, true, CVAR_SAVE)

/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

/* Misc::loadImageFromEntry
 * Loads an image from <entry> into <image>. Returns false if the
 * given entry wasn't a valid image, true otherwise
 *******************************************************************/
bool Misc::loadImageFromEntry(SImage* image, ArchiveEntry* entry, int index) {
	if (!entry)
		return false;

	// Detect entry type if it isn't already
	if (entry->getType() == EntryType::unknownType())
		EntryType::detectEntryType(entry);

	// Check for format "image" property
	if (!entry->getType()->extraProps().propertyExists("image")) {
		Global::error = "Entry type is not a valid image";
		return false;
	}

	// Firstly try SIFormat system
	if (image->open(entry->getMCData(), index))
		return true;

	// Raw images are a special case (not reliably possible to detect just from data)
	string format = entry->getType()->getFormat();
	if (format == "img_raw" && SIFormat::rawFormat()->isThisFormat(entry->getMCData()))
		return SIFormat::rawFormat()->loadImage(*image, entry->getMCData());

	// Font formats are still manually loaded for now
	else if (S_CMPNOCASE(format, "font_doom_alpha"))
		return image->loadFont0(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "font_zd_console"))
		return image->loadFont1(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "font_zd_big"))
		return image->loadFont2(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "font_bmf"))
		return image->loadBMF(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "font_mono"))
		return image->loadFontM(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "font_wolf"))
		return image->loadWolfFont(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "font_jedi_fnt"))
		return image->loadJediFNT(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "font_jedi_font"))
		return image->loadJediFONT(entry->getData(), entry->getSize());

	// Lastly, try detecting/loading via FreeImage
	else if (SIFormat::generalFormat()->isThisFormat(entry->getMCData()))
		return SIFormat::generalFormat()->loadImage(*image, entry->getMCData());

	// Otherwise use old loading stuff
	/*
	if (S_CMPNOCASE(format, "img_doom"))
		return image->loadDoomGfx(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_doom_alpha"))
		return image->loadDoomGfxA(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_doom_beta"))
		return image->loadDoomGfxB(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_doom_snea"))
		return image->loadDoomSnea(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_doom_arah")) {
		int transindex = (entry->getType()->extraProps().propertyExists("zerotransparent")) ?  0 : 255;
		return image->loadDoomArah(entry->getData(), entry->getSize(), transindex);
	}
	else if (S_CMPNOCASE(format, "img_imgz"))
		return image->loadImgz(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_quake"))
		return image->loadQuake(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_qspr"))
		return image->loadQuakeSprite(entry->getData(), entry->getSize(), index);
	else if (S_CMPNOCASE(format, "img_quaketex"))
		return image->loadQuakeTex(entry->getData(), entry->getSize(), index);
	else if (S_CMPNOCASE(format, "img_quake2wal"))
		return image->loadQuakeIIWal(entry->getData(), entry->getSize(), index);
	else if (S_CMPNOCASE(format, "img_planar"))
		return image->loadPlanar(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_4bitchunk"))
		return image->load4bitChunk(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_raw"))
		return image->loadDoomFlat(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_scsprite"))
		return image->loadSCSprite(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_scwall"))
		return image->loadSCWall(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_rott"))
		return image->loadRottGfx(entry->getData(), entry->getSize(), false);
	else if (S_CMPNOCASE(format, "img_rottmask"))
		return image->loadRottGfx(entry->getData(), entry->getSize(), true);
	else if (S_CMPNOCASE(format, "img_rottwall"))
		return image->loadDoomFlat(entry->getData(), entry->getSize(), true);
	else if (S_CMPNOCASE(format, "img_rottlbm"))
		return image->loadRottLbm(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_rottraw"))
		return image->loadRottRaw(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_rottpic"))
		return image->loadRottPic(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_wolfpic"))
		return image->loadWolfPic(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_wolfsprite"))
		return image->loadWolfSprite(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_mipimage"))
		return image->loadAnaMip(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_arttile"))
		return image->loadBuildTile(entry->getData(), entry->getSize(), index);
	else if (S_CMPNOCASE(format, "img_m8"))
		return image->loadHeretic2M8(entry->getData(), entry->getSize(), index);
	else if (S_CMPNOCASE(format, "img_m32"))
		return image->loadHeretic2M32(entry->getData(), entry->getSize(), index);
	else if (S_CMPNOCASE(format, "img_hlt"))
		return image->loadHalfLifeTex(entry->getData(), entry->getSize(), index);
	else if (S_CMPNOCASE(format, "img_jedi_bm"))
		return image->loadJediBM(entry->getData(), entry->getSize(), index);
	else if (S_CMPNOCASE(format, "img_jedi_fme"))
		return image->loadJediFME(entry->getData(), entry->getSize());
	else if (S_CMPNOCASE(format, "img_jedi_wax"))
		return image->loadJediWAX(entry->getData(), entry->getSize(), index);
	
	else {
		if (!image->loadImage(entry->getData(true), entry->getSize())) {
			Global::error = "Image format not supported by FreeImage";
			return false;
		}
		else
			return true;
	}
	*/

	// Unknown image type
	Global::error = "Entry is not a known image format";
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
	else if (entry->getType()->getFormat() == "img_doom_arah"	&& entry->getName() == "shadowpage")
		return PAL_SHADOWHACK;	// Shadowcaster
	else if (entry->getType()->getFormat() == "img_rott"		&& entry->getName() == "NICOLAS")
		return PAL_ROTTNHACK;	// Rise of the Triad
	else if (entry->getType()->getFormat() == "img_rott"		&& entry->getName() == "FINLDOOR")
		return PAL_ROTTDHACK;	// Rise of the Triad
	else if (entry->getType()->getFormat() == "img_rott"		&& entry->getName() == "FINLFIRE")
		return PAL_ROTTFHACK;	// Rise of the Triad
	else if ((entry->getType()->getFormat() == "img_rott"		&& entry->getName() == "AP_TITL")
			||(entry->getType()->getFormat() == "img_rottraw"	&& entry->getName() == "AP_WRLD"))
		return PAL_ROTTAHACK;	// Rise of the Triad
	else if (entry->getType()->getFormat() == "img_wolfpic"		&& entry->getName().Matches("IDG*"))
		return PAL_SODIDHACK;	// Spear of Destiny team screens
	else if (entry->getType()->getFormat() == "img_wolfpic"		&& entry->getName().Matches("TIT*"))
		return PAL_SODTITLEHACK;// Spear of Destiny title screens
	else if (entry->getType()->getFormat() == "img_wolfpic"		&& entry->getName().Matches("END*")) {
		long endscreen;			// Spear of Destiny ending screens (extra-hacky!)
		if (entry->getName().Right(3).ToLong(&endscreen))
			return PAL_SODENDHACK + endscreen - 81;
	}

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
	bool sixbit = false;
	ArchiveEntry* playpal = NULL;
	if (lump == PAL_ALPHAHACK)
		playpal = archive->getEntry("TITLEPAL", true);
	else if (lump == PAL_HERETICHACK)
		playpal = archive->getEntry("E2PAL", true);
	else if (lump == PAL_SHADOWHACK)
		playpal = archive->getEntry("shadowpage+1", true), sixbit = true;
	else if (lump == PAL_ROTTNHACK)
		playpal = archive->getEntry("NICPAL", true);
	else if (lump == PAL_ROTTDHACK)
		playpal = archive->getEntry("FINDRPAL", true);
	else if (lump == PAL_ROTTFHACK)
		playpal = archive->getEntry("FINFRPAL", true);
	else if (lump == PAL_ROTTAHACK)
		playpal = archive->getEntry("AP_PAL", true);
	else if (lump == PAL_SODIDHACK)
		playpal = archive->getEntry("PAL00163", true), sixbit = true;
	else if (lump == PAL_SODTITLEHACK)
		playpal = archive->getEntry("PAL00153", true), sixbit = true;
	else if (lump >= PAL_SODENDHACK) {
		int endscreen = lump - PAL_SODENDHACK;
		endscreen += 154;
		string palname = S_FMT("PAL%05d", endscreen);
		playpal = archive->getEntry(palname, true);
		sixbit = true;
	}
	if (!playpal || playpal->getSize() < 768) {
		// Search archive for any palette
		Archive::search_options_t opt;

		// Search "PLAYPAL" first
		opt.match_type = EntryType::getType("palette");
		opt.match_name = "PLAYPAL";
		opt.search_subdirs = true;
		playpal = archive->findFirst(opt);

		// Otherwise any palette will do
		if (!playpal) {
			opt.match_name = "";
			playpal = archive->findFirst(opt);
		}
	}
	if (!playpal || playpal->getSize() < 768)
		playpal = archive->getEntry("PAL", true);

	// Check it was found
	if (!playpal || playpal->getSize() < 768)
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
		if (sixbit) {
			r = (r<<2) | (r>>4);
			g = (g<<2) | (g>>4);
			b = (b<<2) | (b>>4);
		}
		pal->setColour(a, rgba_t(r, g,  b, 255));
	}

	return true;
}

/* Misc::sizeAsString
 * Converts <size> to a string representing it as a 'bytes' size, ie
 * "1.24kb", "4.00mb". Sizes under 1kb aren't given an appendage
 *******************************************************************/
string Misc::sizeAsString(uint32_t size) {
	if (size < 1024 || !size_as_string) {
		return S_FMT("%d", size);
	}
	else if (size < 1024*1024) {
		double kb = (double)size / 1024;
		return S_FMT("%1.2fkb", kb);
	}
	else {
		double mb = (double)size / (1024*1024);
		return S_FMT("%1.2fmb", mb);
	}
}

/* Misc::massRenameFilter
 * Creates a mass rename filter string from [names]
 *******************************************************************/
string Misc::massRenameFilter(wxArrayString& names) {
	// Check any names were given
	if (names.size() == 0)
		return "";

	// Init filter string
	string filter = names[0];

	// Go through names
	for (unsigned a = 1; a < names.size(); a++) {
		// If the filter string is shorter than this name, extend it with *s
		while (filter.size() < names[a].size())
			filter += '*';

		// Check each character
		for (unsigned c = 0; c < names[a].size(); c++) {
			// Skip if current filter character is *
			if (filter[c] == '*')
				continue;

			// Check for character mismatch
			if (filter[c] != names[a][c])
				filter[c] = '*';
		}
	}

	return filter;
}

/* Misc::doMassRename
 * Performs a mass rename on [names] using the filter [name_filter].
 * Any * in the filter means that character should not be changed
 *******************************************************************/
void Misc::doMassRename(wxArrayString& names, string name_filter) {
	// Go through names
	for (unsigned a = 0; a < names.size(); a++) {
		string& name = names[a];

		// If this name is shorter than the filter string, extend it with spaces
		//while (name.size() < name_filter.size())
		//	name += " ";

		// If the filter string is shorter than the name, just truncate the name
		if (name_filter.size() < name.size())
			name.Truncate(name_filter.size());

		// Go through filter characters
		for (unsigned c = 0; c < name_filter.size(); c++) {
			// Check character
			if (name_filter[c] == '*')
				continue;					// Skip if *
			else {
				// First check that we aren't past the end of the name
				if (c >= name.size()) {
					// If we are, pad it with spaces
					while (name.size() <= c)
						name += " ";
				}

				// Replace character
				name[c] = name_filter[c];
			}
		}
	}
}

/* Misc::rgbToHsl
 * Converts a colour from RGB to HSL colourspace
 *******************************************************************/
hsl_t Misc::rgbToHsl(double r, double g, double b) {
	hsl_t ret;
	double v_min = MIN(r, MIN(g, b));
	double v_max = MAX(r, MAX(g, b));
	double delta = v_max - v_min;

	// Determine V
	ret.l = (v_max + v_min) * 0.5;

	if (delta == 0)
		ret.h = ret.s = 0;		// Grey (r==g==b)
	else {
		// Determine S
		if (ret.l < 0.5)
			ret.s = delta / (v_max + v_min);
		else
			ret.s = delta / (2.0 - v_max - v_min);

		// Determine H
		if (r == v_max)
			ret.h = (g - b) / delta;
		else if (g == v_max)
			ret.h = 2.0 + (b - r) / delta;
		else if (b == v_max)
			ret.h = 4.0 + (r - g) / delta;

		ret.h /= 6.0;

		if (ret.h < 0)
			ret.h += 1.0;
	}

	return ret;
}





// CRC-32 stuff

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
uint32_t update_crc(uint32_t crc, const uint8_t *buf, uint32_t len) {
	uint32_t c = crc;

	if (!crc_table_computed)
		make_crc_table();

	for (uint32_t n = 0; n < len; n++)
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);

	return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
uint32_t Misc::crc(const uint8_t *buf, uint32_t len) {
	return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}
