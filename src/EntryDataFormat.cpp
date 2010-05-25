/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    EntryDataFormat.cpp
 * Description: Entry data format detection system
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
#include "EntryDataFormat.h"
#include "ZipArchive.h"
#include "WadArchive.h"
#include "LibArchive.h"
#include "DatArchive.h"
#include "BinaryControlLump.h"
#include "Parser.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
// Declare hash map class to hold EntryDataFormats
WX_DECLARE_STRING_HASH_MAP(EntryDataFormat*, EDFMap);
EDFMap data_formats;
EntryDataFormat*	edf_any = 0;


/*******************************************************************
 * ENTRYDATAFORMAT CLASS FUNCTIONS
 *******************************************************************/

EntryDataFormat::EntryDataFormat(string id) {
	// Init variables
	size_min = 0;

	// Add to hash map
	data_formats[id] = this;
}

EntryDataFormat::~EntryDataFormat() {
}

bool EntryDataFormat::isThisFormat(MemChunk& mc) {
	return true;
}

void EntryDataFormat::copyToFormat(EntryDataFormat& target) {
	target.patterns = patterns;
	target.size_min = size_min;
}



EntryDataFormat* EntryDataFormat::getFormat(string id) {
	EDFMap::iterator i = data_formats.find(id);
	if (i == data_formats.end())
		return edf_any;
	else
		return i->second;
}

EntryDataFormat* EntryDataFormat::anyFormat() {
	return edf_any;
}

bool EntryDataFormat::readDataFormatDefinition(MemChunk& mc) {
	// Parse the definition
	Parser p;
	p.parseText(mc);

	// Get data_formats tree
	ParseTreeNode* pt_formats = (ParseTreeNode*)(p.parseTreeRoot()->getChild("data_formats"));

	// Check it exists
	if (!pt_formats)
		return false;

	// Go through all parsed types
	for (unsigned a = 0; a < pt_formats->nChildren(); a++) {
		// Get child as ParseTreeNode
		ParseTreeNode* formatnode = (ParseTreeNode*)pt_formats->getChild(a);

		// Create+add new data format
		EntryDataFormat* edf = new EntryDataFormat(formatnode->getName().Lower());

		// Copy from existing type if inherited
		if (!formatnode->getInherit().IsEmpty()) {
			/*
			EntryType* parent_type = EntryType::getType(formatnode->getInherit());

			if (parent_type != EntryType::unknownType())
				parent_type->copyToType(ntype);
			else
				wxLogMessage(_T("Warning: Entry type %s inherits from unknown type %s"), chr(ntype->getId()), chr(typenode->getInherit()));
			*/
		}
	}
	return true;
}




// Special format that always returns true on detection
// Used when a format is requested that doesn't exist
class AnyDataFormat : public EntryDataFormat {
public:
	AnyDataFormat() : EntryDataFormat(_T("any")) {}
	~AnyDataFormat() {}

	bool isThisFormat(MemChunk& mc) { return false; }
};

class PNGDataFormat : public EntryDataFormat {
public:
	PNGDataFormat() : EntryDataFormat(_T("img_png")) {}
	~PNGDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class BMPDataFormat : public EntryDataFormat {
public:
	BMPDataFormat() : EntryDataFormat(_T("img_bmp")) {};
	~BMPDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 14) {
			// Check for BMP header
			if (mc[0] == 'B' && mc[1] == 'M' && mc[6] == 0 && mc[7] == 0 && mc[8] == 0 && mc[9] == 0)
				return true;
		}

		return false;
	}
};

class GIFDataFormat : public EntryDataFormat {
public:
	GIFDataFormat() : EntryDataFormat(_T("img_gif")) {};
	~GIFDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 6) {
			// Check for GIF header
			if (mc[0] == 'G' && mc[1] == 'I' && mc[2] == 'F' && mc[3] == '8' &&
				(mc[4] == '7' || mc[4] =='9') && mc[5] == 'a')
				return true;
		}

		return false;
	}
};

