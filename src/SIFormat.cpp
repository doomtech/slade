
#include "Main.h"
#include "SIFormat.h"
#include <FreeImage.h>

vector<SIFormat*>	simage_formats;

SIFormat::SIFormat(string id) {
	// Init variables
	this->id = id;
	this->name = "Unknown";
	this->extension = "dat";

	// Add to list of formats
	simage_formats.push_back(this);
}

SIFormat::~SIFormat() {
}






class SIFPng : public SIFormat {
private:
	// Generic PNG chunk struct
	struct png_chunk_t {
		uint32_t	size;
		char		name[4];
		MemChunk	data;
		uint32_t	crc;
	};

	// grAb chunk struct
	struct grab_chunk_t {
		int32_t xoff;
		int32_t yoff;
	};

	// IHDR chunk struct
	struct ihdr_chunk_t {
		uint32_t	width;
		uint32_t	height;
		uint8_t		bpp;
		uint8_t		coltype;
		uint8_t		compression;
		uint8_t		filter;
		uint8_t		interlace;
	};

	void readChunk(MemChunk& mc, png_chunk_t& chunk) {
		// Read size and chunk name
		mc.read(&chunk.size, 4);
		mc.read(&chunk.name, 4);

		// Read chunk data
		chunk.data.clear();
		mc.readMC(chunk.data, chunk.size);

		// Read crc
		mc.read(&chunk.crc, 4);
	}

public:
	SIFPng() : SIFormat("png") {
		name = "PNG";
		extension = "png";
	}

