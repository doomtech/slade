
#include "Main.h"
#include "MapEditor.h"
#include "ColourConfiguration.h"
#include "ArchiveManager.h"
#include "WxStuff.h"
#include "MapEditorWindow.h"
#include "GameConfiguration.h"
#include "MathStuff.h"

double grid_sizes[] = { 0.05, 0.1, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };
CVAR(Bool, things_sprites, false, CVAR_SAVE)
CVAR(Bool, things_force_dir, false, CVAR_SAVE)
CVAR(Bool, things_square, false, CVAR_SAVE)

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

void MapEditor::drawThings(double xmin, double ymin, double xmax, double ymax, double view_scale) {
	// Enable textures
	if (!things_square)
		glEnable(GL_TEXTURE_2D);
	rgba_t(255, 255, 255, 255, 0).set_gl();
	glLineWidth(2.0f);

	// Go through things
	MapThing* thing = NULL;
	GLTexture* tex_last = NULL;
	GLTexture* tex = NULL;
	double x, y, radius;
	bool flip, sprite;
	vector<int> things_arrows;
	for (unsigned a = 0; a < map.things.size(); a++) {
		// Get thing info
		thing = map.things[a];
		x = thing->xPos();
		y = thing->yPos();

		// Get thing type properties from game configuration
		ThingType& tt = theGameConfiguration->thingType(thing->getType());
		radius = tt.getRadius();

		// Ignore if outside of screen
		if (x+radius < xmin || x-radius > xmax || y+radius < ymin || y-radius > ymax)
			continue;

		// Check if the thing is worth drawing
		if (radius*view_scale < 1)
			continue;

		// Translate to thing position
		glPushMatrix();
		glTranslated(x, y, 0);
		flip = false;
		sprite = false;
		tex = NULL;

		if (!things_square) {
			// Round or sprite things

			// Check for unknown type
			if (tt.getName() == "Unknown") {
				tex = theMapEditor->textureManager().getThingImage("unknown");
				flip = true;
				COL_WHITE.set_gl();
			}

			// Check for 'things as sprites' option
			if (!tex && things_sprites) {
				tex = theMapEditor->textureManager().getSprite(tt.getSprite());
				if (tex) {
					sprite = true;
					COL_WHITE.set_gl();

					// Add to list if we need to draw the direction arrow later
					if (tt.isAngled() || things_force_dir)
						things_arrows.push_back(a);
				}
			}

			// Normal thing image
			if (!tex) {
				tt.getColour().set_gl();

				// Rotate to angle (if needed)
				if (tt.isAngled() || things_force_dir) {
					glRotated(thing->prop("angle").getIntValue(), 0, 0, 1);
					tex = theMapEditor->textureManager().getThingImage("normal_d");	// Set thing+angle indicator texture
				}
				else
					tex = theMapEditor->textureManager().getThingImage("normal_n");	// Set no arrow indicator texture
			}

			// Bind texture
			if (tex && tex != tex_last) {
				tex->bind();
				tex_last = tex;	// Avoid unnecessary texture binding
			}

			// Draw thing
			if (sprite) {
				double hw = tex->getWidth()*0.5;
				double hh = tex->getHeight()*0.5;
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 1.0f);	glVertex2d(-hw, -hh);
				glTexCoord2f(0.0f, 0.0f);	glVertex2d(-hw, hh);
				glTexCoord2f(1.0f, 0.0f);	glVertex2d(hw, hh);
				glTexCoord2f(1.0f, 1.0f);	glVertex2d(hw, -hh);
				glEnd();
			}
			else if (flip) {
				// Thing texture is drawn upside-down normally, so flip on y axis for things with icons
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 1.0f);	glVertex2d(-radius, -radius);
				glTexCoord2f(0.0f, 0.0f);	glVertex2d(-radius, radius);
				glTexCoord2f(1.0f, 0.0f);	glVertex2d(radius, radius);
				glTexCoord2f(1.0f, 1.0f);	glVertex2d(radius, -radius);
				glEnd();
			}
			else {
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);	glVertex2d(-radius, -radius);
				glTexCoord2f(0.0f, 1.0f);	glVertex2d(-radius, radius);
				glTexCoord2f(1.0f, 1.0f);	glVertex2d(radius, radius);
				glTexCoord2f(1.0f, 0.0f);	glVertex2d(radius, -radius);
				glEnd();
			}
		}
		else {
			// Square things

			// Draw background
			COL_BLACK.set_gl();
			glBegin(GL_QUADS);
			glVertex2d(-radius-2, -radius-2);
			glVertex2d(-radius-2, radius+2);
			glVertex2d(radius+2, radius+2);
			glVertex2d(radius+2, -radius-2);
			glEnd();

			// Draw base
			tt.getColour().set_gl();
			glBegin(GL_QUADS);
			glVertex2d(-radius, -radius);
			glVertex2d(-radius, radius);
			glVertex2d(radius, radius);
			glVertex2d(radius, -radius);
			glEnd();

			// Draw angle indicator (if needed)
			if (tt.isAngled() || things_force_dir) {
				COL_BLACK.set_gl();
				glRotated(thing->prop("angle").getIntValue(), 0, 0, 1);
				glBegin(GL_LINES);
				glVertex2d(0, 0);
				glVertex2d(radius, 0);
				glEnd();
			}
		}

		glPopMatrix();
	}

	// Draw any thing direction arrows needed
	if (things_sprites && !things_square) {
		GLTexture* tex_arrow = theMapEditor->textureManager().getThingImage("arrow");
		if (tex_arrow) {
			tex_arrow->bind();

			for (unsigned a = 0; a < things_arrows.size(); a++) {
				MapThing* thing = map.getThing(things_arrows[a]);
				x = thing->xPos();
				y = thing->yPos();

				glPushMatrix();
				glTranslated(x, y, 0);
				glRotated(thing->prop("angle").getIntValue(), 0, 0, 1);

				COL_WHITE.set_gl();
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 1.0f);	glVertex2d(-32, -32);
				glTexCoord2f(0.0f, 0.0f);	glVertex2d(-32, 32);
				glTexCoord2f(1.0f, 0.0f);	glVertex2d(32, 32);
				glTexCoord2f(1.0f, 1.0f);	glVertex2d(32, -32);
				glEnd();

				glPopMatrix();
			}
		}
	}

	// Disable textures
	if (!things_square)
		glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0f);
}

