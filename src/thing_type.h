
#ifndef __THING_TYPE_H__
#define __THING_TYPE_H__

#include "structs.h"

#define	TTYPE_ANGLE		0x01
#define TTYPE_HANGING	0x02
#define TTYPE_HASARGS	0x04
#define TTYPE_DECORATE	0x08

class Texture;

class ThingType
{
public:
	int			type;
	string		name;
	string		group;
	rgba_t		colour;
	int			radius;
	string		spritename;
	BYTE		flags;
	float		alpha;
	float		x_scale;
	float		y_scale;
	int			x_offset;
	int			y_offset;
	Texture		*sprite;

	string		args[5];
	string		arg_types[5];

	ThingType(int type = -1, string name = _T("Unknown"), string group = _T(""));
	ThingType(ThingType *ttype);
	~ThingType();

	void setup(int type = -1, string name = _T("Unknown"), string group = _T(""));
	void set_flag(BYTE flag, bool value);
	void parse(Tokenizer *tz);
};

#endif
