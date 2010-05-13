/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    EntryType.cpp
 * Description: Entry Type detection system
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
#include "EntryType.h"
#include "Tokenizer.h"
#include "Console.h"
#include "ArchiveManager.h"
#include "ZipArchive.h"
#include "WadArchive.h"
#include "BinaryControlLump.h"
#include <wx/dir.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/

// Basic formats system for now
struct id_format_t {
	string 		id;
	uint16_t	format;
};
id_format_t formats[] = {
#define xa(id, name, val)			{ name,	id },
#define xb(id, name)				{ name,	id },
#define xx(id, name, func)			{ name,	id },
#define xy(id, name, func, load)	{ name,	id },
#define xz(id, name)				{ name,	id },
#include "EntryTypeList.h"

	{ "",				EDF_ANY }, // Dummy type to mark end of list
};

vector<EntryType*>	entry_types;	// The big list of all entry types

// Special entry types
EntryType			etype_unknown;	// The default, 'unknown' entry type
EntryType			etype_folder;	// Folder entry type
EntryType			etype_map;		// Map marker type


/*******************************************************************
 * ENTRYDATAFORMAT CLASS FUNCTIONS
 *******************************************************************/
bool EntryDataFormat::isFormat(MemChunk& mc, uint16_t format) {
	switch (format) {
#define xa(id, name, val)
#define xb(id, name)
#define xx(id, name, func)			case id: return func(mc);
#define xy(id, name, func, load)	case id: return func(mc);
#include "EntryTypeList.h"
	default:
		return false;
	}
}

bool EntryDataFormat::detectPng(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 8) {
		// Check for PNG header
		if (mc[0] == 137 && mc[1] == 80 &&
		        mc[2] == 78 && mc[3] == 71 &&
		        mc[4] == 13 && mc[5] == 10 &&
		        mc[6] == 26 && mc[7] == 10)
			return true;
	}

	return false;
}

bool EntryDataFormat::detectAnsi(MemChunk& mc) {
	// Check size: 25 lines of 80 characters coded on 2 bytes
	if (mc.getSize() == 4000) {
			return true;
	}

	return false;
}

bool EntryDataFormat::detectBmp(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 14) {
		// Check for BMP header
		if (mc[0] == 'B' && mc[1] == 'M' && mc[6] == 0 && mc[7] == 0 && mc[8] == 0 && mc[9] == 0)
			return true;
	}

	return false;
}

bool EntryDataFormat::detectGif(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 6) {
		// Check for GIF header
		if (mc[0] == 'G' && mc[1] == 'I' && mc[2] == 'F' && mc[3] == '8' &&
			(mc[4] == '7' || mc[4] =='9') && mc[5] == 'a')
			return true;
	}

	return false;
}

bool EntryDataFormat::detectPcx(MemChunk& mc) {
	// Check size
	if (mc.getSize() < 129)
		return false;
	// Manufacturer and encoding fields: must always be 10 and 1 respectively
	if (mc[0] != 0x0A || mc[2] != 0x01)
		return false;
	// Version field: only 0, 2, 3, 4 and 5 exist
	if (mc[1] > 5 || mc[1] == 1)
		return false;
	// Bit depth and color plane fields are used in combination
	switch (mc[3]) {
		case 1: // Monochrome, EGA or VGA
			if (mc[65] != 1 && mc[65] != 3 && mc[65] != 4)	return false;	break;
		case 2: // CGA
			if (mc[65] != 1)								return false;	break;
		case 4: // EGA or VGA
			if (mc[65] != 1 && mc[65] != 2)					return false;	break;
		case 8: // VGA, SVGA or SVGA with alpha
			if (mc[65] != 1 && mc[65] != 3 && mc[65] != 4)	return false;	break;
		default: // Not a valid bit depth
			return false;
	}
	// In 256-color mode, the palette, if any, is contained at the end
	// of the file and preceded by a 0x0C. Only version 5 is concerned.
	if (mc[1] == 5 && ((mc[3] == 8 && mc[65] == 1) || (mc[3] == 4 && mc[65] == 2))) {
		size_t filesize = mc.getSize();
		if (filesize < 900 || mc[filesize-769] != 12)
			return false;
	}
	// Reserved value; theoretically values other than 0 can be valid
	// if the image was created by some old version of Paintbrush, but
	// it's unlikely such pictures would be manipulated by SLADE3, so
	// instead we use it to cull false positives.
	if (mc[64] != 0)
		return false;
	// Padding filler bits; theoretically they might be set to garbage
	// values but again it's better to use them to cull false positives.
	for (size_t i = 74; i < 128; ++i)
		if (mc[i] != 0)
			return false;
	// Min/Max fields
	int16_t offsx, offsy, limx, limy, width, height;
	offsx = (int16_t) (mc[ 4] + (mc[ 5]<<8));
	offsy = (int16_t) (mc[ 6] + (mc[ 7]<<8));
	limx  = (int16_t) (mc[ 8] + (mc[ 9]<<8));
	limy  = (int16_t) (mc[10] + (mc[11]<<8));
	width = 1 + limx - offsx; height = 1 + limy - offsy;
	// Compute number of bytes needed per scanline, and account for possible padding
	int16_t bnpsl = (width * mc[3]) / 8; if (bnpsl % 2) bnpsl++;
	// Bytes per scanline field is always an even number and should correspond to guessed value
	int16_t bpsl = (int16_t) (mc[66] + (mc[67]<<8));
	if (bpsl%2 || bpsl != bnpsl)
		return false;
	// Passed all tests, so this seems to be a valid PCX
	return true;
}

