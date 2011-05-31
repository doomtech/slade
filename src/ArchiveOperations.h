
#ifndef __ARCHIVE_OPERATIONS_H__
#define __ARCHIVE_OPERATIONS_H__

#include "Archive.h"

namespace ArchiveOperations {
	bool	removeUnusedPatches(Archive* archive);
	bool	checkDuplicateEntryNames(Archive* archive);
};

#endif//__ARCHIVE_OPERATIONS_H__
