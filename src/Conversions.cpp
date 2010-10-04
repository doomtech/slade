
#include "Main.h"
#include "Conversions.h"
#include "qmus2mid/qmus2mid.h"

// Some structs for wav conversion
struct wav_chunk_t {
	char id[4];
	uint32_t size;
};

struct wav_fmtchunk_t {
	wav_chunk_t header;
	uint16_t tag;
	uint16_t channels;
	uint32_t samplerate;
	uint32_t datarate;
	uint16_t blocksize;
	uint16_t bps;
};

struct dsnd_header_t {
	uint16_t three;
	uint16_t samplerate;
	uint32_t samples;
};

bool Conversions::doomSndToWav(MemChunk& in, MemChunk& out) {
	// --- Read Doom sound ---

	// Read doom sound header
	dsnd_header_t header;
	in.seek(0, SEEK_SET);
	in.read(&header, 8);

	// Format checks
	if (header.three != 3) {	// Check for magic number
		Global::error = "Invalid Doom Sound";
		return false;
	}
	if (header.samples > (in.getSize() - 8) || header.samples <= 4) {	// Check for sane values
		Global::error = "Invalid Doom Sound";
		return false;
	}

	// Read samples
	uint8_t* samples = new uint8_t[header.samples];
	in.read(samples, header.samples);


	// --- Write WAV ---

	wav_chunk_t whdr, wdhdr;
	wav_fmtchunk_t fmtchunk;

	// Setup data header
	char did[4] = { 'd', 'a', 't', 'a' };
	memcpy(&wdhdr.id, &did, 4);
	wdhdr.size = header.samples;

	// Setup fmt chunk
	char fid[4] = { 'f', 'm', 't', ' ' };
	memcpy(&fmtchunk.header.id, &fid, 4);
	fmtchunk.header.size = 16;
	fmtchunk.tag = 1;
	fmtchunk.channels = 1;
	fmtchunk.samplerate = header.samplerate;
	fmtchunk.datarate = header.samplerate;
	fmtchunk.blocksize = 1;
	fmtchunk.bps = 8;

	// Setup main header
	char wid[4] = { 'R', 'I', 'F', 'F' };
	memcpy(&whdr.id, &wid, 4);
	whdr.size = wdhdr.size + fmtchunk.header.size + 8;

	// Write chunks
	out.write(&whdr, 8);
	out.write("WAVE", 4);
	out.write(&fmtchunk, sizeof(wav_fmtchunk_t));
	out.write(&wdhdr, 8);
	out.write(samples, header.samples);

	// Ensure data ends on even byte boundary
	if (header.samples % 2 != 0)
		out.write('\0', 1);

	return true;
}

/* Conversions::d64SfxToWav
 * These entries are in raw 22050 Hz 16-bit-per-sample PCM format.
 *******************************************************************/
bool Conversions::d64SfxToWav(MemChunk& in, MemChunk& out) {
	// Ridiculously weak format check.
	// but that's the best we can do with raw data.
	if (in.getSize() % 2) {
		Global::error = "Invalid Doom 64 SFX";
		return false;
	}

	// --- Write WAV ---
	wav_chunk_t whdr, wdhdr;
	wav_fmtchunk_t fmtchunk;

	// Setup data header
	char did[4] = { 'd', 'a', 't', 'a' };
	memcpy(&wdhdr.id, &did, 4);
	wdhdr.size = in.getSize();

	// Setup fmt chunk
	char fid[4] = { 'f', 'm', 't', ' ' };
	memcpy(&fmtchunk.header.id, &fid, 4);
	fmtchunk.header.size = 16;
	fmtchunk.tag = 1;
	fmtchunk.channels = 1;
	fmtchunk.samplerate = 22050;
	fmtchunk.datarate = 44100;
	fmtchunk.blocksize = 2;
	fmtchunk.bps = 16;

	// Setup main header
	char wid[4] = { 'R', 'I', 'F', 'F' };
	memcpy(&whdr.id, &wid, 4);
	whdr.size = wdhdr.size + fmtchunk.header.size + 8;

	// Write chunks
	out.write(&whdr, 8);
	out.write("WAVE", 4);
	out.write(&fmtchunk, sizeof(wav_fmtchunk_t));
	out.write(&wdhdr, 8);
	out.write(in.getData(), in.getSize());

	return true;
}

