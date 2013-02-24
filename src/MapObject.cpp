
#include "Main.h"
#include "MapObject.h"
#include "SLADEMap.h"
#include "GameConfiguration.h"
#include "MainApp.h"
#include "Console.h"

//unsigned	current_id = 1;
long		prop_backup_time = -1;

//testing
//vector<MapObject*> all_objects;

MapObject::MapObject(int type, SLADEMap* parent) {
	// Init variables
	this->type = type;
	this->parent_map = parent;
	this->index = 0;
	this->filtered = false;
	this->modified_time = theApp->runTimer();
	this->id = 0;//current_id++;
	this->obj_backup = NULL;

	if (parent)
		parent->addMapObject(this);

	//all_objects.push_back(this);
}

MapObject::~MapObject() {
	properties.clear();
	//all_objects[id - 1] = NULL;
	if (obj_backup)
		delete obj_backup;
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

void MapObject::setModified() {
	// Backup current properties if required
	if (modified_time < prop_backup_time) {
		if (obj_backup) delete obj_backup;
		obj_backup = new mobj_backup_t();
		backup(obj_backup);
	}

	modified_time = theApp->runTimer();
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
	setModified();
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
	// Update modified time
	setModified();

	// Set property
	properties[key] = value;
}

void MapObject::setIntProperty(string key, int value) {
	// Update modified time
	setModified();

	// Set property
	properties[key] = value;
}

void MapObject::setFloatProperty(string key, double value) {
	// Update modified time
	setModified();

	// Set property
	properties[key] = value;
}

void MapObject::setStringProperty(string key, string value) {
	// Update modified time
	setModified();

	// Set property
	properties[key] = value;
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
	setModified();
}

mobj_backup_t* MapObject::getBackup(bool remove) {
	mobj_backup_t* bak = obj_backup;
	if (remove) obj_backup = NULL;
	return bak;
}

//void MapObject::resetIdCounter() {
//	current_id = 1;
//}

long MapObject::propBackupTime() {
	return prop_backup_time;
}

void MapObject::beginPropBackup(long current_time) {
	prop_backup_time = current_time;
}

void MapObject::endPropBackup() {
	prop_backup_time = -1;
}

//CONSOLE_COMMAND(m_next_id, 0, false) {
//	theConsole->logMessage(S_FMT("Next Object ID: %d", current_id));
//}

//CONSOLE_COMMAND(mobj_check_deleted, 0) {
//	theConsole->logMessage(S_FMT("Checking %d ids", all_objects.size()));
//	for (unsigned a = 0; a < all_objects.size(); a++) {
//		if (all_objects[a])
//			theConsole->logMessage(S_FMT("Object id %d not deleted", a+1));
//	}
//}
