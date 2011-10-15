
#include "Main.h"
#include "MapEditor.h"
#include "ColourConfiguration.h"
#include "ArchiveManager.h"
#include "WxStuff.h"
#include "MapEditorWindow.h"
#include "GameConfiguration.h"
#include "MathStuff.h"
#include "Console.h"
#include "MapCanvas.h"

double grid_sizes[] = { 0.05, 0.1, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };

MapEditor::MapEditor() {
	// Init variables
	edit_mode = MODE_LINES;
	hilight_item = -1;
	gridsize = 9;
	canvas = NULL;
}

MapEditor::~MapEditor() {
}

double MapEditor::gridSize() {
	return grid_sizes[gridsize];
}

void MapEditor::setEditMode(int mode) {
	edit_mode = mode;
	hilight_item = -1;
	selection.clear();
}

bool MapEditor::openMap(Archive::mapdesc_t map) {
	wxLogMessage("Opening map %s", CHR(map.name));
	return this->map.readMap(map);
}

void MapEditor::clearMap() {
	// Clear map
	map.clearMap();

	// Clear selection
	selection.clear();
	hilight_item = -1;
}

bool MapEditor::updateHilight(fpoint2_t mouse_pos, double dist_scale) {
	int current = hilight_item;

	// Update hilighted object depending on mode
	if (edit_mode == MODE_VERTICES)
		hilight_item = map.nearestVertex(mouse_pos.x, mouse_pos.y, 32/dist_scale);
	else if (edit_mode == MODE_LINES)
		hilight_item = map.nearestLine(mouse_pos.x, mouse_pos.y, 32/dist_scale);
	else if (edit_mode == MODE_SECTORS)
		hilight_item = map.inSector(mouse_pos.x, mouse_pos.y);
	else if (edit_mode == MODE_THINGS) {
		hilight_item = -1;

		// Get (possibly multiple) nearest-thing(s)
		vector<int> nearest = map.nearestThingMulti(mouse_pos.x, mouse_pos.y);
		if (nearest.size() == 1) {
			MapThing* t = map.getThing(nearest[0]);
			ThingType* type = theGameConfiguration->thingType(t->getType());
			double dist = MathStuff::distance(mouse_pos.x, mouse_pos.y, t->xPos(), t->yPos());
			if (dist <= type->getRadius() + (32/dist_scale))
				hilight_item = nearest[0];
		}
		else {
			for (unsigned a = 0; a < nearest.size(); a++) {
				MapThing* t = map.getThing(nearest[a]);
				ThingType* type = theGameConfiguration->thingType(t->getType());
				double dist = MathStuff::distance(mouse_pos.x, mouse_pos.y, t->xPos(), t->yPos());
				if (dist <= type->getRadius() + (32/dist_scale))
					hilight_item = nearest[a];
			}
		}
	}

	return current != hilight_item;
}

void MapEditor::clearSelection() {
	if (canvas) canvas->itemsSelected(selection, false);
	selection.clear();
}

void MapEditor::selectAll() {
	// Clear selection initially
	selection.clear();

	// Select all items depending on mode
	if (edit_mode == MODE_VERTICES) {
		for (unsigned a = 0; a < map.vertices.size(); a++)
			selection.push_back(a);
	}
	else if (edit_mode == MODE_LINES) {
		for (unsigned a = 0; a < map.lines.size(); a++)
			selection.push_back(a);
	}
	else if (edit_mode == MODE_SECTORS) {
		for (unsigned a = 0; a < map.sectors.size(); a++)
			selection.push_back(a);
	}
	else if (edit_mode == MODE_THINGS) {
		for (unsigned a = 0; a < map.things.size(); a++)
			selection.push_back(a);
	}

	if (canvas)
		canvas->itemsSelected(selection);
}

