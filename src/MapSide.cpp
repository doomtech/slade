

#include "Main.h"
#include "MapSide.h"
#include "MapSector.h"

MapSide::MapSide(MapSector* sector, SLADEMap* parent) : MapObject(MOBJ_SIDE, parent) {
	// Init variables
	this->sector = sector;
	this->parent = NULL;

	// Add to parent sector
	if (sector) sector->connected_sides.push_back(this);
}

MapSide::~MapSide() {
	// Remove side from current sector, if any
	if (this->sector) {
		for (unsigned a = 0; a < sector->connected_sides.size(); a++) {
			if (sector->connected_sides[a] == this) {
				sector->connected_sides.erase(sector->connected_sides.begin() + a);
				break;
			}
		}
	}
}

void MapSide::setSector(MapSector* sector) {
	// Remove side from current sector, if any
	if (this->sector) {
		for (unsigned a = 0; a < sector->connected_sides.size(); a++) {
			if (sector->connected_sides[a] == this) {
				sector->connected_sides.erase(sector->connected_sides.begin() + a);
				break;
			}
		}
	}

	// Add side to new sector
	this->sector = sector;
	sector->connected_sides.push_back(this);
}
