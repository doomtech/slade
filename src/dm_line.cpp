
#include "main.h"
#include "doom_map.h"
#include "dm_line.h"
#include "dm_vertex.h"
#include "dm_side.h"
#include "dm_sector.h"
#include "draw.h"
#include "game_config.h"

extern rgba_t col_hilight;
extern rgba_t col_selection;
extern rgba_t col_tagged;
extern rgba_t col_moving;
extern rgba_t col_line_solid;
extern rgba_t col_line_2s;
extern rgba_t col_line_monster;
extern rgba_t col_line_nofirst;
extern rgba_t col_line_special;
extern float hilight_size, selection_size, moving_size;
extern GameConfig game;

EXTERN_CVAR(Bool, line_aa)
EXTERN_CVAR(Float, line_size)

Line::Line(DoomMap *parent)
{
	this->parent = parent;

	v1 = NULL;
	v2 = NULL;
	s1 = NULL;
	s2 = NULL;

	flags = 1;
	type = 0;
	sector_tag = 0;

	for (int a = 0; a < 5; a++)
		args[a] = 0;

	if (parent)
	{
		parent->add_line(this);
		v1 = parent->vertex(-1);
		v2 = parent->vertex(-1);
		s1 = parent->side(-1);
		s2 = parent->side(-1);
	}

	index = -1;
}

Line::Line(Vertex* v1, Vertex* v2, DoomMap* parent)
{
	this->parent = parent;

	this->v1 = v1;
	this->v2 = v2;
	v1->add_ref();
	v2->add_ref();
	s1 = NULL;
	s2 = NULL;

	flags = 1;
	type = 0;
	sector_tag = 0;

	for (int a = 0; a < 5; a++)
		args[a] = 0;

	if (parent)
	{
		parent->add_line(this);
		s1 = parent->side(-1);
		s2 = parent->side(-1);
	}

	index = -1;
}

Line::Line(doomline_t l, DoomMap *parent, bool &ok)
{
	this->parent = parent;

	flags = wxINT16_SWAP_ON_BE(l.flags);
	type = wxINT16_SWAP_ON_BE(l.type);
	sector_tag = wxINT16_SWAP_ON_BE(l.sector_tag);

	v1 = NULL;
	v2 = NULL;
	s1 = NULL;
	s2 = NULL;
	index = -1;

	if (!parent)
		return;

	// Vertices
	v1 = parent->vertex(wxINT16_SWAP_ON_BE(l.vertex1));
	v2 = parent->vertex(wxINT16_SWAP_ON_BE(l.vertex2));

	if (!parent->valid(v1))
	{
		log_message(s_fmt(_T("Invalid first vertex %d"), l.vertex1));
		ok = false;
	}
	else
		v1->add_ref();

	if (!parent->valid(v2))
	{
		log_message(s_fmt(_T("Invalid second vertex %d"), l.vertex2));
		ok = false;
	}
	else
		v2->add_ref();

	// Sides
	int side1 = wxINT16_SWAP_ON_BE(l.side1);
	int side2 = wxINT16_SWAP_ON_BE(l.side2);

	if (parent->n_sides() > 32767)
	{
		side1 = wxINT16_SWAP_ON_BE(static_cast<unsigned short>(l.side1));
		side2 = wxINT16_SWAP_ON_BE(static_cast<unsigned short>(l.side2));
	}

	if ((side1 < -1 || side1 > parent->n_sides()) && side1 != 65535)
	{
		log_message(s_fmt(_T("Invalid first side %d"), side1));
		s1 = parent->side(-1);
	}
	else
		s1 = parent->side(side1);

	if ((side2 < -1 || side2 > parent->n_sides()) && side2 != 65535)
	{
		log_message(s_fmt(_T("Invalid second side %d"), side2));
		s2 = parent->side(-1);
	}
	else
		s2 = parent->side(side2);

	//sector1 = parent->index(s1->get_sector());
	//sector2 = parent->index(s2->get_sector());

	if (parent && ok)
		parent->add_line(this);
}

