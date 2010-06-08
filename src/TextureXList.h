
#ifndef __TEXTUREXLIST_H__
#define __TEXTUREXLIST_H__

#include "ArchiveEntry.h"

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
	uint8_t					txformat;
	tx_texture_t			tex_invalid;

public:
	TextureXList();
	~TextureXList();

	uint32_t	nTextures() { return textures.size(); }

	tx_texture_t	getTexture(size_t index);
	tx_texture_t	getTexture(string name);

	void	addTexture(tx_texture_t& tex, int position = -1);

	void	clear(bool clear_patches = false);
	void	removePatch(unsigned index);

	bool	readTEXTUREXData(ArchiveEntry* texturex);
};

#endif//__TEXTUREXLIST_H__
