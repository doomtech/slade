
#include "main.h"
#include "doom_map.h"
#include "dm_thing.h"
#include "game_config.h"
#include "draw.h"
#include "editor_window.h"

Thing last_thing;

extern EditorWindow *editor_window;
extern GameConfig game;
extern rgba_t col_hilight, col_selection, col_moving;

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

	if (parent)
		parent->add_thing(this);
}

Thing::Thing(doomthing_t t, DoomMap *parent)
{
	this->parent = parent;

	x = t.x;
	y = t.y;
	type = t.type;
	flags = t.flags;
	angle = t.angle;
	tid = 0;
	z = 0;
	special = 0;
	memset(args, 0, 5);
	ttype = game.get_ttype(type);
	index = -1;

	if (parent)
		parent->add_thing(this);
}

Thing::Thing(hexenthing_t t, DoomMap *parent)
{
	this->parent = parent;

	x = t.x;
	y = t.y;
	type = t.type;
	flags = t.flags;
	angle = t.angle;
	tid = t.tid;
	z = t.z;
	special = t.special;
	memcpy(args, t.args, 5);
	ttype = game.get_ttype(type);
	index = -1;

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
	return (flags & flag);
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
		return "East";
	else if (angle == 45) // northeast
		return "Northeast";
	else if (angle == 90) // north
		return "North";
	else if (angle == 135) // northwest
		return "Northwest";
	else if (angle == 180) // west
		return "West";
	else if (angle == 225) // southwest
		return "Southwest";
	else if (angle == 270) // south
		return "South";
	else if (angle == 315) // southeast
		return "Southeast";
	else // Invalid angle
		return "Invalid Angle";
}

string Thing::difficulty_string()
{
	if (!(flags & THING_EASY)
		&& !(flags & THING_MEDIUM)
		&& !(flags & THING_HARD))
		return "None";

	string str;

	if (flags & THING_EASY)
		str += "Easy ";

	if (flags & THING_MEDIUM)
		str += "Med ";

	if (flags & THING_HARD)
		str += "Hard";

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
				tex = "_thing";

			draw_texture(t_rect.x1(), t_rect.y1(), t_rect.width(), t_rect.height(), tex, -1, ttype->colour);
		}

		// Draw the angle (if needed)
		point2_t p(x, y);
		if (ttype->flags & TTYPE_ANGLE || thing_force_angle)
		{
			int x2, y2;

			// east
			if (angle == 0)			{ x2 = p.x + r; y2 = p.y; }
			// northeast
			else if (angle == 45)	{ x2 = p.x + (r*0.75); y2 = p.y + (r*0.75); }
			// north
			else if (angle == 90)	{ x2 = p.x; y2 = p.y + r; }
			// northwest
			else if (angle == 135)	{ x2 = p.x - (r*0.75); y2 = p.y + (r*0.75); }
			// west
			else if (angle == 180)	{ x2 = p.x - r; y2 = p.y; }
			// southwest
			else if (angle == 225)	{ x2 = p.x - (r*0.75); y2 = p.y - (r*0.75); }
			// south
			else if (angle == 270)	{ x2 = p.x; y2 = p.y - r; }
			// southeast
			else if (angle == 315)	{ x2 = p.x + (r*0.75); y2 = p.y - (r*0.75); }
			// Invalid angle
			else	{ x2 = p.x; y2 = p.y; }

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
		draw_rect(rect_t(x, y, r*2, r*2, RECT_CENTER), col_hilight, true);

	if (style == MISTYLE_SELECTED)
	{
		r += 4;
		draw_rect(rect_t(x, y, r*2, r*2, RECT_CENTER), col_selection, true);
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

	ret.x = (short)x;
	ret.y = (short)y;
	ret.angle = (short)angle;
	ret.flags = (short)flags;
	ret.type = (short)type;

	return ret;
}

hexenthing_t Thing::to_hexenformat()
{
	hexenthing_t ret;

	ret.x = (short)x;
	ret.y = (short)y;
	ret.z = (short)z;
	ret.tid = (short)tid;
	ret.angle = (short)angle;
	ret.flags = (short)flags;
	ret.special = (BYTE)special;
	ret.type = (short)type;
	
	for (int a = 0; a < 5; a++)
		ret.args[a] = args[a];

	return ret;
}

void Thing::move(int xd, int yd)
{
	x += xd;
	y += yd;

	if (parent)
		parent->change_level(MC_THINGS);
}

void Thing::copy(Thing *t)
{
	this->parent = t->get_parent();

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

	for (int a = 0; a < 5; a++)
		args[a] = t->arg(a);
}