class PCXDataFormat : public EntryDataFormat {
public:
	PCXDataFormat() : EntryDataFormat(_T("img_pcx")) {};
	~PCXDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class TGADataFormat : public EntryDataFormat {
public:
	TGADataFormat() : EntryDataFormat(_T("img_tga")) {};
	~TGADataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class TIFFDataFormat : public EntryDataFormat {
public:
	TIFFDataFormat() : EntryDataFormat(_T("img_tiff")) {};
	~TIFFDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class JPEGDataFormat : public EntryDataFormat {
public:
	JPEGDataFormat() : EntryDataFormat(_T("img_jpeg")) {};
	~JPEGDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class DoomGfxDataFormat : public EntryDataFormat {
public:
	DoomGfxDataFormat() : EntryDataFormat(_T("img_doom")) {};
	~DoomGfxDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class DoomGfxAlphaDataFormat : public EntryDataFormat {
public:
	DoomGfxAlphaDataFormat() : EntryDataFormat(_T("img_doom_alpha")) {};
	~DoomGfxAlphaDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class DoomGfxBetaDataFormat : public EntryDataFormat {
public:
	DoomGfxBetaDataFormat() : EntryDataFormat(_T("img_doom_beta")) {};
	~DoomGfxBetaDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class DoomSneaDataFormat : public EntryDataFormat {
public:
	DoomSneaDataFormat() : EntryDataFormat(_T("img_doom_snea")) {};
	~DoomSneaDataFormat() {}

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
	bool isThisFormat(MemChunk& mc) {
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
};

class DoomArahDataFormat : public EntryDataFormat {
public:
	DoomArahDataFormat() : EntryDataFormat(_T("img_doom_arah")) {};
	~DoomArahDataFormat() {}

	/* This format is used in Doom alpha 0.2. DeuTex doesn't know it,
	 * but it seems a really simple format, basically a eight-byte
	 * header for size and offsets followed by a raw format dump.
	 * Therefore I christened it the ARAH format: Alpha Raw And Header.
	 * The header has the same format as the final patch format.
	 * To be honest, I'm not actually sure there are offset fields
	 * since those values always seem to be set to 0, but hey.
	 */
	bool isThisFormat(MemChunk& mc) {
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
};

class IMGZDataFormat : public EntryDataFormat {
public:
	IMGZDataFormat() : EntryDataFormat(_T("img_imgz")) {};
	~IMGZDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class LegacyGfxDataFormat : public EntryDataFormat {
public:
	LegacyGfxDataFormat() : EntryDataFormat(_T("img_legacy")) {};
	~LegacyGfxDataFormat() {}

	// A data format found while rifling through some Legacy mods,
	// specifically High Tech Hell 2. It seems to be how it works.
	bool isThisFormat(MemChunk& mc) {
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
};

class ShadowCasterSpriteFormat: public EntryDataFormat {
public:
	ShadowCasterSpriteFormat() : EntryDataFormat(_T("img_scsprite")) {};
	~ShadowCasterSpriteFormat() {}

	bool isThisFormat(MemChunk& mc) {
		int size = mc.getSize();
		if (size < 3)
			return false;
		int width = mc[2] + (mc[3]<<8);
		if (width == 0)
			return false;
		int height = 0;
		for (int j = 0; j < width; ++j)
		{
			int offstart = mc[(j<<1)+4]+(mc[(j<<1)+5]<<8);
			if (offstart == 0) continue;
			if (offstart < 0 || size < offstart+2 || offstart < (width*2+4))
				return false;
			int start		= mc[offstart];
			int stop		= mc[offstart+1];
			int colheight= start - stop;
			if (colheight < 0 || size < offstart+colheight+1)
				return false;
			if (colheight > height)
				height = colheight;
		}
		return (height > 0);
	}
}; 

class WadDataFormat : public EntryDataFormat {
public:
	WadDataFormat() : EntryDataFormat(_T("archive_wad")) {};
	~WadDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		return WadArchive::isWadArchive(mc);
	}
};


/*
bool EntryDataFormat::detectZip(MemChunk& mc) {
	return ZipArchive::isZipArchive(mc);
}
*/
class ZipDataFormat : public EntryDataFormat {
public:
	ZipDataFormat() : EntryDataFormat(_T("archive_zip")) {};
	~ZipDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		return ZipArchive::isZipArchive(mc);
	}
};

class LibDataFormat : public EntryDataFormat {
public:
	LibDataFormat() : EntryDataFormat(_T("archive_lib")) {};
	~LibDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		return LibArchive::isLibArchive(mc);
	}
};

class DatDataFormat : public EntryDataFormat {
public:
	DatDataFormat() : EntryDataFormat(_T("archive_dat")) {};
	~DatDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		return DatArchive::isDatArchive(mc);
	}
};


class MUSDataFormat : public EntryDataFormat {
public:
	MUSDataFormat() : EntryDataFormat(_T("mus")) {};
	~MUSDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 16) {
			// Check for MUS header
			if (mc[0] == 'M' && mc[1] == 'U' && mc[2] == 'S' && mc[3] == 0x1A)
				return true;
		}

		return false;
	}
};

class MIDIDataFormat : public EntryDataFormat {
public:
	MIDIDataFormat() : EntryDataFormat(_T("midi")) {};
	~MIDIDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 16) {
			// Check for MIDI header
			if (mc[0] == 'M' && mc[1] == 'T' && mc[2] == 'h' && mc[3] == 'd')
				return true;
		}

		return false;
	}
};

class ITModuleDataFormat : public EntryDataFormat {
public:
	ITModuleDataFormat() : EntryDataFormat(_T("mod_it")) {};
	~ITModuleDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 32) {
			// Check for IT header
			if (mc[0] == 'I' && mc[1] == 'M' && mc[2] == 'P' && mc[3] == 'M')
				return true;
		}

		return false;
	}
};

class XMModuleDataFormat : public EntryDataFormat {
public:
	XMModuleDataFormat() : EntryDataFormat(_T("mod_xm")) {};
	~XMModuleDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class S3MModuleDataFormat : public EntryDataFormat {
public:
	S3MModuleDataFormat() : EntryDataFormat(_T("mod_s3m")) {};
	~S3MModuleDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 60) {
			// Check for s3m header
			if (mc[44] == 'S' && mc[45] == 'C' && mc[46] == 'R' && mc[47] == 'M')
				return true;
		}

