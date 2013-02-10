

#include "Main.h"
#include "MapSide.h"
#include "MapSector.h"
#include "SLADEMap.h"
#include "MainApp.h"

MapSide::MapSide(MapSector* sector, SLADEMap* parent) : MapObject(MOBJ_SIDE, parent) {
	// Init variables
	this->sector = sector;
	this->parent = NULL;

	// Add to parent sector
	if (sector) sector->connected_sides.push_back(this);
}

MapSide::~MapSide() {
}

void MapSide::setSector(MapSector* sector) {
	// Remove side from current sector, if any
	if (this->sector)
		this->sector->disconnectSide(this);

	// Add side to new sector
	this->sector = sector;
	sector->connectSide(this);

	// Update modified time
	modified_time = theApp->runTimer();
}

int MapSide::intProperty(string key) {
	if (key == "sector") {
		if (sector)
			return sector->getIndex();
		else
			return -1;
	}
	else
		return MapObject::intProperty(key);
}

void MapSide::setIntProperty(string key, int value) {
	if (key == "sector" && parent_map)
		setSector(parent_map->getSector(value));
	else
		MapObject::setIntProperty(key, value);

	// Update modified time
	modified_time = theApp->runTimer();
}

void MapSide::writeBackup(mobj_backup_t* backup) {
	// Sector
	if (sector)
		backup->properties["sector"] = sector->getId();
	else
		backup->properties["sector"] = 0;
}

void MapSide::readBackup(mobj_backup_t* backup) {
	// Sector
	MapObject* s = parent_map->getObjectById(backup->properties["sector"]);
	if (s) {
		sector->disconnectSide(this);
		sector = (MapSector*)s;
		sector->connectSide(this);
	}
	else {
		if (sector)
			sector->disconnectSide(this);
		sector = NULL;
	}
}
