
#ifndef __MAPLINE_H__
#define __MAPLINE_H__

class MapVertex;
class MapSide;

class MapLine {
private:
	MapVertex*	vertex1;
	MapVertex*	vertex2;
	MapSide*	side1;
	MapSide*	side2;

public:
	MapLine(){}
	~MapLine(){}
};

#endif //__MAPLINE_H__
