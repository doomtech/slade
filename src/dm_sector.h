
#ifndef __DM_SECTOR_H__
#define __DM_SECTOR_H__

#include "textures.h"

class DoomMap;

struct doomsector_t
{
	short	f_height;
	short	c_height;
	char	f_tex[8];
	char	c_tex[8];
	short	light;
	short	special;
	short	tag;
};

class Sector
{
private:
	DoomMap	*parent;

	int		f_height;
	int		c_height;
	string	f_tex;
	string	c_tex;
	int		light;
	int		special;
	int		tag;

	Texture	*tex_f;
	Texture	*tex_c;

	props_t	ex_props;

	int		index;

public:
	Sector(DoomMap *parent = NULL);
	Sector(doomsector_t s, DoomMap *parent);
	~Sector();

	int		ceiling()			{ return c_height; }
	int		floor()				{ return f_height; }
	int		light_level()		{ return light;	}
	int		action_special()	{ return special; }
	int		sector_tag()		{ return tag; }
	string	tex_ceil()			{ return c_tex; }
	string	tex_floor()			{ return f_tex; }

	Texture	*get_tex(bool floor = true);

	void	set_ftex(string tex);
	void	set_ctex(string tex);
	void	set_floor(int height) { f_height = height; }
	void	set_ceil(int height) { c_height = height; }
	void	set_light(int light) { this->light = light; }
	void	set_special(int spec) { special = spec; }
	void	set_tag(int tag) { this->tag = tag; }
	void	set_parent(DoomMap* parent) { this->parent = parent; }

	int		get_index() { return index; }
	void	set_index(int i) { index = i; }

	int		stripped_special();

	static int	strip_special(int spec, bool hexen);

	void	copy(Sector* s, bool update_tex = true);

	doomsector_t	to_doomformat();
};

#endif
