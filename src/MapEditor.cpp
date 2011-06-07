
#include "Main.h"
#include "MapEditor.h"
#include "ColourConfiguration.h"

MapEditor::MapEditor() {
	// Init variables
	edit_mode = MODE_LINES;
	hilight_item = -1;
}

MapEditor::~MapEditor() {
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

	// Draw vertex hilight
	if (edit_mode == MODE_VERTICES && hilight_item >= 0 && hilight_item < (int)map.vertices.size() - 1) {
		rgba_t(255, 255, 0, 180, 1).set_gl();
		glPointSize(10.0f);
		glBegin(GL_POINTS);
		glVertex2d(map.vertices[hilight_item]->xPos(), map.vertices[hilight_item]->yPos());
		glEnd();
		glPointSize(6.0f);
	}
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

		// Check for hilight
		if (edit_mode == MODE_LINES && hilight_item == a) {
			glLineWidth(3.0f);
			ColourConfiguration::getColour("map_hilight").set_gl();
			glBegin(GL_LINES);
			glVertex2d(x1, y1);
			glVertex2d(x2, y2);
			glEnd();
			glLineWidth(1.5f);
		}
	}
}

void MapEditor::drawThings(double xmin, double ymin, double xmax, double ymax) {
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

	// Sectors mode stuff
	if (edit_mode == MODE_SECTORS) {
		// Check hilight
		if (hilight_item >= 0) {
			// Get all lines belonging to the hilighted sector
			vector<MapLine*> lines;
			map.getLinesOfSector(hilight_item, lines);

			// Set hilight colour etc
			glLineWidth(3.0f);
			ColourConfiguration::getColour("map_hilight").set_gl();

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

			glLineWidth(1.5f);
		}
	}
}

bool MapEditor::updateHilight() {
	int current = hilight_item;

	// Update hilighted object depending on mode
	if (edit_mode == MODE_VERTICES)
		hilight_item = map.nearestVertex(mouse_pos.x, mouse_pos.y);
	else if (edit_mode == MODE_LINES)
		hilight_item = map.nearestLine(mouse_pos.x, mouse_pos.y);
	else if (edit_mode == MODE_SECTORS)
		hilight_item = map.inSector(mouse_pos.x, mouse_pos.y);
	else if (edit_mode == MODE_THINGS)
		hilight_item = map.nearestThing(mouse_pos.x, mouse_pos.y);

	return current != hilight_item;
}
