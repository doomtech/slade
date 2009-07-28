
#ifndef __MAPTHING_H__
#define __MAPTHING_H__

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
	uint16_t	tid;
	short		x;
	short		y;
	short		z;
	short		angle;
	short		type;
	short		flags;
	uint8_t		special;
	uint8_t		args[5];
};

class MapThing {
private:
	double x;
	double y;

public:
	MapThing(){}
	~MapThing(){}
};

#endif //__MAPTHING_H__
