
#include "Main.h"
#include "MapEditor.h"

// Entirely temporary
rgba_t col_background(0, 0, 0, 255, 0);
rgba_t col_vertex(140, 140, 255, 255, 0);
rgba_t col_line_normal(255, 255, 255, 255, 0);
rgba_t col_line_special(100, 120, 255, 255, 0);
rgba_t col_line_invalid(255, 0, 0, 255, 0);

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

void MapEditor::drawVertices() {
	// Set to vertex colour
	col_vertex.set_gl();

	// Draw all vertices for now
	glBegin(GL_POINTS);
	for (unsigned a = 0; a < map.vertices.size(); a++)
		glVertex2d(map.vertices[a]->xPos(), map.vertices[a]->yPos());
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

void MapEditor::drawLines(bool show_direction) {
	// Setup opengl line properties
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.5f);

	// Temp for now
	float fade_coeff = 0.5f;
	
	// Draw all lines
	rgba_t col;
	MapLine* line = NULL;
	for (unsigned a = 0; a < map.lines.size(); a++) {
		line = map.lines[a];
		
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
		glVertex2d(line->v1()->xPos(), line->v1()->yPos());
		glVertex2d(line->v2()->xPos(), line->v2()->yPos());
		glEnd();

		// Check for hilight
		if (edit_mode == MODE_LINES && hilight_item == a) {
			glLineWidth(3.0f);
			rgba_t(255, 255, 0, 180, 1).set_gl();
			glBegin(GL_LINES);
			glVertex2d(line->v1()->xPos(), line->v1()->yPos());
			glVertex2d(line->v2()->xPos(), line->v2()->yPos());
			glEnd();
			glLineWidth(1.5f);
		}
	}
}

void MapEditor::drawThings() {
}

void MapEditor::drawMap() {
	// Draw lines
	drawLines(edit_mode == MODE_LINES);
	
	// Draw vertices (if in vertices mode)
	if (edit_mode == MODE_VERTICES)
		drawVertices();
		
	// Draw things (if in things mode)
	if (edit_mode == MODE_THINGS)
		drawThings();

	// Sectors mode stuff
	if (edit_mode == MODE_SECTORS) {
		// Check hilight
		if (hilight_item >= 0) {
			// Get all lines belonging to the hilighted sector
			vector<MapLine*> lines;
			map.getLinesOfSector(hilight_item, lines);

			// Set hilight colour etc
			glLineWidth(3.0f);
			rgba_t(255, 255, 0, 180, 1).set_gl();

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
