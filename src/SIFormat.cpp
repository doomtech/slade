
#include "Main.h"
#undef BOOL
#include <FreeImage.h>
#include "Misc.h"
#include "EntryType.h"
#include "SIFormat.h"
#include "SIFImages.h"
#include "SIFDoom.h"
#include "SIFHexen.h"
#include "SIFZDoom.h"
#include "SIFQuake.h"
#include "SIFOther.h"
#include "SIFRott.h"

vector<SIFormat*>	simage_formats;
SIFormat*			sif_raw = NULL;
SIFormat*			sif_flat = NULL;
SIFormat*			sif_unknown = NULL;

// Define valid raw sizes (external for now, until i get rid of SImageFormats.cpp)
extern uint32_t valid_flat_size[][3];
extern uint32_t n_valid_flat_sizes;


// 'Unknown' format
class SIFUnknown : public SIFormat {
protected:
	bool readImage(SImage& image, MemChunk& data, int index) { return false; }

public:
	SIFUnknown() : SIFormat("unknown") {}
	~SIFUnknown() {}

	bool			isThisFormat(MemChunk& mc) { return false; }
	SImage::info_t	getInfo(MemChunk& mc, int index) { return SImage::info_t(); }
};


// Raw format is a special case - not detectable
class SIFRaw : public SIFormat {
protected:
	bool validSize(unsigned size) {
		for (unsigned a = 0; a < n_valid_flat_sizes; a++) {
			if (valid_flat_size[a][0] * valid_flat_size[a][1] == size)
				return true;
		}

		return false;
	}

	bool validSize(int width, int height) {
		for (unsigned a = 0; a < n_valid_flat_sizes; a++) {
			if (valid_flat_size[a][0] == width && valid_flat_size[a][1] == height && valid_flat_size[a][2] == 1)
				return true;
		}

		return false;
	}

	bool readImage(SImage& image, MemChunk& data, int index) {
		// Get info
		SImage::info_t info = getInfo(data, index);

		// Create image from data
		image.create(info.width, info.height, PALMASK);
		data.read(imageData(image), info.width*info.height, 0);
		image.fillAlpha(255);

		return true;
	}

public:
	SIFRaw(string id = "raw") : SIFormat(id) {
		this->name = "Raw";
		this->extension = "dat";
	}
	~SIFRaw() {}

	bool isThisFormat(MemChunk& mc) {
		// Just check the size
		return validSize(mc.getSize());
	}

	SImage::info_t getInfo(MemChunk& mc, int index) {
		SImage::info_t info;
		unsigned size = mc.getSize();

		// Determine dimensions
		bool valid_size = false;
		for (uint32_t a = 0; a < n_valid_flat_sizes; a++) {
			uint32_t w = valid_flat_size[a][0];
			uint32_t h = valid_flat_size[a][1];

			if (size == w * h || size - 4 == w * h) {
				info.width = w;
				info.height = h;
				valid_size = true;
				break;
			}
		}
		if (size == 8776) { // Inkworks and its signature at the end of COLORMAPS
			size = 8704;
		}
		if (!valid_size) {
			if (size % 320 == 0)	{	// This should handle any custom AUTOPAGE
				info.width = 320;
				info.height = size/320;
			}
			else if (size % 256 == 0) { // This allows display of COLORMAPS
				info.width = 256;
				info.height = size/256;
			}
		}

		// Setup other info
		info.colformat = PALMASK;
		info.format = "raw";

		return info;
	}

	bool canWriteType(SIType type) {
		// Raw format only supports paletted images
		if (type == PALMASK)
			return true;
		else
			return false;
	}
};

class SIFRawFlat : public SIFRaw {
protected:
	bool writeImage(SImage& image, MemChunk& data, Palette8bit* pal, int index) {
		// Can't write if RGBA
		if (image.getType() == RGBA)
			return false;

		// Check size
		if (!validSize(image.getWidth(), image.getHeight()))
			return false;

		// Just dump image data to memchunk
		data.clear();
		data.write(imageData(image), image.getWidth()*image.getHeight());

		return true;
	}

public:
	SIFRawFlat() : SIFRaw("raw_flat") {
		this->name = "Doom Flat";
		this->extension = "lmp";
	}
	~SIFRawFlat() {}

