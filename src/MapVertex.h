
#ifndef __MAPVERTEX_H__
#define __MAPVERTEX_H__

class MapLine;

class MapVertex
{
private:
	double	x;
	double	y;

	vector<MapLine*>	connected_lines;

public:
	MapVertex();
	~MapVertex();
};

#endif //__MAPVERTEX_H__
