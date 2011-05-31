

#include "Main.h"
#include "MapLine.h"
#include "MapVertex.h"

MapLine::MapLine(MapVertex* v1, MapVertex* v2, MapSide* s1, MapSide* s2) {
	// Init variables
	vertex1 = v1;
	vertex2 = v2;
	side1 = s1;
	side2 = s2;

	// Connect to vertices
	v1->connectLine(this);
	v2->connectLine(this);
}
