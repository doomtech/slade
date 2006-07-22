
#include "main.h"
#include "doom_map.h"
#include "dm_vertex.h"
#include "draw.h"

extern rgba_t col_vertex, col_selection, col_hilight, col_moving;
extern BYTE hilight_style;

Vertex::Vertex(DoomMap *parent)
{
	this->parent = parent;
	x = 0;
	y = 0;
	ref_count = 0;
	index = -1;

	if (parent)
		parent->add_vertex(this);
}

Vertex::Vertex(int x, int y, DoomMap *parent)
{
	this->parent = parent;
	this->x = x;
	this->y = y;
	ref_count = 0;
	index = -1;

	if (parent)
		parent->add_vertex(this);
}

Vertex::Vertex(doomvertex_t v, DoomMap *parent)
{
	this->parent = parent;
	this->x = v.x;
	this->y = v.y;
	ref_count = 0;
	index = -1;

	if (parent)
		parent->add_vertex(this);
}

Vertex::~Vertex()
{
}

point2_t Vertex::pos()
{
	return point2_t(x, y);
}

string Vertex::ex_prop(string p)
{
	return ex_props[p];
}

void Vertex::add_ref()
{
	ref_count++;
}

bool Vertex::rem_ref()
{
	ref_count--;
	return (ref_count == 0);
}

void Vertex::draw(BYTE style)
{
	if (style == MISTYLE_NORMAL)
		draw_point(x, y, 6, col_vertex);

	int size = 8;
	if (hilight_style == 1) size = 6;
	if (style == MISTYLE_HILIGHTED)
		draw_point(x, y, size, col_hilight);

	size = 10;
	if (hilight_style == 1) size = 6;
	if (style == MISTYLE_SELECTED)
		draw_point(x, y, size, col_selection);

	if (hilight_style == 1) size = 6;
	if (style == MISTYLE_MOVING)
		draw_point(x, y, size, col_moving);
}

doomvertex_t Vertex::to_doomformat()
{
	doomvertex_t ret;

	ret.x = (short)x;
	ret.y = (short)y;

	return ret;
}

void Vertex::move(int xd, int yd)
{
	x += xd;
	y += yd;

	if (parent)
		parent->change_level(MC_NODE_REBUILD);
}
