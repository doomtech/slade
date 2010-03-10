
#ifndef __MISC_H__
#define	__MISC_H__

#include "SImage.h"
#include "Archive.h"
#include "ArchiveEntry.h"

enum {
	PAL_NOHACK = 0,
	PAL_ALPHAHACK,
	PAL_HERETICHACK,
};

namespace Misc {
	bool	loadImageFromEntry(SImage* image, ArchiveEntry* entry);
	int		detectPaletteHack(ArchiveEntry* entry);
	bool	loadPaletteFromArchive(Palette8bit* pal, Archive* archive, ArchiveEntry* entry = NULL);
	string	sizeAsString(uint32_t size);
}

#endif //__MISC_H__