Line::Line(hexenline_t l, DoomMap *parent, bool &ok)
{
	this->parent = parent;

	flags = wxINT16_SWAP_ON_BE(l.flags);
	type = l.type;
	memcpy(args, l.args, 5);

	v1 = NULL;
	v2 = NULL;
	s1 = NULL;
	s2 = NULL;
	index = -1;

	if (!parent)
		return;

	// Vertices
	v1 = parent->vertex(wxINT16_SWAP_ON_BE(l.vertex1));
	v2 = parent->vertex(wxINT16_SWAP_ON_BE(l.vertex2));

	if (!parent->valid(v1))
	{
		log_message(s_fmt(_T("Invalid first vertex %d"), l.vertex1));
		ok = false;
	}
	else
		v1->add_ref();

	if (!parent->valid(v2))
	{
		log_message(s_fmt(_T("Invalid second vertex %d"), l.vertex2));
		ok = false;
	}
	else
		v2->add_ref();

	// Sides
	int side1 = l.side1;
	int side2 = l.side2;

	if (parent->n_sides() > 32767)
	{
		side1 = wxINT16_SWAP_ON_BE(static_cast<unsigned short>(l.side1));
		side2 = wxINT16_SWAP_ON_BE(static_cast<unsigned short>(l.side2));
	}

	if ((side1 < -1 || side1 > parent->n_sides()) && side1 != 65535)
	{
		log_message(s_fmt(_T("Invalid first side %d"), side1));
		s1 = parent->side(-1);
	}
	else
		s1 = parent->side(side1);

	if ((side2 < -1 || side2 > parent->n_sides()) && side2 != 65535)
	{
		log_message(s_fmt(_T("Invalid second side %d"), side2));
		s2 = parent->side(-1);
	}
	else
		s2 = parent->side(side2);

	//sector1 = parent->index(s1->get_sector());
	//sector2 = parent->index(s2->get_sector());

	if (parent && ok)
		parent->add_line(this);
}

Line::~Line()
{
}

int	Line::x1()
{
	if (v1)
		return v1->x_pos();
	else
		return 0;
}

int	Line::y1()
{
	if (v1)
		return v1->y_pos();
	else
		return 0;
}

int Line::x2()
{
	if (v2)
		return v2->x_pos();
	else
		return 0;
}

int Line::y2()
{
	if (v2)
		return v2->y_pos();
	else
		return 0;
}

WORD Line::get_trigger()
{
	return ((flags & 0x1c00) >> 10);
}

string Line::get_trigger_string()
{
	switch(get_trigger()) {
		case 0:
			return _T("Player Crosses Line");
		case 1:
			return _T("Player Uses Line");
		case 2:
			return _T("Monster Crosses Line");
		case 3:
			return _T("Projectile Hits Line");
		case 4:
			return _T("Player Bumps Line");
		case 5:
			return _T("Projectile Crosses Line");
		case 6:
			return _T("Player Uses Line (PassThru)");
		case 7:
			return _T("Projectile Hits OR Crosses Line");
		default:
			return _T("Unknown");
	}
}

rect_t Line::get_rect()
{
	return rect_t(x1(), y1(), x2(), y2());
}

rgba_t Line::get_colour()
{
	if (!parent)
		return rgba_t(col_line_nofirst);

	rgba_t ret;

	if (flags & LINE_IMPASSIBLE || !parent->valid(s2))
		ret.set(col_line_solid);
	else
	{
		ret.set(col_line_2s);

		if (flags & LINE_BLOCKMONSTERS)
			ret.set(col_line_monster);
	}

	if (type != 0)
	{
		rgba_t spec2s(col_line_special.r * 0.8, col_line_special.g * 0.8,
					col_line_special.b * 0.8, col_line_special.a, col_line_special.blend);

		if (flags & LINE_TWOSIDED)
			ret.set(spec2s);
		else
			ret.set(col_line_special);
	}

	if (flags & LINE_TWOSIDED && !parent->valid(s2))
		ret.set(120, 0, 0, 255);

	if (!parent->valid(s1))
		ret.set(col_line_nofirst);

	return ret;
}

void Line::set_trigger(WORD trigger)
{
	trigger = trigger << 10;

	flags &= ~0x1c00;
	flags |= trigger;
}

bool Line::toggle_flag(WORD flag)
{
	flags ^= flag;

	return !!(flags & flag);
}

void Line::draw(bool side, BYTE style)
{
	if (style == MISTYLE_NORMAL)
		draw_line(get_rect(), get_colour(), line_aa, side, line_size);

	if (style == MISTYLE_HILIGHTED)
		draw_line(get_rect(), col_hilight, line_aa, side, hilight_size);

	if (style == MISTYLE_SELECTED)
		draw_line(get_rect(), col_selection, line_aa, side, selection_size);

	if (style == MISTYLE_TAGGED)
		draw_line(get_rect(), col_tagged, line_aa, side, hilight_size);

	if (style == MISTYLE_MOVING)
		draw_line(get_rect(), col_moving, line_aa, side, moving_size);

	//point2_t sp = get_side_point(true);
	//draw_point(sp.x, sp.y, 4, COL_WHITE);
}