bool EntryDataFormat::detectTga(MemChunk& mc) {
	// Size check for the header
	if (mc.getSize() < 18)
		return false;

	// Check dimensions, both ZDoom and Vavoom refuse to load TGA
	// with image sizes greater than 2048 so let's use that as well
	uint16_t width  = mc[12] + (mc[13]<<8);
	uint16_t height = mc[14] + (mc[15]<<8);
	if (width > 2048 || height > 2048)
		return false;

	// Check image type, must be a value between 1 and 3 or 9 and 11
	if (mc[2] == 0 || mc[2] > 11 || (mc[2] > 3 && mc[2] < 9))
		return false;

	// The colormap bool must be 0 or 1
	if (mc[1] != 0 && mc[1] != 1)
		return false;

	// Bits per pixel can be 8, 15, 16, 24 or 32
	if (mc[16] != 8 && mc[16] != 15 && mc[16] != 16 && mc[16] !=24 && mc[16] !=32)
		return false;

	// ZDoom and Vavoom both refuse exotic directions in the descriptor, so same
	if ((mc[17] & 16) != 0)
		return false;

	return true;
}

bool EntryDataFormat::detectTiff(MemChunk& mc) {
	// Check size, minimum size is 26 if I'm not mistaken:
	// 8 for the image header, +2 for at least one image
	// file directory, +12 for at least one directory entry,
	// +4 for a NULL offset for the next IFD
	size_t size = mc.getSize();
	if (size < 26)
		return false;
	// First two bytes must be identical, and either II or MM
	if (mc[0] != mc[1] || (mc[0] != 0x49 && mc[0] != 0x4D))
		return false;
	bool littleendian = (mc[0] == 'I');
	// The value of 42 (0x2A) is present in the next two bytes,
	// in the given endianness
	if (42 != (littleendian ?
		wxUINT16_SWAP_ON_BE((const uint16_t)(mc[2])) :
		wxUINT16_SWAP_ON_LE((const uint16_t)(mc[2]))))
		return false;
	// First offset must be on a word boundary (therefore, %2 == 0) and
	// somewhere within the file, but not in the header of course.
	size_t offset = (littleendian ?
		wxUINT32_SWAP_ON_BE((const uint32_t)(mc[4])) :
		wxUINT32_SWAP_ON_LE((const uint32_t)(mc[4])));
	if (offset < 8 || offset >= size || offset %2)
		return false;
	// Check the first IFD for validity
	uint16_t numentries = (littleendian ?
		wxUINT16_SWAP_ON_BE((const uint16_t)(mc[offset])) :
		wxUINT16_SWAP_ON_LE((const uint16_t)(mc[offset])));
	if (offset + 6 + (numentries * 12) > size)
		return false;
	// Okay, it seems valid so far
	return true;
}

bool EntryDataFormat::detectJpeg(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 128) {
		// Check for JPEG header
		if ((mc[6] == 'J' && mc[7] == 'F' && mc[8] == 'I' && mc[9] == 'F') ||
		        (mc[6] == 'E' && mc[7] == 'x' && mc[8] == 'i' && mc[9] == 'f')) {
			if (mc[0] == 255 && mc[1] == 216 && mc[2] == 255) {
				return true;
			}
		}
	}

	return false;
}

bool EntryDataFormat::detectDoomGfx(MemChunk& mc) {
	const uint8_t* data = mc.getData();

	// Check size
	if (mc.getSize() > sizeof(patch_header_t)) {
		const patch_header_t *header = (const patch_header_t *)data;

		// Check header values are 'sane'
		if (header->height > 0 && header->height < 4096 &&
		        header->width > 0 && header->width < 4096 &&
		        header->top > -2000 && header->top < 2000 &&
		        header->left > -2000 && header->left < 2000) {
			uint32_t *col_offsets = (uint32_t *)((const uint8_t *)data + sizeof(patch_header_t));

			// Check there is room for needed column pointers
			if (mc.getSize() < sizeof(patch_header_t) + (header->width * sizeof(uint32_t)))
				return false;

			// Check column pointers are within range
			for (int a = 0; a < header->width; a++) {
				if (col_offsets[a] > mc.getSize() || col_offsets[a] < sizeof(patch_header_t))
					return false;
			}

			// Passed all checks, so probably is doom gfx
			return true;
		}
	}

	return false;
}

bool EntryDataFormat::detectDoomGfxAlpha(MemChunk& mc) {
	// Get entry data
	const uint8_t* data = mc.getData();

	// Check size
	if (mc.getSize() > sizeof(oldpatch_header_t)) {
		// Check that it ends on a FF byte
		if (mc[mc.getSize() -1] != 0xFF)
			return false;

		const oldpatch_header_t *header = (const oldpatch_header_t *)data;

		// Check header values are 'sane'
		if (header->width > 0 && header->height > 0) {
			uint16_t col_offsets[255]; // Old format headers do not allow dimensions greater than 255.
			for (uint8_t a = 0; a < header->width; a++) {
				const uint8_t * offsetpos = data + sizeof(oldpatch_header_t) + a * sizeof(uint16_t);
				const uint16_t * colofsa = (uint16_t *)(offsetpos);
				col_offsets[a] = wxUINT16_SWAP_ON_BE(*colofsa);
			}

			// Check there is room for needed column pointers
			if (mc.getSize() < sizeof(oldpatch_header_t) + (header->width * sizeof(uint16_t)))
				return false;

			// Check column pointers are within range
			for (int a = 0; a < header->width; a++) {
				if (col_offsets[a] > mc.getSize() || col_offsets[a] < sizeof(oldpatch_header_t))
					return false;
			}

			// Passed all checks, so probably is doom gfx
			return true;
		}
	}

	return false;
}

