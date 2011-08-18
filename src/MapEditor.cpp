
#include "Main.h"
#include "MapEditor.h"
#include "ColourConfiguration.h"
#include "ArchiveManager.h"
#include "WxStuff.h"
#include "MapEditorWindow.h"

double grid_sizes[] = { 0.05, 0.1, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };

EXTERN_CVAR(Int, vertex_size)

MapEditor::MapEditor() {
	// Init variables
	edit_mode = MODE_LINES;
	hilight_item = -1;
	gridsize = 9;
}

MapEditor::~MapEditor() {
}

double MapEditor::gridSize() {
	return grid_sizes[gridsize];
}

void MapEditor::setEditMode(int mode) {
	edit_mode = mode;
	updateHilight();
	selection.clear();
}

bool MapEditor::openMap(Archive::mapdesc_t map) {
	wxLogMessage("Opening map %s", CHR(map.name));
	return this->map.readMap(map);
}

void MapEditor::drawVertices(double xmin, double ymin, double xmax, double ymax) {
	// Set to vertex colour
	ColourConfiguration::getColour("map_vertex").set_gl();

	// Go through vertices
	glBegin(GL_POINTS);
	double x, y;
	for (unsigned a = 0; a < map.vertices.size(); a++) {
		// Get vertex position
		x = map.vertices[a]->xPos();
		y = map.vertices[a]->yPos();

		// Draw if within limits
		if (xmin <= x && x <= xmax && ymin <= y && y <= ymax)
			glVertex2d(x, y);
	}
	glEnd();
}

void MapEditor::drawLines(double xmin, double ymin, double xmax, double ymax, bool show_direction) {
	// Temp for now
	float fade_coeff = 0.5f;

	// Get line colours
	rgba_t col_background = ColourConfiguration::getColour("map_background");
	rgba_t col_line_normal = ColourConfiguration::getColour("map_line_normal");
	rgba_t col_line_special = ColourConfiguration::getColour("map_line_special");
	
	// Draw all lines
	rgba_t col;
	MapLine* line = NULL;
	double x1, y1, x2, y2;
	for (unsigned a = 0; a < map.lines.size(); a++) {
		// Get line info
		line = map.lines[a];
		x1 = line->v1()->xPos();
		y1 = line->v1()->yPos();
		x2 = line->v2()->xPos();
		y2 = line->v2()->yPos();

		// Skip if outside limits (quick check, will still draw some lines outside the screen)
		if ((x1 > xmax && x2 > xmax) ||
			(x1 < xmin && x2 < xmin) ||
			(y1 > ymax && y2 > ymax) ||
			(y1 < ymin && y2 < ymin))
			continue;
		
		// Check for special line
		if ((int)line->prop("special") > 0)
			col.set(col_line_special);
		else
			col.set(col_line_normal);
		
		// Check for two-sided line
		if (line->s2())
			col.set(col.r*fade_coeff+col_background.r*(1.0-fade_coeff),
					col.g*fade_coeff+col_background.g*(1.0-fade_coeff),
					col.b*fade_coeff+col_background.b*(1.0-fade_coeff), 255, 0);
		
		// Set line colour
		col.set_gl();
		
		// Draw the line
		glBegin(GL_LINES);
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		glEnd();

		// Direction tab
		if (show_direction) {
			double xmid = x1 + ((x2 - x1) * 0.5);
			double ymid = y1 + ((y2 - y1) * 0.5);
			double x = (-(y2 - y1)) * 0.125;
			double y = (x2 - x1) * 0.125;

			glBegin(GL_LINES);
			glVertex2d(xmid, ymid);
			glVertex2d(xmid - x, ymid - y);
			glEnd();
		}
	}
}

void MapEditor::drawThings(double xmin, double ymin, double xmax, double ymax) {
	// Enable textures
	glEnable(GL_TEXTURE_2D);
	tex_thing.bind();
	rgba_t(255, 255, 255, 255, 0).set_gl();
	
	// Load thing texture
	// (temporary solution for now)
	if (!tex_thing.isLoaded()) {
		Archive* pres = theArchiveManager->programResourceArchive();
		ArchiveEntry* entry = pres->entryAtPath("images/thing.png");
		SImage image;
		image.open(entry->getMCData());
		tex_thing.setFilter(GLTexture::MIPMAP);
		tex_thing.loadImage(&image);
	}
	
	// Go through things
	MapThing* thing = NULL;
	double x, y;
	for (unsigned a = 0; a < map.things.size(); a++) {
		// Get thing info
		thing = map.things[a];
		x = thing->xPos();
		y = thing->yPos();
		
		// Ignore if outside of screen
		if (x < xmin || x > xmax || y < ymin || y > ymax)
			continue;
			
		// Translate to thing position
		glPushMatrix();
		glTranslated(x, y, 0);
		
		// Draw thing (32x32 for now)
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);	glVertex2d(-16, -16);
		glTexCoord2f(0.0f, 1.0f);	glVertex2d(-16, 16);
		glTexCoord2f(1.0f, 1.0f);	glVertex2d(16, 16);
		glTexCoord2f(1.0f, 0.0f);	glVertex2d(16, -16);
		glEnd();
		
		glPopMatrix();
	}
	
	// Disable textures
	glDisable(GL_TEXTURE_2D);
}

