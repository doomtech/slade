

#include "Main.h"
#include "MapThing.h"

MapThing::MapThing(doomthing_t t) {
	this->type = t.type;
	this->angle = t.angle;
	this->x = t.x;
	this->y = t.y;
	this->z = 0;
	this->tid = 0;
	if (t.flags & 1) {
		udmf_props["skill1"] = "true";
		udmf_props["skill2"] = "true";
	}
	if (t.flags & 2) {
		udmf_props["skill3"] = "true";
	}
	if (t.flags & 4) {
		udmf_props["skill4"] = "true";
		udmf_props["skill5"] = "true";
	}
}

MapThing::MapThing(hexenthing_t t) {
	this->type = t.type;
	this->angle = t.angle;
	this->x = t.x;
	this->y = t.y;
	this->z = t.z;
	this->tid = t.tid;
	if (t.flags & 1) {
		udmf_props["skill1"] = "true";
		udmf_props["skill2"] = "true";
	}
	if (t.flags & 2) {
		udmf_props["skill3"] = "true";
	}
	if (t.flags & 4) {
		udmf_props["skill4"] = "true";
		udmf_props["skill5"] = "true";
	}
}

MapThing::MapThing(doom64thing_t t) {
	this->type = t.type;
	this->angle = t.angle;
	this->x = t.x;
	this->y = t.y;
	this->z = t.z;
	this->tid = t.tid;
	if (t.flags & 1) {
		udmf_props["skill1"] = "true";
		udmf_props["skill2"] = "true";
	}
	if (t.flags & 2) {
		udmf_props["skill3"] = "true";
	}
	if (t.flags & 4) {
		udmf_props["skill4"] = "true";
		udmf_props["skill5"] = "true";
	}
}

bool MapThing::parseUDMF(Tokenizer& tz) {
	// Skip opening {
	tz.getToken();

	// TODO: Init required thing attributes

	// TODO: Read thing definition
	string token = tz.getToken();
	while (token != "}") {

		token = tz.getToken();
	}

	// TODO: Check that all required attributes were defined

	return true;
}
