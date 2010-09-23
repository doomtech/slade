
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
	if (header.three != 3)	// Check for magic number
		return false;
	if (header.samples > (in.getSize() - 8) || header.samples <= 4)	// Check for sane values
		return false;

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
	return false;
}

bool Conversions::midiToMus(MemChunk& in, MemChunk& out) {
	// Not implemented (ever? is there really any need?)
	return false;
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

	return false;
}
