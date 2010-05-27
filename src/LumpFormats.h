#ifndef LUMPFORMATS_H
#define LUMPFORMATS_H

class TextureXDataFormat : public EntryDataFormat {
public:
	TextureXDataFormat() : EntryDataFormat("texturex") {};
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
	PNamesDataFormat() : EntryDataFormat("pnames") {};
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
	BoomAnimatedDataFormat() : EntryDataFormat("animated") {};
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
	BoomSwitchesDataFormat() : EntryDataFormat("switches") {};
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

class ZGLNodesDataFormat : public EntryDataFormat {
public:
	ZGLNodesDataFormat() : EntryDataFormat("zgln") {};
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
	ZGLNodes2DataFormat() : EntryDataFormat("zgl2") {};
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

#endif //LUMPFORMATS_H
