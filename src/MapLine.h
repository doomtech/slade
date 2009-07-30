
#ifndef __MAPLINE_H__
#define __MAPLINE_H__

#include "Tokenizer.h"

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

	PropertyList	udmf_props;

public:
	MapLine(){}
	MapLine(doomline_t l);
	MapLine(hexenline_t l);
	~MapLine(){}

	MapVertex*	v1() { return vertex1; }
	MapVertex*	v2() { return vertex2; }
	MapSide*	s1() { return side1; }
	MapSide*	s2() { return side2; }

	bool parseUDMF(Tokenizer& tz);
};

#endif //__MAPLINE_H__
