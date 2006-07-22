
#ifndef __ACTION_SPECIAL_H__
#define __ACTION_SPECIAL_H__

class ActionSpecial
{
public:
	int		type;
	string	name;
	string	group;
	char	tagtype;	// 0=sector, 1=thing
	char	arg_tag;	// The arg to act as a tag (0 for sector tag, -1 for none)
	string	args[5];
	string	arg_types[5];

	ActionSpecial(string group = "");
	~ActionSpecial();

	void parse(Tokenizer *tz);
};

#endif //__ACTION_SPECIAL_H__
