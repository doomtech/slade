
#ifndef __ENTRYOPERATIONS_H__
#define	__ENTRYOPERATIONS_H__

#include "ArchiveEntry.h"

namespace EntryOperations {
	bool	openExternal(ArchiveEntry* entry);
	bool	modifyGfxOffsets(ArchiveEntry* entry, int auto_type, point2_t offsets, bool xc, bool yc, bool relative);
};

#endif//__ENTRYOPERATIONS_H__
