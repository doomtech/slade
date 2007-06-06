
#include "main.h"
#include "doom_map.h"
#include "dm_thing.h"
#include "game_config.h"
#include "draw.h"
#include "editor_window.h"
#include "input.h"
#include "edit.h"

Thing last_thing;

extern EditorWindow *editor_window;
extern GameConfig game;
extern rgba_t col_hilight, col_selection, col_moving, col_linedraw;

EXTERN_CVAR(Bool, thing_sprites)
EXTERN_CVAR(Bool, thing_force_angle)
EXTERN_CVAR(Bool, line_aa)

Thing::Thing(DoomMap *parent)
{
	this->parent = parent;

	x = 0;
	y = 0;
	type = 1;
	flags = 15;
	angle = 0;
	tid = 0;
	z = 0;
	special = 0;
	memset(args, 0, 5);
	ttype = game.get_ttype(type);
	index = -1;
	sector = -2;

	if (parent)
		parent->add_thing(this);
}

Thing::Thing(doomthing_t t, DoomMap *parent)
{
	this->parent = parent;

	x = wxINT16_SWAP_ON_BE(t.x);
	y = wxINT16_SWAP_ON_BE(t.y);
	type = wxINT16_SWAP_ON_BE(t.type);
	flags = wxINT16_SWAP_ON_BE(t.flags);
	angle = wxINT16_SWAP_ON_BE(t.angle);
	tid = 0;
	z = 0;
	special = 0;
	memset(args, 0, 5);
	ttype = game.get_ttype(type);
	index = -1;
	sector = -2;

	if (parent)
		parent->add_thing(this);
}

Thing::Thing(hexenthing_t t, DoomMap *parent)
{
	this->parent = parent;

	x = wxINT16_SWAP_ON_BE(t.x);
	y = wxINT16_SWAP_ON_BE(t.y);
	type = wxINT16_SWAP_ON_BE(t.type);
	flags = wxINT16_SWAP_ON_BE(t.flags);
	angle = wxINT16_SWAP_ON_BE(t.angle);
	tid = wxINT16_SWAP_ON_BE(t.tid);
	z = wxINT16_SWAP_ON_BE(t.z);
	special = t.special;
	memcpy(args, t.args, 5);
	ttype = game.get_ttype(type);
	index = -1;
	sector = -2;

	if (parent)
		parent->add_thing(this);
}

Thing::~Thing()
{
}

ThingType* Thing::get_ttype()
{
	if (!ttype)
		ttype = game.get_ttype(type);

	return ttype;
}

void Thing::update_ttype()
{
	ttype = game.get_ttype(type);
}

bool Thing::check_flag(int flag)
{
	return !!(flags & flag);
}

void Thing::set_flag(int flag, bool value)
{
	if (value)
		flags |= flag;
	else
		flags = (flags & ~flag);
}

string Thing::angle_string()
{
	if (angle == 0) // east
		return _T("East");
	else if (angle == 45) // northeast
		return _T("Northeast");
	else if (angle == 90) // north
		return _T("North");
	else if (angle == 135) // northwest
		return _T("Northwest");
	else if (angle == 180) // west
		return _T("West");
	else if (angle == 225) // southwest
		return _T("Southwest");
	else if (angle == 270) // south
		return _T("South");
	else if (angle == 315) // southeast
		return _T("Southeast");
	else // Invalid angle
		return _T("Invalid Angle");
}

string Thing::difficulty_string()
{
	if (!(flags & THING_EASY)
		&& !(flags & THING_MEDIUM)
		&& !(flags & THING_HARD))
		return _T("None");

	string str;

	if (flags & THING_EASY)
		str += _T("Easy ");

	if (flags & THING_MEDIUM)
		str += _T("Med ");

	if (flags & THING_HARD)
		str += _T("Hard");

	return str;
}

