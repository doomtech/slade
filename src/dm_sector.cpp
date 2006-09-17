
#include "main.h"
#include "doom_map.h"
#include "dm_sector.h"
#include "textures.h"
#include "game_config.h"

extern GameConfig game;
extern bool startup;

/*
string	def_ftex = "MFLR8_1";
string	def_ctex = "MFLR8_1";
short	def_fheight = 0;
short	def_cheight = 128;
short	def_light = 160;
*/

Sector::Sector(DoomMap *parent)
{
	this->parent = parent;

	special = 0;
	tag = 0;
	tex_f = NULL;
	tex_c = NULL;
	index = -1;

	if (startup)
	{
		f_height = game.def_fheight;
		c_height = game.def_cheight;
		f_tex = game.def_ftex;
		c_tex = game.def_ctex;
		light = game.def_light;
	}

	if (parent)
		parent->add_sector(this);
}

Sector::Sector(doomsector_t s, DoomMap *parent)
{
	Texture *tex = NULL;
	tex_f = NULL;
	tex_c = NULL;
	index = -1;

	this->parent = parent;

	f_height = wxINT16_SWAP_ON_BE(s.f_height);
	c_height = wxINT16_SWAP_ON_BE(s.c_height);
	light = wxINT16_SWAP_ON_BE(s.light);
	special = wxINT16_SWAP_ON_BE(s.special);
	tag = wxINT16_SWAP_ON_BE(s.tag);

	char temp[9];
	temp[8] = 0;

	memcpy(temp, s.f_tex, 8);
	f_tex = wxString::FromAscii(temp).Upper();
	tex = get_texture(f_tex);
	if (tex) tex->use_count++;

	memcpy(temp, s.c_tex, 8);
	c_tex = wxString::FromAscii(temp).Upper();
	tex = get_texture(c_tex);
	if (tex) tex->use_count++;

	if (parent)
		parent->add_sector(this);
}

Sector::~Sector()
{
	Texture* tex = NULL;

	tex = get_texture(this->f_tex);
	if (tex) tex->use_count--;

	tex = get_texture(this->c_tex);
	if (tex) tex->use_count--;
}

int Sector::strip_special(int spec, bool hexen)
{
	int realtype = spec;
	int mult = 0;
	if (hexen) mult = 8;

	realtype = realtype & ~(32 * mult);
	realtype = realtype & ~(64 * mult);
	realtype = realtype & ~(128 * mult);
	realtype = realtype & ~(256 * mult);
	realtype = realtype & ~(512 * mult);

	return realtype;
}

int Sector::stripped_special()
{
	if (!parent)
		return special;
	else
		return strip_special(special, parent->hexen());
}

doomsector_t Sector::to_doomformat()
{
	doomsector_t ret;

	ret.c_height = wxINT16_SWAP_ON_BE((short)c_height);
	ret.f_height = wxINT16_SWAP_ON_BE((short)f_height);
	ret.light = wxINT16_SWAP_ON_BE((short)light);
	ret.special = wxINT16_SWAP_ON_BE((short)special);
	ret.tag = wxINT16_SWAP_ON_BE((short)tag);

	for (unsigned int a = 0; a < 8; a++)
	{
		ret.f_tex[a] = 0;
		ret.c_tex[a] = 0;
	}

	for (unsigned int a = 0; a < f_tex.size(); a++)
		ret.f_tex[a] = f_tex[a];

	for (unsigned int a = 0; a < c_tex.size(); a++)
		ret.c_tex[a] = c_tex[a];

	return ret;
}

void Sector::set_ftex(string tex)
{
	Texture* t = get_texture(f_tex);
	if (t) t->use_count--;
	f_tex = tex;
	t = get_texture(f_tex);
	if (t) t->use_count++;

	tex_f = NULL;
}

void Sector::set_ctex(string tex)
{
	Texture* t = get_texture(c_tex);
	if (t) t->use_count--;
	c_tex = tex;
	t = get_texture(c_tex);
	if (t) t->use_count++;

	tex_c = NULL;
}

Texture* Sector::get_tex(bool floor)
{
	if (floor)
	{
		if (!tex_f)
			tex_f = get_texture(f_tex, 2);

		return tex_f;
	}
	else
	{
		if (!tex_c)
			tex_c = get_texture(c_tex, 2);

		return tex_c;
	}
}

void Sector::copy(Sector* s)
{
	parent = s->parent;
	index = -1;

	f_height = s->floor();
	c_height = s->ceiling();
	light = s->light_level();
	special = s->action_special();
	tag = s->sector_tag();

	set_ctex(s->tex_ceil());
	set_ftex(s->tex_floor());
}
