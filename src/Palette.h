
#ifndef __PALETTES_H__
#define	__PALETTES_H__

#include "Archive.h"

class Palette {
private:
	rgba_t	colours[255];
	short	index_trans;

public:
	Palette();
	~Palette();

	// Define static 'global' palette
	static Palette& globalPalette() {
		static Palette global;
		return global;
	}

	rgba_t	colour(uint8_t index) { return colours[index]; }
	short	transIndex() { return index_trans; }

	void	setColour(uint8_t index, rgba_t col) { colours[index] = col; }
	void	setTransIndex(short index) { index_trans = index; }

	void	copyPalette(Palette& copy);
	bool	loadFromArchive(Archive* archive);
};

#endif //__PALETTES_H__
