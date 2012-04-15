
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
#include "MapObjectPropsPanel.h"
#include "SectorBuilder.h"

double grid_sizes[] = { 0.05, 0.1, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };

MapEditor::MapEditor() {
	// Init variables
	edit_mode = MODE_LINES;
	hilight_item = -1;
	gridsize = 9;
	canvas = NULL;
	hilight_locked = false;
	sector_mode = SECTOR_BOTH;
	grid_snap = true;
	copy_thing = new MapThing(NULL);
	copy_sector = new MapSector(NULL);
}

MapEditor::~MapEditor() {
	delete copy_thing;
	delete copy_sector;
}

double MapEditor::gridSize() {
	return grid_sizes[gridsize];
}

void MapEditor::setEditMode(int mode) {
	// Check if we are changing to the same mode
	if (mode == edit_mode) {
		// Cycle sector edit mode
		if (mode == MODE_SECTORS)
			setSectorEditMode(sector_mode + 1);
		
		// Do nothing otherwise
		return;
	}
	
	// Set edit mode
	edit_mode = mode;
	sector_mode = SECTOR_BOTH;
	
	// Clear hilight and selection stuff
	hilight_item = -1;
	selection.clear();
	tagged_sectors.clear();
	tagged_lines.clear();
	tagged_things.clear();

	// Add editor message
	switch (edit_mode) {
	case MODE_VERTICES: addEditorMessage("Vertices mode"); break;
	case MODE_LINES:	addEditorMessage("Lines mode"); break;
	case MODE_SECTORS:	addEditorMessage("Sectors mode (Normal)"); break;
	case MODE_THINGS:	addEditorMessage("Things mode"); break;
	default: break;
	};
}

void MapEditor::setSectorEditMode(int mode) {
	// Set sector mode
	sector_mode = mode;
	if (sector_mode > SECTOR_CEILING || sector_mode < 0)
		sector_mode = SECTOR_BOTH;
	
	// Editor message
	if (sector_mode == SECTOR_BOTH)
		addEditorMessage("Sectors mode (Normal)");
	else if (sector_mode == SECTOR_FLOOR)
		addEditorMessage("Sectors mode (Floors)");
	else
		addEditorMessage("Sectors mode (Ceilings)");
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
	// Do nothing if hilight is locked
	if (hilight_locked)
		return false;

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

	// Update tagged lists if the hilight changed
	if (current != hilight_item) {
		// Clear tagged lists
		tagged_sectors.clear();
		tagged_lines.clear();
		tagged_things.clear();

		// Special
		if (hilight_item >= 0 && (edit_mode == MODE_LINES || edit_mode == MODE_THINGS)) {
			MapSector* back = NULL;
			MapSector* front = NULL;
			int needs_tag, tag, arg2, arg3, arg4, arg5;
			// Line specials have front and possibly back sectors
			if (edit_mode == MODE_LINES) {
				MapLine* line = map.getLine(hilight_item);
				if (line->s2()) back = line->s2()->getSector();
				if (line->s1()) front = line->s1()->getSector();
				needs_tag = theGameConfiguration->actionSpecial((int)line->prop("special"))->needsTag();
				tag = line->prop("arg0");
				arg2 = line->prop("arg1");
				arg3 = line->prop("arg2");
				arg4 = line->prop("arg3");
				arg5 = line->prop("arg4");

			// Hexen and UDMF things can have specials too
			} else /* edit_mode == MODE_THINGS */ {
				MapThing* thing = map.getThing(hilight_item);
				needs_tag = theGameConfiguration->actionSpecial((int)thing->prop("special"))->needsTag();
				tag = thing->prop("arg0");
				arg2 = thing->prop("arg1");
				arg3 = thing->prop("arg2");
				arg4 = thing->prop("arg3");
				arg5 = thing->prop("arg4");
			}

			// Sector tag
			if (needs_tag == AS_TT_SECTOR ||
				needs_tag == AS_TT_SECTOR_AND_BACK && tag > 0)
				map.getSectorsByTag(tag, tagged_sectors);

			// Backside sector (for local doors)
			else if ((needs_tag == AS_TT_SECTOR_BACK || needs_tag == AS_TT_SECTOR_AND_BACK) && back)
				tagged_sectors.push_back(back);

			// Sector tag *or* backside sector (for zdoom local doors)
			else if (needs_tag == AS_TT_SECTOR_OR_BACK) {
				if (tag > 0)
					map.getSectorsByTag(tag, tagged_sectors);
				else if (back)
					tagged_sectors.push_back(back);
			}

			// Thing ID
			else if (needs_tag == AS_TT_THING)
				map.getThingsById(tag, tagged_things);

			// Line ID
			else if (needs_tag == AS_TT_LINE)
				map.getLinesById(tag, tagged_lines);

			// ZDoom quirkiness
			else if (needs_tag) {
				switch (needs_tag) {
					case AS_TT_1THING_2SECTOR:
					case AS_TT_1THING_3SECTOR:
					case AS_TT_1SECTOR_2THING:
					{
						int thingtag = (needs_tag == AS_TT_1THING_2SECTOR) ? arg2 : tag;
						int sectag = (needs_tag == AS_TT_1THING_2SECTOR) ? tag :
							(needs_tag == AS_TT_1THING_2SECTOR) ? arg2 : arg3;
						if ((thingtag | sectag) == 0)
							break;
						else if (thingtag == 0)
							map.getSectorsByTag(sectag, tagged_sectors);
						else if (sectag == 0)
							map.getThingsById(thingtag, tagged_things);
						else // neither thingtag nor sectag are 0
							map.getThingsByIdInSectorTag(thingtag, sectag, tagged_things);
					}	break;
					case AS_TT_1THING_2THING_3THING:
						map.getThingsById(arg3, tagged_things);
					case AS_TT_1THING_2THING:
						map.getThingsById(arg2, tagged_things);
					case AS_TT_1THING_4THING:
						map.getThingsById(tag, tagged_things);
					case AS_TT_4THING:
						if (needs_tag == AS_TT_1THING_4THING || needs_tag == AS_TT_4THING)
							map.getThingsById(arg4, tagged_things);
						break;
					case AS_TT_5THING:
						map.getThingsById(arg5, tagged_things);
						break;
					case AS_TT_LINE_NEGATIVE:
						map.getLinesById(abs(tag), tagged_lines);
						break;
					case AS_TT_1LINEID_2LINE:
						map.getLinesById(arg2, tagged_lines);
						break;
					case AS_TT_1LINE_2SECTOR:
						map.getLinesById(tag, tagged_lines);
						map.getSectorsByTag(arg2, tagged_sectors);
						break;
					case AS_TT_1SECTOR_2THING_3THING_5THING:
						if (arg5) map.getThingsById(arg5, tagged_things);
						map.getThingsById(arg3, tagged_things);
					case AS_TT_1SECTOR_2SECTOR_3SECTOR_4SECTOR:
						if (arg4) map.getSectorsByTag(arg4, tagged_sectors);
						if (arg3) map.getSectorsByTag(arg3, tagged_sectors);
					case AS_TT_1SECTOR_2SECTOR:
						if (arg2) map.getSectorsByTag(arg2, tagged_sectors);
						if (tag ) map.getSectorsByTag(tag , tagged_sectors);
						break;
					case AS_TT_SECTOR_2IS3_LINE:
						if (arg2 == 3) map.getLinesById(tag, tagged_lines);
						else map.getSectorsByTag(tag, tagged_sectors);
						break;
					default:
						break;
				}
			}
		}
	}

	// Update map object properties panel if the hilight changed
	if (current != hilight_item && selection.size() == 0) {
		switch (edit_mode) {
		case MODE_VERTICES: theMapEditor->propsPanel()->openObject(map.getVertex(hilight_item)); break;
		case MODE_LINES: theMapEditor->propsPanel()->openObject(map.getLine(hilight_item)); break;
		case MODE_SECTORS: theMapEditor->propsPanel()->openObject(map.getSector(hilight_item)); break;
		case MODE_THINGS: theMapEditor->propsPanel()->openObject(map.getThing(hilight_item)); break;
		default: break;
		}
	}

	return current != hilight_item;
}

