
#ifndef __PALETTE_H__
#define	__PALETTE_H__

#include "Archive.h"

class Palette8bit {
private:
	rgba_t	colours[255];
	short	index_trans;

public:
	Palette8bit();
	~Palette8bit();

	// Define static 'global' palette
	static Palette8bit& globalPalette() {
		static Palette8bit global;
		return global;
	}

	rgba_t	colour(uint8_t index) { return colours[index]; }
	short	transIndex() { return index_trans; }

	void	setColour(uint8_t index, rgba_t col) { colours[index] = col; }
	void	setTransIndex(short index) { index_trans = index; }

	void	copyPalette(Palette8bit& copy);
	bool	loadFromArchive(Archive* archive);
};

#endif //__PALETTE_H__
