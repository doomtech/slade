
#ifndef __MAPSECTOR_H__
#define __MAPSECTOR_H__

#include "Tokenizer.h"
#include "PropertyList.h"

class MapSide;

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

struct doom64sector_t
{
	short		f_height;
	short		c_height;
	uint16_t	f_tex;
	uint16_t	c_tex;
	uint16_t	color[5];
	short		special;
	short		tag;
	uint16_t	flags;
};

class MapSector {
friend class SLADEMap;
friend class MapSide;
private:
	// Basic data
	unsigned	index;
	string		f_tex;
	string		c_tex;

	// Internal info
	vector<MapSide*>	connected_sides;

	// Properties
	PropertyList	udmf_props;

public:
	MapSector(){}
	MapSector(string f_tex, string c_tex) { this->f_tex = f_tex; this->c_tex = c_tex; }
	~MapSector(){}

	unsigned	getIndex() { return index; }
	string		floorTexture() { return f_tex; }
	string		ceilingTexture() { return c_tex; }

	PropertyList&	props()				{ return udmf_props; }
	Property&		prop(string key)	{ return udmf_props[key]; }
};

#endif //__MAPSECTOR_H__
