
#ifndef __ARGS_H__
#define __ARGS_H__

struct namevalue_t
{
	string	name;
	short	value;
	bool	flag;

	namevalue_t() { flag = false; value = 0; }
};

struct argtype_t
{
	vector<namevalue_t> values;
	string				name;
	bool				has_flags;
	bool				has_values;

	argtype_t();
	//argtype_t(string name);

	void add_value(string name, short value, bool flag)
	{
		namevalue_t newval;
		newval.name = name;
		newval.value = value;
		newval.flag = flag;
		values.push_back(newval);

		if (flag)
			this->has_flags = true;
		else
			this->has_values = true;
	}

	string get_name(short value);
	void parse(Tokenizer *tz);
};

argtype_t* get_arg_type(string name);

#endif //__ARGS_H__
