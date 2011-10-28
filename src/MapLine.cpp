
#include "Main.h"
#include "MapLine.h"
#include "MapVertex.h"
#include "MapSide.h"
#include "MathStuff.h"
#include "SLADEMap.h"

MapLine::MapLine(SLADEMap* parent) : MapObject(MOBJ_LINE, parent) {
	// Init variables
	vertex1 = NULL;
	vertex2 = NULL;
	side1 = NULL;
	side2 = NULL;
	length = -1;
}

MapLine::MapLine(MapVertex* v1, MapVertex* v2, MapSide* s1, MapSide* s2, SLADEMap* parent) : MapObject(MOBJ_LINE, parent) {
	// Init variables
	vertex1 = v1;
	vertex2 = v2;
	side1 = s1;
	side2 = s2;
	length = -1;

	// Connect to vertices
	if (v1) v1->connectLine(this);
	if (v2) v2->connectLine(this);

	// Connect to sides
	if (s1) s1->parent = this;
	if (s2) s2->parent = this;
}

MapLine::~MapLine() {
}

MapSector* MapLine::frontSector() {
	if (side1)
		return side1->sector;
	else
		return NULL;
}

MapSector* MapLine::backSector() {
	if (side2)
		return side2->sector;
	else
		return NULL;
}

double MapLine::x1() {
	return vertex1->xPos();
}

double MapLine::y1() {
	return vertex1->yPos();
}

double MapLine::x2() {
	return vertex2->xPos();
}

double MapLine::y2() {
	return vertex2->yPos();
}

int MapLine::v1Index() {
	if (vertex1)
		return vertex1->getIndex();
	else
		return -1;
}

int MapLine::v2Index() {
	if (vertex2)
		return vertex2->getIndex();
	else
		return -1;
}

int MapLine::s1Index() {
	if (side1)
		return side1->getIndex();
	else
		return -1;
}

int MapLine::s2Index() {
	if (side2)
		return side2->getIndex();
	else
		return -1;
}

double MapLine::getLength() {
	if (!vertex1 || !vertex2)
		return -1;

	if (length < 0) {
		length = MathStuff::distance(vertex1->xPos(), vertex1->yPos(), vertex2->xPos(), vertex2->yPos());
		ca = (vertex2->xPos() - vertex1->xPos()) / length;
		sa = (vertex2->yPos() - vertex1->yPos()) / length;
	}

	return length;
}

bool MapLine::doubleSector() {
	// Check both sides exist
	if (!side1 || !side2)
		return false;

	return (side1->getSector() == side2->getSector());
}
