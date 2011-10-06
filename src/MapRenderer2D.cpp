
#include "Main.h"
#include "WxStuff.h"
#include "MapRenderer2D.h"
#include "SLADEMap.h"
#include "GameConfiguration.h"
#include "ColourConfiguration.h"
#include "MapEditorWindow.h"
#include "GLTexture.h"

CVAR(Bool, use_dlist, true, CVAR_SAVE)
CVAR(Bool, things_force_dir, false, CVAR_SAVE)

#define FORCE_NO_VBO 0

MapRenderer2D::MapRenderer2D(SLADEMap* map) {
	// Init variables
	this->map = map;
	this->vbo_vertices = 0;
	this->vbo_lines = 0;
	this->ibo_selection = 0;
	this->list_vertices = 0;
	this->list_lines = 0;
	this->lines_dirs = false;
}

MapRenderer2D::~MapRenderer2D() {
	if (vbo_vertices > 0)		glDeleteBuffers(1, &vbo_vertices);
	if (vbo_lines > 0)			glDeleteBuffers(1, &vbo_lines);
	if (ibo_selection > 0)		glDeleteBuffers(1, &ibo_selection);
	if (list_vertices > 0)		glDeleteLists(list_vertices, 1);
	if (list_lines > 0)			glDeleteLists(list_lines, 1);
}

void MapRenderer2D::renderVertices() {
	if (GLEW_ARB_vertex_buffer_object && !FORCE_NO_VBO)
		renderVerticesVBO();
	else
		renderVerticesImmediate();
}

void MapRenderer2D::renderVerticesImmediate() {
	if (list_vertices > 0)
		glCallList(list_vertices);
	else {
		list_vertices = glGenLists(1);
		glNewList(list_vertices, GL_COMPILE_AND_EXECUTE);

		// Set to vertex colour
		ColourConfiguration::getColour("map_vertex").set_gl();

		// Draw all vertices
		glBegin(GL_POINTS);
		for (unsigned a = 0; a < map->nVertices(); a++)
			glVertex2d(map->getVertex(a)->xPos(), map->getVertex(a)->yPos());
		glEnd();

		glEndList();
	}
}

void MapRenderer2D::renderVerticesVBO() {
	// Do nothing if there are no vertices in the map
	if (map->nVertices() == 0)
		return;

	// Update vertices VBO if required
	if (vbo_vertices == 0)
		updateVerticesVBO();

	// Set to vertex colour
	ColourConfiguration::getColour("map_vertex").set_gl();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Setup VBO pointers
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, 0);

	// Render the VBO
	glDrawArrays(GL_POINTS, 0, map->nVertices());

	// Cleanup state
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MapRenderer2D::renderLines(bool show_direction) {
	if (GLEW_ARB_vertex_buffer_object && !FORCE_NO_VBO)
		renderLinesVBO(show_direction);
	else
		renderLinesImmediate(show_direction);
}

void MapRenderer2D::renderLinesImmediate(bool show_direction) {
	// Use display list if it's built
	if (list_lines > 0 && show_direction == lines_dirs) {
		glCallList(list_lines);
		return;
	}

	// Rebuild display list if we're switching from simple lines to directional lines
	if (list_lines > 0)
		glDeleteLists(list_lines, 1);

	// Create display list
	list_lines = glGenLists(1);
	glNewList(list_lines, GL_COMPILE_AND_EXECUTE);

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
	for (unsigned a = 0; a < map->nLines(); a++) {
		// Get line info
		line = map->getLine(a);
		x1 = line->v1()->xPos();
		y1 = line->v1()->yPos();
		x2 = line->v2()->xPos();
		y2 = line->v2()->yPos();

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

	glEndList();
	lines_dirs = show_direction;
}

void MapRenderer2D::renderLinesVBO(bool show_direction) {
	// Do nothing if there are no lines in the map
	if (map->nLines() == 0)
		return;

	// Update lines VBO if required
	if (vbo_lines == 0 || show_direction != lines_dirs)
		updateLinesVBO(show_direction);

	// Disable any blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Setup VBO pointers
	glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 20, 0);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, 20, ((char*)NULL + 8));

	// Render the VBO
	if (show_direction)
		glDrawArrays(GL_LINES, 0, map->nLines()*4);
	else
		glDrawArrays(GL_LINES, 0, map->nLines()*2);

	// Clean state
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	lines_dirs = show_direction;
}