void MapEditor::drawMap(double xmin, double ymin, double xmax, double ymax) {
	// Draw lines
	drawLines(xmin, ymin, xmax, ymax, edit_mode == MODE_LINES);
	
	// Draw vertices (if in vertices mode)
	if (edit_mode == MODE_VERTICES)
		drawVertices(xmin, ymin, xmax, ymax);
		
	// Draw things (if in things mode)
	if (edit_mode == MODE_THINGS)
		drawThings(xmin, ymin, xmax, ymax);
}

void MapEditor::drawHilight(float flash_level) {
	// Check anything is hilighted
	if (hilight_item < 0)
		return;

	// Set hilight colour
	rgba_t col = ColourConfiguration::getColour("map_hilight");
	col.a *= flash_level;
	col.set_gl();

	glLineWidth(4.5f);
	glPointSize(vertex_size*1.5f);

	// Draw depending on mode
	if (edit_mode == MODE_VERTICES) {
		// Vertex
		glBegin(GL_POINTS);
		glVertex2d(map.vertices[hilight_item]->xPos(), map.vertices[hilight_item]->yPos());
		glEnd();
	}
	else if (edit_mode == MODE_LINES) {
		// Line
		MapLine* line = map.lines[hilight_item];
		double x1 = line->v1()->xPos();
		double y1 = line->v1()->yPos();
		double x2 = line->v2()->xPos();
		double y2 = line->v2()->yPos();
		glBegin(GL_LINES);
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		glEnd();

		// Direction tab
		double xmid = x1 + ((x2 - x1) * 0.5);
		double ymid = y1 + ((y2 - y1) * 0.5);
		double x = (-(y2 - y1)) * 0.125;
		double y = (x2 - x1) * 0.125;
		glBegin(GL_LINES);
		glVertex2d(xmid, ymid);
		glVertex2d(xmid - x, ymid - y);
		glEnd();
	}
	else if (edit_mode == MODE_SECTORS) {
		// Sector

		// Get all lines belonging to the hilighted sector
		vector<MapLine*> lines;
		map.getLinesOfSector(hilight_item, lines);

		// Draw hilight
		MapLine* line = NULL;
		for (unsigned a = 0; a < lines.size(); a++) {
			line = lines[a];
			if (!line) continue;

			// Draw line
			glBegin(GL_LINES);
			glVertex2d(line->v1()->xPos(), line->v1()->yPos());
			glVertex2d(line->v2()->xPos(), line->v2()->yPos());
			glEnd();
		}
	}
	else if (edit_mode == MODE_THINGS) {
		// Thing
		double x = map.things[hilight_item]->xPos();
		double y = map.things[hilight_item]->yPos();
		
		glBegin(GL_QUADS);
		glVertex2d(x - 16, y - 16);
		glVertex2d(x - 16, y + 16);
		glVertex2d(x + 16, y + 16);
		glVertex2d(x + 16, y - 16);
		glEnd();
	}

	glLineWidth(1.5f);
	glPointSize(vertex_size);
}