		return false;
	}
};

class MODModuleDataFormat : public EntryDataFormat {
public:
	MODModuleDataFormat() : EntryDataFormat(_T("mod_mod")) {};
	~MODModuleDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class DoomSoundDataFormat : public EntryDataFormat {
public:
	DoomSoundDataFormat() : EntryDataFormat(_T("snd_doom")) {};
	~DoomSoundDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class DoomPCSpeakerDataFormat : public EntryDataFormat {
public:
	DoomPCSpeakerDataFormat() : EntryDataFormat(_T("snd_speaker")) {};
	~DoomPCSpeakerDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class WAVDataFormat : public EntryDataFormat {
public:
	WAVDataFormat() : EntryDataFormat(_T("snd_wav")) {};
	~WAVDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 8) {
			// Check for wav header
			if (mc[0] == 'R' && mc[1] == 'I' && mc[2] == 'F' && mc[3] == 'F')
				return true;
		}

		return false;
	}
};

class OggDataFormat : public EntryDataFormat {
public:
	OggDataFormat() : EntryDataFormat(_T("snd_ogg")) {};
	~OggDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class FLACDataFormat : public EntryDataFormat {
public:
	FLACDataFormat() : EntryDataFormat(_T("snd_flac")) {};
	~FLACDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for FLAC header. Same comment as for detectSndOgg.
			if (mc[0] == 'f' && mc[1] == 'L' && mc[2] == 'a' && mc[3] == 'C')
				return true;
		}
		return false;
	}
};

class TextureXDataFormat : public EntryDataFormat {
public:
	TextureXDataFormat() : EntryDataFormat(_T("texturex")) {};
	~TextureXDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class PNamesDataFormat : public EntryDataFormat {
public:
	PNamesDataFormat() : EntryDataFormat(_T("pnames")) {};
	~PNamesDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// It's a pretty simple format alright
		const uint8_t * data = mc.getData();
		uint32_t number = data[0] + (data[1]<<8) + (data[2]<<16) + (data[3]<<24);
		if ((int32_t) number < 0)
			return false;
		if (mc.getSize() != (4 + number * 8))
			return false;
		return true;
	}
};

class BoomAnimatedDataFormat : public EntryDataFormat {
public:
	BoomAnimatedDataFormat() : EntryDataFormat(_T("animated")) {};
	~BoomAnimatedDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class BoomSwitchesDataFormat : public EntryDataFormat {
public:
	BoomSwitchesDataFormat() : EntryDataFormat(_T("switches")) {};
	~BoomSwitchesDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		if (mc.getSize() > sizeof(switches_t)) {
			size_t numentries = mc.getSize()/sizeof(switches_t);

			// Check that the last entry ends on a SWCH_STOP type
			if (((mc[numentries*sizeof(switches_t) -1]<<8)
				+ mc[numentries*sizeof(switches_t) -2]) == SWCH_STOP)
				return true;
		}
		return false;
	}
};

class Font0DataFormat : public EntryDataFormat {
public:
	Font0DataFormat() : EntryDataFormat(_T("font_doom_alpha")) {};
	~Font0DataFormat() {}

	bool isThisFormat(MemChunk& mc) {
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
};

class Font1DataFormat : public EntryDataFormat {
public:
	Font1DataFormat() : EntryDataFormat(_T("font_zd_console")) {};
	~Font1DataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for FON1 header
			if (mc[0] == 'F' && mc[1] == 'O' && mc[2] == 'N' && mc[3] == '1')
				return true;
		}
		return false;
	}
};

