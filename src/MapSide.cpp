

#include "Main.h"
#include "MapSide.h"
#include "MapSector.h"
#include "SLADEMap.h"
#include "MainApp.h"

MapSide::MapSide(MapSector* sector, SLADEMap* parent) : MapObject(MOBJ_SIDE, parent) {
	// Init variables
	this->sector = sector;
	this->parent = NULL;
	this->offset_x = 0;
	this->offset_y = 0;

	// Add to parent sector
	if (sector) sector->connected_sides.push_back(this);
}

MapSide::MapSide(SLADEMap* parent) : MapObject(MOBJ_SIDE, parent) {
	// Init variables
	this->sector = NULL;
	this->parent = NULL;
}

MapSide::~MapSide() {
}

void MapSide::copy(MapObject* c) {
	if (c->getObjType() != MOBJ_SIDE)
		return;

	// Copy properties
	MapSide* side = (MapSide*)c;
	this->tex_lower = side->tex_lower;
	this->tex_middle = side->tex_middle;
	this->tex_upper = side->tex_upper;
	this->offset_x = side->offset_x;
	this->offset_y = side->offset_y;

	MapObject::copy(c);
}

void MapSide::setSector(MapSector* sector) {
	// Remove side from current sector, if any
	if (this->sector)
		this->sector->disconnectSide(this);

	// Update modified time
	setModified();

	// Add side to new sector
	this->sector = sector;
	sector->connectSide(this);
}

int MapSide::intProperty(string key) {
	if (key == "sector") {
		if (sector)
			return sector->getIndex();
		else
			return -1;
	}
	else if (key == "offsetx")
		return offset_x;
	else if (key == "offsety")
		return offset_y;
	else
		return MapObject::intProperty(key);
}

void MapSide::setIntProperty(string key, int value) {
	// Update modified time
	setModified();

	if (key == "sector" && parent_map)
		setSector(parent_map->getSector(value));
	else if (key == "offsetx")
		offset_x = value;
	else if (key == "offsety")
		offset_y = value;
	else
		MapObject::setIntProperty(key, value);
}

string MapSide::stringProperty(string key) {
	if (key == "texturetop")
		return tex_upper;
	else if (key == "texturemiddle")
		return tex_middle;
	else if (key == "texturebottom")
		return tex_lower;
	else
		return MapObject::stringProperty(key);
}

void MapSide::setStringProperty(string key, string value) {
	// Update modified time
	setModified();

	if (key == "texturetop")
		tex_upper = value;
	else if (key == "texturemiddle")
		tex_middle = value;
	else if (key == "texturebottom")
		tex_lower = value;
	else
		MapObject::setStringProperty(key, value);
}

void MapSide::writeBackup(mobj_backup_t* backup) {
	// Sector
	if (sector)
		backup->properties["sector"] = sector->getId();
	else
		backup->properties["sector"] = 0;

	// Textures
	backup->properties["texturetop"] = tex_upper;
	backup->properties["texturemiddle"] = tex_middle;
	backup->properties["texturebottom"] = tex_lower;

	// Offsets
	backup->properties["offsetx"] = offset_x;
	backup->properties["offsety"] = offset_y;

	//wxLogMessage("Side %d backup sector #%d", id, sector->getIndex());
}

void MapSide::readBackup(mobj_backup_t* backup) {
	// Sector
	MapObject* s = parent_map->getObjectById(backup->properties["sector"]);
	if (s) {
		sector->disconnectSide(this);
		sector = (MapSector*)s;
		sector->connectSide(this);
		//wxLogMessage("Side %d load backup sector #%d", id, s->getIndex());
	}
	else {
		if (sector)
			sector->disconnectSide(this);
		sector = NULL;
	}

	// Textures
	tex_upper = backup->properties["texturetop"].getStringValue();
	tex_middle = backup->properties["texturemiddle"].getStringValue();
	tex_lower = backup->properties["texturebottom"].getStringValue();

	// Offsets
	offset_x = backup->properties["offsetx"].getIntValue();
	offset_y = backup->properties["offsety"].getIntValue();
}