void MapEditor::selectionUpdated() {
	// Open selected objects in properties panel
	vector<MapObject*> objects;

	if (edit_mode == MODE_VERTICES) {
		for (unsigned a = 0; a < selection.size(); a++)
			objects.push_back(map.getVertex(selection[a]));
	}
	else if (edit_mode == MODE_LINES) {
		for (unsigned a = 0; a < selection.size(); a++)
			objects.push_back(map.getLine(selection[a]));
	}
	else if (edit_mode == MODE_SECTORS) {
		for (unsigned a = 0; a < selection.size(); a++)
			objects.push_back(map.getSector(selection[a]));
	}
	else if (edit_mode == MODE_THINGS) {
		for (unsigned a = 0; a < selection.size(); a++)
			objects.push_back(map.getThing(selection[a]));
	}

	theMapEditor->propsPanel()->openObjects(objects);
}

void MapEditor::clearSelection() {
	if (canvas) canvas->itemsSelected(selection, false);
	selection.clear();
	theMapEditor->propsPanel()->openObject(NULL);
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

	addEditorMessage(S_FMT("Selected all %d %s", selection.size(), CHR(getModeString())));

	if (canvas)
		canvas->itemsSelected(selection);

	selectionUpdated();
}

bool MapEditor::selectCurrent(bool clear_none) {
	// If nothing is hilighted
	if (hilight_item == -1) {
		// Clear selection if specified
		if (clear_none) {
			if (canvas) canvas->itemsSelected(selection, false);
			selection.clear();
			selectionUpdated();
			addEditorMessage("Selection cleared");
		}

		return false;
	}

	// Otherwise, check if item is in selection
	for (unsigned a = 0; a < selection.size(); a++) {
		if (selection[a] == hilight_item) {
			// Already selected, deselect
			selection.erase(selection.begin() + a);
			if (canvas) canvas->itemSelected(hilight_item, false);
			selectionUpdated();
			return true;
		}
	}

	// Not already selected, add to selection
	selection.push_back(hilight_item);
	if (canvas) canvas->itemSelected(hilight_item, true);

	selectionUpdated();

	return true;
}

