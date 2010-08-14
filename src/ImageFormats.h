#ifndef IMAGEFORMATS_H
#define IMAGEFORMATS_H

class PNGDataFormat : public EntryDataFormat {
public:
	PNGDataFormat() : EntryDataFormat("img_png") {}
	~PNGDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 8) {
			// Check for PNG header
			if (mc[0] == 137 && mc[1] == 80 &&
					mc[2] == 78 && mc[3] == 71 &&
					mc[4] == 13 && mc[5] == 10 &&
					mc[6] == 26 && mc[7] == 10)
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class BMPDataFormat : public EntryDataFormat {
public:
	BMPDataFormat() : EntryDataFormat("img_bmp") {};
	~BMPDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 14) {
			// Check for BMP header
			if (mc[0] == 'B' && mc[1] == 'M' && mc[6] == 0 && mc[7] == 0 && mc[8] == 0 && mc[9] == 0)
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class GIFDataFormat : public EntryDataFormat {
public:
	GIFDataFormat() : EntryDataFormat("img_gif") {};
	~GIFDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 6) {
			// Check for GIF header
			if (mc[0] == 'G' && mc[1] == 'I' && mc[2] == 'F' && mc[3] == '8' &&
				(mc[4] == '7' || mc[4] =='9') && mc[5] == 'a')
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class PCXDataFormat : public EntryDataFormat {
public:
	PCXDataFormat() : EntryDataFormat("img_pcx") {};
	~PCXDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() < 129)
			return EDF_FALSE;
		// Manufacturer and encoding fields: must always be 10 and 1 respectively
		if (mc[0] != 0x0A || mc[2] != 0x01)
			return EDF_FALSE;
		// Version field: only 0, 2, 3, 4 and 5 exist
		if (mc[1] > 5 || mc[1] == 1)
			return EDF_FALSE;
		// Bit depth and color plane fields are used in combination
		switch (mc[3]) {
			case 1: // Monochrome, EGA or VGA
				if (mc[65] != 1 && mc[65] != 3 && mc[65] != 4)	return EDF_FALSE;	break;
			case 2: // CGA
				if (mc[65] != 1)								return EDF_FALSE;	break;
			case 4: // EGA or VGA
				if (mc[65] != 1 && mc[65] != 2)					return EDF_FALSE;	break;
			case 8: // VGA, SVGA or SVGA with alpha
				if (mc[65] != 1 && mc[65] != 3 && mc[65] != 4)	return EDF_FALSE;	break;
			default: // Not a valid bit depth
				return EDF_FALSE;
		}
		// In 256-color mode, the palette, if any, is contained at the end
		// of the file and preceded by a 0x0C. Only version 5 is concerned.
		if (mc[1] == 5 && ((mc[3] == 8 && mc[65] == 1) || (mc[3] == 4 && mc[65] == 2))) {
			size_t filesize = mc.getSize();
			if (filesize < 900 || mc[filesize-769] != 12)
				return EDF_FALSE;
		}
		// Reserved value; theoretically values other than 0 can be valid
		// if the image was created by some old version of Paintbrush, but
		// it's unlikely such pictures would be manipulated by SLADE3, so
		// instead we use it to cull false positives.
		if (mc[64] != 0)
			return EDF_FALSE;
		// Padding filler bits; theoretically they might be set to garbage
		// values but again it's better to use them to cull false positives.
		for (size_t i = 74; i < 128; ++i)
			if (mc[i] != 0)
				return EDF_FALSE;
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
			return EDF_FALSE;
		// Passed all tests, so this seems to be a valid PCX
		return EDF_TRUE;
	}
};

class TGADataFormat : public EntryDataFormat {
public:
	TGADataFormat() : EntryDataFormat("img_tga") {};
	~TGADataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Size check for the header
		if (mc.getSize() < 18)
			return EDF_FALSE;

		// Check dimensions, both ZDoom and Vavoom refuse to load TGA
		// with image sizes greater than 2048 so let's use that as well
		uint16_t width  = mc[12] + (mc[13]<<8);
		uint16_t height = mc[14] + (mc[15]<<8);
		if (width > 2048 || height > 2048)
			return EDF_FALSE;

		// Let's have halfway "reasonable" limits on the compression ratio
		// that can be expected from a TGA picture...
		if ((unsigned)(5000u * mc.getSize()) < (unsigned)(height * width))
			return EDF_FALSE;

		// Check image type, must be a value between 1 and 3 or 9 and 11
		if (mc[2] == 0 || mc[2] > 11 || (mc[2] > 3 && mc[2] < 9))
			return EDF_FALSE;

		// The colormap bool must be 0 or 1
		if (mc[1] != 0 && mc[1] != 1)
			return EDF_FALSE;

		// Bits per pixel can be 8, 15, 16, 24 or 32
		if (mc[16] != 8 && mc[16] != 15 && mc[16] != 16 && mc[16] !=24 && mc[16] !=32)
			return EDF_FALSE;

		// ZDoom and Vavoom both refuse exotic directions in the descriptor, so same
		if ((mc[17] & 16) != 0)
			return EDF_FALSE;

		return EDF_TRUE;
	}
};