void MapEditor::drawMap(double xmin, double ymin, double xmax, double ymax, double view_scale) {
	// Draw lines
	drawLines(xmin, ymin, xmax, ymax, edit_mode == MODE_LINES);

	// Draw vertices (if in vertices mode)
	if (edit_mode == MODE_VERTICES)
		drawVertices(xmin, ymin, xmax, ymax);

	// Draw things (if in things mode)
	if (edit_mode == MODE_THINGS)
		drawThings(xmin, ymin, xmax, ymax, view_scale);
}

void MapEditor::drawHilight(float flash_level) {
	// Check anything is hilighted
	if (hilight_item < 0)
		return;

	// Set hilight colour
	rgba_t col = ColourConfiguration::getColour("map_hilight");
	col.a *= flash_level;
	col.set_gl();

	glLineWidth(3.0f);
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
		MapThing* thing = map.things[hilight_item];
		double x = thing->xPos();
		double y = thing->yPos();

		// Get thing radius
		double radius = theGameConfiguration->thingType(thing->getType()).getRadius();
		if (things_square)
			radius += 6;
		else
			radius *= 1.1 + (0.2*flash_level);

		if (things_square) {
			glBegin(GL_QUADS);
			glVertex2d(x - radius, y - radius);
			glVertex2d(x - radius, y + radius);
			glVertex2d(x + radius, y + radius);
			glVertex2d(x + radius, y - radius);
			glEnd();
		}
		else {
			// Setup hilight thing texture
			GLTexture* tex = theMapEditor->textureManager().getThingImage("hilight");
			if (tex) {
				glEnable(GL_TEXTURE_2D);
				tex->bind();
			}

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex2d(x - radius, y - radius);
			glTexCoord2f(0.0f, 1.0f);	glVertex2d(x - radius, y + radius);
			glTexCoord2f(1.0f, 1.0f);	glVertex2d(x + radius, y + radius);
			glTexCoord2f(1.0f, 0.0f);	glVertex2d(x + radius, y - radius);
			glEnd();

			glDisable(GL_TEXTURE_2D);
		}
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

	glLineWidth(4.0f);
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
		MapThing* thing = NULL;
		for (unsigned a = 0; a < selection.size(); a++) {
			thing = map.things[selection[a]];
			x = thing->xPos();
			y = thing->yPos();

			// Get thing radius
			double radius = theGameConfiguration->thingType(thing->getType()).getRadius() + 8;

			if (things_square) {
				glBegin(GL_QUADS);
				glVertex2d(x - radius, y - radius);
				glVertex2d(x - radius, y + radius);
				glVertex2d(x + radius, y + radius);
				glVertex2d(x + radius, y - radius);
				glEnd();
			}
			else {
				// Setup hilight thing texture
				GLTexture* tex = theMapEditor->textureManager().getThingImage("hilight");
				if (tex) {
					glEnable(GL_TEXTURE_2D);
					tex->bind();
				}

				// Draw thing selection if on screen
				if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
					glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 0.0f);	glVertex2d(x - radius, y - radius);
					glTexCoord2f(0.0f, 1.0f);	glVertex2d(x - radius, y + radius);
					glTexCoord2f(1.0f, 1.0f);	glVertex2d(x + radius, y + radius);
					glTexCoord2f(1.0f, 0.0f);	glVertex2d(x + radius, y - radius);
					glEnd();
				}

				glDisable(GL_TEXTURE_2D);
			}
		}
	}

	glLineWidth(1.5f);
	glPointSize(vertex_size);
}

bool MapEditor::updateHilight(double dist_scale) {
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
			ThingType& type = theGameConfiguration->thingType(t->getType());
			double dist = MathStuff::distance(mouse_pos.x, mouse_pos.y, t->xPos(), t->yPos());
			if (dist <= type.getRadius() + (32/dist_scale))
				hilight_item = nearest[0];
		}
		else {
			for (unsigned a = 0; a < nearest.size(); a++) {
				MapThing* t = map.getThing(nearest[a]);
				ThingType& type = theGameConfiguration->thingType(t->getType());
				double dist = MathStuff::distance(mouse_pos.x, mouse_pos.y, t->xPos(), t->yPos());
				if (dist <= type.getRadius() + (32/dist_scale))
					hilight_item = nearest[a];
			}
		}
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