bool Conversions::wavToDoomSnd(MemChunk& in, MemChunk& out) {
	// --- Read WAV ---
	wav_chunk_t chunk;

	// Read header
	in.seek(0, SEEK_SET);
	in.read(&chunk, 8);

	// Check header
	if (chunk.id[0] != 'R' || chunk.id[1] != 'I' || chunk.id[2] != 'F' || chunk.id[3] != 'F') {
		Global::error = "Invalid WAV";
		return false;
	}

	// Read format
	char format[4];
	in.read(format, 4);

	// Check format
	if (format[0] != 'W' || format[1] != 'A' || format[2] != 'V' || format[3] != 'E') {
		Global::error = "Invalid WAV format";
		return false;
	}

	// Read fmt chunk
	wav_fmtchunk_t fmtchunk;
	in.read(&fmtchunk, sizeof(wav_fmtchunk_t));

	// Check fmt chunk values
	if (fmtchunk.header.id[0] != 'f' || fmtchunk.header.id[1] != 'm' || fmtchunk.header.id[2] != 't' || fmtchunk.header.id[3] != ' ') {
		Global::error = "Invalid WAV";
		return false;
	}
	if (fmtchunk.channels != 1) {
		Global::error = "Cannot convert, must be mono";
		return false;
	}
	if (fmtchunk.bps != 8) {
		Global::error = "Cannot convert, must be 8bit";
		return false;
	}

	// Read data
	in.read(&chunk, 8);

	// Check data
	if (chunk.id[0] != 'd' || chunk.id[1] != 'a' || chunk.id[2] != 't' || chunk.id[3] != 'a') {
		Global::error = "Invalid WAV";
		return false;
	}

	uint8_t* data = new uint8_t[chunk.size];
	in.read(data, chunk.size);


	// --- Write Doom Sound ---

	// Write header
	dsnd_header_t ds_hdr;
	ds_hdr.three = 3;
	ds_hdr.samplerate = fmtchunk.samplerate;
	ds_hdr.samples = chunk.size;
	out.write(&ds_hdr, 8);

	// Write data
	out.write(data, chunk.size);

	return true;
}

bool Conversions::musToMidi(MemChunk& in, MemChunk& out) {
	string tempmidi = appPath("sladetemp.mid", DIR_TEMP);
	string tempmus = appPath("sladetemp.mus", DIR_TEMP);

	// Dump mus to temp file
	in.exportFile(tempmus);

	// Run qmus2mid
	qmus2mid(chr(tempmus), chr(tempmidi), 1, 0, 128, 0);

	// Read outputted midi to MemChunk
	out.importFile(tempmidi);

	// Clean up
	wxRemoveFile(tempmidi);
	wxRemoveFile(tempmus);

	return true;
}

/* Conversions::vocToWav
 * Creative Voice files to wav format
 *******************************************************************/