void MapRenderer2D::renderRoundThing(double x, double y, double angle, ThingType* tt, float alpha) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return;

	// --- Determine texture to use ---
	GLTexture* tex = NULL;
	bool rotate = false;

	// Check for unknown type
	if (tt->getName() == "Unknown") {
		tex = theMapEditor->textureManager().getThingImage("unknown");
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
	}

	else {
		// Otherwise, normal thing image (for now)
		//tt->getColour().set_gl(false);
		glColor4f(tt->getColour().fr(), tt->getColour().fg(), tt->getColour().fb(), alpha);

		// Check if we want an angle indicator
		if (tt->isAngled() || things_force_dir) {
			rotate = true;	// Also rotate to angle
			tex = theMapEditor->textureManager().getThingImage("normal_d");
		}
		else
			tex = theMapEditor->textureManager().getThingImage("normal_n");
	}

	// If for whatever reason the thing texture doesn't exist, just draw a basic, square thing
	if (!tex) {
		renderSquareThing(x, y, angle, tt, alpha);
		return;
	}

	// Bind texture
	if (tex_last != tex) {
		tex->bind();
		tex_last = tex;
	}

	// Move to thing position
	glPushMatrix();
	glTranslated(x, y, 0);

	// Rotate if needed
	if (rotate)
		glRotated(angle, 0, 0, 1);

	// Draw thing
	double radius = tt->getRadius();
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(-radius, -radius);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(-radius, radius);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(radius, radius);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(radius, -radius);
	glEnd();

	// Restore previous matrix
	glPopMatrix();
}

bool MapRenderer2D::renderSpriteThing(double x, double y, double angle, ThingType* tt, float alpha) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return false;

	// --- Determine texture to use ---
	bool show_angle = false;
	GLTexture* tex = NULL;

	// Attempt to get sprite texture
	tex = theMapEditor->textureManager().getSprite(tt->getSprite(), tt->getTranslation());

	// If sprite not found, just draw as a normal, round thing
	if (!tex) {
		renderRoundThing(x, y, angle, tt, alpha);
		return false;
	}

	// Check if we have to draw the angle arrow later
	if (tt->isAngled() || things_force_dir)
		show_angle = true;

	// If for whatever reason the thing texture doesn't exist, just draw a basic, square thing
	if (!tex) {
		renderSquareThing(x, y, angle, tt, alpha);
		return false;
	}

	// Move to thing position
	glPushMatrix();
	glTranslated(x, y, 0);

	// Bind texture
	if (tex_last != tex) {
		tex->bind();
		tex_last = tex;
	}

	// Draw thing
	double hw = tex->getWidth()*0.5;
	double hh = tex->getHeight()*0.5;
	glColor4f(1.0f, 1.0f, 1.0f, alpha);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(-hw, -hh);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(-hw, hh);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(hw, hh);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(hw, -hh);
	glEnd();

	// Restore previous matrix
	glPopMatrix();

	return show_angle;
}

void MapRenderer2D::renderSquareThing(double x, double y, double angle, ThingType* tt, float alpha) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return;

	// Get thing info
	double radius = tt->getRadius();
	double radius2 = radius * 0.1;

	// Move to thing position
	glPushMatrix();
	glTranslated(x, y, 0);

	// Draw background
	glColor4f(0.0f, 0.0f, 0.0f, alpha);
	glBegin(GL_QUADS);
	glVertex2d(-radius, -radius);
	glVertex2d(-radius, radius);
	glVertex2d(radius, radius);
	glVertex2d(radius, -radius);
	glEnd();

	// Draw base
	//tt->getColour().set_gl(false);
	glColor4f(tt->getColour().fr(), tt->getColour().fg(), tt->getColour().fb(), alpha);
	glBegin(GL_QUADS);
	glVertex2d(-radius+radius2, -radius+radius2);
	glVertex2d(-radius+radius2, radius-radius2);
	glVertex2d(radius-radius2, radius-radius2);
	glVertex2d(radius-radius2, -radius+radius2);
	glEnd();

	// Draw angle indicator (if needed)
	if (tt->isAngled() || things_force_dir) {
		glColor3f(0.0f, 0.0f, 0.0f);
		glRotated(angle, 0, 0, 1);
		glBegin(GL_LINES);
		glVertex2d(0, 0);
		glVertex2d(radius, 0);
		glEnd();
	}

	// Restore previous matrix
	glPopMatrix();
}