class TIFFDataFormat : public EntryDataFormat {
public:
	TIFFDataFormat() : EntryDataFormat("img_tiff") {};
	~TIFFDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size, minimum size is 26 if I'm not mistaken:
		// 8 for the image header, +2 for at least one image
		// file directory, +12 for at least one directory entry,
		// +4 for a NULL offset for the next IFD
		size_t size = mc.getSize();
		if (size < 26)
			return EDF_FALSE;
		// First two bytes must be identical, and either II or MM
		if (mc[0] != mc[1] || (mc[0] != 0x49 && mc[0] != 0x4D))
			return EDF_FALSE;
		bool littleendian = (mc[0] == 'I');
		// The value of 42 (0x2A) is present in the next two bytes,
		// in the given endianness
		if (42 != (littleendian ?
			wxUINT16_SWAP_ON_BE((const uint16_t)(mc[2])) :
			wxUINT16_SWAP_ON_LE((const uint16_t)(mc[2]))))
			return EDF_FALSE;
		// First offset must be on a word boundary (therefore, %2 == 0) and
		// somewhere within the file, but not in the header of course.
		size_t offset = (littleendian ?
			wxUINT32_SWAP_ON_BE((const uint32_t)(mc[4])) :
			wxUINT32_SWAP_ON_LE((const uint32_t)(mc[4])));
		if (offset < 8 || offset >= size || offset %2)
			return EDF_FALSE;
		// Check the first IFD for validity
		uint16_t numentries = (littleendian ?
			wxUINT16_SWAP_ON_BE((const uint16_t)(mc[offset])) :
			wxUINT16_SWAP_ON_LE((const uint16_t)(mc[offset])));
		if (offset + 6 + (numentries * 12) > size)
			return EDF_FALSE;
		// Okay, it seems valid so far
		return EDF_TRUE;
	}
};

class JPEGDataFormat : public EntryDataFormat {
public:
	JPEGDataFormat() : EntryDataFormat("img_jpeg") {};
	~JPEGDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 128) {
			// Check for JPEG header
			if ((mc[6] == 'J' && mc[7] == 'F' && mc[8] == 'I' && mc[9] == 'F') ||
					(mc[6] == 'E' && mc[7] == 'x' && mc[8] == 'i' && mc[9] == 'f')) {
				if (mc[0] == 255 && mc[1] == 216 && mc[2] == 255) {
					return EDF_TRUE;
				}
			}
		}

		return EDF_FALSE;
	}
};

class DoomGfxDataFormat : public EntryDataFormat {
public:
	DoomGfxDataFormat() : EntryDataFormat("img_doom") {};
	~DoomGfxDataFormat() {}