	int canWrite(SImage& image) {
		// If it's the correct size and colour format, it's writable
		if (image.getType() == PALMASK &&
			validSize(image.getWidth(), image.getHeight()))
			return WRITABLE;

		// Otherwise, it can be converted via palettising and cropping
		return CONVERTIBLE;
	}

	bool convertWritable(SImage& image, convert_options_t opt) {
		// Firstly, make image paletted
		image.convertPaletted(opt.pal_target, opt.pal_current);

		// Secondly, remove any alpha information
		image.fillAlpha(255);

		// And finally, find a suitable flat size and crop to that size
		int width = 0;
		int height = 0;
		for (unsigned a = 1; a < n_valid_flat_sizes; a++) {
			// Ignore non-writable flat sizes
			if (valid_flat_size[a][2] == 0)
				continue;

			// Check for exact match (no need to crop)
			if (image.getWidth() == valid_flat_size[a][0] &&
				image.getHeight() == valid_flat_size[a][1])
				return true;

			// If the flat will fit within this size, crop to the previous size
			// (this works because flat sizes list is in size-order)
			if (image.getWidth() <= (int)valid_flat_size[a][0] &&
				image.getHeight() <= (int)valid_flat_size[a][1] &&
				width > 0 && height > 0) {
				image.crop(0, 0, width, height);
				return true;
			}

			// Save 'previous' valid size
			width = valid_flat_size[a][0];
			height = valid_flat_size[a][1];
		}

		return false;
	}
};

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



void SIFormat::initFormats() {
	// Non-detectable formats
	sif_unknown = new SIFUnknown();
	sif_raw = new SIFRaw();
	sif_flat = new SIFRawFlat();
	simage_formats.clear();	// Remove previously created formats from the list

	// Image formats
	new SIFPng();

	// Doom formats
	new SIFDoomGfx();
	new SIFDoomBetaGfx();
	new SIFDoomAlphaGfx();
	new SIFDoomArah();
	new SIFDoomSnea();

	// Hexen formats
	new SIFPlanar();
	new SIF4BitChunk();

	// ZDoom formats
	new SIFImgz();

	// Quake series formats
	new SIFQuakeGfx();
	new SIFQuakeSprite();
	new SIFQuakeTex();
	new SIFQuake2Wal();

	// ROTT formats
	new SIFRottGfx();
	new SIFRottGfxMasked();
	new SIFRottLbm();
	new SIFRottRaw();
	new SIFRottPic();

	// Other game formats
	new SIFHalfLifeTex();
	new SIFSCSprite();
	new SIFSCWall();
	new SIFAnaMip();
	new SIFBuildTile();
	new SIFHeretic2M8();
	new SIFHeretic2M32();
}

SIFormat* SIFormat::getFormat(string id) {
	// Search for format matching id
	for (unsigned a = 0; a < simage_formats.size(); a++) {
		if (simage_formats[a]->id == id)
			return simage_formats[a];
	}

	// Not found, return unknown format
	return sif_unknown;
}

SIFormat* SIFormat::determineFormat(MemChunk& mc) {
	// Go through all registered formats
	for (unsigned a = 0; a < simage_formats.size(); a++) {
		if (simage_formats[a]->isThisFormat(mc))
			return simage_formats[a];
	}

	// Not found, return unknown format
	return sif_unknown;
}

SIFormat* SIFormat::unknownFormat() {
	return sif_unknown;
}

SIFormat* SIFormat::rawFormat() {
	return sif_raw;
}

SIFormat* SIFormat::flatFormat() {
	return sif_flat;
}

void SIFormat::getAllFormats(vector<SIFormat*>& list) {
	// Clear list
	list.clear();

	// Add formats
	for (unsigned a = 0; a < simage_formats.size(); a++)
		list.push_back(simage_formats[a]);

	// Add special formats
	list.push_back(sif_raw);
	list.push_back(sif_flat);
}
