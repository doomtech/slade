
#ifndef __TEXTUREXLIST_H__
#define __TEXTUREXLIST_H__

#include "ArchiveEntry.h"
#include "PatchTable.h"

// Enum for different texturex formats
enum TextureXFormat {
    TXF_NORMAL,
    TXF_STRIFE11,
    TXF_NAMELESS,
};

// Texture flags
#define TX_WORLDPANNING	0x8000

// TEXTUREx teture patch
struct tx_patch_t {
	int16_t		left;
	int16_t		top;
	uint16_t	patch;
};

// TEXTUREx texture definition
struct tx_texture_t {
	string		name;
	uint16_t	flags;
	uint8_t		scale_x;
	uint8_t		scale_y;
	int16_t		width;
	int16_t		height;

	vector<tx_patch_t>	patches;
};

class TextureXList {
private:
	vector<tx_texture_t>	textures;
	PatchTable				patches;
	uint8_t					txformat;
	tx_texture_t			tex_invalid;

public:
	TextureXList();
	~TextureXList();

	PatchTable&	patchTable() { return patches; }

	uint32_t	nTextures() { return textures.size(); }
	uint32_t	nPatches() { return patches.nPatches(); }

	tx_texture_t	getTexture(size_t index);
	tx_texture_t	getTexture(string name);

	void	clear(bool clear_patches = false);

	bool	readTEXTUREXData(ArchiveEntry* texturex, ArchiveEntry* pnames = NULL);
};




/*
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

	static CTexture*	generateCTexture(tx_texture_t& info);
};
*/

#endif//__TEXTUREXLIST_H__
