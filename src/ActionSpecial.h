
#ifndef __ACTION_SPECIAL_H__
#define __ACTION_SPECIAL_H__

#include "Args.h"

// Tag types
enum {
	AS_TT_NO = 0,
	AS_TT_SECTOR,
	AS_TT_LINE,
	AS_TT_THING,
	AS_TT_SECTOR_BACK,
	AS_TT_SECTOR_OR_BACK,
	AS_TT_SECTOR_AND_BACK,
};

class ParseTreeNode;
class ActionSpecial {
friend class GameConfiguration;
private:
	string	name;
	string	group;
	int		tagged;
	arg_t	args[5];
	
public:
	ActionSpecial(string name = "Unknown", string group = "");
	~ActionSpecial() {}

	void	copy(ActionSpecial* copy);
	
	string	getName() { return name; }
	string	getGroup() { return group; }
	int		needsTag() { return tagged; }
	arg_t&	getArg(int index) { if (index >= 0 && index < 5) return args[index]; else return args[0]; }
	
	void	setName(string name) { this->name = name; }
	void	setGroup(string group) { this->group = group; }
	void	setTagged(int tagged) { this->tagged = tagged; }

	string	getArgsString(int args[5]);

	void	reset();
	void	parse(ParseTreeNode* node);
	string	stringDesc();
};

#endif//__ACTION_SPECIAL_H__