bool MapEditor::selectWithin(double xmin, double ymin, double xmax, double ymax, bool add) {
	// Select depending on editing mode
	bool selected;
	vector<int> nsel;
	vector<int> asel;

	// Vertices
	if (edit_mode == MODE_VERTICES) {
		// Go through vertices
		double x, y;
		for (unsigned a = 0; a < map.vertices.size(); a++) {
			// Check if already selected
			if (std::find(selection.begin(), selection.end(), a) != selection.end())
				selected = true;
			else
				selected = false;

			// Get position
			x = map.vertices[a]->xPos();
			y = map.vertices[a]->yPos();

			// Select if vertex is within bounds
			if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
				if (selected)
					asel.push_back(a);
				else
					nsel.push_back(a);
			}
		}
	}

	// Lines
	else if (edit_mode == MODE_LINES) {
		// Go through lines
		MapLine* line;
		double x1, y1, x2, y2;
		for (unsigned a = 0; a < map.lines.size(); a++) {
			// Check if already selected
			if (std::find(selection.begin(), selection.end(), a) != selection.end())
				selected = true;
			else
				selected = false;

			// Get vertex positions
			line = map.lines[a];
			x1 = line->v1()->xPos();
			y1 = line->v1()->yPos();
			x2 = line->v2()->xPos();
			y2 = line->v2()->yPos();

			// Select if both vertices are within bounds
			if (xmin <= x1 && x1 <= xmax && ymin <= y1 && y1 <= ymax &&
				xmin <= x2 && x2 <= xmax && ymin <= y2 && y2 <= ymax) {
				if (selected)
					asel.push_back(a);
				else
					nsel.push_back(a);
			}
		}
	}

	// Sectors
	else if (edit_mode == MODE_SECTORS) {
		// Go through sectors
		fpoint2_t pmin(xmin, ymin);
		fpoint2_t pmax(xmax, ymax);
		for (unsigned a = 0; a < map.sectors.size(); a++) {
			// Check if already selected
			if (std::find(selection.begin(), selection.end(), a) != selection.end())
				selected = true;
			else
				selected = false;

			// Check if sector's bbox fits within the selection box
			if (map.sectors[a]->boundingBox().is_within(pmin, pmax)) {
				if (selected)
					asel.push_back(a);
				else
					nsel.push_back(a);
			}
		}
	}

	// Things
	else if (edit_mode == MODE_THINGS) {
		// Go through things
		double x, y;
		for (unsigned a = 0; a < map.things.size(); a++) {
			// Check if already selected
			if (std::find(selection.begin(), selection.end(), a) != selection.end())
				selected = true;
			else
				selected = false;

			// Get position
			x = map.things[a]->xPos();
			y = map.things[a]->yPos();

			// Select if thing is within bounds
			if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
				if (selected)
					asel.push_back(a);
				else
					nsel.push_back(a);
			}
		}
	}


	// Clear selection if anything was within the box
	if (!add && (nsel.size() > 0 || asel.size() > 0))
		clearSelection();

	// Update selection
	if (!add) {
		for (unsigned a = 0; a < asel.size(); a++)
			selection.push_back(asel[a]);
	}
	for (unsigned a = 0; a < nsel.size(); a++)
		selection.push_back(nsel[a]);

	if (add)
		addEditorMessage(S_FMT("Selected %d %s", asel.size(), CHR(getModeString())));
	else
		addEditorMessage(S_FMT("Selected %d %s", selection.size(), CHR(getModeString())));

	// Animate newly selected items
	if (canvas && nsel.size() > 0) canvas->itemsSelected(nsel);

	selectionUpdated();

	return (nsel.size() > 0);
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

