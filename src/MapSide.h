
#ifndef __MAPSIDE_H__
#define __MAPSIDE_H__

#include "Tokenizer.h"
#include "PropertyList.h"

class MapSector;

struct doomside_t
{
	short	x_offset;
	short	y_offset;
	char	tex_upper[8];
	char	tex_lower[8];
	char	tex_middle[8];
	short	sector;
};

struct doom64side_t
{
	short		x_offset;
	short		y_offset;
	uint16_t	tex_upper;
	uint16_t	tex_lower;
	uint16_t	tex_middle;
	short		sector;
};

class MapSide {
private:
	MapSector*	sector;

	PropertyList	udmf_props;

public:
	MapSide(){}
	MapSide(MapSector* sector);
	~MapSide(){}

	bool	isOk() { return !!sector; }

	PropertyList&	props()				{ return udmf_props; }
	Property&		prop(string key)	{ return udmf_props[key]; }

	bool parseUDMF(Tokenizer& tz);
};

#endif //__MAPSIDE_H__
