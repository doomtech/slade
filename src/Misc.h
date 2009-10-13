
#ifndef __MISC_H__
#define	__MISC_H__

#include "SImage.h"
#include "ArchiveEntry.h"

namespace Misc {
	bool loadImageFromEntry(SImage* image, ArchiveEntry* entry);
}

#endif //__MISC_H__
