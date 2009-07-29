

#include "Main.h"
#include "MapThing.h"

MapThing::MapThing(doomthing_t t) {
	this->x = t.x;
	this->y = t.y;
}

MapThing::MapThing(hexenthing_t t) {
	this->x = t.x;
	this->y = t.y;
}

bool MapThing::parseUDMF(Tokenizer& tz) {
	// Skip opening {
	tz.getToken();

	// TODO: Init required thing attributes

	// TODO: Read thing definition
	string token = tz.getToken();
	while (token != _T("}")) {

		token = tz.getToken();
	}

	// TODO: Check that all required attributes were defined

	return true;
}
