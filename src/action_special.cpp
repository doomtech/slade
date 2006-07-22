
#include "main.h"
#include "action_special.h"

ActionSpecial::ActionSpecial(string group)
{
	type = 0;
	name = "Unknown";
	this->group = group;

	tagtype = 0;
	arg_tag = -1;

	args[0] = "Arg 1";
	args[1] = "Arg 2";
	args[2] = "Arg 3";
	args[3] = "Arg 4";
	args[4] = "Arg 5";
}

ActionSpecial::~ActionSpecial()
{
}

void ActionSpecial::parse(Tokenizer *tz)
{
	type = tz->get_integer();
	name = tz->get_token();

	if (tz->peek_token() == "{")
	{
		tz->check_token("{");
		
		string token = tz->get_token();
		while (token != "}")
		{
			if (token == "arg1")
				args[0] = tz->get_token();
			if (token == "arg2")
				args[1] = tz->get_token();
			if (token == "arg3")
				args[2] = tz->get_token();
			if (token == "arg4")
				args[3] = tz->get_token();
			if (token == "arg5")
				args[4] = tz->get_token();

			if (token == "arg_tag")
				arg_tag = tz->get_integer();
			if (token == "tagtype")
				tagtype = tz->get_integer();

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

			token = tz->get_token();
		}
	}
}
