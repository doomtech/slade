
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