bool EntryDataFormat::detectDoomGfxBeta(MemChunk& mc) {
	// Check size
	if (mc.getSize() <= sizeof(patch_header_t))
		return false;

	const uint8_t* data = mc.getData();

	// Check that it ends on a FF byte.
	if (mc[mc.getSize() -1] != 0xFF) {
		// The lumps in the beta have sometimes up to three garbage 00 bytes; probably a question of byte alignment.
		for (uint8_t i = 1; i < 4; i++) {
			if (mc[mc.getSize() - i] == 0xFF)
				// Cool, we found the ending byte so it's okay.
				break;
			else if (mc[mc.getSize() - i] != 0x00)
				// It's not 00 and it's not FF, so it's a wrong byte.
				return false;
		}
	}

	const patch_header_t *header = (const patch_header_t *)data;

	// Check header values are 'sane'
	if (header->height > 0 && header->height < 4096 &&
			header->width > 0 && header->width < 4096 &&
			header->top > -2000 && header->top < 2000 &&
			header->left > -2000 && header->left < 2000) {
		uint16_t *col_offsets = (uint16_t *)((const uint8_t *)data + sizeof(patch_header_t));

		// Check there is room for needed column pointers
		if (mc.getSize() < sizeof(patch_header_t) + (header->width * sizeof(uint16_t)))
			return false;

		// Check column pointers are within range
		for (int a = 0; a < header->width; a++) {
			if (col_offsets[a] > mc.getSize() || col_offsets[a] < sizeof(patch_header_t))
				return false;
		}

		// Passed all checks, so probably is doom gfx
		return true;
	}

	return false;
}

/* The following is the documentation about sneas from
 * the DeuTex source:
 *	The snea format was used for certain graphics in Doom
 *	alpha 0.4 and 0.5. It consists in a 2-byte header
 *	followed by an interleaved bitmap. The first byte, W, is
 *	the quarter of the width. The second byte, H is the
 *	height. The bitmap is made of 4xWxH bytes. The first WxH
 *	bytes contain the bitmap for columns 0, 4, 8, etc. The
 *	next WxH bytes contain the bitmap for columns 1, 5, 9,
 *	etc., and so on. No transparency.
 */
bool EntryDataFormat::detectDoomGfxSnea(MemChunk& mc) {
	// Check size
	if (mc.getSize() < 2)
		return false;

	const uint8_t* data = mc.getData();
	uint8_t qwidth = data[0]; // quarter of width
	uint8_t height = data[1];
	if (mc.getSize() != (2 + (4 * qwidth * height)) &&
		// The TITLEPIC in the Doom Press-Release Beta has
		// two extraneous null bytes at the end.
		(qwidth != 80 || height != 200 || mc.getSize() != 64004))
		return false;
	return true;
}

/* This format is used in Doom alpha 0.2. DeuTex doesn't know it,
 * but it seems a really simple format, basically a eight-byte
 * header for size and offsets followed by a raw format dump.
 * Therefore I christened it the ARAH format: Alpha Raw And Header.
 * The header has the same format as the final patch format.
 * To be honest, I'm not actually sure there are offset fields
 * since those values always seem to be set to 0, but hey.
 */
bool EntryDataFormat::detectDoomGfxArah(MemChunk& mc) {
	if (mc.getSize() < sizeof(patch_header_t))
		return false;

	const uint8_t* data = mc.getData();
	const patch_header_t *header = (const patch_header_t *)data;

	// Check header values are 'sane'
	if (!(header->height > 0 && header->height < 4096 &&
		header->width > 0 && header->width < 4096 &&
		header->top > -2000 && header->top < 2000 &&
		header->left > -2000 && header->left < 2000))
		return false;

	// Check the size matches
	if (mc.getSize() != (sizeof(patch_header_t) + (header->width * header->height)))
		return false;

	return true;
}

bool EntryDataFormat::detectPlanar(MemChunk& mc) {
	// 640x480 4-bit graphic with 48-bit, 16-color palette.
	if (mc.getSize() != 153648)
		return false;
	return true;
}

bool EntryDataFormat::detect4bitChunk(MemChunk& mc) {
	if (mc.getSize() != 32 && mc.getSize() != 184)
		return false;
	return true;
}

bool EntryDataFormat::detectImgz(MemChunk& mc) {
	// A format created by Randy Heit and used by some crosshairs in ZDoom.
	uint32_t size = mc.getSize();

	if (size < sizeof(imgz_header_t))
		return false;

	const uint8_t* data = mc.getData();
	const imgz_header_t *header = (const imgz_header_t *)data;

	// Check signature
	if (header->magic[0] != 'I' || header->magic[1] != 'M' ||
		header->magic[2] != 'G' || header->magic[3] != 'Z')
		return false;

	// Check that values are sane
	if (header->width == 0xFFFF || !(header->width | header->height))
		return false;

	// The reserved values should all be null
	for (uint8_t i = 0; i < 11 ; ++i)
		if (header->reserved[i]) return false;

	// This is probably a genuine IMGZ
	return true;
}

extern uint32_t	n_valid_flat_sizes;
extern uint32_t valid_flat_size[][2];

bool EntryDataFormat::detectDoomFlat(MemChunk& mc) {
	// Not too sure how I should go about detecting this - just checking size as
	// it is now seems too cheap, but checking both size and position in the
	// archive could make things confusing for the user. Blah.
	uint32_t size = mc.getSize();

	for (size_t i = 0; i < n_valid_flat_sizes; ++i) {
		if (size == valid_flat_size[i][0]*valid_flat_size[i][1])
			return true;
	}
	if (detectTranslationTable(mc)) return true;
	return !(size < 320 || size % 320);
}