bool Conversions::vocToWav(MemChunk& in, MemChunk& out) {
	if (in.getSize() < 26 || in[19] != 26 || in[20] != 26 || in[21] != 0
		|| (0x1234 + ~((in[23]<<8) + in[22])) != (in[24] + (in[25] << 8))) {
		Global::error = "Invalid VOC";
		return false;
	}

	// --- Prepare WAV ---
	wav_chunk_t whdr, wdhdr;
	wav_fmtchunk_t fmtchunk;

	// --- Pre-process the file to make sure we can convert it ---
	int codec = -1;
	int blockcount = 0;
	size_t datasize = 0;
	size_t i = 26, e = in.getSize();
	bool gotextra = false;
	while (i < e) {
		// Parses through blocks
		uint8_t blocktype = in[i];
		size_t blocksize = in[i+1] + (in[i+2]<<8) + (in[i+3]<<16);
		i+=4;
		if (i + blocksize > e && blocktype != 0) {
			Global::error = s_fmt("VOC file cut abruptly in block %i", blockcount);
			return false;
		}
		blockcount++;
		switch (blocktype) {
			case 0: // Terminator, the rest should be ignored
				i = e; break;
			case 1: // Sound data
				if (!gotextra && codec >= 0 && codec != in[i+1]) {
					Global::error = "VOC files with different codecs are not supported";
					return false;
				} else if (codec == -1) {
					fmtchunk.samplerate = 1000000/(256 - in[i]);
					fmtchunk.channels = 1;
					fmtchunk.tag = 1;
					codec = in[i+1];
				}
				datasize += blocksize;
				break;
			case 2: // Sound data continuation
				if (codec == -1) {
						Global::error = "Sound data without codec in VOC file";
						return false;
				}
				datasize += blocksize;
				break;
			case 3: // Silence
			case 4: // Marker
			case 5: // Text
			case 6: // Repeat start point
			case 7: // Repeat end point
				break;
			case 8: // Extra info, overrides any following sound data codec info
				if (codec != -1) {
					Global::error = "Extra info block must precede sound data info block in VOC file";
					return false;
				} else {
					fmtchunk.samplerate = 256000000/((in[i+3] + 1) * (65536 - (in[i] + (in[i+1]<<8))));
					fmtchunk.channels = in[i+3] + 1;
					fmtchunk.tag = 1;
					codec = in[i+2];
				}
				break;
			case 9: // Sound data in new format
				if (codec >= 0 && codec != (in[i+6]+(in[i+7]<<8))) {
					Global::error = "VOC files with different codecs are not supported";
					return false;
				} else if (codec == -1) {
					fmtchunk.samplerate = in[i] + (in[i+1]<<8) + (in[i+2]<<16) + (in[i+3]<<24);
					fmtchunk.bps = in[i+4];
					fmtchunk.channels = in[i+5];
					fmtchunk.tag = 1;
					codec = in[i+6]+(in[i+7]<<8);
				}
				break;
		}
		i += blocksize;
	}
	wdhdr.size = datasize;
	switch (codec) {
		case 0: // 8 bits unsigned PCM
			fmtchunk.bps = 8;
			fmtchunk.datarate = fmtchunk.samplerate;
			fmtchunk.blocksize = 1;
			break;
		case 4: // 16 bits signed PCM
			fmtchunk.bps = 16;
			fmtchunk.datarate = fmtchunk.samplerate<<1;
			fmtchunk.blocksize = 2;
			break;
		case 1: // 4 bits to 8 bits Creative ADPCM
		case 2: // 3 bits to 8 bits Creative ADPCM (AKA 2.6 bits)
		case 3: // 2 bits to 8 bits Creative ADPCM
		case 6: // alaw
		case 7: // ulaw
		case 0x200: // 4 bits to 16 bits Creative ADPCM (only valid in block type 0x09)
			Global::error = s_fmt("Unsupported codec %i in VOC file", codec);
			return false;
		default: 
			Global::error = s_fmt("Unknown codec %i in VOC file", codec);
			return false;
	}

	// --- Write WAV ---

	// Setup data header
	char did[4] = { 'd', 'a', 't', 'a' };
	memcpy(&wdhdr.id, &did, 4);

	// Setup fmt chunk
	char fid[4] = { 'f', 'm', 't', ' ' };
	memcpy(&fmtchunk.header.id, &fid, 4);
	fmtchunk.header.size = 16;

	// Setup main header
	char wid[4] = { 'R', 'I', 'F', 'F' };
	memcpy(&whdr.id, &wid, 4);
	whdr.size = wdhdr.size + fmtchunk.header.size + 8;

	// Write chunks
	out.write(&whdr, 8);
	out.write("WAVE", 4);
	out.write(&fmtchunk, sizeof(wav_fmtchunk_t));
	out.write(&wdhdr, 8);

	// Now go and copy sound data
	const uint8_t * src = in.getData();
	i = 26;
	while (i < e) {
		// Parses through blocks again
		uint8_t blocktype = in[i];
		size_t blocksize = in[i+1] + (in[i+2]<<8) + (in[i+3]<<16);
		i+=4;
		switch (blocktype) {
			case 1: // Sound data
				out.write(src+i+2, blocksize - 2);
				break;
			case 2: // Sound data continuation
				out.write(src+i, blocksize);
				break;
			case 3: // Silence
				// Not supported yet
				break;
			case 9: // Sound data in new format
				out.write(src+i+12, blocksize - 12);
			default:
				break;
		}
		i += blocksize;
	}

	return true;
}
