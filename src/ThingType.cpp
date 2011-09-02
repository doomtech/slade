
#include "Main.h"
#include "ThingType.h"

ThingType::ThingType(string name) {
	// Init variables
	this->name = name;
	this->angled = true;
	this->colour = COL_WHITE;
	this->radius = 16;
	
	// Init args
	args[0].name = "Arg1";
	args[1].name = "Arg2";
	args[2].name = "Arg3";
	args[3].name = "Arg4";
	args[4].name = "Arg5";
}