MapObject* MapEditor::getHilightedObject() {
	if (edit_mode == MODE_VERTICES)
		return getHilightedVertex();
	else if (edit_mode == MODE_LINES)
		return getHilightedLine();
	else if (edit_mode == MODE_SECTORS)
		return getHilightedSector();
	else if (edit_mode == MODE_THINGS)
		return getHilightedThing();
	else
		return NULL;
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

void MapEditor::getSelectedObjects(vector<MapObject*>& list) {
	// Go through selection
	if (selection.size() > 0) {
		for (unsigned a = 0; a < selection.size(); a++) {
			if (edit_mode == MODE_VERTICES)
				list.push_back(map.getVertex(selection[a]));
			else if (edit_mode == MODE_LINES)
				list.push_back(map.getLine(selection[a]));
			else if (edit_mode == MODE_SECTORS)
				list.push_back(map.getSector(selection[a]));
			else if (edit_mode == MODE_THINGS)
				list.push_back(map.getThing(selection[a]));
		}
	}
	else {
		if (edit_mode == MODE_VERTICES)
			list.push_back(map.getVertex(hilight_item));
			else if (edit_mode == MODE_LINES)
				list.push_back(map.getLine(hilight_item));
			else if (edit_mode == MODE_SECTORS)
				list.push_back(map.getSector(hilight_item));
			else if (edit_mode == MODE_THINGS)
				list.push_back(map.getThing(hilight_item));
	}
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

	addEditorMessage(S_FMT("Grid Size: %dx%d", (int)gridSize(), (int)gridSize()));
}

void MapEditor::decrementGrid() {
	// Non-integral grid size disabled for now
	gridsize--;
	if (gridsize < 4)
		gridsize = 4;

	addEditorMessage(S_FMT("Grid Size: %dx%d", (int)gridSize(), (int)gridSize()));
}

double MapEditor::snapToGrid(double position) {
	// This won't work with non-integer grid sizes for now
	int upper, lower;

	for (int i = position; i >= (position - gridSize()); i--) {
		if ((i % (int)gridSize()) == 0) {
			lower = i;
			break;
		}
	}

	for (int i = position; i < (position + gridSize()); i++) {
		if ((i % (int)gridSize()) == 0) {
			upper = i;
			break;
		}
	}

	double mid = lower + ((upper - lower) / 2.0);

	if (position > mid)
		return upper;
	else
		return lower;
}

bool MapEditor::beginMove(fpoint2_t mouse_pos) {
	// Check if we have any selection or hilight
	if (selection.size() == 0 && hilight_item == -1)
		return false;

	// Begin move operation
	move_origin = mouse_pos;

	// Create list of objects to move
	if (selection.size() == 0)
		move_items.push_back(hilight_item);
	else {
		for (unsigned a = 0; a < selection.size(); a++)
			move_items.push_back(selection[a]);
	}

	// Get list of vertices being moved (if any)
	vector<MapVertex*> move_verts;
	if (edit_mode != MODE_THINGS) {
		// Vertices mode
		if (edit_mode == MODE_VERTICES) {
			for (unsigned a = 0; a < move_items.size(); a++)
				move_verts.push_back(map.getVertex(move_items[a]));
		}

		// Lines mode
		else if (edit_mode == MODE_LINES) {
			for (unsigned a = 0; a < move_items.size(); a++) {
				// Duplicate vertices shouldn't matter here
				move_verts.push_back(map.getLine(move_items[a])->v1());
				move_verts.push_back(map.getLine(move_items[a])->v2());
			}
		}

		// Sectors mode
		else if (edit_mode == MODE_SECTORS) {
			for (unsigned a = 0; a < move_items.size(); a++)
				map.getVerticesOfSector(move_items[a], move_verts);
		}
	}

	// Filter out map objects being moved
	if (edit_mode == MODE_THINGS) {
		// Filter moving things
		for (unsigned a = 0; a < move_items.size(); a++)
			map.getThing(move_items[a])->filter(true);
	}
	else {
		// Filter moving lines
		for (unsigned a = 0; a < move_verts.size(); a++) {
			for (unsigned l = 0; l < move_verts[a]->nConnectedLines(); l++)
				move_verts[a]->connectedLine(l)->filter(true);
		}
	}

	return true;
}

void MapEditor::doMove(fpoint2_t mouse_pos) {
	// Special case: single vertex or thing
	if (move_items.size() == 1 && (edit_mode == MODE_VERTICES || edit_mode == MODE_THINGS)) {
		// Get new position
		double nx = snapToGrid(mouse_pos.x);
		double ny = snapToGrid(mouse_pos.y);

		// Update move vector
		if (edit_mode == MODE_VERTICES)
			move_vec.set(nx - map.getVertex(move_items[0])->xPos(), ny - map.getVertex(move_items[0])->yPos());
		else if (edit_mode == MODE_THINGS)
			move_vec.set(nx - map.getThing(move_items[0])->xPos(), ny - map.getThing(move_items[0])->yPos());

		return;
	}

	// Get amount moved
	double dx = mouse_pos.x - move_origin.x;
	double dy = mouse_pos.y - move_origin.y;

	// Update move vector
	move_vec.set(snapToGrid(dx), snapToGrid(dy));
}

void MapEditor::endMove() {
	// Move depending on edit mode
	if (edit_mode == MODE_THINGS) {
		// Move things
		for (unsigned a = 0; a < move_items.size(); a++) {
			MapThing* t = map.getThing(move_items[a]);
			map.moveThing(move_items[a], t->xPos() + move_vec.x, t->yPos() + move_vec.y);
		}
	}
	else {
		// Any other edit mode we're technically moving vertices

		// Get list of vertices being moved
		bool* move_verts = new bool[map.nVertices()];
		memset(move_verts, 0, map.nVertices());

		if (edit_mode == MODE_VERTICES) {
			for (unsigned a = 0; a < move_items.size(); a++)
				move_verts[move_items[a]] = true;
		}
		else if (edit_mode == MODE_LINES) {
			for (unsigned a = 0; a < move_items.size(); a++) {
				MapLine* line = map.getLine(move_items[a]);
				if (line->v1()) move_verts[map.vertexIndex(line->v1())] = true;
				if (line->v2()) move_verts[map.vertexIndex(line->v2())] = true;
			}
		}
		else if (edit_mode == MODE_SECTORS) {
			vector<MapVertex*> sv;
			for (unsigned a = 0; a < move_items.size(); a++)
				map.getVerticesOfSector(move_items[a], sv);

			for (unsigned a = 0; a < sv.size(); a++)
				move_verts[map.vertexIndex(sv[a])] = true;
		}

		// Move vertices
		vector<fpoint2_t> merge_points;
		for (unsigned a = 0; a < map.nVertices(); a++) {
			if (!move_verts[a])
				continue;
			fpoint2_t np(map.getVertex(a)->xPos() + move_vec.x, map.getVertex(a)->yPos() + move_vec.y);
			map.moveVertex(a, np.x, np.y);
			merge_points.push_back(np);
		}

		// Merge vertices and split lines
		for (unsigned a = 0; a < merge_points.size(); a++) {
			MapVertex* v = map.mergeVerticesPoint(merge_points[a].x, merge_points[a].y);
			if (v) map.splitLinesAt(v, 1);
		}
	}

	// Un-filter objects
	for (unsigned a = 0; a < map.nLines(); a++)
		map.getLine(a)->filter(false);
	for (unsigned a = 0; a < map.nThings(); a++)
		map.getThing(a)->filter(false);

	// Clear moving items
	move_items.clear();

	// Remove any resulting zero-length lines
	map.removeZeroLengthLines();

	// Update map item indices
	map.refreshIndices();
}

void MapEditor::copyProperties() {
	// Sectors mode
	if (edit_mode == MODE_SECTORS) {
		// Copy selection/hilight properties
		if (selection.size() > 0)
			copy_sector->copyPropsFrom(map.getSector(selection[0]));
		else if (hilight_item >= 0)
			copy_sector->copyPropsFrom(map.getSector(hilight_item));
		else
			return;

		// Editor message
		addEditorMessage("Copied sector properties");
	}
}

void MapEditor::pasteProperties() {
	// Sectors mode
	if (edit_mode == MODE_SECTORS) {
		// Do nothing if no properties have been copied
		if (copy_sector->ceilingTexture().IsEmpty())
			return;

		// Paste properties to selection/hilight
		if (selection.size() > 0) {
			for (unsigned a = 0; a < selection.size(); a++)
				map.getSector(selection[a])->copyPropsFrom(copy_sector);
		}
		else if (hilight_item >= 0)
			map.getSector(hilight_item)->copyPropsFrom(copy_sector);
		else
			return;

		// Editor message
		addEditorMessage("Pasted sector properties");
	}
}

void MapEditor::splitLine(double x, double y, double min_dist) {
	// Get the closest line
	int lindex = map.nearestLine(x, y, min_dist);
	MapLine* line = map.getLine(lindex);

	// Do nothing if no line is close enough
	if (!line)
		return;

	// Get closest point on the line
	fpoint2_t closest = MathStuff::closestPointOnLine(x, y, line->x1(), line->y1(), line->x2(), line->y2());

	// Create vertex there
	MapVertex* vertex = map.createVertex(closest.x, closest.y);
	int vindex = map.vertexIndex(vertex);

	// Do line split
	map.splitLine(lindex, vindex);
}

void MapEditor::changeSectorHeight(int amount, bool floor, bool ceiling) {
	// Do nothing if not in sectors mode
	if (edit_mode != MODE_SECTORS)
		return;

	// Get selected sectors (if any)
	vector<MapSector*> selection;
	getSelectedSectors(selection);

	// Do nothing if no selection or hilight
	if (selection.size() == 0)
		return;

	// If we're modifying both heights, take sector_mode into account
	if (floor && ceiling) {
		if (sector_mode == SECTOR_FLOOR)
			ceiling = false;
		if (sector_mode == SECTOR_CEILING)
			floor = false;
	}

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		// Change floor height
		if (floor) {
			int height = selection[a]->intProperty("heightfloor");
			selection[a]->setIntProperty("heightfloor", height + amount);
		}

		// Change ceiling height
		if (ceiling) {
			int height = selection[a]->intProperty("heightceiling");
			selection[a]->setIntProperty("heightceiling", height + amount);
		}
	}

	// Add editor message
	string what = "";
	if (floor && !ceiling)
		what = "Floor";
	else if (!floor && ceiling)
		what = "Ceiling";
	else
		what = "Floor and ceiling";
	string inc = "increased";
	if (amount < 0)
		inc = "decreased";
	if (amount < 0)
		amount = -amount;
	addEditorMessage(S_FMT("%s height %s by %d", CHR(what), CHR(inc), amount));

	// Update display
	updateDisplay();
}

