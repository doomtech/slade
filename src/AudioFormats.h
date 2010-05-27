#ifndef AUDIOFORMATS_H
#define AUDIOFORMATS_H

class MUSDataFormat : public EntryDataFormat {
public:
	MUSDataFormat() : EntryDataFormat("mus") {};
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
	MIDIDataFormat() : EntryDataFormat("midi") {};
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

class XMIDataFormat : public EntryDataFormat {
public:
	XMIDataFormat() : EntryDataFormat("xmi") {};
	~XMIDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 50) {
			// Check for XMI header: FORM, XDIRINFO, CAT, XMIDFORM, XMID
			if (mc[0] == 'F' && mc[1] == 'O' && mc[2] == 'R' && mc[3] == 'M' &&
				mc[4] == 0 && mc[5] == 0 && mc [6] == 0 &&
				mc[8] == 'X' && mc [9] == 'D' && mc[10] == 'I' && mc[11] == 'R' &&
				mc[12] == 'I' && mc[13] == 'N' && mc[14] == 'F' && mc[15] == 'O' &&
				mc[16] == 0 && mc[17] == 0 && mc [18] == 0 &&
				mc[22] == 'C' && mc[23] == 'A' && mc[24] == 'T' && 
				mc[30] == 'X' && mc[31] == 'M' && mc[32] == 'I' && mc[33] == 'D' &&
				mc[34] == 'F' && mc[35] && 'O' && mc[36] == 'R' && mc[37] == 'M' &&
				mc[42] == 'X' && mc[43] && 'M' && mc[44] == 'I' && mc[45] == 'D')
				return true;
		}

		return false;
	}
};

class ITModuleDataFormat : public EntryDataFormat {
public:
	ITModuleDataFormat() : EntryDataFormat("mod_it") {};
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
	XMModuleDataFormat() : EntryDataFormat("mod_xm") {};
	~XMModuleDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 80) {
			// Check for mod header
			char temp[17] = "";
			memcpy(temp, mc.getData(), 17);
			if (!s_fmt("%s", temp).Cmp("Extended module: ")) {
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
	S3MModuleDataFormat() : EntryDataFormat("mod_s3m") {};
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
	MODModuleDataFormat() : EntryDataFormat("mod_mod") {};
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
	DoomSoundDataFormat() : EntryDataFormat("snd_doom") {};
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
	DoomPCSpeakerDataFormat() : EntryDataFormat("snd_speaker") {};
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
	WAVDataFormat() : EntryDataFormat("snd_wav") {};
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
	OggDataFormat() : EntryDataFormat("snd_ogg") {};
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
	FLACDataFormat() : EntryDataFormat("snd_flac") {};
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

class VocDataFormat : public EntryDataFormat {
public:
	VocDataFormat() : EntryDataFormat("snd_voc") {};
	~VocDataFormat() {}

	bool isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 26) {
			// Check for header, see specs at http://wiki.multimedia.cx/index.php?title=Creative_Voice
			string header(wxString::FromAscii(mc.getData(), 19));
			if (header == "Creative Voice File" && mc[19] == 26 && mc [20] == 26 && mc[21] == 0) {
				uint16_t version = (mc[23]<<8) + mc[22];
				uint16_t validity = ~version + 0x1234;
				if ((mc[24] + (mc[25] << 8)) == validity)
					return true;
			}
		}
		return false;
	}
};

#endif //AUDIOFORMATS_H
