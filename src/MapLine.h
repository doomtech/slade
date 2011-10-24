
#ifndef __MAPLINE_H__
#define __MAPLINE_H__

#include "MapObject.h"

class MapVertex;
class MapSide;
class MapSector;

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
	uint16_t vertex1;
	uint16_t vertex2;
	uint16_t flags;
	uint16_t type;
	uint16_t sector_tag;
	uint16_t side1;
	uint16_t side2;
};

struct hexenline_t {
	uint16_t	vertex1;
	uint16_t	vertex2;
	uint16_t	flags;
	uint8_t		type;
	uint8_t		args[5];
	uint16_t	side1;
	uint16_t	side2;
};

struct doom64line_t {
	uint16_t vertex1;
	uint16_t vertex2;
	uint32_t flags;
	uint16_t type;
	uint16_t sector_tag;
	uint16_t side1;
	uint16_t side2;
};

class MapLine : public MapObject {
friend class SLADEMap;
private:
	// Basic data
	MapVertex*	vertex1;
	MapVertex*	vertex2;
	MapSide*	side1;
	MapSide*	side2;

	// Internally used info
	double	length;
	double	ca;
	double	sa;

public:
	MapLine(SLADEMap* parent = NULL);
	MapLine(MapVertex* v1, MapVertex* v2, MapSide* s1, MapSide* s2, SLADEMap* parent = NULL);
	~MapLine();

	bool	isOk() { return vertex1 && vertex2; }

	MapVertex*		v1() { return vertex1; }
	MapVertex*		v2() { return vertex2; }
	MapSide*		s1() { return side1; }
	MapSide*		s2() { return side2; }

	MapSector*	frontSector();
	MapSector*	backSector();

	double	x1();
	double	y1();
	double	x2();
	double	y2();

	double	getLength();
	bool	doubleSector();
};

#endif //__MAPLINE_H__
