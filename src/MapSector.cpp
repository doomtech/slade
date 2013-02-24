
#include "Main.h"
#include "MapSector.h"
#include "MapLine.h"
#include "MapSide.h"
#include "MapVertex.h"
#include "MainApp.h"
#include "SLADEMap.h"
#include "MathStuff.h"
#include <wx/colour.h>

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

void MapSector::copy(MapObject* s) {
	// Don't copy a non-sector
	if (s->getObjType() != MOBJ_SECTOR)
		return;

	// Basic variables
	MapSector* sector = (MapSector*)s;
	this->f_tex = sector->f_tex;
	this->c_tex = sector->c_tex;

	// Other properties
	MapObject::copy(s);
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
	// Update modified time
	setModified();

	if (key == "texturefloor")
		f_tex = value;
	else if (key == "textureceiling")
		c_tex = value;
	else
		return MapObject::setStringProperty(key, value);
}

void MapSector::setFloatProperty(string key, double value) {
	// Check if flat offset/scale/rotation is changing (if UDMF + ZDoom)
	if (parent_map->currentFormat() == MAP_UDMF && S_CMPNOCASE(parent_map->udmfNamespace(), "zdoom")) {
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

bool MapSector::isWithin(double x, double y) {
	// Check with bbox first
	if (!boundingBox().point_within(x, y))
		return false;

	// Find nearest line in the sector
	double dist;
	double min_dist = 999999;
	MapLine* nline = NULL;
	for (unsigned a = 0; a < connected_sides.size(); a++) {
		// Calculate distance to line
		dist = connected_sides[a]->getParentLine()->distanceTo(x, y);

		// Check distance
		if (dist < min_dist) {
			nline = connected_sides[a]->getParentLine();
			min_dist = dist;
		}
	}

	// No nearest (shouldn't happen)
	if (!nline)
		return false;

	// Check the side of the nearest line
	double side = MathStuff::lineSide(x, y, nline->x1(), nline->y1(), nline->x2(), nline->y2());
	if (side >= 0 && nline->frontSector() == this)
		return true;
	else if (side < 0 && nline->backSector() == this)
		return true;
	else
		return false;
}

double MapSector::distanceTo(double x, double y, double maxdist) {
	// Init
	if (maxdist < 0)
		maxdist = 9999999;

	// Check bounding box first
	if (!bbox.is_valid())
		updateBBox();
	double min_dist = 9999999;
	double dist = MathStuff::distanceToLine(x, y, bbox.min.x, bbox.min.y, bbox.min.x, bbox.max.y);
	if (dist < min_dist) min_dist = dist;
	dist = MathStuff::distanceToLine(x, y, bbox.min.x, bbox.max.y, bbox.max.x, bbox.max.y);
	if (dist < min_dist) min_dist = dist;
	dist = MathStuff::distanceToLine(x, y, bbox.max.x, bbox.max.y, bbox.max.x, bbox.min.y);
	if (dist < min_dist) min_dist = dist;
	dist = MathStuff::distanceToLine(x, y, bbox.max.x, bbox.min.y, bbox.min.x, bbox.min.y);
	if (dist < min_dist) min_dist = dist;

	if (min_dist > maxdist && !bbox.point_within(x, y))
		return -1;

	// Go through connected sides
	for (unsigned a = 0; a < connected_sides.size(); a++) {
		// Get side parent line
		MapLine* line = connected_sides[a]->getParentLine();
		if (!line) continue;

		// Check distance
		dist = line->distanceTo(x, y);
		if (dist < min_dist)
			min_dist = dist;
	}

	return min_dist;
}

bool MapSector::getLines(vector<MapLine*>& list) {
	// Go through connected sides
	for (unsigned a = 0; a < connected_sides.size(); a++) {
		// Add the side's parent line to the list if it doesn't already exist
		if (std::find(list.begin(), list.end(), connected_sides[a]->getParentLine()) == list.end())
			list.push_back(connected_sides[a]->getParentLine());
	}

	return true;
}

bool MapSector::getVertices(vector<MapVertex*>& list) {
	// Go through connected sides
	MapLine* line;
	for (unsigned a = 0; a < connected_sides.size(); a++) {
		line = connected_sides[a]->getParentLine();

		// Add the side's parent line's vertices to the list if they doesn't already exist
		if (line->v1() && std::find(list.begin(), list.end(), line->v1()) == list.end())
			list.push_back(line->v1());
		if (line->v2() && std::find(list.begin(), list.end(), line->v2()) == list.end())
			list.push_back(line->v2());
	}

	return true;
}

uint8_t MapSector::getLight(int where) {
	// Get sector light level
	int light = intProperty("lightlevel");

	// Check for UDMF+ZDoom namespace
	if (parent_map->currentFormat() == MAP_UDMF && S_CMPNOCASE(parent_map->udmfNamespace(), "zdoom")) {
		// Get specific light level
		if (where == 1) {
			// Floor
			int fl = intProperty("lightfloor");
			if (boolProperty("lightfloorabsolute"))
				light = fl;
			else
				light += fl;
		}
		else if (where == 2) {
			// Ceiling
			int cl = intProperty("lightceiling");
			if (boolProperty("lightceilingabsolute"))
				light = cl;
			else
				light += cl;
		}
	}

	// Clamp light level
	if (light > 255)
		light = 255;
	if (light < 0)
		light = 0;

	return light;
}

void MapSector::changeLight(int amount, int where) {
	// Get current light level
	int light = getLight(where);

	// Clamp amount
	if (light + amount > 255)
		amount -= ((light+amount) - 255);
	else if (light + amount < 0)
		amount = -light;

	// Check for UDMF+ZDoom namespace
	bool separate = false;
	if (parent_map->currentFormat() == MAP_UDMF && S_CMPNOCASE(parent_map->udmfNamespace(), "zdoom"))
		separate = true;

	// Change light level by amount
	if (where == 1 && separate) {
		int cur = intProperty("lightfloor");
		setIntProperty("lightfloor", cur + amount);
	}
	else if (where == 2 && separate) {
		int cur = intProperty("lightceiling");
		setIntProperty("lightceiling", cur + amount);
	}
	else
		setIntProperty("lightlevel", light + amount);
}

rgba_t MapSector::getColour(int where, bool fullbright) {
	// Check for UDMF+ZDoom namespace
	if (parent_map->currentFormat() == MAP_UDMF && S_CMPNOCASE(parent_map->udmfNamespace(), "zdoom")) {
		// Get sector light colour
		int intcol = intProperty("lightcolor");
		wxColour wxcol(intcol);

		// Ignore light level if fullbright
		if (fullbright)
			return rgba_t(wxcol.Blue(), wxcol.Green(), wxcol.Red(), 255);

		// Get sector light level
		int light = intProperty("lightlevel");

		// Get specific light level
		if (where == 1) {
			// Floor
			int fl = intProperty("lightfloor");
			if (boolProperty("lightfloorabsolute"))
				light = fl;
			else
				light += fl;
		}
		else if (where == 2) {
			// Ceiling
			int cl = intProperty("lightceiling");
			if (boolProperty("lightceilingabsolute"))
				light = cl;
			else
				light += cl;
		}

		// Clamp light level
		if (light > 255)
			light = 255;
		if (light < 0)
			light = 0;

		// Calculate and return the colour
		float lightmult = (float)light / 255.0f;
		return rgba_t(wxcol.Blue() * lightmult, wxcol.Green() * lightmult, wxcol.Red() * lightmult, 255);
	}
	else {
		// Other format, simply return the light level
		if (fullbright)
			return rgba_t(255, 255, 255, 255);
		else {
			int light = intProperty("lightlevel");
			return rgba_t(light, light, light, 255);
		}
	}
}

void MapSector::connectSide(MapSide* side) {
	connected_sides.push_back(side);
	poly_needsupdate = true;
	bbox.reset();
	setModified();
}

void MapSector::disconnectSide(MapSide* side) {
	for (unsigned a = 0; a < connected_sides.size(); a++) {
		if (connected_sides[a] == side) {
			connected_sides.erase(connected_sides.begin() + a);
			break;
		}
	}

	setModified();
	poly_needsupdate = true;
	bbox.reset();
}

void MapSector::writeBackup(mobj_backup_t* backup) {
	// Textures
	backup->properties["texturefloor"] = f_tex;
	backup->properties["textureceiling"] = c_tex;
}

void MapSector::readBackup(mobj_backup_t* backup) {
	// General properties
	//MapObject::loadFromBackup(plist);

	// Textures
	f_tex = backup->properties["texturefloor"].getStringValue();
	c_tex = backup->properties["textureceiling"].getStringValue();
}
