

#include "Main.h"
#include "MapSide.h"
#include "MapSector.h"

MapSide::MapSide(MapSector* sector) {
	// Init variables
	this->sector = sector;
	this->parent = NULL;

	// Add to parent sector
	if (sector) sector->connected_sides.push_back(this);
}

MapSide::~MapSide() {
}
