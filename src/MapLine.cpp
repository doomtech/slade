
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

bool MapLine::boolProperty(string key) {
	if (key.StartsWith("side1.") && side1)
		return side1->boolProperty(key.Mid(6));
	else if (key.StartsWith("side2.") && side2)
		return side2->boolProperty(key.Mid(6));
	else
		return MapObject::boolProperty(key);
}

int MapLine::intProperty(string key) {
	if (key.StartsWith("side1.") && side1)
		return side1->intProperty(key.Mid(6));
	else if (key.StartsWith("side2.") && side2)
		return side2->intProperty(key.Mid(6));
	else if (key == "v1")
		return v1Index();
	else if (key == "v2")
		return v2Index();
	else if (key == "sidefront")
		return s1Index();
	else if (key == "sideback")
		return s2Index();
	else
		return MapObject::intProperty(key);
}

double MapLine::floatProperty(string key) {
	if (key.StartsWith("side1.") && side1)
		return side1->floatProperty(key.Mid(6));
	else if (key.StartsWith("side2.") && side2)
		return side2->floatProperty(key.Mid(6));
	else
		return MapObject::floatProperty(key);
}

string MapLine::stringProperty(string key) {
	if (key.StartsWith("side1.") && side1)
		return side1->stringProperty(key.Mid(6));
	else if (key.StartsWith("side2.") && side2)
		return side2->stringProperty(key.Mid(6));
	else
		return MapObject::stringProperty(key);
}

void MapLine::setBoolProperty(string key, bool value) {
	if (key.StartsWith("side1.") && side1)
		return side1->setBoolProperty(key.Mid(6), value);
	else if (key.StartsWith("side2.") && side2)
		return side2->setBoolProperty(key.Mid(6), value);
	else
		MapObject::setBoolProperty(key, value);
}

void MapLine::setIntProperty(string key, int value) {
	if (key.StartsWith("side1.") && side1)
		return side1->setIntProperty(key.Mid(6), value);
	else if (key.StartsWith("side2.") && side2)
		return side2->setIntProperty(key.Mid(6), value);
	else
		MapObject::setIntProperty(key, value);
}

void MapLine::setFloatProperty(string key, double value) {
	if (key.StartsWith("side1.") && side1)
		return side1->setFloatProperty(key.Mid(6), value);
	else if (key.StartsWith("side2.") && side2)
		return side2->setFloatProperty(key.Mid(6), value);
	else
		MapObject::setFloatProperty(key, value);
}

void MapLine::setStringProperty(string key, string value) {
	if (key.StartsWith("side1.") && side1)
		return side1->setStringProperty(key.Mid(6), value);
	else if (key.StartsWith("side2.") && side2)
		return side2->setStringProperty(key.Mid(6), value);
	else
		MapObject::setStringProperty(key, value);
}

fpoint2_t MapLine::midPoint() {
	return fpoint2_t(x1() + ((x2() - x1()) * 0.5), y1() + ((y2() - y1()) * 0.5));
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

fpoint2_t MapLine::dirTabPoint() {
	// Calculate midpoint
	fpoint2_t mid = midPoint();

	// Calculate tab length
	double tablen = getLength() * 0.1;
	if (tablen > 16) tablen = 16;
	if (tablen < 2) tablen = 2;

	// Calculate tab endpoint
	fpoint2_t invdir(-(vertex2->yPos() - vertex1->yPos()), vertex2->xPos() - vertex1->xPos());
	invdir.normalize();
	return fpoint2_t(mid.x - invdir.x*tablen, mid.y - invdir.y*tablen);
}
