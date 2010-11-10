
#ifndef __MAPTHING_H__
#define __MAPTHING_H__

#include "Tokenizer.h"
#include "PropertyList.h"

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
private:
	short	type;
	double	x;
	double	y;
	double	z;
	short	angle;
	short	tid;

	PropertyList	udmf_props;

public:
	MapThing(){}
	MapThing(doomthing_t t);
	MapThing(hexenthing_t t);
	MapThing(doom64thing_t t);
	~MapThing(){}

	bool	parseUDMF(Tokenizer& tz);

	short	getType() { return type; }
};

#endif //__MAPTHING_H__