bool Line::has_vertex(Vertex *vertex)
{
	return (v1 == vertex || v2 == vertex);
}

bool Line::has_side(Side *side)
{
	return (s1 == side || s2 == side);
}

int Line::sector_index(bool front)
{
	if (!parent)
		return -1;

	if (front && !s1)
		return -1;

	if (!front && !s2)
		return -1;

	if (front)
		return parent->index(s1->get_sector());
	else
		return parent->index(s2->get_sector());
}

bool Line::needs_upper(bool front)
{
	if (!parent)
		return false;

	int sector1 = sector_index(true);
	int sector2 = sector_index(false);

	// False if not two-sided
	if (sector1 == -1 || sector2 == -1)
		return false;

	if (front)
	{
		if (parent->sector(sector1)->ceiling() > parent->sector(sector2)->ceiling())
			return true;
		else
			return false;
	}
	else
	{
		if (parent->sector(sector2)->ceiling() > parent->sector(sector1)->ceiling())
			return true;
		else
			return false;
	}

	return false;
}

bool Line::needs_lower(bool front)
{
	if (!parent)
		return false;

	int sector1 = sector_index(true);
	int sector2 = sector_index(false);

	// False if not two-sided
	if (sector1 == -1 || sector2 == -1)
		return false;

	if (front)
	{
		if (parent->sector(sector2)->floor() > parent->sector(sector1)->floor())
			return true;
		else
			return false;
	}
	else
	{
		if (parent->sector(sector1)->floor() > parent->sector(sector2)->floor())
			return true;
		else
			return false;
	}

	return false;
}

bool Line::needs_middle(bool front)
{
	if (!parent)
		return false;

	int side1 = parent->index(s1);
	int side2 = parent->index(s2);

	if (front)
	{
		if (side2 == -1)
			return true;
		else
		{
			if (parent->side(side1)->get_texname(TEX_MIDDLE) == _T("-"))
				return false;
			else
				return true;
		}
	}
	else if (side1 != -1)
	{
		if (parent->side(side2)->get_texname(TEX_MIDDLE) == _T("-"))
			return false;
		else
			return true;
	}

	return false;
}

doomline_t Line::to_doomformat()
{
	doomline_t ret;

	ret.flags = wxINT16_SWAP_ON_BE((short)flags);
	ret.sector_tag = wxINT16_SWAP_ON_BE((short)sector_tag);
	ret.type = wxINT16_SWAP_ON_BE((short)type);
	ret.vertex1 = wxINT16_SWAP_ON_BE((short)parent->index(v1));
	ret.vertex2 = wxINT16_SWAP_ON_BE((short)parent->index(v2));

	int side1 = parent->index(s1);
	int side2 = parent->index(s2);

	ret.side1 = (short)side1;
	ret.side2 = (short)side2;

	if (parent)
	{
		if (parent->n_sides() > 32767)
		{
			WORD temp = side1;
			ret.side1 = static_cast<short>(temp);
			temp = side2;
			ret.side2 = static_cast<short>(temp);
		}
	}

	ret.side1 = wxINT16_SWAP_ON_BE(ret.side1);
	ret.side2 = wxINT16_SWAP_ON_BE(ret.side2);

	return ret;
}

hexenline_t Line::to_hexenformat()
{
	hexenline_t hl;

	hl.flags = wxINT16_SWAP_ON_BE((short)flags);
	hl.type = (BYTE)type;
	hl.vertex1 = wxINT16_SWAP_ON_BE((short)parent->index(v1));
	hl.vertex2 = wxINT16_SWAP_ON_BE((short)parent->index(v2));

	for (int a = 0; a < 5; a++)
		hl.args[a] = args[a];

	int side1 = parent->index(s1);
	int side2 = parent->index(s2);

	hl.side1 = (short)side1;
	hl.side2 = (short)side2;

	if (parent)
	{
		if (parent->n_sides() > 32767)
		{
			WORD temp = side1;
			hl.side1 = static_cast<short>(temp);
			temp = side2;
			hl.side2 = static_cast<short>(temp);
		}
	}

	hl.side1 = wxINT16_SWAP_ON_BE(hl.side1);
	hl.side2 = wxINT16_SWAP_ON_BE(hl.side2);

	return hl;
}

void Line::remove_side1()
{
	// If no parent, simply delete the side
	if (!parent)
	{
		delete s1;
		s1 = NULL;
		return;
	}

	if (!parent->valid(s1))
		return;

	//parent->delete_side(s1);
	s1 = parent->side(-1);

	if (parent->valid(s2))
	{
		flip();

		// Set impassable and unset two-sided
		set_flag(1);
		clear_flag(4);
	}
}

