
#ifndef __MAPTHING_H__
#define __MAPTHING_H__

#include "Tokenizer.h"
#include "PropertyList.h"

// Thing Flags
#define THING_EASY		0x0001
#define THING_MEDIUM	0x0002
#define THING_HARD		0x0004
#define THING_DEAF		0x0008
#define THING_MULTI		0x0010
// Boom
#define THING_BNOTDM	0x0020
#define THING_BNOTCOOP	0x0040
// Hexen
#define THING_HDORMANT	0x0010
#define THING_HCLASS1	0x0020
#define THING_HCLASS2	0x0040
#define THING_HCLASS3	0x0080
#define THING_HSP		0x0100
#define THING_HCOOP		0x0200
#define THING_HDM		0x0400

struct doomthing_t
{
	short	x;
	short	y;
	short	angle;
	short	type;
	short	flags;
};

struct hexenthing_t
{
	short	tid;
	short	x;
	short	y;
	short	z;
	short	angle;
	short	type;
	short	flags;
	uint8_t	special;
	uint8_t	args[5];
};

struct doom64thing_t
{
	short	x;
	short	y;
	short	z;
	short	angle;
	short	type;
	short	flags;
	short	tid;
};

class MapThing {
friend class SLADEMap;
private:
	// Basic data
	unsigned	index;
	short		type;
	double		x;
	double		y;

	// Properties
	PropertyList	udmf_props;

public:
	MapThing(){}
	MapThing(double x, double y, short type) { this->x = x; this->y = y; this->type = type; }
	~MapThing(){}

	double		xPos() { return x; }
	double		yPos() { return y; }
	unsigned	getIndex() { return index; }

	PropertyList&	props()				{ return udmf_props; }
	Property&		prop(string key)	{ return udmf_props[key]; }

	short	getType() { return type; }
};

#endif //__MAPTHING_H__