// A data format found while rifling through some Legacy mods,
// specifically High Tech Hell 2. It seems to be how it works.
bool EntryDataFormat::detectDoomLegacy(MemChunk& mc) {
	uint32_t size = mc.getSize();
	if (size < 9)
		return false;
	// These three values must all be zeroes
	if (mc[2] | mc[6] | mc[7])
		return false;
	if (mc[3] > 4)
		return false;
	uint8_t bpp = (mc[3]?mc[3]:1);
	uint16_t width  = mc[0] + (mc[1]<<8);
	uint16_t height = mc[4] + (mc[5]<<8);
	if (size != (8 + width * height * bpp))
		return false;
	return true;
}

bool EntryDataFormat::detectTranslationTable (MemChunk& mc) {
	// Translation tables and colormaps are essentially the same thing.
	// Can't rely on size because Inkworks adds a signature at the end:
	// "This lump was created by The Cookie Monsters InkWorks for DOOM (ver x.y)"
	uint32_t size = mc.getSize();
	if (size > 8704) {
		if (size < 8776)
			return false;
		bool inkworks = true;
		const char* compare = "This lump was created by The Cookie Monsters InkWorks for DOOM";
		for (size_t i = 0; i < 62; ++i) {
			if (mc[8704+i] != compare[i]) {
				inkworks = false;
				break;
			}
		}
		return inkworks;
	}
	return !(size < 256 || size % 256);
}

bool EntryDataFormat::detectPalette(MemChunk& mc) {
	// Some palettes have crap added to them which makes them
	// not a clean multiple. However, the minimum size should
	// still be 768!
	if (mc.getSize() < 768)
		return false;
	return true;
}

bool EntryDataFormat::detectWad(MemChunk& mc) {
	return WadArchive::isWadArchive(mc);
}

bool EntryDataFormat::detectZip(MemChunk& mc) {
	return ZipArchive::isZipArchive(mc);
}

bool EntryDataFormat::detectMus(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 16) {
		// Check for MUS header
		if (mc[0] == 'M' && mc[1] == 'U' && mc[2] == 'S' && mc[3] == 0x1A)
			return true;
	}

	return false;
}

bool EntryDataFormat::detectMidi(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 16) {
		// Check for MIDI header
		if (mc[0] == 'M' && mc[1] == 'T' && mc[2] == 'h' && mc[3] == 'd')
			return true;
	}

	return false;
}

bool EntryDataFormat::detectModIt(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 32) {
		// Check for IT header
		if (mc[0] == 'I' && mc[1] == 'M' && mc[2] == 'P' && mc[3] == 'M')
			return true;
	}

	return false;
}

bool EntryDataFormat::detectModXm(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 80) {
		// Check for mod header
		char temp[17] = "";
		memcpy(temp, mc.getData(), 17);
		if (!s_fmt(_T("%s"), temp).Cmp(_T("Extended module: "))) {
			if (mc[37] == 0x1a) {
				return true;
			}
		}
	}

	return false;
}

bool EntryDataFormat::detectModS3m(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 60) {
		// Check for s3m header
		if (mc[44] == 'S' && mc[45] == 'C' && mc[46] == 'R' && mc[47] == 'M')
			return true;
	}

	return false;
}

bool EntryDataFormat::detectModMod(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 1084) {
		// Check format
		if (mc[950] >= 1 && mc[950] <= 128 && mc[951] == 127) {
			if ((mc[1080] == 'M' && mc[1081] == '.' && mc[1082] == 'K' && mc[1083] == '.') ||
			        (mc[1080] == 'M' && mc[1081] == '!' && mc[1082] == 'K' && mc[1083] == '!') ||
			        (mc[1080] == 'F' && mc[1081] == 'L' && mc[1082] == 'T' && mc[1083] == '4') ||
			        (mc[1080] == 'F' && mc[1081] == 'L' && mc[1082] == 'T' && mc[1083] == '8') ||
			        (mc[1081] == 'C' && mc[1082] == 'H' && mc[1083] == 'N')) {
				return true;
			}
		}
	}

	return false;
}

bool EntryDataFormat::detectSndDoom(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 8) {
		// Check header
		uint16_t head, samplerate, samples, tail;
		mc.seek(0, SEEK_SET);
		mc.read(&head, 2);
		mc.read(&samplerate, 2);
		mc.read(&samples, 2);
		mc.read(&tail, 2);

		if (head == 3 && tail == 0 && samples <= mc.getSize() - 8 && samples > 4 && mc.getSize() <= 65543)
			return true;
	}

	return false;
}

bool EntryDataFormat::detectSndSpeaker(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check header: the first two bytes must always be null
		if (mc[0] | mc[1])
			return false;
		// Next is the number of samples (LE uint16_t), and
		// each sample is a single byte, so the size can be
		// checked easily.
		if (mc.getSize() == 4 + (mc[2]+(mc[3]<<8)))
			return true;
	}
	return false;
}

bool EntryDataFormat::detectSndWav(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 8) {
		// Check for wav header
		if (mc[0] == 'R' && mc[1] == 'I' && mc[2] == 'F' && mc[3] == 'F')
			return true;
	}

	return false;
}

bool EntryDataFormat::detectSndOgg(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for OGG Vorbis header -- a lot more tests could be made
		// to make sure the data is valid, though.
		// Maybe later when a mediaplayer is actually implemented...
		if (mc[0] == 'O' && mc[1] == 'g' && mc[2] == 'g' && mc[3] == 'S')
			return true;
	}
	return false;
}

