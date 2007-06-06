
#ifndef __DM_SIDE_H__
#define __DM_SIDE_H__

class Texture;
class DoomMap;
class Sector;

#define	TEX_UPPER	0
#define	TEX_MIDDLE	1
#define TEX_LOWER	2

struct doomside_t
{
	short	x_offset;
	short	y_offset;
	char	tex_upper[8];
	char	tex_lower[8];
	char	tex_middle[8];
	short	sector;
};

class Side
{
private:
	DoomMap *parent;

	string	tex_lower;
	string	tex_middle;
	string	tex_upper;
	int		x_offset, y_offset;
	Sector	*sector;

	Texture	*tx_lower;
	Texture	*tx_middle;
	Texture	*tx_upper;

	props_t ex_props;

	int		index;
	
public:
	Side(DoomMap *parent = NULL);
	Side(doomside_t s, DoomMap *parent);
	~Side();

	Sector	*get_sector();
	string	get_texname(BYTE part = TEX_MIDDLE);
	Texture	*get_tex(BYTE part = TEX_MIDDLE);
	int		get_xoff() { return x_offset; }
	int		get_yoff() { return y_offset; }

	void	set_xoff(int xoff);
	void	set_yoff(int yoff);
	void	set_texture(string tex, BYTE part = TEX_MIDDLE);
	void	set_sector(int sector);
	void	set_sector(Sector* sector) { this->sector = sector; }
	void	set_parent(DoomMap* parent) { this->parent = parent; }

	int		get_index() { return index; }
	void	set_index(int i) { index = i; }

	void	copy(Side* side, bool update_tex = true);

	void	add_tex_counts();

	doomside_t	to_doomformat();
};

#endif
