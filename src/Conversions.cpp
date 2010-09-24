
#include "Main.h"
#include "Conversions.h"
#include "qmus2mid/qmus2mid.h"

// Some structs for wav conversion
struct wav_chunk_t {
	char id[4];
	long size;
};

struct wav_fmtchunk_t {
	wav_chunk_t header;
	short tag;
	short channels;
	long samplerate;
	long datarate;
	short blocksize;
	short bps;
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
	fmtchunk.samplerate = header.samplerate;
	fmtchunk.datarate = header.samplerate;
	fmtchunk.channels = 1;
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
