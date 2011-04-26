
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
	SIFHalfLifeTex() : SIFormat("img_hlt") {
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
