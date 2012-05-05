
#ifndef __MAPLINE_H__
#define __MAPLINE_H__

#include "MapObject.h"

class MapVertex;
class MapSide;
class MapSector;

// Line texture ids
enum {
	TEX_FRONT_MIDDLE	= 0x01,
	TEX_FRONT_UPPER		= 0x02,
	TEX_FRONT_LOWER		= 0x04,
	TEX_BACK_MIDDLE		= 0x08,
	TEX_BACK_UPPER		= 0x10,
	TEX_BACK_LOWER		= 0x20,
};

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
	double		length;
	double		ca;	// Used for intersection calculations
	double		sa;	// ^^
	fpoint2_t	tab_point;

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

	int	v1Index();
	int	v2Index();
	int	s1Index();
	int	s2Index();

	bool	boolProperty(string key);
	int		intProperty(string key);
	double	floatProperty(string key);
	string	stringProperty(string key);
	void	setBoolProperty(string key, bool value);
	void	setIntProperty(string key, int value);
	void	setFloatProperty(string key, double value);
	void	setStringProperty(string key, string value);

	fpoint2_t	midPoint();
	double		getLength();
	bool		doubleSector();
	fpoint2_t	dirTabPoint();
	double		distanceTo(double x, double y);
	int			needsTexture();
	void		clearUnneededTextures();

	void	resetInternals();
	void	flip(bool sides = true);
};

#endif //__MAPLINE_H__
