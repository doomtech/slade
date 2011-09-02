
#ifndef __ARGS_H__
#define __ARGS_H__

enum {
	ARGT_NUMBER = 0,
	ARGT_YESNO,
	ARGT_NOYES,
	ARGT_ANGLE,
	ARGT_CUSTOM,
};

struct arg_val_t {
	int		value;
	string	name;
};

struct arg_t {
	string				name;
	int					type;
	vector<arg_val_t>	custom_values;
	vector<arg_val_t>	custom_flags;

	arg_t() { name = ""; type = 0; }
};

#endif//__ARGS_H__
