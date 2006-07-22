
#include "main.h"
#include "thing_type.h"
#include "colours.h"

extern vector<string>	spritenames;

ThingType::ThingType(int type, string name, string group)
{
	setup(type, name, group);
}

ThingType::ThingType(ThingType *ttype)
{
	type = ttype->type;
	name = ttype->name;
	group = ttype->group;

	colour.set(ttype->colour);
	radius = ttype->radius;
	spritename = ttype->spritename;
	flags = ttype->flags;
	alpha = ttype->alpha;
	x_scale = ttype->x_scale;
	y_scale = ttype->y_scale;
	x_offset = ttype->x_offset;
	y_offset = ttype->y_offset;

	for (int a = 0; a < 5; a++)
		args[a] = ttype->args[a];

	for (int a = 0; a < 5; a++)
		arg_types[a] = ttype->arg_types[a];
}

ThingType::~ThingType()
{
}

void ThingType::setup(int type, string name, string group)
{
	this->type = type;
	this->name = name;
	this->group = group;

	colour.set(200, 200, 200, 255, 0);
	radius = -1;
	spritename = "_unknownsprite";
	flags = TTYPE_ANGLE;
	alpha = 1.0f;
	x_scale = 1.0f;
	y_scale = 1.0f;
	x_offset = 0;
	y_offset = 0;

	args[0] = "Arg 1";
	args[1] = "Arg 2";
	args[2] = "Arg 3";
	args[3] = "Arg 4";
	args[4] = "Arg 5";

	for (int a = 0; a < 5; a++)
		arg_types[a] = "";
}

void ThingType::set_flag(BYTE flag, bool value)
{
	if (value)
		flags |= flag;
	else
		flags = (flags & ~flag);
}

void ThingType::parse(Tokenizer *tz)
{
	type = tz->get_integer();
	name = tz->get_token();

	if (tz->peek_token() == "{")
	{
		tz->check_token("{");

		string token = tz->get_token();
		while (token != "}")
		{
			if (token == "size")
				radius = tz->get_integer();
			if (token == "colour")
				parse_rgba(tz, &colour);
			if (token == "sprite")
			{
				spritename = tz->get_token();

				if (!(vector_exists(spritenames, spritename)))
					spritenames.push_back(spritename);
			}
			if (token == "angle")
			{
				if (tz->get_bool())
					flags |= TTYPE_ANGLE;
				else
					flags = (flags & ~TTYPE_ANGLE);
			}
			if (token == "hanging")
			{
				if (tz->get_bool())
					flags |= TTYPE_HANGING;
				else
					flags = (flags & ~TTYPE_HANGING);
			}

			if (token == "arg1")
			{
				args[0] = tz->get_token();
				flags |= TTYPE_HASARGS;
			}
			if (token == "arg2")
			{
				args[1] = tz->get_token();
				flags |= TTYPE_HASARGS;
			}
			if (token == "arg3")
			{
				args[2] = tz->get_token();
				flags |= TTYPE_HASARGS;
			}
			if (token == "arg4")
			{
				args[3] = tz->get_token();
				flags |= TTYPE_HASARGS;
			}
			if (token == "arg5")
			{
				args[4] = tz->get_token();
				flags |= TTYPE_HASARGS;
			}

			if (token == "arg1_type")
				arg_types[0] = tz->get_token();
			if (token == "arg2_type")
				arg_types[1] = tz->get_token();
			if (token == "arg3_type")
				arg_types[2] = tz->get_token();
			if (token == "arg4_type")
				arg_types[3] = tz->get_token();
			if (token == "arg5_type")
				arg_types[4] = tz->get_token();

			if (token == "alpha")
				alpha = tz->get_float();
			if (token == "x_scale")
				x_scale = tz->get_float();
			if (token == "y_scale")
				y_scale = tz->get_float();
			if (token == "x_offset")
				x_offset = tz->get_integer();
			if (token == "y_offset")
				y_offset = tz->get_integer();

			token = tz->get_token();
		}
	}
}