void MapEditor::changeSectorLight(int amount) {
	// Do nothing if not in sectors mode
	if (edit_mode != MODE_SECTORS)
		return;

	// Get selected sectors (if any)
	vector<MapSector*> selection;
	getSelectedSectors(selection);

	// Do nothing if no selection or hilight
	if (selection.size() == 0)
		return;

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		// Get modified light value
		int light = selection[a]->intProperty("lightlevel");
		if (light == 255 && amount < -1)
			light = 256;
		light += amount;

		// Clamp it 0-255
		if (light > 255)
			light = 255;
		if (light < 0)
			light = 0;

		// Change light level
		selection[a]->setIntProperty("lightlevel", light);
	}

	// Add editor message
	if (amount < 0) {
		amount = -amount;
		addEditorMessage(S_FMT("Light level decreased by %d", amount));
	}
	else
		addEditorMessage(S_FMT("Light level increased by %d", amount));

	// Update display
	updateDisplay();
}

void MapEditor::changeThingType(int newtype) {
	// Do nothing if not in things mode
	if (edit_mode != MODE_THINGS)
		return;

	// Get selected things (if any)
	vector<MapThing*> selection;
	getSelectedThings(selection);

	// Do nothing if no selection or hilight
	if (selection.size() == 0)
		return;

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++)
		selection[a]->setIntProperty("type", newtype);

	// Add editor message
	string type_name = theGameConfiguration->thingType(newtype)->getName();
	if (selection.size() == 1)
		addEditorMessage(S_FMT("Changed type to \"%s\"", CHR(type_name)));
	else
		addEditorMessage(S_FMT("Changed %d things to type \"%s\"", selection.size(), CHR(type_name)));

	// Update display
	updateDisplay();
}

void MapEditor::thingQuickAngle(fpoint2_t mouse_pos) {
	// Do nothing if not in things mode
	if (edit_mode != MODE_THINGS)
		return;

	// If selection is empty, check for hilight
	if (selection.size() == 0 && hilight_item >= 0) {
		map.thingSetAnglePoint(hilight_item, mouse_pos);
		return;
	}

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++)
		map.thingSetAnglePoint(selection[a], mouse_pos);
}

