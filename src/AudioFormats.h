#ifndef AUDIOFORMATS_H
#define AUDIOFORMATS_H

class MUSDataFormat : public EntryDataFormat {
public:
	MUSDataFormat() : EntryDataFormat("mus") {};
	~MUSDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 16) {
			// Check for MUS header
			if (mc[0] == 'M' && mc[1] == 'U' && mc[2] == 'S' && mc[3] == 0x1A)
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class MIDIDataFormat : public EntryDataFormat {
public:
	MIDIDataFormat() : EntryDataFormat("midi") {};
	~MIDIDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 16) {
			// Check for MIDI header
			if (mc[0] == 'M' && mc[1] == 'T' && mc[2] == 'h' && mc[3] == 'd')
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class XMIDataFormat : public EntryDataFormat {
public:
	XMIDataFormat() : EntryDataFormat("xmi") {};
	~XMIDataFormat() {}

	int isThisFormat(MemChunk& mc) {
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
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class HMIDataFormat : public EntryDataFormat {
public:
	HMIDataFormat() : EntryDataFormat("hmi") {};
	~HMIDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 50) {
			// Check for HMI header: HMI-MIDI
			if (mc[0] == 'H' && mc[1] == 'M' && mc[2] == 'I' && mc[3] == '-' &&
				mc[4] == 'M' && mc[5] == 'I' && mc[6] == 'D' && mc[7] == 'I')
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class HMPDataFormat : public EntryDataFormat {
public:
	HMPDataFormat() : EntryDataFormat("hmp") {};
	~HMPDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 50) {
			// Check for HMI header: HMIMIDIP
			if (mc[0] == 'H' && mc[1] == 'M' && mc[2] == 'I' && mc[3] == 'M' &&
				mc[4] == 'I' && mc[5] == 'D' && mc[6] == 'I' && mc[7] == 'P')
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class ITModuleDataFormat : public EntryDataFormat {
public:
	ITModuleDataFormat() : EntryDataFormat("mod_it") {};
	~ITModuleDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 32) {
			// Check for IT header
			if (mc[0] == 'I' && mc[1] == 'M' && mc[2] == 'P' && mc[3] == 'M')
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class XMModuleDataFormat : public EntryDataFormat {
public:
	XMModuleDataFormat() : EntryDataFormat("mod_xm") {};
	~XMModuleDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 80) {
			// Check for mod header
			char temp[17] = "";
			memcpy(temp, mc.getData(), 17);
			if (!s_fmt("%s", temp).Cmp("Extended module: ")) {
				if (mc[37] == 0x1a) {
					return EDF_TRUE;
				}
			}
		}

		return EDF_FALSE;
	}
};

class S3MModuleDataFormat : public EntryDataFormat {
public:
	S3MModuleDataFormat() : EntryDataFormat("mod_s3m") {};
	~S3MModuleDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 60) {
			// Check for s3m header
			if (mc[44] == 'S' && mc[45] == 'C' && mc[46] == 'R' && mc[47] == 'M')
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class MODModuleDataFormat : public EntryDataFormat {
public:
	MODModuleDataFormat() : EntryDataFormat("mod_mod") {};
	~MODModuleDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 1084) {
			// Check format
			if (mc[950] >= 1 && mc[950] <= 128 && mc[951] == 127) {
				if ((mc[1080] == 'M' && mc[1081] == '.' && mc[1082] == 'K' && mc[1083] == '.') ||
						(mc[1080] == 'M' && mc[1081] == '!' && mc[1082] == 'K' && mc[1083] == '!') ||
						(mc[1080] == 'F' && mc[1081] == 'L' && mc[1082] == 'T' && mc[1083] == '4') ||
						(mc[1080] == 'F' && mc[1081] == 'L' && mc[1082] == 'T' && mc[1083] == '8') ||
						(mc[1081] == 'C' && mc[1082] == 'H' && mc[1083] == 'N')) {
					return EDF_TRUE;
				}
			}
		}

		return EDF_FALSE;
	}
};

class DoomSoundDataFormat : public EntryDataFormat {
public:
	DoomSoundDataFormat() : EntryDataFormat("snd_doom") {};
	~DoomSoundDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 8) {
			// Check header
			uint16_t head, samplerate;
			uint32_t samples;
			mc.seek(0, SEEK_SET);
			mc.read(&head, 2);
			mc.read(&samplerate, 2);
			mc.read(&samples, 4);

			if (head == 3 && samples <= (mc.getSize() - 8) && samples > 4)
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class DoomPCSpeakerDataFormat : public EntryDataFormat {
public:
	DoomPCSpeakerDataFormat() : EntryDataFormat("snd_speaker") {};
	~DoomPCSpeakerDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check header: the first two bytes must always be null
			if (mc[0] | mc[1])
				return EDF_FALSE;
			// Next is the number of samples (LE uint16_t), and
			// each sample is a single byte, so the size can be
			// checked easily.
			if (mc.getSize() == 4 + (mc[2]+(mc[3]<<8)))
				return EDF_TRUE;
		}
		return EDF_FALSE;
	}
};

class WAVDataFormat : public EntryDataFormat {
public:
	WAVDataFormat() : EntryDataFormat("snd_wav") {};
	~WAVDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 8) {
			// Check for wav header
			if (mc[0] == 'R' && mc[1] == 'I' && mc[2] == 'F' && mc[3] == 'F')
				return EDF_TRUE;
		}

		return EDF_FALSE;
	}
};

class OggDataFormat : public EntryDataFormat {
public:
	OggDataFormat() : EntryDataFormat("snd_ogg") {};
	~OggDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for OGG Vorbis header -- a lot more tests could be made
			// to make sure the data is valid, though.
			// Maybe later when a mediaplayer is actually implemented...
			if (mc[0] == 'O' && mc[1] == 'g' && mc[2] == 'g' && mc[3] == 'S')
				return EDF_TRUE;
		}
		return EDF_FALSE;
	}
};

class FLACDataFormat : public EntryDataFormat {
public:
	FLACDataFormat() : EntryDataFormat("snd_flac") {};
	~FLACDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 4) {
			// Check for FLAC header. Same comment as for detectSndOgg.
			if (mc[0] == 'f' && mc[1] == 'L' && mc[2] == 'a' && mc[3] == 'C')
				return EDF_TRUE;
		}
		return EDF_FALSE;
	}
};

class VocDataFormat : public EntryDataFormat {
public:
	VocDataFormat() : EntryDataFormat("snd_voc") {};
	~VocDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 26) {
			// Check for header, see specs at http://wiki.multimedia.cx/index.php?title=Creative_Voice
			if (mc[19] == 26 && mc [20] == 26 && mc[21] == 0) {
				uint16_t version = (mc[23]<<8) + mc[22];
				uint16_t validity = ~version + 0x1234;
				if ((mc[24] + (mc[25] << 8)) == validity) {
					// Lastly, check for header text
					string header(wxString::FromAscii(mc.getData(), 19));
					if (header == "Creative Voice File")
					return EDF_TRUE;
				}
			}
		}
		return EDF_FALSE;
	}
};

#endif //AUDIOFORMATS_H
