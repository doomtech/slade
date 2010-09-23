
#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

namespace Conversions {
	bool	wavToDoomSnd(MemChunk& in, MemChunk& out);
	bool	doomSndToWav(MemChunk& in, MemChunk& out);
	bool	midiToMus(MemChunk& in, MemChunk& out);
	bool	musToMidi(MemChunk& in, MemChunk& out);
};

#endif//__CONVERSIONS_H__