bool EntryDataFormat::detectSndFlac(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for FLAC header. Same comment as for detectSndOgg.
		if (mc[0] == 'f' && mc[1] == 'L' && mc[2] == 'a' && mc[3] == 'C')
			return true;
	}
	return false;
}

bool EntryDataFormat::detectTextureX(MemChunk& mc) {
	// Not the best test in the world. But a text-based texture lump ought
	// to fail it every time; as it would be interpreted as too high a number.
	const uint8_t * data = mc.getData();
	uint32_t ntex = data[0] + (data[1]<<8) + (data[2]<<16) + (data[3]<<24);
	if ((int32_t) ntex < 0)
		return false;
	if (mc.getSize() < (ntex * 24))
		return false;
	return true;
}

bool EntryDataFormat::detectPnames(MemChunk& mc) {
	// It's a pretty simple format alright
	const uint8_t * data = mc.getData();
	uint32_t number = data[0] + (data[1]<<8) + (data[2]<<16) + (data[3]<<24);
	if ((int32_t) number < 0)
		return false;
	if (mc.getSize() != (4 + number * 8))
		return false;
	return true;
}

bool EntryDataFormat::detectAnimated(MemChunk& mc) {
	if (mc.getSize() > sizeof(animated_t)) {
		size_t numentries = mc.getSize()/sizeof(animated_t);
		// The last entry can be incomplete, as it may stop right
		// after the declaration of its type. So if the size is not
		// a perfect multiple, then the last entry is incomplete.
		size_t lastentry = (mc.getSize()%numentries ? numentries : numentries - 1);

		// Check that the last entry ends on an ANIM_STOP type
		if (mc[lastentry*sizeof(animated_t)] == ANIM_STOP)
			return true;
	}
	return false;
}

bool EntryDataFormat::detectSwitches(MemChunk& mc) {
	if (mc.getSize() > sizeof(switches_t)) {
		size_t numentries = mc.getSize()/sizeof(switches_t);

		// Check that the last entry ends on a SWCH_STOP type
		if (((mc[numentries*sizeof(switches_t) -1]<<8)
			+ mc[numentries*sizeof(switches_t) -2]) == SWCH_STOP)
			return true;
	}
	return false;
}

bool EntryDataFormat::detectFontM(MemChunk& mc) {
	// Monochrome, monospaced bitmap font.
	// Each character is one-byte wide, and there are 256 of them,
	// so the height can be obtained from the total size
	if (mc.getSize() % 256)
		return false;
	if (mc.getSize() / 256 < 6 || mc.getSize() / 256 > 36)
		return false;
	return true;
}

bool EntryDataFormat::detectFont0(MemChunk& mc) {
	if (mc.getSize() <= 0x302)
		return false;

	const uint16_t * gfx_data = (const uint16_t *) mc.getData();

	size_t height = wxINT16_SWAP_ON_BE(gfx_data[0]);

	size_t datasize = mc.getSize() - 0x302;
	if (datasize % height)
		return false;

	// It seems okay so far. Check that one
	// character does start at offset 0x302.
	// The offsets are themselves between
	// offsets 0x102 and 0x302. Halved for int16_t.
	for (size_t i = 0x81; i < 0x181; ++i)
		if (gfx_data[i] == wxINT16_SWAP_ON_BE(0x302))
			return true;

	// Doesn't seem to be such a file after all.
	return false;
}

bool EntryDataFormat::detectFont1(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for FON1 header
		if (mc[0] == 'F' && mc[1] == 'O' && mc[2] == 'N' && mc[3] == '1')
			return true;
	}
	return false;
}

bool EntryDataFormat::detectFont2(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for FON2 header
		if (mc[0] == 'F' && mc[1] == 'O' && mc[2] == 'N' && mc[3] == '2')
			return true;
	}
	return false;
}

bool EntryDataFormat::detectBMF(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for BMF header
		if (mc[0] == 0xE1 && mc[1] == 0xE6 && mc[2] == 0xD5 && mc[3] == 0x1A)
			return true;
	}
	return false;
}

bool EntryDataFormat::detectZGLNodes(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for ZGLN header
		if (mc[0] == 'Z' && mc[1] == 'G' && mc[2] == 'L' && mc[3] == 'N')
			return true;
	}
	return false;
}

bool EntryDataFormat::detectZGLNodes2(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for ZGL2 header
		if (mc[0] == 'Z' && mc[1] == 'G' && mc[2] == 'L' && mc[3] == '2')
			return true;
	}
	return false;
}

// Model formats. More checks could probably be made...
bool EntryDataFormat::detectDMD(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for DMDM header
		if (mc[0] == 'D' && mc[1] == 'M' && mc[2] == 'D' && mc[3] == 'M')
			return true;
	}
	return false;
}

bool EntryDataFormat::detectMDL(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for IDPO header
		if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == 'P' && mc[3] == 'O')
			return true;
	}
	return false;
}

bool EntryDataFormat::detectMD2(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for IDP2 header
		if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == 'P' && mc[3] == '2')
			return true;
	}
	return false;
}

bool EntryDataFormat::detectMD3(MemChunk& mc) {
	// Check size
	if (mc.getSize() > 4) {
		// Check for IDP3 header
		if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == 'P' && mc[3] == '3')
			return true;
	}
	return false;
}

/*******************************************************************
 * ENTRYTYPE CLASS FUNCTIONS
 *******************************************************************/

/* EntryType::EntryType
 * EntryType class constructor
 *******************************************************************/