	bool isThisFormat(MemChunk& mc) {
		// Reset MemChunk
		mc.seek(0, SEEK_SET);

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

	simginfo_t getInfo(MemChunk& mc) {
		simginfo_t inf;
		inf.format = "png";
		inf.width = 0;
		inf.height = 0;

		// Read first chunk
		mc.seek(8, SEEK_SET);
		png_chunk_t chunk;
		readChunk(mc, chunk);
		// Should be IHDR
		if (chunk.name[0] == 'I' && chunk.name[1] == 'H' && chunk.name[2] == 'D' && chunk.name[3] == 'R') {
			// Read IHDR data
			ihdr_chunk_t ihdr;
			chunk.data.read(&ihdr, 13);

			// Set info from IHDR
			inf.width = ihdr.width;
			inf.height = ihdr.height;
			if (ihdr.coltype == 3 && ihdr.bpp == 8)
				// Only 8bpp 'indexed' pngs are counted as PALMASK for now, all others will be converted to RGBA
				inf.colformat = PALMASK;
			else
				inf.colformat = RGBA;
		}

		return inf;
	}

	bool loadImage(SImage& image, MemChunk& data) {
		// Check data is correct format
		if (!isThisFormat(data))
			return false;

		// Create FreeImage bitmap from entry data
		FIMEMORY* mem = FreeImage_OpenMemory((BYTE*)data.getData(), data.getSize());
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(mem, 0);
		FIBITMAP *bm = FreeImage_LoadFromMemory(fif, mem, 0);
		FreeImage_CloseMemory(mem);

		// Check it created/read ok
		if (!bm) {
			Global::error = "Error reading PNG data";
			return false;
		}

		// Get image info
		int width = FreeImage_GetWidth(bm);
		int height = FreeImage_GetHeight(bm);
		int bpp = FreeImage_GetBPP(bm);
		SIType type = RGBA;

		// Get image palette if it exists
		RGBQUAD* bm_pal = FreeImage_GetPalette(bm);
		Palette8bit palette;
		if (bpp == 8 && bm_pal) {
			type = PALMASK;
			int a = 0;
			int b = FreeImage_GetColorsUsed(bm);
			if (b > 256)
				b = 256;
			for (; a < b; a++)
				palette.setColour(a, rgba_t(bm_pal[a].rgbRed, bm_pal[a].rgbGreen, bm_pal[a].rgbBlue, 255));
		}

		// Create image
		if (bm_pal)
			image.create(width, height, type, &palette);
		else
			image.create(width, height, type, NULL);
		
		// Load image data
		uint8_t* img_data = imageData(image);
		if (type == PALMASK) {
			// Flip vertically
			FreeImage_FlipVertical(bm);

			// Load paletted data
			uint8_t* bits = FreeImage_GetBits(bm);
			for (int a = 0; a < width * height; a++)
				img_data[a] = bits[a];

			// Set mask
			uint8_t* mask = imageMask(image);
			uint8_t* alphatable = FreeImage_GetTransparencyTable(bm);
			if (alphatable) {
				for (int a = 0; a < width * height; a++)
					mask[a] = alphatable[img_data[a]];
			}
			else
				image.fillAlpha(255);
		}
		else {
			// Convert to 32bpp & flip vertically
			FIBITMAP *rgb = FreeImage_ConvertTo32Bits(bm);
			FreeImage_FlipVertical(rgb);

			// Load raw RGBA data
			uint8_t* bits_rgb = FreeImage_GetBits(rgb);
			int c = 0;
			for (int a = 0; a < width * height; a++) {
				img_data[c++] = bits_rgb[a * 4 + 2];	// Red
				img_data[c++] = bits_rgb[a * 4 + 1];	// Green
				img_data[c++] = bits_rgb[a * 4];		// Blue
				img_data[c++] = bits_rgb[a * 4 + 3];	// Alpha
			}

			FreeImage_Unload(rgb);
		}

		// Read extra info from various PNG chunks
		int32_t xoff = 0;
		int32_t yoff = 0;
		bool alPh_chunk = false;
		bool grAb_chunk = false;
		data.seek(8, SEEK_SET);	// Start after PNG header
		png_chunk_t chunk;
		/*
		while (true) {
			// Read next PNG chunk
			readChunk(data, chunk);

			// Check for 'grAb' chunk
			if (!grAb_chunk && chunk.name[0] == 'g' && chunk.name[1] == 'r' && chunk.name[2] == 'A' && chunk.name[3] == 'b') {
				// Read offsets
				chunk.data.seek(0, SEEK_SET);
				chunk.data.read(&xoff, 4);
				chunk.data.read(&yoff, 4);
				xoff = wxINT32_SWAP_ON_LE(xoff);
				yoff = wxINT32_SWAP_ON_LE(yoff);
				grAb_chunk = true;
			}

			// Check for 'alPh' chunk
			if (!alPh_chunk && chunk.name[0] == 'a' && chunk.name[1] == 'l' && chunk.name[2] == 'P' && chunk.name[3] == 'h')
				alPh_chunk = true;

			// If both chunks are found no need to search further
			if (grAb_chunk && alPh_chunk)
				break;

			// Stop searching when we get to IDAT chunk
			if (chunk.name[0] == 'I' && chunk.name[1] == 'D' && chunk.name[2] == 'A' && chunk.name[3] == 'T')
				break;
		}
		*/

		// Set offsets
		image.setXOffset(xoff);
		image.setYOffset(yoff);

		// If the picture is nothing more than a ZDoom-style alpha map
		if (alPh_chunk) {
			if (type == RGBA) {
				unsigned c = 0;
				for (int a = 0; a < width * height; a++) {
					uint8_t alpha = img_data[c];
					img_data[c++] = 0;
					img_data[c++] = 0;
					img_data[c++] = 0;
					img_data[c++] = alpha;
				}
			}
			else {
				uint8_t* mask = imageMask(image);
				for (int a = 0; a < width * height; a++) {
					mask[a] = img_data[a];
					img_data[a] = 0;
				}
			}
		}

		// Clean up
		FreeImage_Unload(bm);

		return true;
	}
};



void SIFormat::initFormats() {
	// Default, 'unknown' format
	new SIFormat("unknown");

	// Image formats
	new SIFPng();
}

SIFormat* SIFormat::getFormat(string id) {
	// Search for format matching id
	for (unsigned a = 0; a < simage_formats.size(); a++) {
		if (simage_formats[a]->id == id)
			return simage_formats[a];
	}

	// Not found, return unknown format
	return simage_formats[0];
}
