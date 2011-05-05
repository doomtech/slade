
class SIFHalfLifeTex : public SIFormat {
protected:
	bool readImage(SImage& image, MemChunk& data, int index) {
		// Get image info
		imginfo_t info = getInfo(data, index);

		// Sanitize index if needed
		index %= info.numimages;
		if (index < 0)
			index = info.numimages + index;

		// Determine data offset
		size_t data_offset = READ_L32(data.getData(), 24+(index<<2));
		if (!info.width || !info.height || !data_offset || data.getSize() < data_offset + (info.width*info.height)) {
			Global::error = "HLT file: invalid data for mip level";
			return false;
		}

		// Let's build the palette now
		Palette8bit palette;
		size_t pal_offset = READ_L32(data.getData(), 36) + ((READ_L32(data.getData(), 16) >> 3) * (READ_L32(data.getData(), 20) >> 3));
		if (data.getSize() < pal_offset + 5) {
			Global::error = "HLT file: invalid palette offset";
			return false;
		}
		size_t palsize = READ_L16(data.getData(), pal_offset);
		if (palsize == 0 || palsize > 256 || data.getSize() < (pal_offset + 2 + (palsize*3))) {
			wxLogMessage("palsize %d, paloffset %d, entry size %d", palsize, pal_offset, data.getSize());
			Global::error = "HLT file: invalid palette size";
			return false;
		}
		for (size_t c = 0; c < palsize; ++c) {
			rgba_t color;
			color.r = data[(c*3)+pal_offset+2];
			color.g = data[(c*3)+pal_offset+3];
			color.b = data[(c*3)+pal_offset+4];
			palette.setColour(c, color);
		}

		// Create image
		image.create(info.width, info.height, PALMASK, &palette, index, info.numimages);
		image.fillAlpha(255);

		// Fill data with pixel data
		memcpy(imageData(image), data.getData() + data_offset, info.width * info.height);

		return true;
	}
	
public:
	SIFHalfLifeTex() : SIFormat("hlt") {
		name = "Half-Life Texture";
		extension = "hlt";
	}
	~SIFHalfLifeTex() {}
	
	bool isThisFormat(MemChunk& mc) {
		if (EntryDataFormat::getFormat("img_hlt")->isThisFormat(mc))
			return true;
		else
			return false;
	}
	
	imginfo_t getInfo(MemChunk& mc, int index) {
		imginfo_t info;
		
		// Get image info
		info.width = READ_L32(mc.getData(), 16) >> index;
		info.height = READ_L32(mc.getData(), 20) >> index;
		info.numimages = 4;
		info.colformat = PALMASK;
		info.format = id;
		
		return info;
	}
};

class SIFSCSprite : public SIFormat {
protected:
	bool readImage(SImage& image, MemChunk& data, int index) {
		// Get width & height
		imginfo_t info = getInfo(data, index);
		if (info.format != id)
			return false;

		// Create image
		image.create(info.width, info.height, PALMASK);

		// Format has no offsets, so just set them automatically
		image.setXOffset(info.width/2);
		image.setYOffset(info.height);

		// Read pixel data
		uint8_t* img_data = imageData(image);
		uint8_t* img_mask = imageMask(image);
		for (int h = 0, i = 4; h < info.width; ++h, i+=2) {
			int colstart = READ_L16(data.getData(), i);
			if (colstart) {
				int start		= data[colstart];
				int stop		= data[colstart+1];
				int colheight	= start - stop;
				int startheight = info.height - start;

				for (int z = 0; z < colheight; ++z) {
					int mypixel = ((z+startheight)*info.width)+h;
					if (mypixel >= info.width*info.height || unsigned(colstart+2+z) >= data.getSize())
						return false;
					if (mypixel < 0)
						return false;
					img_data[mypixel] = data[colstart+2+z];
					if (img_data[mypixel])
						img_mask[mypixel] = 0xFF;
				}
			}
		}

		return true;
	}
	
public:
	SIFSCSprite() : SIFormat("scsprite") {
		name = "Shadowcaster Sprite";
		extension = "dat";
	}
	~SIFSCSprite() {}
	
	bool isThisFormat(MemChunk& mc) {
		if (EntryDataFormat::getFormat("img_scsprite")->isThisFormat(mc))
			return true;
		else
			return false;
	}
	
	imginfo_t getInfo(MemChunk& mc, int index) {
		imginfo_t info;
		
		// Get image width
		info.width = READ_L16(mc.getData(), 2);

		if (info.width == 0)
			return info;

		// Get image height
		for (int j = 0; j < info.width; ++j) {
			int colstart = READ_L16(mc.getData(), ((j<<1)+4));

			// Columns with a null offset are skipped
			if (colstart == 0) continue;

			if (colstart < 0 || mc.getSize() < (unsigned)colstart+2 || colstart < (info.width*2+4))
				return info;

			int start		= mc[colstart];
			int stop		= mc[colstart+1];
			int colheight	= start - stop;
			if (colheight < 0 || mc.getSize() < (unsigned)colstart+colheight+1)
				return info;

			if (start > info.height)
				info.height = start;
		}

		// Set other properties
		info.colformat = PALMASK;
		info.format = id;
		
		return info;
	}
};

#define SCWALLOFFSET 130 // Headers contain 129 bytes of junk.
class SIFSCWall : public SIFormat {
protected:
	bool readImage(SImage& image, MemChunk& data, int index) {
		// Determine width and height
		int height = data[0]*4;
		int width = 64;
		if (data.getSize() != width * height + SCWALLOFFSET)
			return false;

		// Create image
		image.create(width, height, PALMASK);

		// Read pixel data
		uint8_t* img_data = imageData(image);
		uint8_t* img_mask = imageMask(image);
		int pixelreader = SCWALLOFFSET;
		int brush = 0;
		for (int x = 0; x < width; ++x) {
			for (int y = 0; y < height; ++y, ++pixelreader) {
				brush = (y*64)+x;
				img_data[brush] = data[pixelreader];
				if (img_data[brush] == 0)
					img_mask[brush] = 0;
			}
		}

		return true;
	}
	
public:
	SIFSCWall() : SIFormat("scwall") {
		name = "Shadowcaster Wall";
		extension = "dat";
	}
	~SIFSCWall() {}
	
	bool isThisFormat(MemChunk& mc) {
		if (EntryDataFormat::getFormat("img_scwall")->isThisFormat(mc))
			return true;
		else
			return false;
	}
	
	imginfo_t getInfo(MemChunk& mc, int index) {
		imginfo_t info;
		
		// Get image properties
		info.height = mc[0]*4;
		info.width = 64;
		info.colformat = PALMASK;
		info.format = id;
		
		return info;
	}
};

class SIFAnaMip : public SIFormat {
protected:
	bool readImage(SImage& image, MemChunk& data, int index) {
		return false;
	}
	
public:
	SIFAnaMip() : SIFormat("mipimage") {
		name = "Example";
		extension = "exp";
	}
	~SIFAnaMip() {}
	
	bool isThisFormat(MemChunk& mc) {
		return false;
	}
	
	imginfo_t getInfo(MemChunk& mc, int index) {
		imginfo_t info;
		
		// Get image info
		info.width = READ_L16(mc.getData(), 0);
		info.height = READ_L16(mc.getData(), 2);
		info.colformat = PALMASK;
		
		return info;
	}
};
