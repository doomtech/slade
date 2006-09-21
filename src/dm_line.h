
#ifndef __DM_LINE_H__
#define __DM_LINE_H__

#include "structs.h"

#define	LINE_IMPASSIBLE		0x0001
#define LINE_BLOCKMONSTERS	0x0002
#define	LINE_TWOSIDED		0x0004
#define	LINE_UPPERUNPEGGED	0x0008
#define	LINE_LOWERUNPEGGED	0x0010
#define	LINE_SECRET			0x0020
#define	LINE_BLOCKSOUND		0x0040
#define	LINE_NOTONMAP		0x0080
#define	LINE_STARTONMAP		0x0100

class Vertex;
class Side;
class DoomMap;

struct doomline_t
{
	short vertex1;
	short vertex2;
	short flags;
	short type;
	short sector_tag;
	short side1;
	short side2;
};

struct hexenline_t
{
	short	vertex1;
	short	vertex2;
	short	flags;
	BYTE	type;
	BYTE	args[5];
	short	side1;
	short	side2;
};

class Line
{
private:
	DoomMap	*parent;

	Vertex	*v1, *v2;
	Side	*s1, *s2;
	int		flags;
	int		type;
	int		sector_tag;
	BYTE	args[5];
	
	props_t ex_props;

	int		index;

public:
	Line(DoomMap *parent = NULL);
	Line(Vertex* v1, Vertex* v2, DoomMap* parent);
	Line(doomline_t l, DoomMap *parent, bool &ok);
	Line(hexenline_t l, DoomMap *parent, bool &ok);
	~Line();

	Vertex* vertex1();
	Vertex* vertex2();
	Side*	side1();
	Side*	side2();

	Side*	side(bool front);

	int		get_flags() { return flags; }
	int		get_sector_tag() { return sector_tag; }
	int		get_special() { return type; }
	BYTE	arg(int index) { if (index >= 0 && index < 5) return args[index]; else return 0; }
	string	get_trigger_string();
	WORD	get_trigger();

	void	set_trigger(WORD trigger);
	void	set_special(int special) { type = special; }
	void	set_sector_tag(int tag) { sector_tag = tag; }
	void	set_arg(int index, BYTE value) { if (index >= 0 && index < 5) args[index] = value; }
	void	set_flags(WORD nflags) { flags = nflags; }
	void	set_side1(Side* s) { s1 = s; }
	void	set_side2(Side* s) { s2 = s; }
	void	set_parent(DoomMap* parent, bool init = true);
	void	set_vertex1(Vertex* v);
	void	set_vertex2(Vertex* v);

	bool	toggle_flag(WORD flag);
	void	set_flag(WORD flag)		{ flags |= flag; }
	void	clear_flag(WORD flag)	{ flags = (flags & ~flag); }
	bool	check_flag(WORD flag)	{ return !!(flags & flag); }

	int		get_index() { return index; }
	void	set_index(int i) { index = i; }

	void	remove_side1();
	void	remove_side2();

	void	flip(bool verts = true, bool sides = true);

	void	set_sector(int side, Sector* sector);

	// Geometry
	int		x1();
	int		y1();
	int		x2();
	int		y2();
	rect_t	get_rect();

	point2_t	get_side_point(bool front = true);

	bool	has_vertex(Vertex* vertex);
	bool	has_side(Side* side);

	int		sector_index(bool front = true);

	// Drawing
	rgba_t	get_colour();
	void	draw(bool side = false, BYTE style = 0);

	// Stuff
	bool	needs_upper(bool front = true);
	bool	needs_lower(bool front = true);
	bool	needs_middle(bool front = true);
	void	apply_default_textures(bool front = true, bool back = true);

	Line*	split(Vertex* vert);
	void	copy(Line* line, bool copy_sides = true);

	doomline_t	to_doomformat();
	hexenline_t	to_hexenformat();
};

#endif
