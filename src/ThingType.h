
#ifndef __THING_TYPE_H__
#define __THING_TYPE_H__

#include "Args.h"

class ThingType {
private:
	string	name;
	string	group;
	rgba_t	colour;
	int		radius;
	bool	angled;
	arg_t	args[5];
	
public:
	ThingType(string name = "Unknown");
	~ThingType() {}
};

#endif//__THING_TYPE_H__
