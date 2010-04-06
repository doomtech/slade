
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

struct tx_patch_t {
	int16_t			left;
	int16_t			top;
	uint16_t		patch;
	PropertyList	ex_props;
};

struct tx_texture_t {
	string			name;
	uint16_t		flags;
	uint8_t			scale_x;
	uint8_t			scale_y;
	int16_t			width;
	int16_t			height;
	PropertyList	ex_props;

	vector<tx_patch_t>	patches;

	CTexture*	tex;

	tx_texture_t() { tex = NULL; }
	~tx_texture_t() { if (tex) delete tex; }
};

class TextureXList {
private:
	vector<tx_texture_t>	textures;
	vector<pnames_entry_t>	patches;
	uint8_t					txformat;

public:
	TextureXList();
	~TextureXList();

	uint32_t	nTextures() { return textures.size(); }
	uint32_t	nPatches() { return patches.size(); }

	CTexture*		getCTexture(size_t index);
	CTexture*		getCTexture(string name);
	tx_texture_t	getTexInfo(size_t index);
	tx_texture_t	getTexInfo(string name);
	string			getPatchName(size_t index);
	ArchiveEntry*	getPatchEntry(size_t index);

	void	clear();
	bool	updatePatches(Archive* archive = NULL);

	bool	readTEXTUREXData(ArchiveEntry* texturex, ArchiveEntry* pnames);
	bool	readTEXTURESData(ArchiveEntry* textures);
};

#endif//__TEXTUREXLIST_H__
