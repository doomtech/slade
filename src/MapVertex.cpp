
#include "Main.h"
#include "MapVertex.h"

MapVertex::MapVertex(SLADEMap* parent) : MapObject(MOBJ_VERTEX, parent) {
	// Init variables
	this->x = 0;
	this->y = 0;
}

MapVertex::MapVertex(double x, double y, SLADEMap* parent) : MapObject(MOBJ_VERTEX, parent) {
	// Init variables
	this->x = x;
	this->y = y;
}

MapVertex::~MapVertex() {
}

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

MapLine* MapVertex::connectedLine(unsigned index) {
	if (index > connected_lines.size())
		return NULL;

	return connected_lines[index];
}
