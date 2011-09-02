
#ifndef __THING_TYPE_H__
#define __THING_TYPE_H__

#include "Args.h"

class ParseTreeNode;
class ThingType {
friend class GameConfiguration;
private:
	string	name;
	string	group;
	rgba_t	colour;
	int		radius;
	bool	angled;
	bool	hanging;
	string	sprite;
	arg_t	args[5];
	
public:
	ThingType(string name = "Unknown");
	~ThingType() {}

	string	getName() { return name; }
	string	getGroup() { return group; }
	rgba_t	getColour() { return colour; }
	int		getRadius() { return radius; }
	bool	isAngled() { return angled; }
	string	getSprite() { return sprite; }
	arg_t&	getArg(int index) { if (index >= 0 && index < 5) return args[index]; else return args[0]; }

	void	reset();
	void	parse(ParseTreeNode* node);
	string	stringDesc();
};

#endif//__THING_TYPE_H__