void MapEditor::createObject(double x, double y) {
	// Vertices mode
	if (edit_mode == MODE_VERTICES) {
		// If there are less than 2 vertices currently selected, just create a vertex at x,y
		if (selection.size() < 2)
			createVertex(x, y);
		else {
			// Otherwise, create lines between selected vertices
			for (unsigned a = 0; a < selection.size() - 1; a++)
				map.createLine(map.getVertex(selection[a]), map.getVertex(selection[a+1]));

			// Editor message
			addEditorMessage(S_FMT("Created %d line(s)", selection.size() - 1));

			// Then switch to lines mode
			setEditMode(MODE_LINES);
		}

		return;
	}

	// Sectors mode
	if (edit_mode == MODE_SECTORS) {
		createSector(x, y);
		return;
	}

	// Things mode
	if (edit_mode == MODE_THINGS) {
		createThing(x, y);
		return;
	}
}

void MapEditor::createVertex(double x, double y) {
	// Snap coordinates to grid if necessary
	if (grid_snap) {
		x = snapToGrid(x);
		y = snapToGrid(y);
	}

	// Create vertex
	MapVertex* vertex = map.createVertex(x, y, 2);

	// Editor message
	if (vertex)
		addEditorMessage(S_FMT("Created vertex at (%d, %d)", (int)vertex->xPos(), (int)vertex->yPos()));
}

void MapEditor::createThing(double x, double y) {
	// Snap coordinates to grid if necessary
	if (grid_snap) {
		x = snapToGrid(x);
		y = snapToGrid(y);
	}

	// Create thing
	MapThing* thing = map.createThing(x, y);

	// Editor message
	if (thing)
		addEditorMessage(S_FMT("Created thing at (%d, %d)", (int)thing->xPos(), (int)thing->yPos()));
}

void MapEditor::createSector(double x, double y) {
	// Find nearest line
	int nearest = map.nearestLine(x, y, 99999999);
	MapLine* line = map.getLine(nearest);

	// Determine side
	double side = MathStuff::lineSide(x, y, line->x1(), line->y1(), line->x2(), line->y2());

	// Get sector to copy if we're in sectors mode
	MapSector* sector_copy = NULL;
	if (edit_mode == MODE_SECTORS && selection.size() > 0)
		sector_copy = map.getSector(selection[0]);

	// Run sector builder
	SectorBuilder builder;
	bool ok;
	if (side >= 0)
		ok = builder.buildSector(&map, line, true, sector_copy);
	else
		ok = builder.buildSector(&map, line, false, sector_copy);

	// Set some temporary sector defaults if needed
	if (!sector_copy) {
		MapSector* n_sector = map.getSector(map.nSectors()-1);
		if (n_sector->ceilingTexture().IsEmpty()) {
			n_sector->setStringProperty("texturefloor", "MFLR8_1");
			n_sector->setStringProperty("textureceiling", "MFLR8_1");
			n_sector->setIntProperty("heightceiling", 128);
			n_sector->setIntProperty("lightlevel", 160);
		}
	}

	// Editor message
	if (ok)
		addEditorMessage(S_FMT("Created sector #%d", map.nSectors() - 1));
	else
		addEditorMessage("Sector creation failed: " + builder.getError());

	// Refresh map canvas
	canvas->forceRefreshRenderer();
}

void MapEditor::deleteObject() {
	// Vertices mode
	if (edit_mode == MODE_VERTICES) {
		// Get selected vertices
		vector<MapVertex*> verts;
		getSelectedVertices(verts);
		int index = -1;
		if (verts.size() == 1)
			index = verts[0]->getIndex();

		// Delete them (if any)
		for (unsigned a = 0; a < verts.size(); a++)
			map.removeVertex(verts[a]);

		// Editor message
		if (verts.size() == 1)
			addEditorMessage(S_FMT("Deleted vertex #%d", index));
		else if (verts.size() > 1)
			addEditorMessage(S_FMT("Deleted %d vertices", verts.size()));
	}

	// Lines mode
	else if (edit_mode == MODE_LINES) {
		// Get selected lines
		vector<MapLine*> lines;
		getSelectedLines(lines);
		int index = -1;
		if (lines.size() == 1)
			index = lines[0]->getIndex();

		// Delete them (if any)
		for (unsigned a = 0; a < lines.size(); a++)
			map.removeLine(lines[a]);

		// Editor message
		if (lines.size() == 1)
			addEditorMessage(S_FMT("Deleted line #%d", index));
		else if (lines.size() > 1)
			addEditorMessage(S_FMT("Deleted %d lines", lines.size()));
	}

	// Sectors mode
	else if (edit_mode == MODE_SECTORS) {
		// Get selected sectors
		vector<MapSector*> sectors;
		getSelectedSectors(sectors);
		int index = -1;
		if (sectors.size() == 1)
			index = sectors[0]->getIndex();

		// Delete them (if any), and keep a list of connected sides
		vector<MapSide*> connected_sides;
		for (unsigned a = 0; a < sectors.size(); a++) {
			for (unsigned s = 0; s < sectors[a]->connectedSides().size(); s++)
				connected_sides.push_back(sectors[a]->connectedSides()[s]);

			map.removeSector(sectors[a]);
		}

		// Remove all connected sides
		for (unsigned a = 0; a < connected_sides.size(); a++) {
			// Before removing the side, check if we should flip the line
			MapLine* line = connected_sides[a]->getParentLine();
			if (connected_sides[a] == line->s1() && line->s2())
				line->flip();

			map.removeSide(connected_sides[a]);
		}

		// Editor message
		if (sectors.size() == 1)
			addEditorMessage(S_FMT("Deleted sector #%d", index));
		else if (sectors.size() > 1)
			addEditorMessage(S_FMT("Deleted %d sector", sectors.size()));

		// Refresh map view
		theMapEditor->forceRefresh(true);
	}

	// Things mode
	else if (edit_mode == MODE_THINGS) {
		// Get selected things
		vector<MapThing*> things;
		getSelectedThings(things);
		int index = -1;
		if (things.size() == 1)
			index = things[0]->getIndex();

		// Delete them (if any)
		for (unsigned a = 0; a < things.size(); a++)
			map.removeThing(things[a]);

		// Editor message
		if (things.size() == 1)
			addEditorMessage(S_FMT("Deleted thing #%d", index));
		else if (things.size() > 1)
			addEditorMessage(S_FMT("Deleted %d things", things.size()));
	}

	// Remove detached vertices
	map.removeDetachedVertices();

	// Clear hilight and selection
	selection.clear();
	hilight_item = -1;
}

