/* Various little structures for binary control lumps. */
#ifndef BINARYCONTROLLUMPS_H
#define	BINARYCONTROLLUMPS_H

#include "ArchiveEntry.h"

// If the compiler pads the structures, this completely breaks parsing.
#pragma pack(1)

/*******************************************************************
 * BOOM'S ANIMATED LUMP
 *******************************************************************/

enum
{
	ANIM_FLAT = 0,
	ANIM_TEXTURE = 1,
	ANIM_MASK = 1,
	ANIM_DECALS = 2, // ZDoom uses bit 1 to flag whether decals are allowed.
	ANIM_STOP = 255,
};

// The format of an entry in an ANIMATED lump
struct animated_t
{
	uint8_t		type;		// flat or texture, with or without decals
	char		last [9];	// first name in the animation
	char		first[9];	// last name in the animation
	uint32_t	speed;		// amount of tics between two frames
};

/*******************************************************************
 * BOOM'S SWITCHES LUMP
 *******************************************************************/

enum
{
	SWCH_STOP = 0,
	SWCH_DEMO = 1,
	SWCH_FULL = 2,
	SWCH_COMM = 3,
};

struct switches_t
{
	char		off[9];
	char		on[9];
	uint16_t	type;
};

#endif //BINARYCONTROLLUMPS_H