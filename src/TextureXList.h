
#ifndef __TEXTUREXLIST_H__
#define __TEXTUREXLIST_H__

#include "CTexture.h"
#include "ArchiveEntry.h"

class TextureXList {
private:
	vector<CTexture*>		textures;
	vector<ArchiveEntry*>	patches;

public:
	TextureXList();
	~TextureXList();

	uint32_t	nTextures() { return textures.size(); }
	uint32_t	nPatches() { return patches.size(); }

	CTexture*		getTexture(int index);
	CTexture*		getTexture(string name);
	ArchiveEntry*	getPatch(int index);
	ArchiveEntry*	getPatch(string name);

	void	clear();

	bool	readTEXTUREXData(ArchiveEntry* texturex, ArchiveEntry* pnames);
	bool	readTEXTURESData(ArchiveEntry* textures);
};

#endif//__TEXTUREXLIST_H__
