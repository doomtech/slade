
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
	if (parent_map)
		return parent_map->objectIndex(this);
	else
		return index;
}

void MapObject::copy(MapObject* c) {
	// Can't copy an object of a different type
	if (c->type != type)
		return;

	// Reset properties
	properties.clear();

	// Copy object properties
	c->properties.copyTo(properties);
	this->parent_map = c->parent_map;
	this->filtered = c->filtered;
}
