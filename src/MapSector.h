
#ifndef __MAPSECTOR_H__
#define __MAPSECTOR_H__

class MapSide;

class MapSector
{
private:
	vector<MapSide*>	connected_sides;

public:
	MapSector();
	~MapSector();
};

#endif //__MAPSECTOR_H__
