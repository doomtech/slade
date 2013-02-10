
#include "Main.h"
#include "MapObject.h"
#include "SLADEMap.h"
#include "GameConfiguration.h"
#include "MainApp.h"
#include "Console.h"

unsigned	current_id = 1;

MapObject::MapObject(int type, SLADEMap* parent) {
	// Init variables
	this->type = type;
	this->parent_map = parent;
	this->index = 0;
	this->filtered = false;
	this->modified_time = theApp->runTimer();
	this->id = current_id++;

	if (parent)
		parent->addMapObject(this);
}

MapObject::~MapObject() {
	properties.clear();
}

unsigned MapObject::getIndex() {
	if (parent_map)
		return parent_map->objectIndex(this);
	else
		return index;
}

string MapObject::getTypeName() {
	switch (type)
	{
	case MOBJ_VERTEX:
		return "Vertex";
	case MOBJ_SIDE:
		return "Side";
	case MOBJ_LINE:
		return "Line";
	case MOBJ_SECTOR:
		return "Sector";
	case MOBJ_THING:
		return "Thing";
	default:
		return "Unknown";
	}
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

	// Update modified time
	modified_time = theApp->runTimer();
}

bool MapObject::boolProperty(string key) {
	// If the property exists already, return it
	if (properties.propertyExists(key))
		return properties[key].getBoolValue();

	// Otherwise check the game configuration for a default value
	else {
		UDMFProperty* prop = theGameConfiguration->getUDMFProperty(key, type);
		if (prop)
			return prop->getDefaultValue().getBoolValue();
		else
			return false;
	}
}

int MapObject::intProperty(string key) {
	// If the property exists already, return it
	if (properties.propertyExists(key))
		return properties[key].getIntValue();

	// Otherwise check the game configuration for a default value
	else {
		UDMFProperty* prop = theGameConfiguration->getUDMFProperty(key, type);
		if (prop)
			return prop->getDefaultValue().getIntValue();
		else
			return 0;
	}
}

double MapObject::floatProperty(string key) {
	// If the property exists already, return it
	if (properties.propertyExists(key))
		return properties[key].getFloatValue();

	// Otherwise check the game configuration for a default value
	else {
		UDMFProperty* prop = theGameConfiguration->getUDMFProperty(key, type);
		if (prop)
			return prop->getDefaultValue().getFloatValue();
		else
			return 0;
	}
}

string MapObject::stringProperty(string key) {
	// If the property exists already, return it
	if (properties.propertyExists(key))
		return properties[key].getStringValue();

	// Otherwise check the game configuration for a default value
	else {
		UDMFProperty* prop = theGameConfiguration->getUDMFProperty(key, type);
		if (prop)
			return prop->getDefaultValue().getStringValue();
		else
			return "";
	}
}

void MapObject::setBoolProperty(string key, bool value) {
	// Set property
	properties[key] = value;

	// Update modified time
	modified_time = theApp->runTimer();
}

void MapObject::setIntProperty(string key, int value) {
	// Set property
	properties[key] = value;

	// Update modified time
	modified_time = theApp->runTimer();
}

void MapObject::setFloatProperty(string key, double value) {
	// Set property
	properties[key] = value;

	// Update modified time
	modified_time = theApp->runTimer();
}

void MapObject::setStringProperty(string key, string value) {
	// Set property
	properties[key] = value;

	// Update modified time
	modified_time = theApp->runTimer();
}

void MapObject::backup(mobj_backup_t* backup) {
	// Save basic info
	backup->id = id;
	backup->type = type;

	// Save general properties to list
	properties.copyTo(backup->properties);

	// Object-specific properties
	writeBackup(backup);
}

void MapObject::loadFromBackup(mobj_backup_t* backup) {
	// Check type match
	if (backup->type != type) {
		wxLogMessage("loadFromBackup: Mobj type mismatch, %d != %d", type, backup->type);
		return;
	}
	// Check id match
	if (backup->id != id) {
		wxLogMessage("loadFromBackup: Mobj id mismatch, %d != %d", id, backup->id);
		return;
	}

	// Load general properties from list
	properties.clear();
	backup->properties.copyTo(properties);
	
	// Object-specific properties
	readBackup(backup);

	// Update modified time
	modified_time = theApp->runTimer();
}

void MapObject::resetIdCounter() {
	current_id = 1;
}

CONSOLE_COMMAND(m_next_id, 0) {
	theConsole->logMessage(S_FMT("Next Object ID: %d", current_id));
}