	int isThisFormat(MemChunk& mc) {
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
					return EDF_FALSE;

				// Check column pointers are within range
				for (int a = 0; a < header->width; a++) {
					if (col_offsets[a] > mc.getSize() || col_offsets[a] < sizeof(patch_header_t))
						return EDF_FALSE;
				}

				// Check if total size is reasonable; this computation corresponds to the most inefficient
				// possible use of space by the format (horizontal stripes of 1 pixel, 1 pixel apart).
				int numpixels = (header->height + 2 + header->height%2)/2;
				int maxcolsize = sizeof(uint32_t) + (numpixels*5) + 1;
				if (mc.getSize() > (sizeof(patch_header_t) + (header->width * maxcolsize))) {
					return EDF_UNLIKELY;	// This may still be good anyway 
				}

				// Passed all checks, so probably is doom gfx
				return EDF_TRUE;
			}
		}

		return EDF_FALSE;
	}
};

class DoomGfxAlphaDataFormat : public EntryDataFormat {
public:
	DoomGfxAlphaDataFormat() : EntryDataFormat("img_doom_alpha") {};
	~DoomGfxAlphaDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Get entry data
		const uint8_t* data = mc.getData();

		// Check size
		if (mc.getSize() > sizeof(oldpatch_header_t)) {
			// Check that it ends on a FF byte
			if (mc[mc.getSize() -1] != 0xFF)
				return EDF_FALSE;

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
					return EDF_FALSE;

				// Check column pointers are within range
				for (int a = 0; a < header->width; a++) {
					if (col_offsets[a] > mc.getSize() || col_offsets[a] < sizeof(oldpatch_header_t))
						return EDF_FALSE;
				}

				// Check if total size is reasonable; this computation corresponds to the most inefficient
				// possible use of space by the format (horizontal stripes of 1 pixel, 1 pixel apart).
				int numpixels = (header->height + 2 + header->height%2)/2;
				int maxcolsize = sizeof(uint16_t) + (numpixels*3) + 1;
				if (mc.getSize() > (sizeof(oldpatch_header_t) + (header->width * maxcolsize))) {
					return EDF_FALSE;
				}

				// Passed all checks, so probably is doom gfx
				return EDF_TRUE;
			}
		}

		return EDF_FALSE;
	}
};

class DoomGfxBetaDataFormat : public EntryDataFormat {
public:
	DoomGfxBetaDataFormat() : EntryDataFormat("img_doom_beta") {};
	~DoomGfxBetaDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() <= sizeof(patch_header_t))
			return EDF_FALSE;

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
					return EDF_FALSE;
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
				return EDF_FALSE;

			// Check column pointers are within range
			for (int a = 0; a < header->width; a++) {
				if (col_offsets[a] > mc.getSize() || col_offsets[a] < sizeof(patch_header_t))
					return EDF_FALSE;
			}

			// Check if total size is reasonable; this computation corresponds to the most inefficient
			// possible use of space by the format (horizontal stripes of 1 pixel, 1 pixel apart).
			int numpixels = (header->height + 2 + header->height%2)/2;
			int maxcolsize = sizeof(uint16_t) + (numpixels*3) + 1;
			if (mc.getSize() > (sizeof(patch_header_t) + (header->width * maxcolsize))) {
				return EDF_FALSE;
			}

			// Passed all checks, so probably is doom gfx
			return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class DoomSneaDataFormat : public EntryDataFormat {
public:
	DoomSneaDataFormat() : EntryDataFormat("img_doom_snea") {};
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
	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() < 2)
			return EDF_FALSE;

		const uint8_t* data = mc.getData();
		uint8_t qwidth = data[0]; // quarter of width
		uint8_t height = data[1];
		if (mc.getSize() != (2 + (4 * qwidth * height)) &&
			// The TITLEPIC in the Doom Press-Release Beta has
			// two extraneous null bytes at the end, for padding.
			(qwidth != 80 || height != 200 || mc.getSize() != 64004))
			return EDF_FALSE;
		return EDF_TRUE;
	}
};

class DoomArahDataFormat : public EntryDataFormat {
public:
	DoomArahDataFormat() : EntryDataFormat("img_doom_arah") {};
	~DoomArahDataFormat() {}