void MapEditor::drawSelection(double xmin, double ymin, double xmax, double ymax) {
	// Check anything is selected
	if (selection.size() == 0)
		return;

	// Set selection colour
	ColourConfiguration::getColour("map_selection").set_gl();

	glLineWidth(6.0f);
	glPointSize(vertex_size*1.5f);

	// Draw depending on mode
	if (edit_mode == MODE_VERTICES) {
		// Vertices
		double x, y;
		glBegin(GL_POINTS);
		for (unsigned a = 0; a < selection.size(); a++) {
			x = map.vertices[selection[a]]->xPos();
			y = map.vertices[selection[a]]->yPos();

			// Draw vertex selection if on screen
			if (xmin <= x && x <= xmax && ymin <= y && y <= ymax)
				glVertex2d(x, y);
		}
		glEnd();
	}
	else if (edit_mode == MODE_LINES) {
		// Lines
		MapLine* line;
		double x1, y1, x2, y2;
		glBegin(GL_LINES);
		for (unsigned a = 0; a < selection.size(); a++) {
			line = map.lines[selection[a]];
			x1 = line->v1()->xPos();
			y1 = line->v1()->yPos();
			x2 = line->v2()->xPos();
			y2 = line->v2()->yPos();

			// Skip if outside limits (quick check, will still draw some lines outside the screen)
			if ((x1 > xmax && x2 > xmax) ||
				(x1 < xmin && x2 < xmin) ||
				(y1 > ymax && y2 > ymax) ||
				(y1 < ymin && y2 < ymin))
				continue;

			// Draw line
			glVertex2d(x1, y1);
			glVertex2d(x2, y2);

			// Direction tab
			double xmid = x1 + ((x2 - x1) * 0.5);
			double ymid = y1 + ((y2 - y1) * 0.5);
			double x = (-(y2 - y1)) * 0.125;
			double y = (x2 - x1) * 0.125;
			glVertex2d(xmid, ymid);
			glVertex2d(xmid - x, ymid - y);
		}
		glEnd();
	}
	else if (edit_mode == MODE_SECTORS) {
		// Sectors

		// Get selected sector(s)
		vector<MapSector*> selected_sectors;
		for (unsigned a = 0; a < selection.size(); a++)
			selected_sectors.push_back(map.sectors[selection[a]]);

		// Go through map lines
		MapLine* line;
		double x1, y1, x2, y2;
		glBegin(GL_LINES);
		for (unsigned a = 0; a < map.lines.size(); a++) {
			line = map.lines[a];

			// Check if line is in a selected sector
			bool selected = false;
			for (unsigned b = 0; b < selected_sectors.size(); b++) {
				if (map.lineInSector(line, selected_sectors[b])) {
					selected = true;
					break;
				}
			}

			// Skip if not selected
			if (!selected)
				continue;

			// Skip if outside limits (quick check, will still draw some lines outside the screen)
			x1 = line->v1()->xPos();
			y1 = line->v1()->yPos();
			x2 = line->v2()->xPos();
			y2 = line->v2()->yPos();
			if ((x1 > xmax && x2 > xmax) ||
				(x1 < xmin && x2 < xmin) ||
				(y1 > ymax && y2 > ymax) ||
				(y1 < ymin && y2 < ymin))
				continue;

			// Draw line
			glVertex2d(x1, y1);
			glVertex2d(x2, y2);
		}
		glEnd();
	}
	else if (edit_mode == MODE_THINGS) {
		// Things
		double x, y;
		for (unsigned a = 0; a < selection.size(); a++) {
			x = map.things[selection[a]]->xPos();
			y = map.things[selection[a]]->yPos();

			// Draw thing selection if on screen
			if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
				glBegin(GL_QUADS);
				glVertex2d(x - 20, y - 20);
				glVertex2d(x - 20, y + 20);
				glVertex2d(x + 20, y + 20);
				glVertex2d(x + 20, y - 20);
				glEnd();
			}
		}
	}

	glLineWidth(1.5f);
	glPointSize(vertex_size);
}

bool MapEditor::updateHilight(double dist_scale) {
	int current = hilight_item;

	// Update hilighted object depending on mode
	string objtype = "";
	if (edit_mode == MODE_VERTICES) {
		hilight_item = map.nearestVertex(mouse_pos.x, mouse_pos.y, 32/dist_scale);
		objtype = "Vertex";
	}
	else if (edit_mode == MODE_LINES) {
		hilight_item = map.nearestLine(mouse_pos.x, mouse_pos.y, 32/dist_scale);
		objtype = "Line";
	}
	else if (edit_mode == MODE_SECTORS) {
		hilight_item = map.inSector(mouse_pos.x, mouse_pos.y);
		objtype = "Sector";
	}
	else if (edit_mode == MODE_THINGS) {
		hilight_item = map.nearestThing(mouse_pos.x, mouse_pos.y, 32);
		objtype = "Thing";
	}

	return current != hilight_item;
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
}

bool MapEditor::selectCurrent(bool clear_none) {
	// If nothing is hilighted
	if (hilight_item == -1) {
		// Clear selection if specified
		if (clear_none)
			selection.clear();

		return false;
	}

	// Otherwise, check if item is in selection
	for (unsigned a = 0; a < selection.size(); a++) {
		if (selection[a] == hilight_item) {
			// Already selected, deselect
			selection.erase(selection.begin() + a);
			return true;
		}
	}

	// Not already selected, add to selection
	selection.push_back(hilight_item);

	return true;
}

bool MapEditor::selectWithin(double xmin, double ymin, double xmax, double ymax) {
	// Select depending on editing mode
	bool new_sel = false;

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
				selection.push_back(a);
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
				selection.push_back(a);
				new_sel = true;
			}
		}
	}

	// Sectors
	else if (edit_mode == MODE_SECTORS) {
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
				selection.push_back(a);
				new_sel = true;
			}
		}
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