fpoint2_t MapEditor::lineDrawPoint(unsigned index) {
	// Check index
	if (index >= draw_points.size())
		return fpoint2_t(0, 0);
	
	return draw_points[index];
}

bool MapEditor::addLineDrawPoint(fpoint2_t point, bool nearest) {
	// Snap to nearest vertex if necessary
	if (nearest) {
		int vertex = map.nearestVertex(point.x, point.y);
		if (vertex >= 0) {
			point.x = map.getVertex(vertex)->xPos();
			point.y = map.getVertex(vertex)->yPos();
		}
	}

	// Otherwise, snap to grid if necessary
	else if (grid_snap) {
		point.x = snapToGrid(point.x);
		point.y = snapToGrid(point.y);
	}

	// Check if this is the same as the last point
	if (draw_points.size() > 0 && point.x == draw_points.back().x && point.y == draw_points.back().y) {
		// End line drawing
		endLineDraw(true);
		return true;
	}

	// Add point
	draw_points.push_back(point);

	// Check if first and last points match
	if (draw_points.size() > 1 && point.x == draw_points[0].x && point.y == draw_points[0].y) {
		endLineDraw(true);
		return true;
	}

	return false;
}

void MapEditor::removeLineDrawPoint() {
	draw_points.pop_back();
}

unsigned MapEditor::numEditorMessages() {
	return editor_messages.size();
}

struct me_ls_t {
	MapLine*	line;
	bool		front;
	bool		ignore;
	me_ls_t(MapLine* line, bool front) { this->line = line; this->front = front; ignore = false; }
};

void MapEditor::endLineDraw(bool apply) {
	// Check if we want to 'apply' the line draw (ie. create the lines)
	if (apply && draw_points.size() > 1) {
		// Create vertices
		for (unsigned a = 0; a < draw_points.size(); a++)
			map.createVertex(draw_points[a].x, draw_points[a].y, 1);

		// Create lines
		unsigned nl_start = map.nLines();
		for (unsigned a = 0; a < draw_points.size() - 1; a++) {
			// Check for intersections
			vector<fpoint2_t> intersect = map.cutLines(draw_points[a].x, draw_points[a].y, draw_points[a+1].x, draw_points[a+1].y);
			//wxLogMessage("%d intersect points", intersect.size());

			// Create line normally if no intersections
			if (intersect.size() == 0)
				map.createLine(draw_points[a].x, draw_points[a].y, draw_points[a+1].x, draw_points[a+1].y, 1);
			else {
				// Intersections exist, create multiple lines between intersection points

				// From first point to first intersection
				map.createLine(draw_points[a].x, draw_points[a].y, intersect[0].x, intersect[0].y, 1);

				// Between intersection points
				for (unsigned p = 0; p < intersect.size() - 1; p++)
					map.createLine(intersect[p].x, intersect[p].y, intersect[p+1].x, intersect[p+1].y, 1);

				// From last intersection to next point
				map.createLine(intersect.back().x, intersect.back().y, draw_points[a+1].x, draw_points[a+1].y, 1);
			}
		}

		// Create a list of line sides (edges) to perform sector creation with
		vector<me_ls_t> edges;
		for (unsigned a = nl_start; a < map.nLines(); a++) {
			edges.push_back(me_ls_t(map.getLine(a), true));
			edges.push_back(me_ls_t(map.getLine(a), false));
		}
		
		// Build sectors
		SectorBuilder builder;
		int runs = 0;
		for (unsigned a = 0; a < edges.size(); a++) {
			// Skip if edge is ignored
			if (edges[a].ignore)
				continue;

			// Run sector builder on current edge
			builder.buildSector(&map, edges[a].line, edges[a].front);
			runs++;

			// Set some temporary sector defaults
			MapSector* n_sector = map.getSector(map.nSectors()-1);
			if (n_sector->ceilingTexture().IsEmpty()) {
				n_sector->setStringProperty("texturefloor", "MFLR8_1");
				n_sector->setStringProperty("textureceiling", "MFLR8_1");
				n_sector->setIntProperty("heightceiling", 128);
				n_sector->setIntProperty("lightlevel", 160);
			}

			// Ignore any subsequent edges that were part of the sector created
			for (unsigned e = a; e < edges.size(); e++) {
				if (edges[e].ignore)
					continue;

				for (unsigned b = 0; b < builder.nEdges(); b++) {
					if (edges[e].line == builder.getEdgeLine(b) &&
						edges[e].front == builder.edgeIsFront(b))
						edges[e].ignore = true;
				}
			}
		}

		wxLogMessage("Ran sector builder %d times", runs);

		// Check if any of the created lines should be flipped
		for (unsigned a = nl_start; a < map.nLines(); a++) {
			MapLine* line = map.getLine(a);
			if (line->backSector() && !line->frontSector())
				line->flip(true);
		}
	}

	// Clear draw points
	draw_points.clear();
}

