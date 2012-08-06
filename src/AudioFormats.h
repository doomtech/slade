
#ifndef AUDIOFORMATS_H
#define AUDIOFORMATS_H

/* CheckForTags
 * Looks whether the memory chunk starts with an ID3 tag, and if
 * there is one, returns the index at which the true audio data
 * begins. Returns 0 if there is no tag before audio data.
 *******************************************************************/
size_t checkForTags(MemChunk & mc) {
	if (mc.getSize() > 14) {
		// Check for ID3 header (ID3v2). Version and revision numbers cannot be FF.
		// Only the four upper flags are valid.
		if (mc[0] == 'I' && mc[1] == 'D' && mc[2] == '3' &&
			mc[3] != 0xFF && mc[4] != 0xFF && ((mc[5] & 0x0F) == 0) &&
			mc[6] < 0x80 && mc[7] < 0x80 && mc[8] < 0x80 && mc[9] < 0x80)
		{
			// Compute size. It is stored as a "synchsafe integer", that is to say,
			// a big-endian value where the highest bit of each byte is not used.
			size_t size = (mc[6] << 21) + (mc[7] << 14) + (mc[8] << 7) + mc[9] + 10;
			// If there is a footer, then add 10 more to the size
			if (mc[5] & 0x10) size += 10;
			// Needs to be at least that big
			if (mc.getSize() < size + 4)
				return 0;
			return size;
		}
	}
	// It's also possible to get an ID3v1 (or v1.1) tag.
	// Though normally they're at the end of the file.
	if (mc.getSize() > 132) {
		// Check for ID3 header (ID3v1).
		if (mc[0] == 'T' && mc[1] == 'A' && mc[2] == 'G')
		{
			return 128;
		}
	}
	return 0;
}

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
				mc[34] == 'F' && mc[35] == 'O' && mc[36] == 'R' && mc[37] == 'M' &&
				mc[42] == 'X' && mc[43] == 'M' && mc[44] == 'I' && mc[45] == 'D')
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

class GMIDDataFormat : public EntryDataFormat {
public:
	GMIDDataFormat() : EntryDataFormat("gmid") {};
	~GMIDDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 8) {
			// Check for GMID header: MIDI followed by BE data size
			if (mc[0] == 'M' && mc[1] == 'I' && mc[2] == 'D' && mc[3] == 'I' &&
				((READ_B32(mc, 4) + 8) == mc.getSize()))
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
			char temp[18] = "";
			memcpy(temp, mc.getData(), 18);
			temp[17] = 0;
			if (!S_FMT("%s", temp).CmpNoCase("Extended module: ")) {
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
			// Check format; note: byte 951 is used by NoiseTracker to indicate restart byte so it can be any value
			if (mc[950] >= 1 && mc[950] <= 128 /*&& (mc[951] & 127) == 127*/) {
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

class IMFDataFormat : public EntryDataFormat {
public:
	IMFDataFormat() : EntryDataFormat("imf") {};
	~IMFDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 13) {
			// Check format
			if (mc[0] == 'A' && mc[1] == 'D' && mc[2] == 'L' &&
				mc[3] == 'I' && mc[4] == 'B' &&	mc[5] == 1 &&
				mc[6] == 0 && mc[7] == 0 && mc[8] == 1) {
					return EDF_TRUE;
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

			if (head == 3 && samples == (mc.getSize() - 8) && samples > 4)
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
			if (mc.getSize() == 4 + READ_L16(mc, 2))
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

// This function was written using the following page as reference:
// http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm
int validMPEG(MemChunk& mc, uint8_t layer, size_t size) {
		// Check size
		if (mc.getSize() > 4+size) {
			// Check for MP3 frame header. Warning, it is a very weak signature.
			uint16_t framesync = ((mc[0+size]<<4) + (mc[1+size]>>4)) & 0xFFE;
			// Check for presence of the sync word (the first eleven bits, all set)
			if (framesync == 0xFFE) {
				uint8_t version = (mc[1+size]>>3) & 3;
				uint8_t mylayer = (mc[1+size]>>1) & 3;
				// Version: 0 MPEG v2.5 (unofficial), 1 invalid, 2 MPEG v2, 3 MPEG v3
				// Layer: 0 invalid, 1 III, 2 II, 3 I (this sure makes sense :p)
				if (version != 1 && mylayer == (4 - layer)) {
					// The bitrate index has values that depend on version and layer,
					// but 1111b is invalid across the board. Same for sample rate,
					// 11b is invalid. Finally, an emphasis setting of 10b is bad, too.
					uint8_t rates = (mc[2+size]>>2);
					uint8_t emphasis = mc[3+size] & 3;
					if (rates != 0x3F && emphasis != 2) {
						// More checks could be done here, notably to compute frame length
						// and check that it corresponds to either another frame or EOF...
						return EDF_MAYBE;
					}
				}
			}
		}
		return EDF_FALSE;
}

class MP2DataFormat : public EntryDataFormat {
public:
	MP2DataFormat() : EntryDataFormat("snd_mp2") {};
	~MP2DataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return validMPEG(mc, 2, checkForTags(mc));
	}
};

class MP3DataFormat : public EntryDataFormat {
public:
	MP3DataFormat() : EntryDataFormat("snd_mp3") {};
	~MP3DataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return validMPEG(mc, 3, checkForTags(mc));
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
				uint16_t version = READ_L16(mc, 22);
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

class WolfSoundDataFormat : public EntryDataFormat {
public:
	WolfSoundDataFormat() : EntryDataFormat("snd_wolf") {};
	~WolfSoundDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		return (mc.getSize() > 0 ? EDF_MAYBE : EDF_FALSE);
	}
};

// Blood SFX+RAW format
class BloodSFXDataFormat : public EntryDataFormat {
public:
	BloodSFXDataFormat() : EntryDataFormat("snd_bloodsfx") {};
	~BloodSFXDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size, must be between 22 and 29 included
		if (mc.getSize() > 21 && mc.getSize() < 30) {
			if (mc[12] == 1 || mc[12] == 5) {
				size_t i = 20;
				for (; i < mc.getSize() - 1; ++i) {
					// Check that the entry does give a purely alphanumeric ASCII name
					if ((mc[i] < '0' || (mc[i] > '9' && mc[i] < 'A') ||
						(mc[i] > 'Z' && mc[i] < 'a') || mc[i] > 'z') && mc[i] != '_')
						return EDF_FALSE;
				}
				// And check that it null-terminates
				if (mc[i] == 0)
					return EDF_TRUE;
			}
		}
		return EDF_FALSE;
	}
};

// SNES SPC format, supported by ZDoom and Eternity
class SPDCDataFormat : public EntryDataFormat {
public:
	SPDCDataFormat() : EntryDataFormat("snd_spc") {};
	~SPDCDataFormat() {}

	int isThisFormat(MemChunk& mc) {
		// Check size
		if (mc.getSize() > 35) {
			// Check for header text using official signature string
			string header(wxString::FromAscii(mc.getData(), 35));
			if (header == "SNES-SPC700 Sound File Data v0.30\x1A\x1A")
				return EDF_TRUE;
		}
		return EDF_FALSE;
	}
};

#endif //AUDIOFORMATS_H