void MapRenderer2D::renderThings(int mode, float alpha) {
	renderThingsImmediate(mode, alpha);
}

void MapRenderer2D::renderThingsImmediate(int mode, float alpha) {
	// Display lists aren't really good for this, better to check for
	// visibility and just render things in immediate mode

	// Enable textures
	if (mode > 0)
		glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, alpha);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	tex_last = NULL;

	// Go through things
	MapThing* thing = NULL;
	double x, y;
	vector<int> things_arrows;
	for (unsigned a = 0; a < map->nThings(); a++) {
		if (vis_t[a] > 0)
			continue;

		// Get thing info
		thing = map->getThing(a);
		x = thing->xPos();
		y = thing->yPos();

		// Get thing type properties from game configuration
		ThingType* tt = theGameConfiguration->thingType(thing->getType());

		// Draw thing depending on 'things_drawtype' cvar
		if (mode == 2) {		// Drawtype 2: Sprites
			// Check if we need to draw the direction arrow for this thing
			if (renderSpriteThing(x, y, thing->prop("angle").getFloatValue(), tt, alpha))
				things_arrows.push_back(a);
		}
		else if (mode == 1)	// Drawtype 1: Round
			renderRoundThing(x, y, thing->prop("angle").getFloatValue(), tt, alpha);
		else							// Drawtype 0 (or other): Square
			renderSquareThing(x, y, thing->prop("angle").getFloatValue(), tt, alpha);
	}

	// Draw any thing direction arrows needed
	if (mode == 2) {
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		GLTexture* tex_arrow = theMapEditor->textureManager().getThingImage("arrow");
		if (tex_arrow) {
			tex_arrow->bind();

			for (unsigned a = 0; a < things_arrows.size(); a++) {
				MapThing* thing = map->getThing(things_arrows[a]);
				x = thing->xPos();
				y = thing->yPos();

				glPushMatrix();
				glTranslated(x, y, 0);
				glRotated(thing->prop("angle").getIntValue(), 0, 0, 1);

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
	if (mode > 0)
		glDisable(GL_TEXTURE_2D);
}

void MapRenderer2D::updateVerticesVBO() {
	// Create VBO if needed
	if (vbo_vertices == 0)
		glGenBuffers(1, &vbo_vertices);

	// Fill vertices VBO
	int nfloats = map->nVertices()*2;
	GLfloat* verts = new GLfloat[nfloats];
	unsigned i = 0;
	for (unsigned a = 0; a < map->nVertices(); a++) {
		verts[i++] = map->getVertex(a)->xPos();
		verts[i++] = map->getVertex(a)->yPos();
	}
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*nfloats, verts, GL_STATIC_DRAW);

	// Clean up
	delete[] verts;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MapRenderer2D::updateLinesVBO(bool show_direction) {
	// Create VBO if needed
	if (vbo_lines == 0)
		glGenBuffers(1, &vbo_lines);

	// Get line colours
	rgba_t col_background = ColourConfiguration::getColour("map_background");
	rgba_t col_line_normal = ColourConfiguration::getColour("map_line_normal");
	rgba_t col_line_special = ColourConfiguration::getColour("map_line_special");

	// Determine the number of vertices per line
	int vpl = 2;
	if (show_direction) vpl = 4;

	// Fill lines VBO
	int nverts = map->nLines()*vpl;
	glvert_t* lines = new glvert_t[nverts];
	unsigned v = 0;
	rgba_t col;
	float x1, y1, x2, y2;
	for (unsigned a = 0; a < map->nLines(); a++) {
		MapLine* line = map->getLine(a);

		// Check for special line
		if ((int)line->prop("special") > 0)
			col.set(col_line_special);
		else
			col.set(col_line_normal);

		// Check for two-sided line
		if (line->s2())
			col.set(col.r*0.5+col_background.r*(1.0-0.5),
					col.g*0.5+col_background.g*(1.0-0.5),
					col.b*0.5+col_background.b*(1.0-0.5), 255, 0);

		// Set line vertices
		lines[v].x = line->v1()->xPos();
		lines[v].y = line->v1()->yPos();
		lines[v+1].x = line->v2()->xPos();
		lines[v+1].y = line->v2()->yPos();

		// Set line colour(s)
		lines[v].r = lines[v+1].r = col.fr();
		lines[v].g = lines[v+1].g = col.fg();
		lines[v].b = lines[v+1].b = col.fb();

		// Direction tab if needed
		if (show_direction) {
			x1 = line->v1()->xPos();
			y1 = line->v1()->yPos();
			x2 = line->v2()->xPos();
			y2 = line->v2()->yPos();
			lines[v+2].x = x1 + ((x2 - x1) * 0.5);
			lines[v+2].y = y1 + ((y2 - y1) * 0.5);
			lines[v+3].x = lines[v+2].x - ((-(y2 - y1)) * 0.125);
			lines[v+3].y = lines[v+2].y - ((x2 - x1) * 0.125);

			// Colours
			lines[v+2].r = lines[v+3].r = col.fr();
			lines[v+2].g = lines[v+3].g = col.fg();
			lines[v+2].b = lines[v+3].b = col.fb();
		}

		// Next line
		v += vpl;
	}
	glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glvert_t)*nverts, lines, GL_STATIC_DRAW);

	// Clean up
	delete[] lines;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MapRenderer2D::updateVisibility(fpoint2_t view_tl, fpoint2_t view_br, double view_scale) {
	/*
	// Vertex visibility
	if (map->nVertices() != vis_v.size()) {
		// Number of vertices changed, reset array
		vis_v.clear();
		for (unsigned a = 0; a < map->nVertices(); a++)
			vis_v.push_back(0);
	}
	double x, y;
	for (unsigned a = 0; a < vis_v.size(); a++) {
		x = map->getVertex(a)->xPos();
		y = map->getVertex(a)->yPos();
		vis_v[a] = 0;
		if (x < view_tl.x)
			vis_v[a] |= VIS_LEFT;
		if (x > view_br.x)
			vis_v[a] |= VIS_RIGHT;
		if (y < view_tl.y)
			vis_v[a] |= VIS_BELOW;
		if (y > view_br.y)
			vis_v[a] |= VIS_ABOVE;
	}

	// Line visibility
	if (map->nLines() != vis_l.size()) {
		// Number of lines changed, reset array
		vis_l.clear();
		for (unsigned a = 0; a < map->nLines(); a++)
			vis_l.push_back(0);
	}
	uint8_t vis_v1, vis_v2;
	for (unsigned a = 0; a < vis_l.size(); a++) {
		vis_v1 = vis_v[map->getLine(a)->v1()->getIndex()];
		vis_v2 = vis_v[map->getLine(a)->v2()->getIndex()];
		vis_l[a] = 0;
		if (vis_v1 & VIS_LEFT && vis_v2 & VIS_LEFT)
			vis_l[a] |= VIS_LEFT;
		if (vis_v1 & VIS_RIGHT && vis_v2 & VIS_RIGHT)
			vis_l[a] |= VIS_RIGHT;
		if (vis_v1 & VIS_BELOW && vis_v2 & VIS_BELOW)
			vis_l[a] |= VIS_BELOW;
		if (vis_v1 & VIS_ABOVE && vis_v2 & VIS_ABOVE)
			vis_l[a] |= VIS_ABOVE;
	}
	*/

	// Thing visibility
	if (map->nThings() != vis_t.size()) {
		// Number of things changed, reset array
		vis_t.clear();
		for (unsigned a = 0; a < map->nThings(); a++)
			vis_t.push_back(0);
	}
	double x, y;
	double radius;
	for (unsigned a = 0; a < vis_t.size(); a++) {
		vis_t[a] = 0;
		x = map->getThing(a)->xPos();
		y = map->getThing(a)->yPos();

		// Get thing type properties from game configuration
		ThingType* tt = theGameConfiguration->thingType(map->getThing(a)->getType());
		radius = tt->getRadius();

		// Ignore if outside of screen
		if (x+radius < view_tl.x || x-radius > view_br.x || y+radius < view_tl.y || y-radius > view_br.y)
			vis_t[a] = 1;

		// Check if the thing is worth drawing
		else if (radius*view_scale < 2)
			vis_t[a] = 1;
	}
}
