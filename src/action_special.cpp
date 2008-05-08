
#include "main.h"
#include "action_special.h"

ActionSpecial::ActionSpecial(string group)
{
	type = 0;
	name = _T("Unknown");
	this->group = group;

	tagtype = 0;
	arg_tag = -1;

	args[0] = _T("Arg 1");
	args[1] = _T("Arg 2");
	args[2] = _T("Arg 3");
	args[3] = _T("Arg 4");
	args[4] = _T("Arg 5");
}

ActionSpecial::~ActionSpecial()
{
}

void ActionSpecial::parse(Tokenizer *tz)
{
	type = tz->get_integer();
	name = tz->get_token();

	if (tz->peek_token() == _T("{"))
	{
		tz->check_token(_T("{"));
		
		string token = tz->get_token();
		while (token != _T("}"))
		{
			if (token == _T("arg1"))
				args[0] = tz->get_token();
			if (token == _T("arg2"))
				args[1] = tz->get_token();
			if (token == _T("arg3"))
				args[2] = tz->get_token();
			if (token == _T("arg4"))
				args[3] = tz->get_token();
			if (token == _T("arg5"))
				args[4] = tz->get_token();

			if (token == _T("arg_tag"))
				arg_tag = tz->get_integer();
			if (token == _T("tagtype"))
				tagtype = tz->get_integer();

			if (token == _T("arg1_type"))
				arg_types[0] = tz->get_token();
			if (token == _T("arg2_type"))
				arg_types[1] = tz->get_token();
			if (token == _T("arg3_type"))
				arg_types[2] = tz->get_token();
			if (token == _T("arg4_type"))
				arg_types[3] = tz->get_token();
			if (token == _T("arg5_type"))
				arg_types[4] = tz->get_token();

			token = tz->get_token();
		}
	}
}
