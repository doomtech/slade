
#include "Main.h"
#include "MapObject.h"
#include "SLADEMap.h"
#include "GameConfiguration.h"

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

bool MapObject::boolProperty(string key) {
	if (properties.propertyExists(key))
		return properties[key].getBoolValue();
	else {
		UDMFProperty* prop = theGameConfiguration->getUDMFProperty(key, type);
		if (prop)
			return prop->getDefaultValue().getBoolValue();
		else
			return false;
	}
}

int MapObject::intProperty(string key) {
	if (properties.propertyExists(key))
		return properties[key].getIntValue();
	else {
		UDMFProperty* prop = theGameConfiguration->getUDMFProperty(key, type);
		if (prop)
			return prop->getDefaultValue().getIntValue();
		else
			return 0;
	}
}

double MapObject::floatProperty(string key) {
	if (properties.propertyExists(key))
		return properties[key].getFloatValue();
	else {
		UDMFProperty* prop = theGameConfiguration->getUDMFProperty(key, type);
		if (prop)
			return prop->getDefaultValue().getFloatValue();
		else
			return 0;
	}
}

string MapObject::stringProperty(string key) {
	if (properties.propertyExists(key))
		return properties[key].getStringValue();
	else {
		UDMFProperty* prop = theGameConfiguration->getUDMFProperty(key, type);
		if (prop)
			return prop->getDefaultValue().getStringValue();
		else
			return "";
	}
}