	/* This format is used in Doom alpha 0.2. DeuTex doesn't know it,
	 * but it seems a really simple format, basically a eight-byte
	 * header for size and offsets followed by a raw format dump.
	 * Therefore I christened it the ARAH format: Alpha Raw And Header.
	 * The header has the same format as the final patch format.
	 * To be honest, I'm not actually sure there are offset fields
	 * since those values always seem to be set to 0, but hey.
	 */
	int isThisFormat(MemChunk& mc) {
		if (mc.getSize() < sizeof(patch_header_t))
			return EDF_FALSE;

		const uint8_t* data = mc.getData();
		const patch_header_t *header = (const patch_header_t *)data;

		// Check header values are 'sane'
		if (!(header->height > 0 && header->height < 4096 &&
			header->width > 0 && header->width < 4096 &&
			header->top > -2000 && header->top < 2000 &&
			header->left > -2000 && header->left < 2000))
			return EDF_FALSE;

		// Check the size matches
		if (mc.getSize() != (sizeof(patch_header_t) + (header->width * header->height)))
			return EDF_FALSE;

		return EDF_TRUE;
	}
};

class IMGZDataFormat : public EntryDataFormat {
public:
	IMGZDataFormat() : EntryDataFormat("img_imgz") {};
	~IMGZDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// A format created by Randy Heit and used by some crosshairs in ZDoom.
		uint32_t size = mc.getSize();

		if (size < sizeof(imgz_header_t))
			return EDF_FALSE;

		const uint8_t* data = mc.getData();
		const imgz_header_t *header = (const imgz_header_t *)data;

		// Check signature
		if (header->magic[0] != 'I' || header->magic[1] != 'M' ||
			header->magic[2] != 'G' || header->magic[3] != 'Z')
			return EDF_FALSE;

		// Check that values are sane
		if (header->width == 0xFFFF || !header->width || !header->height)
			return EDF_FALSE;

		// The reserved values should all be null
		for (uint8_t i = 0; i < 11 ; ++i)
			if (header->reserved[i]) return EDF_FALSE;

		// This is probably a genuine IMGZ
		return EDF_TRUE;
	}
};

class LegacyGfxDataFormat : public EntryDataFormat {
public:
	LegacyGfxDataFormat() : EntryDataFormat("img_legacy") {};
	~LegacyGfxDataFormat() {}

	// A data format found while rifling through some Legacy mods,
	// specifically High Tech Hell 2. It seems to be how it works.
	int isThisFormat(MemChunk& mc) {
		uint32_t size = mc.getSize();
		if (size < 9)
			return EDF_FALSE;
		// These three values must all be zeroes
		if (mc[2] | mc[6] | mc[7])
			return EDF_FALSE;
		if (mc[3] > 4)
			return EDF_FALSE;
		uint8_t bpp = (mc[3]?mc[3]:1);
		uint16_t width  = mc[0] + (mc[1]<<8);
		uint16_t height = mc[4] + (mc[5]<<8);
		if (size != (8 + width * height * bpp))
			return EDF_FALSE;
		return EDF_TRUE;
	}
};

class ShadowCasterSpriteFormat: public EntryDataFormat {
public:
	ShadowCasterSpriteFormat() : EntryDataFormat("img_scsprite") {};
	~ShadowCasterSpriteFormat() {}

	int isThisFormat(MemChunk& mc) {
		int size = mc.getSize();
		if (size < 4)
			return EDF_FALSE;
		int width = mc[2] + (mc[3]<<8);
		if (width <= 0)
			return EDF_FALSE;
		int height = 0;
		// Error checking with average column height and proportion of empty columns
		int avgcolheight = 0, pnumemptycol = 0;
		for (int j = 0; j < width; ++j) {
			int offstart = mc[(j<<1)+4]+(mc[(j<<1)+5]<<8);
			if (offstart == 0) continue;
			if (offstart < 0 || size < offstart+2 || offstart < (width*2+4))
				return EDF_FALSE;
			int start		= mc[offstart];
			int stop		= mc[offstart+1];
			int colheight = start - stop;
			if (colheight < 0 || size < offstart+colheight+1)
				return EDF_FALSE;
			if (start > height)
				height = start;
			avgcolheight += colheight;
			if (colheight == 0)
				pnumemptycol++;
		}
		if (height = 0)
			return EDF_FALSE;

		avgcolheight *= 16;	avgcolheight /= width;
		pnumemptycol *= 16;	pnumemptycol /= width;
		
		// Arbitrary value: sprite must be at least about 12% filled
		if ((avgcolheight < height / 2) || (pnumemptycol > 14))
			return EDF_FALSE;

		// Least efficient sprites: single rows (amounts to 6 bytes per pixel + 4 header bytes)
		return (size < (5 + ((5 + height) * width)));
	}
};