string MapEditor::getEditorMessage(int index) {
	// Check index
	if (index < 0 || index >= (int)editor_messages.size())
		return "";

	return editor_messages[index].message;
}

long MapEditor::getEditorMessageTime(int index) {
	// Check index
	if (index < 0 || index >= (int)editor_messages.size())
		return -1;

	return theApp->runTimer() - editor_messages[index].act_time;
}

void MapEditor::addEditorMessage(string message) {
	// Remove oldest message if there are too many active
	if (editor_messages.size() >= 4)
		editor_messages.erase(editor_messages.begin());

	// Add message to list
	editor_msg_t msg;
	msg.message = message;
	msg.act_time = theApp->runTimer();
	editor_messages.push_back(msg);
}

string MapEditor::getModeString() {
	switch (edit_mode) {
	case MODE_VERTICES: return "Vertices";
	case MODE_LINES: return "Lines";
	case MODE_SECTORS: return "Sectors";
	case MODE_THINGS: return "Things";
	default: return "Items";
	};
}

bool MapEditor::handleKeyBind(string key, fpoint2_t position) {
	// --- General keybinds ---

	// Increment grid
	if (key == "me2d_grid_inc")
		incrementGrid();

	// Decrement grid
	else if (key == "me2d_grid_dec")
		decrementGrid();

	// Select all
	else if (key == "select_all")
		selectAll();

	// Clear selection
	else if (key == "me2d_clear_selection") {
		clearSelection();
		addEditorMessage("Selection cleared");
	}

	// Lock/unlock hilight
	else if (key == "me2d_lock_hilight") {
		// Toggle lock
		hilight_locked = !hilight_locked;

		// Add editor message
		if (hilight_locked)
			addEditorMessage("Locked current hilight");
		else
			addEditorMessage("Unlocked hilight");

		return true;
	}

	// --- Line mode keybinds ---
	else if (key.StartsWith("me2d_line") && edit_mode == MODE_LINES) {
		// Split line
		if (key == "me2d_line_split")	splitLine(position.x, position.y);
	}

	// --- Sector mode keybinds ---
	else if (key.StartsWith("me2d_sector") && edit_mode == MODE_SECTORS) {
		// Height changes
		if		(key == "me2d_sector_floor_up8")	changeSectorHeight(8, true, false);
		else if (key == "me2d_sector_floor_up")		changeSectorHeight(1, true, false);
		else if (key == "me2d_sector_floor_down8")	changeSectorHeight(-8, true, false);
		else if (key == "me2d_sector_floor_down")	changeSectorHeight(-1, true, false);
		else if (key == "me2d_sector_ceil_up8")		changeSectorHeight(8, false, true);
		else if (key == "me2d_sector_ceil_up")		changeSectorHeight(1, false, true);
		else if (key == "me2d_sector_ceil_down8")	changeSectorHeight(-8, false, true);
		else if (key == "me2d_sector_ceil_down")	changeSectorHeight(-1, false, true);
		else if (key == "me2d_sector_height_up8")	changeSectorHeight(8, true, true);
		else if (key == "me2d_sector_height_up")	changeSectorHeight(1, true, true);
		else if (key == "me2d_sector_height_down8")	changeSectorHeight(-8, true, true);
		else if (key == "me2d_sector_height_down")	changeSectorHeight(-1, true, true);

		// Light changes
		else if (key == "me2d_sector_light_up16")	changeSectorLight(16);
		else if (key == "me2d_sector_light_up")		changeSectorLight(1);
		else if (key == "me2d_sector_light_down16")	changeSectorLight(-16);
		else if (key == "me2d_sector_light_down")	changeSectorLight(-1);
	}

	// Not handled
	return false;
}

void MapEditor::updateDisplay() {
	// Update map object properties panel
	vector<MapObject*> selection;
	getSelectedObjects(selection);
	theMapEditor->propsPanel()->openObjects(selection);

	// Update canvas info overlay
	if (canvas) {
		canvas->updateInfoOverlay();
		canvas->Refresh();
	}
}




CONSOLE_COMMAND(m_show_item, 1) {
	int index = atoi(CHR(args[0]));
	theMapEditor->mapEditor().showItem(index);
}

CONSOLE_COMMAND(m_vertex_attached, 1) {
	MapVertex* vertex = theMapEditor->mapEditor().getMap().getVertex(atoi(CHR(args[0])));
	if (vertex) {
		wxLogMessage("Attached lines:");
		for (unsigned a = 0; a < vertex->nConnectedLines(); a++)
			wxLogMessage("Line #%d", vertex->connectedLine(a)->getIndex());
	}
}
