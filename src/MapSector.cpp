

#include "Main.h"
#include "MapSector.h"

MapSector::MapSector(doomsector_t s) {
	// TODO: MapSector constructor from doomsector_t
}

MapSector::MapSector(doom64sector_t s) {
	// TODO: MapSector constructor from doom64sector_t
}

bool MapSector::parseUDMF(Tokenizer& tz) {
	// Skip opening {
	tz.getToken();

	// TODO: Init required sector attributes

	// TODO: Read sector definition
	string token = tz.getToken();
	while (token != "}") {

		token = tz.getToken();
	}

	// TODO: Check that all required attributes were defined

	return true;
}
