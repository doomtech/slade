
#include "Main.h"
#include "MapSector.h"
#include "MapLine.h"
#include "MapSide.h"
#include "MapVertex.h"
#include "GameConfiguration.h"

MapSector::MapSector(SLADEMap* parent) : MapObject(MOBJ_SECTOR, parent) {
	// Init variables
	poly_needsupdate = true;
}

MapSector::MapSector(string f_tex, string c_tex, SLADEMap* parent) : MapObject(MOBJ_SECTOR, parent) {
	// Init variables
	this->f_tex = f_tex;
	this->c_tex = c_tex;
	poly_needsupdate = true;
}

MapSector::~MapSector() {
}

string MapSector::stringProperty(string key) {
	if (key == "texturefloor")
		return f_tex;
	else if (key == "textureceiling")
		return c_tex;
	else
		return MapObject::stringProperty(key);
}

void MapSector::setStringProperty(string key, string value) {
	if (key == "texturefloor")
		f_tex = value;
	else if (key == "textureceiling")
		c_tex = value;
	else
		MapObject::setStringProperty(key, value);
}

void MapSector::setFloatProperty(string key, double value) {
	// Check if flat offset/scale/rotation is changing (if UDMF + ZDoom)
	if (theGameConfiguration->getMapFormat() == MAP_UDMF && S_CMPNOCASE(theGameConfiguration->udmfNamespace(), "zdoom")) {
		if (key == "xpanningfloor" || key == "ypanningfloor" ||
			key == "xpanningceiling" || key == "ypanningceiling" ||
			key == "xscalefloor" || key == "yscalefloor" ||
			key == "xscaleceiling" || key == "yscaleceiling" ||
			key == "rotationfloor" || key == "rotationceiling")
			polygon.setTexture(NULL);	// Clear texture to force update
	}

	MapObject::setFloatProperty(key, value);
}

fpoint2_t MapSector::midPoint() {
	bbox_t bbox = boundingBox();
	return fpoint2_t(bbox.min.x + ((bbox.max.x-bbox.min.x)*0.5),
						bbox.min.y + ((bbox.max.y-bbox.min.y)*0.5));
}

void MapSector::updateBBox() {
	// Reset bounding box
	bbox.reset();

	for (unsigned a = 0; a < connected_sides.size(); a++) {
		MapLine* line = connected_sides[a]->getParentLine();
		if (!line) continue;
		bbox.extend(line->v1()->xPos(), line->v1()->yPos());
		bbox.extend(line->v2()->xPos(), line->v2()->yPos());
	}
}

bbox_t MapSector::boundingBox() {
	// Update bbox if needed
	if (!bbox.is_valid())
		updateBBox();

	return bbox;
}

Polygon2D* MapSector::getPolygon() {
	if (poly_needsupdate) {
		polygon.openSector(this);
		poly_needsupdate = false;
	}

	return &polygon;
}

void MapSector::connectSide(MapSide* side) {
	connected_sides.push_back(side);
	poly_needsupdate = true;
	bbox.reset();
}

void MapSector::disconnectSide(MapSide* side) {
	for (unsigned a = 0; a < connected_sides.size(); a++) {
		if (connected_sides[a] == side) {
			connected_sides.erase(connected_sides.begin() + a);
			break;
		}
	}

	poly_needsupdate = true;
	bbox.reset();
}
