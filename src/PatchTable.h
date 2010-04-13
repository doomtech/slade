
#ifndef __PATCH_TABLE_H__
#define __PATCH_TABLE_H__

#include "ArchiveEntry.h"

struct patch_t {
	string			name;
	ArchiveEntry*	entry;

	patch_t() { entry = NULL; }
};

class PatchTable {
private:
	Archive*		parent;
	vector<patch_t>	patches;

public:
	PatchTable(Archive* parent = NULL);
	~PatchTable();
	
	size_t	nPatches() { return patches.size(); }

	string			patchName(size_t index);
	ArchiveEntry*	patchEntry(size_t index);
	ArchiveEntry*	patchEntry(string name);
	int32_t			patchIndex(string name);
	int32_t			patchIndex(ArchiveEntry* entry);

	bool	loadPNAMES(ArchiveEntry* pnames, Archive* parent = NULL);
};

#endif//__PATCH_TABLE_T__
