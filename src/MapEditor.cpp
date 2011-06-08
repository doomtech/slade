
#include "Main.h"
#include "MapEditor.h"
#include "ColourConfiguration.h"
#include "ArchiveManager.h"

EXTERN_CVAR(Int, vertex_size)

MapEditor::MapEditor() {
	// Init variables
	edit_mode = MODE_LINES;
	hilight_item = -1;
}

MapEditor::~MapEditor() {
}

void MapEditor::setEditMode(int mode) {
	edit_mode = mode;
	updateHilight();
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
		glBegin(GL_LINES);
		glVertex2d(line->v1()->xPos(), line->v1()->yPos());
		glVertex2d(line->v2()->xPos(), line->v2()->yPos());
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
