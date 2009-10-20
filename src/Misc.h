
#ifndef __MISC_H__
#define	__MISC_H__

#include "SImage.h"
#include "Archive.h"
#include "ArchiveEntry.h"

namespace Misc {
	bool loadImageFromEntry(SImage* image, ArchiveEntry* entry);
	bool loadPaletteFromArchive(Palette8bit* pal, Archive* archive);
}

#endif //__MISC_H__
