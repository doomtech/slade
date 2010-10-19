
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
	PAL_ROTTNHACK,
	PAL_ROTTDHACK,
	PAL_ROTTFHACK,
	PAL_ROTTAHACK,
};

namespace Misc {
	bool		loadImageFromEntry(SImage* image, ArchiveEntry* entry, int index = 0);
	int			detectPaletteHack(ArchiveEntry* entry);
	bool		loadPaletteFromArchive(Palette8bit* pal, Archive* archive, int lump = PAL_NOHACK);
	string		sizeAsString(uint32_t size);
	uint32_t	crc(uint8_t *buf, uint32_t len);

	// Mass Rename
	string	massRenameFilter(wxArrayString& names);
	void	doMassRename(wxArrayString& names, string name_filter);
}

#endif //__MISC_H__
