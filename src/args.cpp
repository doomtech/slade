
#include "main.h"
#include "args.h"

argtype_t::argtype_t()
{
	this->name = _T("");
	this->has_flags = false;
	this->has_values = false;
	//arg_types.push_back(this);
}

//argtype_t::argtype_t(string name);

void argtype_t::parse(Tokenizer *tz)
{
	this->name = tz->get_token();

	tz->check_token(_T("{"));
	string token = tz->get_token();

	while (token != _T("}"))
	{
		if (token == _T("value"))
		{
			short val = tz->get_integer();
			string nname = tz->get_token();
			add_value(nname, val, false);
		}

		if (token == _T("flag"))
		{
			short val = tz->get_integer();
			string nname = tz->get_token();
			add_value(nname, val, true);
		}

		token = tz->get_token();
	}

	//log_message("Arg type %s parsed\n", this->name.c_str());
}

string argtype_t::get_name(short value)
{
	string ret = _T("");
	short tempval = value;

	if (has_values)
	{
		// Strip flags
		for (int a = 0; a < values.size(); a++)
		{
			if (values[a].flag)
				tempval = (tempval & ~values[a].value);
		}

		// Get value name
		for (int a = 0; a < values.size(); a++)
		{
			if (!values[a].flag && tempval == values[a].value)
			{
				ret = values[a].name;
				//ret += " ";
			}
		}
	}

	if (has_flags)
	{
		if (has_values)
			ret += _T(" (");

		bool flag = false;
		for (int a = 0; a < values.size(); a++)
		{
			if (values[a].flag && value & values[a].value)
			{
				ret += values[a].name;
				ret += _T(", ");
				flag = true;
			}
		}

		if (flag)
		{
			ret.erase(ret.end() - 1);
			ret.erase(ret.end() - 1);
		}

		if (has_values)
		{
			if (!flag)
				ret.erase(ret.end() - 1);
			else
				ret += _T(")");
		}
		else if (!flag)
			ret = _T("None");
	}

	if (ret.empty() || ret == _T(" "))
		ret = _T("Unknown");

	return ret;
}
