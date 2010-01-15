
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

	void	clear();

	bool	readTEXTUREXData(ArchiveEntry* texturex, ArchiveEntry* pnames);
	bool	readTEXTURESData(ArchiveEntry* textures);
};

#endif//__TEXTUREXLIST_H__
