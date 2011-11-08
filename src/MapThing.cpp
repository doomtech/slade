
#include "Main.h"
#include "MapThing.h"

MapThing::MapThing(SLADEMap* parent) : MapObject(MOBJ_THING, parent) {
	// Init variables
	this->x = 0;
	this->y = 0;
	this->type = 1;
}

MapThing::MapThing(double x, double y, short type, SLADEMap* parent) : MapObject(MOBJ_THING, parent) {
	// Init variables
	this->x = x;
	this->y = y;
	this->type = type;
}

MapThing::~MapThing() {
}

int MapThing::intProperty(string key) {
	if (key == "type")
		return type;
	else if (key == "x")
		return (int)x;
	else if (key == "y")
		return (int)y;
	else
		return MapObject::intProperty(key);
}

double MapThing::floatProperty(string key) {
	if (key == "x")
		return x;
	else if (key == "y")
		return y;
	else
		return MapObject::floatProperty(key);
}

void MapThing::setIntProperty(string key, int value) {
	if (key == "type")
		type = value;
	else if (key == "x")
		x = value;
	else if (key == "y")
		y = value;
	else
		MapObject::setIntProperty(key, value);
}

void MapThing::setFloatProperty(string key, double value) {
	if (key == "x")
		x = value;
	else if (key == "y")
		y = value;
	else
		MapObject::setFloatProperty(key, value);
}