void Line::remove_side2()
{
	// If no parent, simply delete the side
	if (!parent)
	{
		delete s2;
		s2 = NULL;
		return;
	}

	if (!parent->valid(s2))
		return;

	//parent->delete_side(s2);
	s2 = parent->side(-1);
	set_flag(1);
	clear_flag(4);
}

void Line::flip(bool verts, bool sides)
{
	if (verts)
	{
		Vertex *v = v1;
		v1 = v2;
		v2 = v;
	}

	if (sides)
	{
		Side *s = s1;
		s1 = s2;
		s2 = s;
	}

	if (parent)
		parent->change_level(MC_NODE_REBUILD);
}

void Line::set_sector(int side, Sector* sector)
{
	if (!parent)
		return;

	if (side == 1 && s1)
	{
		if (!parent->valid(sector))
		{
			remove_side1();
			return;
		}

		if (!parent->valid(s1))
		{
			s1 = new Side(parent);
			apply_default_textures(true, false);
		}

		s1->set_sector(sector);
	}
	else if (s2)
	{
		if (!parent->valid(sector))
		{
			remove_side2();
			return;
		}

		if (!parent->valid(s2))
		{
			if (parent->valid(s1))
			{
				s2 = new Side(parent);
				apply_default_textures(false, true);

				set_flag(4);
				clear_flag(1);

				if (parent->valid(s1))
					s1->set_texture(_T("-"));
			}
			else
			{
				s1 = new Side(parent);
				set_flag(1);
				clear_flag(4);
				flip(true, false);
				apply_default_textures(true, false);
				s1->set_sector(sector);
				return;
			}
		}

		s2->set_sector(sector);
		//s2->set_texture("-");
	}
}

void Line::apply_default_textures(bool front, bool back)
{
	if (front)
	{
		if (needs_upper())
			s1->set_texture(game.def_uptex, TEX_UPPER);

		if (needs_lower())
			s1->set_texture(game.def_lotex, TEX_LOWER);

		if (needs_middle())
			s1->set_texture(game.def_midtex, TEX_MIDDLE);
	}

	if (back)
	{
		if (needs_upper(false))
			s2->set_texture(game.def_uptex, TEX_UPPER);

		if (needs_lower(false))
			s2->set_texture(game.def_lotex, TEX_LOWER);

		if (needs_middle(false))
			s2->set_texture(game.def_midtex, TEX_MIDDLE);
	}
}

point2_t Line::get_side_point(bool front)
{
	rect_t linerect = get_rect();
	point2_t mid = linerect.middle();
	point2_t vec(linerect.x2() - linerect.x1(), linerect.y2() - linerect.y1());
	
	int x = 0;
	int y = 0;

	if (vec.y < 0)
		x = -1;
	if (vec.y > 0)
		x = 1;

	if (vec.x < 0)
		y = -1;
	if (vec.x > 0)
		y = 1;

	x = -x;

	point2_t side;

	if (front)
		side.set(mid.x - x, mid.y - y);
	else
		side.set(mid.x + x, mid.y + y);

	return side;
}

Line* Line::split(Vertex* vert)
{
	if (parent)
	{
		if (!parent->valid(vert))
			return this;
	}

	Line* l = new Line(vert, v2, parent);
	v2->rem_ref();
	v2 = vert;
	v2->add_ref();
	l->copy(this);
	return l;
}

void Line::copy(Line *line)
{
	flags = line->get_flags();
	type = line->get_special();
	sector_tag = line->get_sector_tag();
	index = -1;

	for (int a = 0; a < 5; a++)
		args[a] = line->arg(a);

	if (parent)
	{
		if (parent->valid(line->side1()))
		{
			if (!parent->valid(s1))
				s1 = new Side(parent);

			s1->copy(line->side1());
		}

		if (parent->valid(line->side2()))
		{
			if (!parent->valid(s2))
				s2 = new Side(parent);

			s2->copy(line->side2());
		}
	}
}

void Line::set_vertex1(Vertex* v)
{
	if (parent)
	{
		if (!parent->valid(v))
			return;
	}

	v1->rem_ref();
	v1 = v;
	v1->add_ref();
}

void Line::set_vertex2(Vertex* v)
{
	if (parent)
	{
		if (!parent->valid(v))
			return;
	}

	v2->rem_ref();
	v2 = v;
	v2->add_ref();
}