bool Thing::draw(rect_t vis_area, BYTE style)
{
	// Setup the radius
	int r = ttype->radius;
	if (ttype->radius == -1) r = 8;

	// Don't draw if it's out of the visible area
	rect_t t_rect(x, y, r*2, r*2, RECT_CENTER);
	if (t_rect.x2() < vis_area.x1() || t_rect.x1() > vis_area.x2() || t_rect.y2() < vis_area.y1() || t_rect.y1() > vis_area.y2())
		return false;

	if (style == MISTYLE_NORMAL)
	{
		// Draw thing
		if (thing_sprites)
		{
			Texture* tex = get_texture(ttype->spritename, 3);
			tex->gen_gl_tex();
			float width = tex->width * ttype->x_scale;
			float height = tex->height * ttype->y_scale;

			rgba_t col;
			if (ttype->spritename[0] == '_')
				col.set(ttype->colour);
			else
				col.set(255, 255, 255, 255*ttype->alpha);

			draw_texture(x - (width / 2), y - (height / 2), width, height, tex->name, 3, col);
		}
		else
		{
			string tex;

			if (ttype->spritename[0] == '_')
				tex = ttype->spritename;
			else
				tex = _T("_thing");

			draw_texture(t_rect.x1(), t_rect.y1(), t_rect.width(), t_rect.height(), tex, -1, ttype->colour);
		}

		// Draw the angle (if needed)
		point2_t p(x, y);
		if (ttype->flags & TTYPE_ANGLE || thing_force_angle)
		{
			int x2, y2;

			switch(angle) {
				case 0: // east
					x2 = p.x + r;
					y2 = p.y;
					break;
				case 45: // northeast
					x2 = p.x + (r * 3) / 4;
					y2 = p.y + (r * 3) / 4;
					break;
				case 90: // north
					x2 = p.x;
					y2 = p.y + r;
					break;
				case 135: // northwest
					x2 = p.x - (r * 3) / 4;
					y2 = p.y + (r * 3) / 4;
					break;
				case 180: // west
					x2 = p.x - r;
					y2 = p.y;
					break;
				case 225: // southwest
					x2 = p.x - (r * 3) / 4;
					y2 = p.y - (r * 3) / 4;
					break;
				case 270: // south	
					x2 = p.x;
					y2 = p.y - r;
					break;
				case 315: // southeast
					x2 = p.x + (r * 3) / 4;
					y2 = p.y - (r * 3) / 4;
					break;
				default: // Invalid angle
					x2 = p.x;
					y2 = p.y;
					break;
			}

			glEnable(GL_POINT_SMOOTH);

			if (thing_sprites)
			{
				double zoom = editor_window->map()->get_zoom();
				draw_point(p.x, p.y, 8 * zoom, rgba_t(255, 255, 255, 150));
				draw_line(rect_t(p.x, p.y, x2, y2), rgba_t(255, 255, 255, 150), line_aa, false, 2.0f);
			}
			else
				draw_line(rect_t(p.x, p.y, x2, y2), rgba_t(0, 0, 0, 200), line_aa, false, 2.0f);

			glLineWidth(1.0f);
		}
	}

	if (style == MISTYLE_HILIGHTED)
	{
		draw_rect(rect_t(x, y, r*2, r*2, RECT_CENTER), col_hilight, true);

		if (state(STATE_THINGANGLE))
			draw_line(rect_t(pos(), mouse_pos(true)), col_linedraw, line_aa, false, 1.0f);
	}

	if (style == MISTYLE_SELECTED)
	{
		r += 4;
		draw_rect(rect_t(x, y, r*2, r*2, RECT_CENTER), col_selection, true);

		if (state(STATE_THINGANGLE))
			draw_line(rect_t(pos(), mouse_pos(true)), col_linedraw, line_aa, false, 1.0f);
	}

	if (style == MISTYLE_MOVING)
	{
		r += 4;
		draw_rect(rect_t(x, y, r*2, r*2, RECT_CENTER), col_moving, true);
	}

	return true;
}

doomthing_t	Thing::to_doomformat()
{
	doomthing_t ret;

	ret.x = wxINT16_SWAP_ON_BE((short)x);
	ret.y = wxINT16_SWAP_ON_BE((short)y);
	ret.angle = wxINT16_SWAP_ON_BE((short)angle);
	ret.flags = wxINT16_SWAP_ON_BE((short)flags);
	ret.type = wxINT16_SWAP_ON_BE((short)type);

	return ret;
}

hexenthing_t Thing::to_hexenformat()
{
	hexenthing_t ret;

	ret.x = wxINT16_SWAP_ON_BE((short)x);
	ret.y = wxINT16_SWAP_ON_BE((short)y);
	ret.z = wxINT16_SWAP_ON_BE((short)z);
	ret.tid = wxINT16_SWAP_ON_BE((short)tid);
	ret.angle = wxINT16_SWAP_ON_BE((short)angle);
	ret.flags = wxINT16_SWAP_ON_BE((short)flags);
	ret.special = (BYTE)special;
	ret.type = wxINT16_SWAP_ON_BE((short)type);
	
	for (int a = 0; a < 5; a++)
		ret.args[a] = args[a];

	return ret;
}

int Thing::get_sector(bool check)
{
	if (sector == -2 && parent && check)
		sector = parent->get_hilight_sector(point2_t(x, y));

	return sector;
}

void Thing::move(int xd, int yd)
{
	x += xd;
	y += yd;

	if (parent)
	{
		parent->change_level(MC_THINGS);
		sector = parent->get_hilight_sector(point2_t(x, y));
	}
}

void Thing::set_pos(int x, int y)
{
	this->x = x;
	this->y = y;

	if (parent)
	{
		parent->change_level(MC_THINGS);
		sector = parent->get_hilight_sector(point2_t(x, y));
	}
}

void Thing::set_type(int type)
{
	this->type = type;
	ttype = game.get_ttype(type);
}

void Thing::set_angle(int angle)
{
	if (angle > 315)
		angle = 0;
	if (angle < 0)
		angle = 315;

	this->angle = angle;
}

void Thing::set_sector(int sec)
{
	sector = sec;
}

void Thing::copy(Thing *t)
{
	//this->parent = t->get_parent();

	x = t->pos().x;
	y = t->pos().y;
	type = t->get_type();
	flags = t->get_flags();
	angle = t->get_angle();
	tid = t->get_tid();
	z = t->z_pos();
	special = t->get_special();
	ttype = game.get_ttype(type);
	index = -1;
	sector = t->sector;

	for (int a = 0; a < 5; a++)
		args[a] = t->arg(a);
}
