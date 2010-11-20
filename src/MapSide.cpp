

#include "Main.h"
#include "MapSide.h"

MapSide::MapSide(MapSector* sector) {
	this->sector = sector;
}

bool MapSide::parseUDMF(Tokenizer& tz) {
	// Skip opening {
	tz.getToken();

	// TODO: Init required side attributes

	// TODO: Read side definition
	string token = tz.getToken();
	while (token != "}") {

		token = tz.getToken();
	}

	// TODO: Check that all required attributes were defined

	return true;
}
