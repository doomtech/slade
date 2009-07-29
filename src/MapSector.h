
#ifndef __MAPSECTOR_H__
#define __MAPSECTOR_H__

#include "Tokenizer.h"

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

class MapSector {
private:
	vector<MapSide*>	connected_sides;

	PropertyList	udmf_props;

public:
	MapSector(){}
	MapSector(doomsector_t s);
	~MapSector(){}

	bool parseUDMF(Tokenizer& tz);
};

#endif //__MAPSECTOR_H__