EntryType::EntryType(string id) {
	this->id = id;
	name = _T("Unknown");
	format = EDF_ANY;
	extension = _T("dat");
	icon = _T("e_default");
	size_limit[0] = -1;
	size_limit[1] = -1;
	editor = _T("default");
	detectable = true;
	section = _T("none");
	reliability = 255;
}

/* EntryType::~EntryType
 * EntryType class constructor
 *******************************************************************/
EntryType::~EntryType() {
	match_extension.clear();
	match_name.clear();
	match_size.clear();
	size_multiple.clear();
}

/* EntryType::addToList
 * Adds the type to the list of entry types
 *******************************************************************/
void EntryType::addToList() {
	entry_types.push_back(this);
	index = entry_types.size() - 1;
}

/* EntryType::dump
 * Dumps entry type info to the log
 *******************************************************************/
void EntryType::dump() {
	wxLogMessage(s_fmt(_T("Type %s \"%s\", format %d, extension %s"), chr(id), chr(name), format, chr(extension)));
	wxLogMessage(s_fmt(_T("Size limit: %d-%d"), size_limit[0], size_limit[1]));

	for (size_t a = 0; a < match_extension.size(); a++)
		wxLogMessage(s_fmt(_T("Match Extension: \"%s\""), chr(match_extension[a])));

	for (size_t a = 0; a < match_name.size(); a++)
		wxLogMessage(s_fmt(_T("Match Name: \"%s\""), chr(match_name[a])));

	for (size_t a = 0; a < match_size.size(); a++)
		wxLogMessage(s_fmt(_T("Match Size: %d"), match_size[a]));

	for (size_t a = 0; a < size_multiple.size(); a++)
		wxLogMessage(s_fmt(_T("Size Multiple: %d"), size_multiple[a]));

	wxLogMessage(_T("---"));
}

void EntryType::copyToType(EntryType* target) {
	// Copy type attributes
	target->setEditor(editor);
	target->setExtension(extension);
	target->setIcon(icon);
	target->setName(name);
	target->setReliability(reliability);

	// Copy type match criteria
	target->setFormat(format);
	target->setMaxSize(size_limit[1]);
	target->setMinSize(size_limit[0]);
	target->setSection(section);

	// Copy match names
	for (size_t a = 0; a < match_name.size(); a++)
		target->addMatchName(match_name[a]);

	// Copy match extensions
	for (size_t a = 0; a < match_extension.size(); a++)
		target->addMatchExtension(match_extension[a]);

	// Copy match sizes
	for (size_t a = 0; a < match_size.size(); a++)
		target->addMatchSize(match_size[a]);

	// Copy size multiples
	for (size_t a = 0; a < size_multiple.size(); a++)
		target->addSizeMultiple(size_multiple[a]);
}

/* EntryType::isThisType
 * Returns true if [entry] matches the EntryType's criteria, false
 * otherwise
 *******************************************************************/
bool EntryType::isThisType(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Check type is detectable
	if (!detectable)
		return false;

	// Get full entry name as filename
	wxFileName fn(entry->getName());

	// Check min size
	if (size_limit[0] >= 0 && entry->getSize() < (unsigned)size_limit[0])
		return false;

	// Check max size
	if (size_limit[1] >= 0 && entry->getSize() > (unsigned)size_limit[1])
		return false;

	// Check for size match if needed
	if (match_size.size() > 0) {
		bool match = false;
		for (size_t a = 0; a < match_size.size(); a++) {
			if (entry->getSize() == match_size[a]) {
				match = true;
				break;
			}
		}

		if (!match)
			return false;
	}

	// Check for name match if needed
	if (match_name.size() > 0) {
		string name = fn.GetName().Lower();
		bool match = false;
		for (size_t a = 0; a < match_name.size(); a++) {
			if (name.Matches(match_name[a].Lower())) {
				match = true;
				break;
			}
		}

		if (!match)
			return false;
	}

	// Check for extension match if needed
	if (match_extension.size() > 0) {
		bool match = false;
		for (size_t a = 0; a < match_extension.size(); a++) {
			if (!fn.GetExt().CmpNoCase(match_extension[a])) {
				match = true;
				break;
			}
		}

		if (!match)
			return false;
	}

	// Check for size multiple match if needed
	if (size_multiple.size() > 0) {
		bool match = false;
		for (size_t a = 0; a < size_multiple.size(); a++) {
			if (entry->getSize() % size_multiple[a] == 0) {
				match = true;
				break;
			}
		}

		if (!match)
			return false;
	}

	// Check for entry section match if needed
	if (section != _T("none")) {
		// Check entry is part of an archive (if not it can't be in a section)
		if (!entry->getParent())
			return false;

		string e_section = entry->getParent()->detectEntrySection(entry);

		if (e_section != section)
			return false;
	}

	// Check for data format match if needed
	if (format == EDF_TEXT) {
		// Text is a special case, as other data formats can sometimes be detected as 'text',
		// we'll only check for it if text data is specified in the entry type
		if (memchr(entry->getData(), 0, entry->getSize()-1) != NULL)
			return false;
	}
	else if (format != EDF_ANY) {
		if (!EntryDataFormat::isFormat(entry->getMCData(), format))
			return false;
	}

	// Passed all checks, so we have a match
	return true;
}

/* EntryType::readEntryTypeDefinition
 * Reads in a block of entry type definitions. Returns false if there
 * was a parsing error, true otherwise
 *******************************************************************/
