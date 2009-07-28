
#include "Main.h"
#include "MapVertex.h"

MapVertex::MapVertex(doomvertex_t v) {
	this->x = v.x;
	this->y = v.y;
}

bool MapVertex::parseUDMF(Tokenizer& tz) {
	// Skip opening {
	tz.getToken();

	// Init required attributes
	bool x_set = false;
	bool y_set = false;

	// Read definition
	string token = tz.getToken();
	while (token != _T("}")) {
		// X position
		if (token == _T("x") && tz.peekToken() == _T("=")) {
			tz.getToken();	// Skip =
			x = tz.getFloat();
			x_set = true;
		}

		// Y position
		if (token == _T("y") && tz.peekToken() == _T("=")) {
			// Skip =
			tz.getToken();
			y = tz.getFloat();
			y_set = true;
		}

		token = tz.getToken();
	}

	// Check that all required attributes were defined
	if (x_set && y_set)
		return true;
	else {
		Global::error = _T("UDMF Vertex definition missing x or y value");
		return false;
	}
}
