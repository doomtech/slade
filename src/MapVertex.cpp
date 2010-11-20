
#include "Main.h"
#include "MapVertex.h"

/*
MapVertex::MapVertex(doomvertex_t v) {
	this->x = v.x;
	this->y = v.y;
}

MapVertex::MapVertex(doom64vertex_t v) {
	this->x = v.x/65536;
	this->y = v.y/65536;
}
*/

void MapVertex::connectLine(MapLine* line) {
	for (unsigned a = 0; a < connected_lines.size(); a++) {
		if (connected_lines[a] == line)
			return;
	}

	connected_lines.push_back(line);
}

void MapVertex::disconnectLine(MapLine* line) {
	for (unsigned a = 0; a < connected_lines.size(); a++) {
		if (connected_lines[a] == line) {
			connected_lines.erase(connected_lines.begin() + a);
			return;
		}
	}
}

bool MapVertex::parseUDMF(Tokenizer& tz) {
	// Skip opening {
	tz.getToken();

	// Init required attributes
	bool x_set = false;
	bool y_set = false;

	// Read definition
	string token = tz.getToken();
	while (token != "}") {
		// X position
		if (token == "x" && tz.peekToken() == "=") {
			tz.getToken();	// Skip =
			x = tz.getFloat();
			x_set = true;
		}

		// Y position
		if (token == "y" && tz.peekToken() == "=") {
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
		Global::error = "UDMF Vertex definition missing x or y value";
		return false;
	}
}
