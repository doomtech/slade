
#ifndef __MAPSECTOR_H__
#define __MAPSECTOR_H__

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

public:
	MapSector(){}
	~MapSector(){}
};

#endif //__MAPSECTOR_H__
