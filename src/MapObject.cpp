
#include "Main.h"
#include "MapObject.h"
#include "SLADEMap.h"

MapObject::MapObject(int type, SLADEMap* parent) {
	// Init variables
	this->type = type;
	this->parent_map = parent;
	this->index = 0;
	this->filtered = false;
}

MapObject::~MapObject() {
}

unsigned MapObject::getIndex() {
	return index;
}