class ShadowCasterWallFormat: public EntryDataFormat {
public:
	ShadowCasterWallFormat() : EntryDataFormat("img_scwall") {};
	~ShadowCasterWallFormat() {}

	int isThisFormat(MemChunk& mc) {
		int size = mc.getSize();
		// Minimum valid size for such a picture to be 
		// successfully loaded: 130 header, +1 line of 64.
		if (size < 194)
			return EDF_FALSE;
		return (size == (mc[0]*256) + 130);
	}
}; 

class AnaMipImageFormat: public EntryDataFormat {
public:
	AnaMipImageFormat() : EntryDataFormat("img_mipimage") {};
	~AnaMipImageFormat() {}

	int isThisFormat(MemChunk& mc) {
		size_t size = mc.getSize();
		if (size < 4)
			return EDF_FALSE;
		size_t width = mc[0] + (mc[1]<<8);
		if (width == 0)
			return EDF_FALSE;
		size_t height = mc[2] + (mc[3]<<8);
		if (height == 0)
			return EDF_FALSE;
		size_t pixels = width * height;
		return (size >= (pixels + 4) && size < 2 * pixels + 4);
	}
};

class Font0DataFormat : public EntryDataFormat {
public:
	Font0DataFormat() : EntryDataFormat("font_doom_alpha") {};
	~Font0DataFormat() {}

	int isThisFormat(MemChunk& mc) {
		if (mc.getSize() <= 0x302)
			return EDF_FALSE;

		const uint16_t * gfx_data = (const uint16_t *) mc.getData();

		size_t height = wxINT16_SWAP_ON_BE(gfx_data[0]);

		size_t datasize = mc.getSize() - 0x302;
		if (datasize % height)
			return EDF_FALSE;

		// It seems okay so far. Check that one
		// character does start at offset 0x302.
		// The offsets are themselves between
		// offsets 0x102 and 0x302. Halved for int16_t.
		for (size_t i = 0x81; i < 0x181; ++i)
			if (gfx_data[i] == wxINT16_SWAP_ON_BE(0x302))
				return EDF_TRUE;

		// Doesn't seem to be such a file after all.
		return EDF_FALSE;
	}
};

class Font1DataFormat : public EntryDataFormat {
public:
	Font1DataFormat() : EntryDataFormat("font_zd_console") {};
	~Font1DataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for FON1 header
			if (mc[0] == 'F' && mc[1] == 'O' && mc[2] == 'N' && mc[3] == '1')
				return EDF_TRUE;
		}
		return EDF_FALSE;
	}
};

class Font2DataFormat : public EntryDataFormat {
public:
	Font2DataFormat() : EntryDataFormat("font_zd_big") {};
	~Font2DataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for FON2 header
			if (mc[0] == 'F' && mc[1] == 'O' && mc[2] == 'N' && mc[3] == '2')
				return EDF_TRUE;
		}
		return EDF_FALSE;
	}
};

class BMFontDataFormat : public EntryDataFormat {
public:
	BMFontDataFormat() : EntryDataFormat("font_bmf") {};
	~BMFontDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for BMF header
			if (mc[0] == 0xE1 && mc[1] == 0xE6 && mc[2] == 0xD5 && mc[3] == 0x1A)
				return EDF_TRUE;
		}
		return EDF_FALSE;
	}
};

#endif //IMAGEFORMATS_H