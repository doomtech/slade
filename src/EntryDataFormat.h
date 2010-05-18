
#ifndef __ENTRYDATAFORMAT_H__
#define __ENTRYDATAFORMAT_H__

class EntryDataFormat {
private:
	// Struct to specify a range for a byte (min <= valid >= max)
	// If max == min, only 1 valid value
	struct byte_vrange_t {
		uint8_t	min;
		uint8_t	max;

		byte_vrange_t() { min = 0; max = 255; }
	};

	// Struct to specify valid values for a byte at pos
	struct byte_pattern_t {
		unsigned				pos;
		vector<byte_vrange_t>	valid_values;

		bool match(uint8_t value) {
			for (unsigned a = 0; a < valid_values.size(); a++) {
				if (value < valid_values[a].min || value > valid_values[a].max)
					return false;
			}

			return true;
		}
	};

	// Info
	string	extension;
	string	category;
	string	editor;

	// Detection
	unsigned				size_min;
	unsigned				size_max;
	vector<byte_pattern_t>	patterns;
	/* Also needed:
	 * Some way to check more complex values (eg. multiply byte 0 and 1, result must be in a certain range)
	 */

public:
	EntryDataFormat();
	~EntryDataFormat();

	bool	isThisFormat(MemChunk& mc);


	static bool	readDataFormatDefinition(MemChunk& mc);

	// Builtin detection routines (try to minimise these as much as possible :))
	static bool detectPng(MemChunk& mc);
	static bool detectBmp(MemChunk& mc);
	static bool detectGif(MemChunk& mc);
	static bool detectPcx(MemChunk& mc);
	static bool detectTga(MemChunk& mc);
	static bool detectTiff(MemChunk& mc);
	static bool detectJpeg(MemChunk& mc);
	static bool detectDoomGfx(MemChunk& mc);
	static bool detectDoomGfxAlpha(MemChunk& mc);
	static bool detectDoomGfxBeta(MemChunk& mc);
	static bool detectDoomGfxArah(MemChunk& mc);
	static bool detectDoomGfxSnea(MemChunk& mc);
	static bool detectDoomFlat(MemChunk& mc);
	static bool detectDoomLegacy(MemChunk& mc);
	static bool detectPlanar(MemChunk& mc);
	static bool detect4bitChunk(MemChunk& mc);
	static bool detectImgz(MemChunk& mc);
	static bool detectPalette(MemChunk& mc);
	static bool detectTranslationTable(MemChunk& mc);
	static bool detectWad(MemChunk& mc);
	static bool detectZip(MemChunk& mc);
	static bool detectMus(MemChunk& mc);
	static bool detectMidi(MemChunk& mc);
	static bool detectModIt(MemChunk& mc);
	static bool detectModXm(MemChunk& mc);
	static bool detectModS3m(MemChunk& mc);
	static bool detectModMod(MemChunk& mc);
	static bool detectSndDoom(MemChunk& mc);
	static bool detectSndWav(MemChunk& mc);
	static bool detectSndOgg(MemChunk& mc);
	static bool detectSndFlac(MemChunk& mc);
	static bool detectSndSpeaker(MemChunk& mc);
	static bool detectTextureX(MemChunk& mc);
	static bool detectPnames(MemChunk& mc);
	static bool detectAnimated(MemChunk& mc);
	static bool detectSwitches(MemChunk& mc);
	static bool detectFontM(MemChunk& mc);
	static bool detectFont0(MemChunk& mc);
	static bool detectFont1(MemChunk& mc);
	static bool detectFont2(MemChunk& mc);
	static bool detectBMF(MemChunk& mc);
	static bool detectAnsi(MemChunk& mc);
	static bool detectZGLNodes(MemChunk& mc);
	static bool detectZGLNodes2(MemChunk& mc);
	static bool detectDMD(MemChunk& mc);
	static bool detectMDL(MemChunk& mc);
	static bool detectMD2(MemChunk& mc);
	static bool detectMD3(MemChunk& mc);
};

#endif//__ENTRYDATAFORMAT_H__
