
#ifndef __DM_THING_H__
#define __DM_THING_H__

#include "thing_type.h"

#define THING_EASY		0x0001
#define THING_MEDIUM	0x0002
#define THING_HARD		0x0004
#define THING_DEAF		0x0008

class DoomMap;

struct doomthing_t
{
	short	x;
	short	y;
	short	angle;
	short	type;
	short	flags;
};

struct hexenthing_t
{
	WORD	tid;
	short	x;
	short	y;
	short	z;
	short	angle;
	short	type;
	short	flags;
	BYTE	special;
	BYTE	args[5];
};

class Thing
{
private:
	DoomMap	*parent;

	int		x;
	int		y;
	int		type;
	int		flags;
	int		angle;
	int		tid;
	int		z;
	BYTE	special;
	BYTE	args[5];

	ThingType	*ttype;
	int			sector;
	props_t		ex_props;

	int		index;

public:
	Thing(DoomMap *parent = NULL);
	Thing(doomthing_t t, DoomMap *parent);
	Thing(hexenthing_t t, DoomMap *parent);
	~Thing();

	DoomMap*	get_parent() { return parent; }
	int			get_type() { return type; }
	int			get_angle() { return angle; }
	int			get_tid() { return tid; }
	point2_t	pos() { return point2_t(x, y); }
	int			z_pos() { return z; }
	BYTE		get_special() { return special; }
	BYTE		arg(int index) { if (index >= 0 && index < 5) return args[index]; else return 0; }
	ThingType	*get_ttype();
	bool		check_flag(int flag);
	string		angle_string();
	string		difficulty_string();
	int			get_flags() { return flags; }
	int			get_sector(bool check = true);

	void		set_flag(int flag, bool value);
	void		set_type(int type);
	void		set_z(int z) { this->z = z; }
	void		set_angle(int angle);
	void		set_tid(int tid) { this->tid = tid; }
	void		set_special(BYTE special) { this->special = special; }
	void		set_arg(int index, BYTE value) { if (index >= 0 && index < 5) args[index] = value; }
	void		set_flags(int nflags) { flags = nflags; }
	void		set_parent(DoomMap* parent) { this->parent = parent; }
	void		set_sector(int sec);

	int		get_index() { return index; }
	void	set_index(int i) { index = i; }

	void	move(int xd, int yd);
	void	set_pos(int x, int y);

	void	update_ttype();

	bool draw(rect_t vis_area, BYTE style = 0);

	doomthing_t		to_doomformat();
	hexenthing_t	to_hexenformat();

	void copy(Thing* t);
};

#endif
