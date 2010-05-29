
#ifndef __MISC_H__
#define	__MISC_H__

#include "SImage.h"
#include "Archive.h"
#include "ArchiveEntry.h"

enum {
	PAL_NOHACK = 0,
	PAL_ALPHAHACK,
	PAL_HERETICHACK,
	PAL_SHADOWHACK,
};

namespace Misc {
	bool		loadImageFromEntry(SImage* image, ArchiveEntry* entry);
	int			detectPaletteHack(ArchiveEntry* entry);
	bool		loadPaletteFromArchive(Palette8bit* pal, Archive* archive, int lump = PAL_NOHACK);
	string		sizeAsString(uint32_t size);
	uint32_t	crc(uint8_t *buf, uint32_t len);

	// Below are some hacky hard-coded things that I plan to remove eventually
	// (but they'll do for now :P)
	bool		addPatchEntry(Archive* archive, ArchiveEntry* entry);
}

#endif //__MISC_H__
