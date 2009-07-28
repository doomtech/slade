
#ifndef __MAPSIDE_H__
#define __MAPSIDE_H__

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

class MapSide {
private:
	MapSector*	sector;

public:
	MapSide(){}
	~MapSide(){}
};

#endif //__MAPSIDE_H__
