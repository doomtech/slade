
#ifndef __MAPLINE_H__
#define __MAPLINE_H__

class MapVertex;
class MapSide;

struct doomline_t
{
	short vertex1;
	short vertex2;
	short flags;
	short type;
	short sector_tag;
	short side1;
	short side2;
};

struct hexenline_t
{
	short	vertex1;
	short	vertex2;
	short	flags;
	uint8_t	type;
	uint8_t	args[5];
	short	side1;
	short	side2;
};

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
