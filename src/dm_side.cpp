
#include "main.h"
#include "doom_map.h"
#include "dm_side.h"
#include "textures.h"
//#include "game_config.h"

//string def_midtex = "STONE2";
//string def_uptex = "STONE2";
//string def_lotex = "STONE2";
//extern GameConfig game;

Side::Side(DoomMap *parent)
{
	this->parent = parent;

	tx_upper = NULL;
	tx_middle = NULL;
	tx_lower = NULL;
	x_offset = 0;
	y_offset = 0;
	sector = NULL;
	index = -1;

	tex_middle = "-";
	tex_upper = "-";
	tex_lower = "-";

	if (parent)
		parent->add_side(this);
}

Side::Side(doomside_t s, DoomMap *parent)
{
	tx_upper = NULL;
	tx_middle = NULL;
	tx_lower = NULL;
	index = -1;

	Texture *tex = NULL;

	this->parent = parent;

	this->x_offset = s.x_offset;
	this->y_offset = s.y_offset;

	char temp[9] = "";
	temp[8] = 0;

	memcpy(temp, s.tex_lower, 8);
	this->tex_lower = strupr(temp);
	tex = get_texture(this->tex_lower);
	if (tex) tex->use_count++;

	memcpy(temp, s.tex_middle, 8);
	this->tex_middle = strupr(temp);
	tex = get_texture(this->tex_middle);
	if (tex) tex->use_count++;

	memcpy(temp, s.tex_upper, 8);
	this->tex_upper = strupr(temp);
	tex = get_texture(this->tex_upper);
	if (tex) tex->use_count++;

	if (parent)
	{
		parent->add_side(this);
		sector = parent->sector(s.sector);
		if (!parent->valid(sector))
			log_message(s_fmt("Invalid sector %d", s.sector));
	}
}

Side::~Side()
{
	Texture* tex = NULL;

	tex = get_texture(this->tex_lower, 1);
	if (tex) tex->use_count--;
	tex = NULL;

	tex = get_texture(this->tex_middle, 1);
	if (tex) tex->use_count--;
	tex = NULL;

	tex = get_texture(this->tex_upper, 1);
	if (tex) tex->use_count--;
	tex = NULL;
}

Sector* Side::get_sector()
{
	return sector;
}

string Side::get_texname(BYTE part)
{
	if (part == TEX_UPPER)
		return tex_upper;
	if (part == TEX_LOWER)
		return tex_lower;

	return tex_middle;
}

Texture* Side::get_tex(BYTE part)
{
	if (part == TEX_UPPER)
	{
		if (!tx_upper)
			tx_upper = get_texture(tex_upper, 1);
		
		return tx_upper;
	}

	if (part == TEX_LOWER)
	{
		if (!tx_lower)
			tx_lower = get_texture(tex_lower, 1);
		
		return tx_lower;
	}

	if (!tx_middle)
		tx_middle = get_texture(tex_middle, 1);

	return tx_middle;
}

doomside_t Side::to_doomformat()
{
	doomside_t ret;

	ret.sector = short(parent->index(sector));
	ret.x_offset = (short)x_offset;
	ret.y_offset = (short)y_offset;

	for (int a = 0; a < 8; a++)
	{
		ret.tex_lower[a] = 0;
		ret.tex_middle[a] = 0;
		ret.tex_upper[a] = 0;
	}

	for (int a = 0; a < tex_lower.size(); a++)
		ret.tex_lower[a] = tex_lower[a];

	for (int a = 0; a < tex_middle.size(); a++)
		ret.tex_middle[a] = tex_middle[a];

	for (int a = 0; a < tex_upper.size(); a++)
		ret.tex_upper[a] = tex_upper[a];

	return ret;
}

void Side::set_xoff(int xoff)
{
	x_offset = xoff;
}

void Side::set_yoff(int yoff)
{
	y_offset = yoff;
}

void Side::set_texture(string tex, BYTE part)
{
	if (part == TEX_UPPER)
	{
		Texture* t = get_texture(tex_upper, 1);
		if (t) t->use_count--;
		t = get_texture(tex, 1);
		if (t) t->use_count++;
		tex_upper = tex;
		tx_upper = NULL;
	}

	if (part == TEX_MIDDLE)
	{
		Texture* t = get_texture(tex_middle, 1);
		if (t) t->use_count--;
		t = get_texture(tex, 1);
		if (t) t->use_count++;
		tex_middle = tex;
		tx_middle = NULL;
	}

	if (part == TEX_LOWER)
	{
		Texture* t = get_texture(tex_lower, 1);
		if (t) t->use_count--;
		t = get_texture(tex, 1);
		if (t) t->use_count++;
		tex_lower = tex;
		tx_lower = NULL;
	}
}

void Side::set_sector(int sector)
{
	if (!parent)
		return;

	if (parent->valid(parent->sector(sector)))
		this->sector = parent->sector(sector);
}

void Side::copy(Side* side)
{
	index = -1;

	set_texture(side->get_texname(0), 0);
	set_texture(side->get_texname(1), 1);
	set_texture(side->get_texname(2), 2);

	set_xoff(side->get_xoff());
	set_yoff(side->get_yoff());

	sector = side->get_sector();
}
