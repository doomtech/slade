

#include "Main.h"
#include "MapLine.h"
#include "MapVertex.h"
#include "MapSide.h"

MapLine::MapLine(MapVertex* v1, MapVertex* v2, MapSide* s1, MapSide* s2) {
	// Init variables
	vertex1 = v1;
	vertex2 = v2;
	side1 = s1;
	side2 = s2;

	// Connect to vertices
	if (v1) v1->connectLine(this);
	if (v2) v2->connectLine(this);

	// Connect to sides
	if (s1) s1->parent = this;
	if (s2) s2->parent = this;
}