class Font2DataFormat : public EntryDataFormat {
public:
	Font2DataFormat() : EntryDataFormat(_T("font_zd_big")) {};
	~Font2DataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for FON2 header
			if (mc[0] == 'F' && mc[1] == 'O' && mc[2] == 'N' && mc[3] == '2')
				return true;
		}
		return false;
	}
};

class BMFontDataFormat : public EntryDataFormat {
public:
	BMFontDataFormat() : EntryDataFormat(_T("font_bmf")) {};
	~BMFontDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for BMF header
			if (mc[0] == 0xE1 && mc[1] == 0xE6 && mc[2] == 0xD5 && mc[3] == 0x1A)
				return true;
		}
		return false;
	}
};

class ZGLNodesDataFormat : public EntryDataFormat {
public:
	ZGLNodesDataFormat() : EntryDataFormat(_T("zgln")) {};
	~ZGLNodesDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for ZGLN header
			if (mc[0] == 'Z' && mc[1] == 'G' && mc[2] == 'L' && mc[3] == 'N')
				return true;
		}
		return false;
	}
};

class ZGLNodes2DataFormat : public EntryDataFormat {
public:
	ZGLNodes2DataFormat() : EntryDataFormat(_T("zgl2")) {};
	~ZGLNodes2DataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for ZGL2 header
			if (mc[0] == 'Z' && mc[1] == 'G' && mc[2] == 'L' && mc[3] == '2')
				return true;
		}
		return false;
	}
};

class DMDModelDataFormat : public EntryDataFormat {
public:
	DMDModelDataFormat() : EntryDataFormat(_T("mesh_dmd")) {};
	~DMDModelDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for DMDM header
			if (mc[0] == 'D' && mc[1] == 'M' && mc[2] == 'D' && mc[3] == 'M')
				return true;
		}
		return false;
	}
};

class MDLModelDataFormat : public EntryDataFormat {
public:
	MDLModelDataFormat() : EntryDataFormat(_T("mesh_mdl")) {};
	~MDLModelDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for IDPO header
			if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == 'P' && mc[3] == 'O')
				return true;
		}
		return false;
	}
};

class MD2ModelDataFormat : public EntryDataFormat {
public:
	MD2ModelDataFormat() : EntryDataFormat(_T("mesh_md2")) {};
	~MD2ModelDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for IDP2 header
			if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == 'P' && mc[3] == '2')
				return true;
		}
		return false;
	}
};

class MD3ModelDataFormat : public EntryDataFormat {
public:
	MD3ModelDataFormat() : EntryDataFormat(_T("mesh_md3")) {};
	~MD3ModelDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for IDP3 header
			if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == 'P' && mc[3] == '3')
				return true;
		}
		return false;
	}
};


void EntryDataFormat::initBuiltinFormats() {
	// Create the 'any' format
	edf_any = new AnyDataFormat();

	// Just need to create an instance of each builtin format class
	new PNGDataFormat();
	new BMPDataFormat();
	new GIFDataFormat();
	new PCXDataFormat();
	new TGADataFormat();
	new TIFFDataFormat();
	new JPEGDataFormat();
	new DoomGfxDataFormat();
	new DoomGfxAlphaDataFormat();
	new DoomGfxBetaDataFormat();
	new DoomSneaDataFormat();
	new DoomArahDataFormat();
	new ShadowCasterSpriteFormat();
	new IMGZDataFormat();
	new LegacyGfxDataFormat();
	new WadDataFormat();
	new ZipDataFormat();
	new LibDataFormat();
	new DatDataFormat();
	new MUSDataFormat();
	new MIDIDataFormat();
	new ITModuleDataFormat();
	new XMModuleDataFormat();
	new S3MModuleDataFormat();
	new MODModuleDataFormat();
	new DoomSoundDataFormat();
	new DoomPCSpeakerDataFormat();
	new WAVDataFormat();
	new OggDataFormat();
	new FLACDataFormat();
	new TextureXDataFormat();
	new PNamesDataFormat();
	new BoomAnimatedDataFormat();
	new BoomSwitchesDataFormat();
	new Font0DataFormat();
	new Font1DataFormat();
	new Font2DataFormat();
	new BMFontDataFormat();
	new ZGLNodesDataFormat();
	new ZGLNodes2DataFormat();
	new DMDModelDataFormat();
	new MDLModelDataFormat();
	new MD2ModelDataFormat();
	new MD3ModelDataFormat();

	// And here are some dummy formats needed for certain image formats
	// that can't be detected by anything but size (which is done in EntryType detection anyway)
	new EntryDataFormat(_T("img_raw"));
	new EntryDataFormat(_T("img_planar"));
	new EntryDataFormat(_T("img_4bitchunk"));
	new EntryDataFormat(_T("font_mono"));
}

