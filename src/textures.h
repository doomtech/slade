

#ifndef __TEXTURES_H__
#define __TEXTURES_H__

#define TEX_STRETCH	0x01
#define TEX_NOALPHA	0x02
#define TEX_IWAD	0x04

#include <GL/gl.h>
#include <GL/glu.h>
#include "structs.h"

class Wad;

class Texture
{
protected:
	bool		id_generated;
	GLuint		gl_id;
	BYTE		*data;

public:
	BYTE		filter;
	BYTE		flags;
	string		name;
	int			width, height;		// Texture dimansions (in game units)
	int			rwidth, rheight;	// Texture dimensions (in pixels)
	float		t_x, t_y;
	int			use_count;
	rgba_t		avg_col;

	Texture();
	~Texture();

	bool load_file(string filename);
	void gen_from_data(int width, int height, BYTE* data, int swidth = -1, int sheight = -1, bool save_data = false);
	GLuint get_gl_id();
	void purge();

	virtual bool gen_gl_tex();
};

class SimpleTexture : public Texture
{
private:
	Wad		*data_wad;
	string	lump_name;

public:
	SimpleTexture();
	~SimpleTexture();

	void set_data(Wad* wad, string lump);

	bool gen_gl_tex();
};

struct patch_header_t
{
	short	width;
	short	height;
	short	left;
	short	top;
};

struct col_header_t
{
	BYTE	row_offset;
	BYTE	n_pixels;
};

struct patch_info_t
{
	short	xoff;
	short	yoff;
	Wad*	wad;
	string	patch;
};

class DoomTexture : public Texture
{
private:
	vector<patch_info_t>	patches;

public:
	DoomTexture();
	~DoomTexture();

	void add_patch(short xoff, short yoff, Wad* wad, string patch)
	{
		patch_info_t p;
		p.xoff = xoff;
		p.yoff = yoff;
		p.wad = wad;
		p.patch = patch;
		patches.push_back(p);
	}

	bool gen_gl_tex();
};

class DoomFlat : public Texture
{
private:
	Wad*	data_wad;
	int		data_lump;

public:
	DoomFlat();
	~DoomFlat();

	void set_data(Wad* wad, int index)
	{
		data_wad = wad;
		data_lump = index;
	}

	bool gen_gl_tex();
};

class FileTexture : public Texture
{
private:
	string	filename;

public:
	FileTexture(string filename) { this->filename = filename; }
	~FileTexture();

	bool gen_gl_tex();
};

#define TEXTURES_WALLS		1
#define	TEXTURES_FLATS		2
#define TEXTURES_SPRITES	3
#define TEXTURES_EDITOR		4

Texture* get_texture(string name, int type = 0, bool force = false);
void load_textures();
void init_textures();
void load_flats();
void load_sprites();
void load_tx_textures();
void load_hirestex_textures();
void purge_textures();

#endif
