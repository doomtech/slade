#define	LINE_IMPASSIBLE		0x0001
#define LINE_BLOCKMONSTERS	0x0002
#define	LINE_TWOSIDED		0x0004
#define	LINE_UPPERUNPEGGED	0x0008
#define	LINE_LOWERUNPEGGED	0x0010
#define	LINE_SECRET			0x0020
#define	LINE_BLOCKSOUND		0x0040
#define	LINE_NOTONMAP		0x0080
#define	LINE_STARTONMAP		0x0100
#define LINE_REPEATABLE		0x0200
#define LINE_MONSTERCANACT	0x2000
#define LINE_BLOCKALL		0x8000

#define LINE_SPAC_CROSS			0
#define LINE_SPAC_USE			1
#define LINE_SPAC_MCROSS		2
#define LINE_SPAC_IMPACT		3
#define LINE_SPAC_PUSH			4
#define LINE_SPAC_PCROSS		5
#define LINE_SPAC_USETHROUGH	6
#define LINE_SPAC_PTOUCH		7

struct linedef_t
{
	// Common info
	WORD	vertex1;
	WORD	vertex2;
	short	flags;
	WORD	type;
	long	side1;
	long	side2;

	// Doom format only
	short	sector_tag;

	// Hexen format only
	BYTE	args[5];

	linedef_t()
	{
		// Common info
		vertex1 = 0;
		vertex2 = 0;
		flags = 0;
		type = 0;
		side1 = -1;
		side2 = -1;

		// Doom info
		sector_tag = 0;

		// Hexen info
		for (int a = 0; a < 5; a++)
			args[a] = 0;
	}

	linedef_t(int V1, int V2)
	{
		vertex1 = V1;
		vertex2 = V2;

		flags = 0;
		type = 0;
		side1 = -1;
		side2 = -1;
		sector_tag = 0;
	
		for (int a = 0; a < 5; a++)
			args[a] = 0;
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

	WORD get_trigger()
	{
		return ((flags & 0x1c00) >> 10);
	}

	void set_trigger(WORD trigger)
	{
		trigger = trigger << 10;

		flags &= ~0x1c00;
		flags |= trigger;
	}

	string get_trigger_string()
	{
		WORD trigger = get_trigger();

		if (trigger == 0)
			return "Player Crosses Line";
		if (trigger == 1)
			return "Player Uses Line";
		if (trigger == 2)
			return "Monster Crosses Line";
		if (trigger == 3)
			return "Projectile Hits Line";
		if (trigger == 4)
			return "Player Bumps Line";
		if (trigger == 5)
			return "Projectile Crosses Line";
		if (trigger == 6)
			return "Player Uses Line (PassThru)";
		if (trigger == 7)
			return "Projectile Hits OR Crosses Line";

		return "Unknown";
	}
};
