
#ifndef __MAPLINE_H__
#define __MAPLINE_H__

#include "Tokenizer.h"
#include "PropertyList.h"

class MapVertex;
class MapSide;

// Line flags
#define	LINE_IMPASSIBLE		0x0001
#define LINE_BLOCKMONSTERS	0x0002
#define	LINE_TWOSIDED		0x0004
#define	LINE_UPPERUNPEGGED	0x0008
#define	LINE_LOWERUNPEGGED	0x0010
#define	LINE_SECRET			0x0020
#define	LINE_BLOCKSOUND		0x0040
#define	LINE_NOTONMAP		0x0080
#define	LINE_STARTONMAP		0x0100
// Boom
#define LINE_BPASSTHROUGH	0x0200
// Hexen
#define LINE_HREPEATABLE	0x0200
#define LINE_HMONSTERACT	0x2000
#define LINE_HBLOCKALL		0x8000

struct doomline_t {
	short vertex1;
	short vertex2;
	short flags;
	short type;
	short sector_tag;
	short side1;
	short side2;
};

struct hexenline_t {
	short	vertex1;
	short	vertex2;
	short	flags;
	uint8_t	type;
	uint8_t	args[5];
	short	side1;
	short	side2;
};

struct doom64line_t {
	short vertex1;
	short vertex2;
	uint32_t flags;
	short type;
	short sector_tag;
	short side1;
	short side2;
};

class MapLine {
friend class SLADEMap;
private:
	// Basic data
	unsigned	index;
	MapVertex*	vertex1;
	MapVertex*	vertex2;
	MapSide*	side1;
	MapSide*	side2;

	// Properties
	PropertyList	udmf_props;

public:
	MapLine(){}
	MapLine(MapVertex* v1, MapVertex* v2, MapSide* s1, MapSide* s2);
	~MapLine(){}

	bool	isOk() { return vertex1 && vertex2; }

	unsigned		getIndex() { return index; }
	MapVertex*		v1() { return vertex1; }
	MapVertex*		v2() { return vertex2; }
	MapSide*		s1() { return side1; }
	MapSide*		s2() { return side2; }

	PropertyList&	props()				{ return udmf_props; }
	Property&		prop(string key)	{ return udmf_props[key]; }
};

#endif //__MAPLINE_H__