bool EntryType::readEntryTypeDefinition(MemChunk& mc) {
	Tokenizer tz;

	// Open the given text data
	if (!tz.openMem((const char*)mc.getData(), mc.getSize())) {
		wxLogMessage(_T("Unable to open file"));
		return false;
	}

	// Get first token
	string token = tz.getToken();

	// If it's an entry_types definition, read it
	if (!token.Cmp(_T("entry_types"))) {
		// Check for opening brace
		if (!tz.checkToken(_T("{")))
			return false;

		// Parse all definitions until closing brace
		token = tz.getToken(); // Get type id
		while (token.Cmp(_T("}"))) {
			// Check if we reached the end of the file for some reason
			if (!token.Cmp(_T("")))
				return false;

			// Check for inherited type
			string inherit = _T("");
			if (tz.peekToken() == _T(":")) {
				// Skip :
				tz.getToken();

				// Read inherited type id
				inherit = tz.getToken();
			}

			// Check for opening brace
			if (!tz.checkToken(_T("{")))
				return false;

			// Begin parsing entry type
			EntryType* ntype = new EntryType(token);
			token = tz.getToken();

			// Copy from existing type if inherited
			if (!inherit.IsEmpty()) {
				EntryType* parent_type = EntryType::getType(inherit);

				if (parent_type != EntryType::unknownType())
					parent_type->copyToType(ntype);
				else
					wxLogMessage(_T("Warning: Entry type %s inherits from unknown type %s"), chr(ntype->getId()), chr(inherit));
			}

			// Read all fields until we get to the closing brace
			while (token.Cmp(_T("}"))) {
				// Name field
				if (!token.Cmp(_T("name"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string name = tz.getToken();	// Get name value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setName(name);			// Set type name
				}

				// Detectable field
				if (!token.Cmp(_T("detectable"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					bool detect = tz.getBool();		// Get detectable value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setDetectable(detect);	// Set detectable
				}

				// Extension field
				if (!token.Cmp(_T("export_ext"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string ext = tz.getToken();		// Get extension value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setExtension(ext);		// Set type extension
				}

				// Format field
				if (!token.Cmp(_T("format"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string format = tz.getToken();	// Get format value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					// Get format type matching format string
					bool fmt_exists = false;
					for (int a = 0; a < EDF_UNKNOWN; a++) {
						if (formats[a].id.IsEmpty())
							break;

						if (!format.Cmp(formats[a].id)) {
							ntype->setFormat(formats[a].format);
							fmt_exists = true;
						}
					}

					if (!fmt_exists)
						ntype->setFormat(EDF_UNKNOWN);
				}

				// Icon field
				if (!token.Cmp(_T("icon"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string icon = tz.getToken();	// Get format value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setIcon(icon);			// Set type icon
				}

				// Editor field
				if (!token.Cmp(_T("editor"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string editor = tz.getToken();	// Get name value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setEditor(editor);		// Set type editor
				}

				// Section field
				if (!token.Cmp(_T("section"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					string section = tz.getToken();	// Get name value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setSection(section);		// Set type section
				}

				// MatchExtension field
				if (!token.Cmp(_T("match_ext"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					while (true) {
						string ext = tz.getToken();	// Get extension value

						// Add the extension to the type
						ntype->addMatchExtension(ext);

						string symbol = tz.getToken();
						if (!symbol.Cmp(_T(";")))
							break;					// If ; found, we're done
						else if (symbol.Cmp(_T(",")))
							return false;			// Error if token isn't ; or ,
					}
				}

				// MatchName field
				if (!token.Cmp(_T("match_name"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					while (true) {
						string name = tz.getToken();	// Get name value

						// Add the match name to the type
						ntype->addMatchName(name);

						string symbol = tz.getToken();
						if (!symbol.Cmp(_T(";")))
							break;					// If ; found, we're done
						else if (symbol.Cmp(_T(",")))
							return false;			// Error if token isn't ; or ,
					}
				}

				// Size field
				if (!token.Cmp(_T("size"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					while (true) {
						int size = tz.getInteger();	// Get size value

						// Add the match size to the type
						ntype->addMatchSize(size);

						string symbol = tz.getToken();
						if (!symbol.Cmp(_T(";")))
							break;					// If ; found, we're done
						else if (symbol.Cmp(_T(",")))
							return false;			// Error if token isn't ; or ,
					}
				}

				// MinSize field
				if (!token.Cmp(_T("min_size"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					int size = tz.getInteger();		// Get size value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setMinSize(size);		// Set min size
				}

				// MaxSize field
				if (!token.Cmp(_T("max_size"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					int size = tz.getInteger();		// Get size value

					if (!tz.checkToken(_T(";")))	// Check for ;
						return false;

					ntype->setMaxSize(size);		// Set max size
				}

				// SizeMultiple field
				if (!token.Cmp(_T("size_multiple"))) {
					if (!tz.checkToken(_T("=")))	// Check for =
						return false;

					while (true) {
						int size = tz.getInteger();	// Get size value

						// Add the size multiple to the type
						ntype->addSizeMultiple(size);

						string symbol = tz.getToken();
						if (!symbol.Cmp(_T(";")))
							break;					// If ; found, we're done
						else if (symbol.Cmp(_T(",")))
							return false;			// Error if token isn't ; or ,
					}
				}

				// Reliability field
				if (s_cmp(token, _T("reliability"))) {
					if (!tz.checkToken(_T("=")))		// Check for =
						return false;

					int reliability = tz.getInteger();	// Get reliability value

					if (!tz.checkToken(_T(";")))		// Check for ;
						return false;

					ntype->setReliability(reliability);	// Set reliability
				}

				token = tz.getToken();
			}

			//if (!inherit.IsEmpty())
			//	wxLogMessage(_T("EntryType %s inherit from type %s"), ntype->getName().c_str(), inherit.c_str());

			//ntype->dump();
			ntype->addToList();
			token = tz.getToken();
		}
	}

	return true;
}

/* EntryType::loadEntryTypes
 * Loads all built-in and custom user entry types
 *******************************************************************/
bool EntryType::loadEntryTypes() {
	// Setup unknown type
	etype_unknown.setIcon(_T("e_unknown"));
	etype_unknown.setDetectable(false);
	etype_unknown.setReliability(0);
	etype_unknown.addToList();

	// Setup folder type
	etype_folder.setIcon(_T("e_folder"));
	etype_folder.setName(_T("Folder"));
	etype_folder.setDetectable(false);
	etype_folder.addToList();

	// Setup map marker type
	etype_map.setIcon(_T("e_map"));
	etype_map.setName(_T("Map Marker"));
	etype_map.setDetectable(false);
	etype_map.addToList();

	// -------- READ BUILT-IN TYPES ---------

	// Get builtin entry types from resource archive
	Archive* res_archive = theArchiveManager->programResourceArchive();

	// Check resource archive exists
	if (!res_archive) {
		wxLogMessage(_T("Error: No resource archive open!"));
		return false;
	}

	// Get entry types directory
	zipdir_t* et_dir = ((ZipArchive*)res_archive)->getDirectory(_T("config/entry_types"));

	// Check it exists
	if (!et_dir) {
		wxLogMessage(_T("Error: config/entry_types does not exist in slade.pk3"));
		return false;
	}

	// Read in each file in the directory
	bool etypes_read = false;
	for (unsigned a = 0; a < et_dir->numEntries(); a++) {
		if (readEntryTypeDefinition(et_dir->entries[a]->getMCData()))
			etypes_read = true;
	}

	// Warn if no types were read (this shouldn't happen unless the resource archive is corrupted)
	if (!etypes_read)
		wxLogMessage(_T("Warning: No built-in entry types could be loaded from slade.pk3"));

	// -------- READ CUSTOM TYPES ---------

	// If the directory doesn't exist create it
	if (!wxDirExists(appPath(_T("entry_types"), DIR_USER)))
		wxMkdir(appPath(_T("entry_types"), DIR_USER));

	// Open the custom palettes directory
	wxDir res_dir;
	res_dir.Open(appPath(_T("entry_types"), DIR_USER));

	// Go through each file in the directory
	string filename = wxEmptyString;
	bool files = res_dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
	while (files) {
		// Load file data
		MemChunk mc;
		mc.importFile(res_dir.GetName() + _T("/") + filename);

		// Parse file
		readEntryTypeDefinition(mc);

		// Next file
		files = res_dir.GetNext(&filename);
	}

	return true;
}

/* EntryType::detectEntryType
 * Attempts to detect the given entry's type
 *******************************************************************/
bool EntryType::detectEntryType(ArchiveEntry* entry) {
	// Do nothing if the entry is a folder or a map marker
	if (entry->getType() == &etype_folder || entry->getType() == &etype_map)
		return false;

	// Reset entry type
	entry->setType(&etype_unknown);

	// Go through all registered types
	for (size_t a = 0; a < entry_types.size(); a++) {
		// Check for possible type match
		if (entry_types[a]->isThisType(entry)) {
			// Type matches, test against currently detected type's reliability
			if (entry->getType()->getReliability() < entry_types[a]->getReliability()) {
				// This type is more reliable, so set it
				entry->setType(entry_types[a]);

				// No need to continue if the type is 100% reliable
				if (entry_types[a]->getReliability() == 255)
					return true;
			}
		}
	}

	// Return t/f depending on if a matching type was found
	if (entry->getType() == &etype_unknown)
		return false;
	else
		return true;
}

/* EntryType::getType
 * Returns the entry type with the given id, or etype_unknown if no
 * id match is found
 *******************************************************************/
EntryType* EntryType::getType(string id) {
	for (size_t a = 0; a < entry_types.size(); a++) {
		if (entry_types[a]->getId() == id)
			return entry_types[a];
	}

	return &etype_unknown;
}

/* EntryType::unknownType
 * Returns the global 'unknown' entry type
 *******************************************************************/
EntryType* EntryType::unknownType() {
	return &etype_unknown;
}

/* EntryType::folderType
 * Returns the global 'folder' entry type
 *******************************************************************/
EntryType* EntryType::folderType() {
	return &etype_folder;
}

/* EntryType::mapMarkerType
 * Returns the global 'map marker' entry type
 *******************************************************************/
EntryType* EntryType::mapMarkerType() {
	return &etype_map;
}

wxArrayString EntryType::getIconList() {
	wxArrayString list;

	for (size_t a = 0; a < entry_types.size(); a++)
		list.Add(entry_types[a]->getIcon());

	return list;
}

void EntryType::cleanupEntryTypes() {
	for (size_t a = 3; a < entry_types.size(); a++)
		delete entry_types[a];
}


/* Console Command - "test_entry_types"
 * Testingggg
 *******************************************************************/
void c_test_entry_types(vector<string> args) {
	EntryType::loadEntryTypes();
}
ConsoleCommand et_test_entry_types(_T("test_entry_types"), &c_test_entry_types, 0);


/* Console Command - "test_entry_type_detection"
 * Testingggg moar
 *******************************************************************/
void c_test_entry_type_detection(vector<string> args) {
	Archive* archive = theArchiveManager->getArchive(0);

	for (uint32_t a = 0; a < archive->numEntries(); a++) {
		EntryType::detectEntryType(archive->getEntry(a));
	}
}
ConsoleCommand et_test_entry_type_detection(_T("test_entry_type_detection"), &c_test_entry_type_detection, 0);