bool MapEditor::selectCurrent(bool clear_none) {
	// If nothing is hilighted
	if (hilight_item == -1) {
		// Clear selection if specified
		if (clear_none) {
			if (canvas) canvas->itemsSelected(selection, false);
			selection.clear();
		}

		return false;
	}

	// Otherwise, check if item is in selection
	for (unsigned a = 0; a < selection.size(); a++) {
		if (selection[a] == hilight_item) {
			// Already selected, deselect
			selection.erase(selection.begin() + a);
			if (canvas) canvas->itemSelected(hilight_item, false);
			return true;
		}
	}

	// Not already selected, add to selection
	selection.push_back(hilight_item);
	if (canvas) canvas->itemSelected(hilight_item, true);

	return true;
}

bool MapEditor::selectWithin(double xmin, double ymin, double xmax, double ymax, bool add) {
	// Select depending on editing mode
	bool new_sel = false;
	vector<int> nsel;

	// Vertices
	if (edit_mode == MODE_VERTICES) {
		// Go through vertices
		double x, y;
		for (unsigned a = 0; a < map.vertices.size(); a++) {
			// Skip if vertex is already selected
			if (std::find(selection.begin(), selection.end(), a) != selection.end())
				continue;

			// Get position
			x = map.vertices[a]->xPos();
			y = map.vertices[a]->yPos();

			// Select if vertex is within bounds
			if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
				//selection.push_back(a);
				nsel.push_back(a);
				new_sel = true;
			}
		}
	}

	// Lines
	else if (edit_mode == MODE_LINES) {
		// Go through lines
		MapLine* line;
		double x1, y1, x2, y2;
		for (unsigned a = 0; a < map.lines.size(); a++) {
			// Skip if line is already selected
			if (std::find(selection.begin(), selection.end(), a) != selection.end())
				continue;

			// Get vertex positions
			line = map.lines[a];
			x1 = line->v1()->xPos();
			y1 = line->v1()->yPos();
			x2 = line->v2()->xPos();
			y2 = line->v2()->yPos();

			// Select if both vertices are within bounds
			if (xmin <= x1 && x1 <= xmax && ymin <= y1 && y1 <= ymax &&
				xmin <= x2 && x2 <= xmax && ymin <= y2 && y2 <= ymax) {
				//selection.push_back(a);
				nsel.push_back(a);
				new_sel = true;
			}
		}
	}

	// Sectors
	else if (edit_mode == MODE_SECTORS) {
		// Go through sectors
		fpoint2_t pmin(xmin, ymin);
		fpoint2_t pmax(xmax, ymax);
		for (unsigned a = 0; a < map.sectors.size(); a++) {
			// Check if sector's bbox fits within the selection box
			if (map.sectors[a]->boundingBox().is_within(pmin, pmax)) {
				//selection.push_back(a);
				nsel.push_back(a);
			}
		}
	}

	// Things
	else if (edit_mode == MODE_THINGS) {
		// Go through things
		double x, y;
		for (unsigned a = 0; a < map.things.size(); a++) {
			// Skip if thing is already selected
			if (std::find(selection.begin(), selection.end(), a) != selection.end())
				continue;

			// Get position
			x = map.things[a]->xPos();
			y = map.things[a]->yPos();

			// Select if thing is within bounds
			if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
				//selection.push_back(a);
				nsel.push_back(a);
				new_sel = true;
			}
		}
	}

	// Clear selection if something was within the box
	if (nsel.size() > 0 && !add)
		clearSelection();

	// Update selection
	if (nsel.size() > 0) {
		for (unsigned a = 0; a < nsel.size(); a++)
			selection.push_back(nsel[a]);

		// Animate
		if (canvas) canvas->itemsSelected(nsel);
	}

	return new_sel;
}

MapVertex* MapEditor::getHilightedVertex() {
	// Check edit mode is correct
	if (edit_mode != MODE_VERTICES)
		return NULL;

	// Having one item selected counts as a hilight
	if (hilight_item == -1 && selection.size() == 1)
		return map.getVertex(selection[0]);

	return map.getVertex(hilight_item);
}

