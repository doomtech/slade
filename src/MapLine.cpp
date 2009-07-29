

#include "Main.h"
#include "MapLine.h"

MapLine::MapLine(doomline_t l) {
	// TODO: MapLine constructor from doomline_t
}

MapLine::MapLine(hexenline_t l) {
	// TODO: MapLine constructor from hexenline_t
}

bool MapLine::parseUDMF(Tokenizer& tz) {
	// Skip opening {
	tz.getToken();

	// TODO: Init required line attributes

	// TODO: Read line definition
	string token = tz.getToken();
	while (token != _T("}")) {

		token = tz.getToken();
	}

	// TODO: Check that all required attributes were defined

	return true;
}
