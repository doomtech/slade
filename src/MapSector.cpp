

#include "Main.h"
#include "MapSector.h"
#include "MapLine.h"
#include "MapSide.h"
#include "MapVertex.h"

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
