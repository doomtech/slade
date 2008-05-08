
#include "thing_type.h"

// Doom flags
#define THING_EASY		0x0001
#define THING_MEDIUM	0x0002
#define THING_HARD		0x0004
#define THING_DEAF		0x0008
#define THING_MULTI		0x0010

// Extra hexen flags
#define THING_DORMANT		0x0010	// Also zdoom
#define THING_FIGHTER		0x0020
#define THING_CLERIC		0x0040
#define THING_MAGE			0x0080
#define THING_SINGLE		0x0100	// Also zdoom
#define THING_COOPERATIVE	0x0200	//     ''
#define THING_DEATHMATCH	0x0400	//     ''

// Extra zdoom flags
#define THING_TRANSLUCENT	0x0800
#define THING_INVISIBLE		0x1000
#define THING_FRIENDLY		0x2000
#define THING_STANDSTILL	0x4000

struct thing_t
{
	// Doom format
	short	x;
	short	y;
	short	type;
	short	flags;
	short	angle;

	// Hexen format extras
	WORD	tid;
	short	z;
	BYTE	special;
	BYTE	args[5];

	// Editor stuff
	thing_type_t	*ttype;

	thing_t()
	{
		x = 0;
		y = 0;
		z = 0;
		type = 1;
		angle = 0;
		flags = THING_EASY|THING_MEDIUM|THING_HARD|THING_DEAF|THING_SINGLE|THING_COOPERATIVE|THING_FIGHTER|THING_CLERIC|THING_MAGE;
		tid = 0;
		special = 0;
		//draw_angle = false;

		for (int a = 0; a < 5; a++)
			args[a] = 0;

		//colour.set(255, 255, 255, 255);
		//radius = 20;
	}

	bool toggle_flag(WORD flag)
	{
		flags ^= flag;

		return !!(flags & flag);
	}

	void set_flag(WORD flag)
	{
		flags |= flag;
	}

	void clear_flag(WORD flag)
	{
		flags = (flags & ~flag);
	}

	void increment_angle(int amount)
	{
		angle += amount;

		if (angle >= 360)
			angle = 0;

		if (angle < 0)
			angle = 315;
	}

	string angle_string()
	{
		if (angle == 0) // east
			return "East";
		else if (angle == 45) // northeast
			return "Northeast";
		else if (angle == 90) // north
			return "North";
		else if (angle == 135) // northwest
			return "Northwest";
		else if (angle == 180) // west
			return "West";
		else if (angle == 225) // southwest
			return "Southwest";
		else if (angle == 270) // south
			return "South";
		else if (angle == 315) // southeast
			return "Southeast";
		else // Invalid angle
			return "Invalid Angle";
	}

	string difficulty_string()
	{
		if (!(flags & THING_EASY)
			&& !(flags & THING_MEDIUM)
			&& !(flags & THING_HARD))
			return "None";

		string str;

		if (flags & THING_EASY)
			str += "Easy ";

		if (flags & THING_MEDIUM)
			str += "Med ";

		if (flags & THING_HARD)
			str += "Hard";

		return str;
	}
};
