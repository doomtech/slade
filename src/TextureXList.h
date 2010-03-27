
#ifndef __TEXTUREXLIST_H__
#define __TEXTUREXLIST_H__

#include "CTexture.h"
#include "ArchiveEntry.h"

enum TextureXFormat {
	TXF_NORMAL,
	TXF_STRIFE11,
	TXF_NAMELESS,
};

struct pnames_entry_t {
	string			name;
	ArchiveEntry*	entry;

	pnames_entry_t() { entry = NULL; }
};

class TextureXList {
private:
	vector<CTexture*>		textures;
	vector<pnames_entry_t>	patches;
	uint8_t					txformat;

public:
	TextureXList();
	~TextureXList();

	uint32_t	nTextures() { return textures.size(); }
	uint32_t	nPatches() { return patches.size(); }

	CTexture*		getTexture(size_t index);
	CTexture*		getTexture(string name);
	string			getPatchName(size_t index);
	ArchiveEntry*	getPatchEntry(size_t index);

	void	clear();

	bool	readTEXTUREXData(ArchiveEntry* texturex, ArchiveEntry* pnames);
	bool	readTEXTURESData(ArchiveEntry* textures);
};

#endif//__TEXTUREXLIST_H__