MapLine* MapEditor::getHilightedLine() {
	// Check edit mode is correct
	if (edit_mode != MODE_LINES)
		return NULL;

	// Having one item selected counts as a hilight
	if (hilight_item == -1 && selection.size() == 1)
		return map.getLine(selection[0]);

	return map.getLine(hilight_item);
}

MapSector* MapEditor::getHilightedSector() {
	// Check edit mode is correct
	if (edit_mode != MODE_SECTORS)
		return NULL;

	// Having one item selected counts as a hilight
	if (hilight_item == -1 && selection.size() == 1)
		return map.getSector(selection[0]);

	return map.getSector(hilight_item);
}

MapThing* MapEditor::getHilightedThing() {
	// Check edit mode is correct
	if (edit_mode != MODE_THINGS)
		return NULL;

	// Having one item selected counts as a hilight
	if (hilight_item == -1 && selection.size() == 1)
		return map.getThing(selection[0]);

	return map.getThing(hilight_item);
}

void MapEditor::getSelectedVertices(vector<MapVertex*>& list) {
	if (edit_mode != MODE_VERTICES)
		return;

	// Multiple selection
	if (selection.size() > 1) {
		for (unsigned a = 0; a < selection.size(); a++)
			list.push_back(map.getVertex(selection[a]));
	}

	// Single selection
	else if (selection.size() == 1)
		list.push_back(map.getVertex(selection[0]));

	// No selection (use hilight)
	else if (hilight_item >= 0)
		list.push_back(map.getVertex(hilight_item));
}

void MapEditor::getSelectedLines(vector<MapLine*>& list) {
	if (edit_mode != MODE_LINES)
		return;

	// Multiple selection
	if (selection.size() > 1) {
		for (unsigned a = 0; a < selection.size(); a++)
			list.push_back(map.getLine(selection[a]));
	}

	// Single selection
	else if (selection.size() == 1)
		list.push_back(map.getLine(selection[0]));

	// No selection (use hilight)
	else if (hilight_item >= 0)
		list.push_back(map.getLine(hilight_item));
}

void MapEditor::getSelectedSectors(vector<MapSector*>& list) {
	if (edit_mode != MODE_SECTORS)
		return;

	// Multiple selection
	if (selection.size() > 1) {
		for (unsigned a = 0; a < selection.size(); a++)
			list.push_back(map.getSector(selection[a]));
	}

	// Single selection
	else if (selection.size() == 1)
		list.push_back(map.getSector(selection[0]));

	// No selection (use hilight)
	else if (hilight_item >= 0)
		list.push_back(map.getSector(hilight_item));
}

void MapEditor::getSelectedThings(vector<MapThing*>& list) {
	if (edit_mode != MODE_THINGS)
		return;

	// Multiple selection
	if (selection.size() > 1) {
		for (unsigned a = 0; a < selection.size(); a++)
			list.push_back(map.getThing(selection[a]));
	}

	// Single selection
	else if (selection.size() == 1)
		list.push_back(map.getThing(selection[0]));

	// No selection (use hilight)
	else if (hilight_item >= 0)
		list.push_back(map.getThing(hilight_item));
}

void MapEditor::showItem(int index) {
	selection.clear();
	int max = 0;
	switch (edit_mode) {
	case MODE_VERTICES: max = map.nVertices(); break;
	case MODE_LINES: max = map.nLines(); break;
	case MODE_SECTORS: max = map.nSectors(); break;
	case MODE_THINGS: max = map.nThings(); break;
	default: max = 0; break;
	}

	if (index < max) {
		selection.push_back(index);
		if (canvas) canvas->viewShowObject();
	}
}

void MapEditor::incrementGrid() {
	gridsize++;
	if (gridsize > 18)
		gridsize = 18;
}

void MapEditor::decrementGrid() {
	// Non-integral grid size disabled for now
	gridsize--;
	if (gridsize < 4)
		gridsize = 4;
}

CONSOLE_COMMAND(m_show_item, 1) {
	int index = atoi(CHR(args[0]));
	theMapEditor->mapEditor().showItem(index);
}
