
#ifndef __ENTRYOPERATIONS_H__
#define	__ENTRYOPERATIONS_H__

#include "ArchiveEntry.h"

namespace EntryOperations {
	bool	openExternal(ArchiveEntry* entry);
	bool	modifyGfxOffsets(ArchiveEntry* entry, int auto_type, point2_t offsets, bool xc, bool yc, bool relative);
	bool	modifyalPhChunk(ArchiveEntry* entry, bool value);
	bool	modifytRNSChunk(ArchiveEntry* entry, bool value);
	bool	getalPhChunk(ArchiveEntry* entry);
	bool	gettRNSChunk(ArchiveEntry* entry);
	bool	readgrAbChunk(ArchiveEntry* entry, point2_t &offsets);
	bool	addToPatchTable(vector<ArchiveEntry*> entries);
	bool	createTexture(vector<ArchiveEntry*> entries);
	bool	convertTextures(vector<ArchiveEntry*> entries);
	bool	compileACS(ArchiveEntry* entry, bool hexen = false);
	bool	exportAsPNG(ArchiveEntry* entry, string filename);
	bool	optimizePNG(ArchiveEntry* entry);
};

#endif//__ENTRYOPERATIONS_H__
