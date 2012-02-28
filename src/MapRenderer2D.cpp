
#include "Main.h"
#include "WxStuff.h"
#include "MapRenderer2D.h"
#include "SLADEMap.h"
#include "GameConfiguration.h"
#include "ColourConfiguration.h"
#include "MapEditorWindow.h"
#include "GLTexture.h"
#include "Polygon2D.h"

CVAR(Bool, vertex_round, true, CVAR_SAVE)
CVAR(Int, vertex_size, 7, CVAR_SAVE)
CVAR(Float, line_width, 1.5f, CVAR_SAVE)
CVAR(Bool, line_smooth, true, CVAR_SAVE)
CVAR(Int, thing_drawtype, 1, CVAR_SAVE)
CVAR(Bool, thing_force_dir, false, CVAR_SAVE)
CVAR(Bool, thing_overlay_square, false, CVAR_SAVE)
CVAR(Float, flat_brightness, 0.8f, CVAR_SAVE)
CVAR(Bool, flat_ignore_light, false, CVAR_SAVE)
CVAR(Float, thing_shadow, 0.5f, CVAR_SAVE)
CVAR(Bool, sector_hilight_fill, true, CVAR_SAVE)
CVAR(Bool, map_animate_hilight, true, CVAR_SAVE)
CVAR(Bool, map_animate_selection, false, CVAR_SAVE)
CVAR(Bool, map_animate_tagged, true, CVAR_SAVE)

CVAR(Bool, test_ssplit, false, CVAR_SAVE)

// Texture coordinates for rendering square things (since we can't just rotate these)
float sq_thing_tc[] = { 0.0f, 1.0f,
						0.0f, 0.0f,
						1.0f, 0.0f,
						1.0f, 1.0f };

#define FORCE_NO_VBO 0

MapRenderer2D::MapRenderer2D(SLADEMap* map) {
	// Init variables
	this->map = map;
	this->vbo_vertices = 0;
	this->vbo_lines = 0;
	this->vbo_flats = 0;
	this->list_vertices = 0;
	this->list_lines = 0;
	this->lines_dirs = false;
	this->n_vertices = 0;
	this->n_lines = 0;
	this->n_things = 0;
}

MapRenderer2D::~MapRenderer2D() {
	if (vbo_vertices > 0)		glDeleteBuffers(1, &vbo_vertices);
	if (vbo_lines > 0)			glDeleteBuffers(1, &vbo_lines);
	if (vbo_flats > 0)			glDeleteBuffers(1, &vbo_flats);
	if (list_vertices > 0)		glDeleteLists(list_vertices, 1);
	if (list_lines > 0)			glDeleteLists(list_lines, 1);
}

bool MapRenderer2D::setupVertexRendering(float size_scale) {
	// Setup rendering properties
	float vs = vertex_size*size_scale;
	if (view_scale < 1.0) vs *= view_scale;
	if (vs < 2.0) vs = 2.0;
	glPointSize(vs);

	// Setup point sprites if supported
	bool point = false;
	if (OpenGL::pointSpriteSupport()) {
		// Get appropriate vertex texture
		GLTexture* tex;
		if (vertex_round) tex = theMapEditor->textureManager().getEditorImage("vertex_r");
		else tex = theMapEditor->textureManager().getEditorImage("vertex_s");

		// If it was found, enable point sprites
		if (tex) {
			glEnable(GL_TEXTURE_2D);
			tex->bind();
			glEnable(GL_POINT_SPRITE);
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
			point = true;
		}
	}

	// No point sprites, use regular points
	if (!point) {
		if (vertex_round)	glEnable(GL_POINT_SMOOTH);
		else				glDisable(GL_POINT_SMOOTH);
	}

	return point;
}

void MapRenderer2D::renderVertices(float alpha) {
	// Check there are any vertices to render
	if (map->nVertices() == 0)
		return;

	// Setup rendering properties
	bool point = setupVertexRendering(1.0f);

	// Set to vertex colour
	rgba_t col = ColourConfiguration::getColour("map_vertex");
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(col.fr(), col.fg(), col.fb(), col.fa()*alpha);

	// Render the vertices depending on what features are supported
	if (GLEW_ARB_vertex_buffer_object && !FORCE_NO_VBO)
		renderVerticesVBO();
	else
		renderVerticesImmediate();

	if (point) {
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_TEXTURE_2D);
	}
}

void MapRenderer2D::renderVerticesImmediate() {
	if (list_vertices > 0 && map->nVertices() == n_vertices && map->geometryUpdated() <= vertices_updated)
		glCallList(list_vertices);
	else {
		list_vertices = glGenLists(1);
		glNewList(list_vertices, GL_COMPILE_AND_EXECUTE);

		// Draw all vertices
		glBegin(GL_POINTS);
		for (unsigned a = 0; a < map->nVertices(); a++)
			glVertex2d(map->getVertex(a)->xPos(), map->getVertex(a)->yPos());
		glEnd();

		glEndList();

		vertices_updated = theApp->runTimer();
	}
}

void MapRenderer2D::renderVerticesVBO() {
	// Do nothing if there are no vertices in the map
	if (map->nVertices() == 0)
		return;

	// Update vertices VBO if required
	if (vbo_vertices == 0 || map->nVertices() != n_vertices || map->geometryUpdated() > vertices_updated)
		updateVerticesVBO();

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

void MapRenderer2D::renderVertexHilight(int index, float fade) {
	// Check hilight
	if (index < 0)
		return;

	// Reset fade if hilight animation is disabled
	if (!map_animate_hilight)
		fade = 1.0f;

	// Set hilight colour
	rgba_t col = ColourConfiguration::getColour("map_hilight");
	col.a *= fade;
	col.set_gl();

	// Setup rendering properties
	bool point = setupVertexRendering(1.7f);

	// Draw vertex
	glBegin(GL_POINTS);
	glVertex2d(map->getVertex(index)->xPos(),
				map->getVertex(index)->yPos());
	glEnd();

	if (point) {
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_TEXTURE_2D);
	}
}

void MapRenderer2D::renderVertexSelection(vector<int>& selection, float fade) {
	// Check anything is selected
	if (selection.size() == 0)
		return;

	// Reset fade if selection animation is disabled
	if (!map_animate_selection)
		fade = 1.0f;

	// Set selection colour
	rgba_t col = ColourConfiguration::getColour("map_selection");
	col.a *= fade;
	col.set_gl();

	// Setup rendering properties
	bool point = setupVertexRendering(1.5f);

	// Draw selected vertices
	glBegin(GL_POINTS);
	for (unsigned a = 0; a < selection.size(); a++)
		glVertex2d(map->getVertex(selection[a])->xPos(), map->getVertex(selection[a])->yPos());
	glEnd();

	if (point) {
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_TEXTURE_2D);
	}
}

rgba_t MapRenderer2D::lineColour(MapLine* line, bool ignore_filter) {
	rgba_t col;

	if (line) {
		// Check for special line
		if ((int)line->prop("special") > 0)
			col.set(ColourConfiguration::getColour("map_line_special"));
		else
			col.set(ColourConfiguration::getColour("map_line_normal"));

		// Check for two-sided line
		if (line->s2())
			col.a *= 0.5f;

		// Check if filtered
		if (line->isFiltered() && !ignore_filter)
			col.a *= 0.25f;
	}

	return col;
}

void MapRenderer2D::renderLines(bool show_direction) {
	// Check there are any lines to render
	if (map->nLines() == 0)
		return;

	// Setup rendering properties
	glLineWidth(line_width);
	if (line_smooth)
		glEnable(GL_LINE_SMOOTH);
	else
		glDisable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Render the lines depending on what features are supported
	if (GLEW_ARB_vertex_buffer_object && !FORCE_NO_VBO)
		renderLinesVBO(show_direction);
	else
		renderLinesImmediate(show_direction);
}

void MapRenderer2D::renderLinesImmediate(bool show_direction) {
	// Use display list if it's built
	if (list_lines > 0 && show_direction == lines_dirs && map->nLines() == n_lines && map->geometryUpdated() <= lines_updated) {
		glCallList(list_lines);
		return;
	}

	// Rebuild display list if we're switching from simple lines to directional lines
	if (list_lines > 0)
		glDeleteLists(list_lines, 1);

	// Create display list
	list_lines = glGenLists(1);
	glNewList(list_lines, GL_COMPILE_AND_EXECUTE);

	// Draw all lines
	rgba_t col;
	MapLine* line = NULL;
	double x1, y1, x2, y2;
	glBegin(GL_LINES);
	for (unsigned a = 0; a < map->nLines(); a++) {
		// Get line info
		line = map->getLine(a);
		x1 = line->v1()->xPos();
		y1 = line->v1()->yPos();
		x2 = line->v2()->xPos();
		y2 = line->v2()->yPos();

		// Get line colour
		col = lineColour(line);

		// Set line colour
		glColor4f(col.fr(), col.fg(), col.fb(), col.fa());

		// Draw the line
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);

		// Direction tab
		if (show_direction) {
			fpoint2_t mid = line->midPoint();
			fpoint2_t tab = line->dirTabPoint();
			glVertex2d(mid.x, mid.y);
			glVertex2d(tab.x, tab.y);
		}
	}
	glEnd();

	glEndList();
	lines_dirs = show_direction;
	lines_updated = theApp->runTimer();
}

void MapRenderer2D::renderLinesVBO(bool show_direction) {
	// Do nothing if there are no lines in the map
	if (map->nLines() == 0)
		return;

	// Update lines VBO if required
	if (vbo_lines == 0 || show_direction != lines_dirs || map->nLines() != n_lines || map->geometryUpdated() > lines_updated)
		updateLinesVBO(show_direction);

	// Disable any blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Setup VBO pointers
	glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 24, 0);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 24, ((char*)NULL + 8));

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

void MapRenderer2D::renderLineHilight(int index, float fade) {
	// Check hilight
	if (index < 0)
		return;

	// Reset fade if hilight animation is disabled
	if (!map_animate_hilight)
		fade = 1.0f;

	// Set hilight colour
	rgba_t col = ColourConfiguration::getColour("map_hilight");
	col.a *= fade;
	col.set_gl();

	// Setup rendering properties
	glLineWidth(line_width*3);

	// Render line
	MapLine* line = map->getLine(index);
	double x1 = line->v1()->xPos();
	double y1 = line->v1()->yPos();
	double x2 = line->v2()->xPos();
	double y2 = line->v2()->yPos();
	glBegin(GL_LINES);
	glVertex2d(x1, y1);
	glVertex2d(x2, y2);
	glEnd();

	// Direction tab
	fpoint2_t mid = line->midPoint();
	fpoint2_t tab = line->dirTabPoint();
	glBegin(GL_LINES);
	glVertex2d(mid.x, mid.y);
	glVertex2d(tab.x, tab.y);
	glEnd();
}

void MapRenderer2D::renderLineSelection(vector<int>& selection, float fade) {
	// Check anything is selected
	if (selection.size() == 0)
		return;

	// Reset fade if selection animation is disabled
	if (!map_animate_selection)
		fade = 1.0f;

	// Set selection colour
	rgba_t col = ColourConfiguration::getColour("map_selection");
	col.a *= fade;
	col.set_gl();

	// Setup rendering properties
	glLineWidth(line_width*4);

	// Render selected lines
	MapLine* line;
	double x1, y1, x2, y2;
	glBegin(GL_LINES);
	for (unsigned a = 0; a < selection.size(); a++) {
		// Get line properties
		line = map->getLine(selection[a]);
		x1 = line->v1()->xPos();
		y1 = line->v1()->yPos();
		x2 = line->v2()->xPos();
		y2 = line->v2()->yPos();

		// Draw line
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);

		// Direction tab
		fpoint2_t mid = line->midPoint();
		fpoint2_t tab = line->dirTabPoint();
		glVertex2d(mid.x, mid.y);
		glVertex2d(tab.x, tab.y);
	}
	glEnd();
}

void MapRenderer2D::renderTaggedLines(vector<MapLine*>& lines, float fade) {
	// Reset fade if tagged animation is disabled
	if (!map_animate_tagged)
		fade = 1.0f;

	// Set hilight colour
	rgba_t col = ColourConfiguration::getColour("map_tagged");
	col.a *= fade;
	col.set_gl();

	// Setup rendering properties
	glLineWidth(line_width*3);

	// Go through tagged lines
	double x1, y1, x2, y2;
	for (unsigned a = 0; a < lines.size(); a++) {
		// Render line
		MapLine* line = lines[a];
		x1 = line->v1()->xPos();
		y1 = line->v1()->yPos();
		x2 = line->v2()->xPos();
		y2 = line->v2()->yPos();
		glBegin(GL_LINES);
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		glEnd();

		// Direction tab
		fpoint2_t mid = line->midPoint();
		fpoint2_t tab = line->dirTabPoint();
		glBegin(GL_LINES);
		glVertex2d(mid.x, mid.y);
		glVertex2d(tab.x, tab.y);
		glEnd();
	}
}

bool MapRenderer2D::setupThingOverlay() {
	// Get hilight texture
	GLTexture* tex = theMapEditor->textureManager().getEditorImage("thing/hilight");
	if (thing_drawtype == 0 || thing_drawtype == 3)
		tex = theMapEditor->textureManager().getEditorImage("thing/square/hilight");

	// Nothing to do if thing_overlay_square is true and thing_drawtype is 1 or 2 (circles or sprites)
	// or if the hilight circle texture isn't found for some reason
	if (!tex || (thing_overlay_square && (thing_drawtype == 1 || thing_drawtype == 2))) {
		glDisable(GL_TEXTURE_2D);
		return false;
	}

	// Otherwise, we want the textured selection overlay
	glEnable(GL_TEXTURE_2D);
	tex->bind();

	// Setup point sprites if supported
	bool point = false;
	if (OpenGL::pointSpriteSupport()) {
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		point = true;
	}

	return point;
}

void MapRenderer2D::renderThingOverlay(double x, double y, double radius, bool point) {
	// Simplest case, thing_overlay_square is true and thing_drawtype is 1 or 2 (circles or sprites)
	if (thing_overlay_square && (thing_drawtype == 1 || thing_drawtype == 2)) {
		// Draw square
		glBegin(GL_QUADS);
		glVertex2d(x - radius, y - radius);
		glVertex2d(x - radius, y + radius);
		glVertex2d(x + radius, y + radius);
		glVertex2d(x + radius, y - radius);
		glEnd();

		return;
	}

	// Otherwise, we want the circle selection overlay
	double ps = radius*2*view_scale;

	// Draw it
	if (point && ps <= OpenGL::maxPointSize()) {
		// Point sprite
		glPointSize(ps);
		glBegin(GL_POINTS);
		glVertex2d(x, y);
		glEnd();
	}
	else {
		// Textured quad
		if (point) glDisable(GL_POINT_SPRITE);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);	glVertex2d(x - radius, y - radius);
		glTexCoord2f(0.0f, 1.0f);	glVertex2d(x - radius, y + radius);
		glTexCoord2f(1.0f, 1.0f);	glVertex2d(x + radius, y + radius);
		glTexCoord2f(1.0f, 0.0f);	glVertex2d(x + radius, y - radius);
		glEnd();
		if (point) glEnable(GL_POINT_SPRITE);
	}
}

void MapRenderer2D::renderRoundThing(double x, double y, double angle, ThingType* tt, float alpha) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return;

	// --- Determine texture to use ---
	GLTexture* tex = NULL;
	bool rotate = false;

	// Set colour
	glColor4f(tt->getColour().fr(), tt->getColour().fg(), tt->getColour().fb(), alpha);

	// Check for custom thing icon
	if (!tt->getIcon().IsEmpty())
		tex = theMapEditor->textureManager().getEditorImage(S_FMT("thing/%s", CHR(tt->getIcon())));

	if (!tex) {
		// Otherwise, normal thing image

		// Check if we want an angle indicator
		if (tt->isAngled() || thing_force_dir) {
			if (angle != 0) rotate = true;	// Also rotate to angle
			tex = theMapEditor->textureManager().getEditorImage("thing/normal_d");
		}
		else
			tex = theMapEditor->textureManager().getEditorImage("thing/normal_n");
	}

	// If for whatever reason the thing texture doesn't exist, just draw a basic, square thing
	if (!tex) {
		renderSimpleSquareThing(x, y, angle, tt, alpha);
		return;
	}

	// Bind texture
	if (tex_last != tex) {
		tex->bind();
		tex_last = tex;
	}

	// Rotate if needed
	if (rotate) {
		glPushMatrix();
		glTranslated(x, y, 0);
		glRotated(angle, 0, 0, 1);
		x = y = 0;
	}

	// Draw thing
	double radius = tt->getRadius();
	if (tt->shrinkOnZoom()) radius = scaledRadius(radius);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(x-radius, y-radius);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(x-radius, y+radius);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(x+radius, y+radius);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(x+radius, y-radius);
	glEnd();

	// Restore previous matrix if rotated
	if (rotate)
		glPopMatrix();
}

bool MapRenderer2D::renderSpriteThing(double x, double y, double angle, ThingType* tt, float alpha, bool fitradius) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return false;

	// --- Determine texture to use ---
	bool show_angle = false;
	GLTexture* tex = NULL;

	// Attempt to get sprite texture
	tex = theMapEditor->textureManager().getSprite(tt->getSprite(), tt->getTranslation(), tt->getPalette());

	// If sprite not found, just draw as a normal, round thing
	if (!tex) {
		renderRoundThing(x, y, angle, tt, alpha);
		return false;
	}

	// Check if we have to draw the angle arrow later
	if (tt->isAngled() || thing_force_dir)
		show_angle = true;

	// If for whatever reason the thing texture doesn't exist, just draw a basic, square thing
	if (!tex) {
		renderSquareThing(x, y, angle, tt, alpha);
		return false;
	}

	// Bind texture
	if (tex_last != tex) {
		tex->bind();
		tex_last = tex;
	}

	// Draw thing
	double hw = tex->getWidth()*0.5;
	double hh = tex->getHeight()*0.5;

	// Fit to radius if needed
	if (fitradius) {
		double scale = ((double)tt->getRadius()*0.85) / max(hw, hh);
		hw *= scale;
		hh *= scale;
	}

	// Shadow if needed
	if (thing_shadow > 0.01f && alpha >= 0.9 && !fitradius) {
		double sz = (min(hw, hh))*0.1;
		if (sz < 1) sz = 1;
		glColor4f(0.0f, 0.0f, 0.0f, alpha*(thing_shadow*0.7));
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);	glVertex2d(x-hw-sz, y-hh-sz);
		glTexCoord2f(0.0f, 0.0f);	glVertex2d(x-hw-sz, y+hh+sz);
		glTexCoord2f(1.0f, 0.0f);	glVertex2d(x+hw+sz, y+hh+sz);
		glTexCoord2f(1.0f, 1.0f);	glVertex2d(x+hw+sz, y-hh-sz);
		glEnd();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);	glVertex2d(x-hw-sz, y-hh-sz-sz);
		glTexCoord2f(0.0f, 0.0f);	glVertex2d(x-hw-sz, y+hh+sz);
		glTexCoord2f(1.0f, 0.0f);	glVertex2d(x+hw+sz+sz, y+hh+sz);
		glTexCoord2f(1.0f, 1.0f);	glVertex2d(x+hw+sz+sz, y-hh-sz-sz);
		glEnd();
	}
	// Draw thing
	glColor4f(1.0f, 1.0f, 1.0f, alpha);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(x-hw, y-hh);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(x-hw, y+hh);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(x+hw, y+hh);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(x+hw, y-hh);
	glEnd();


	return show_angle;
}

bool MapRenderer2D::renderSquareThing(double x, double y, double angle, ThingType* tt, float alpha, bool showicon) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return false;

	// --- Determine texture to use ---
	GLTexture* tex = NULL;

	// Set colour
	glColor4f(tt->getColour().fr(), tt->getColour().fg(), tt->getColour().fb(), alpha);

	// Check for custom thing icon
	if (!tt->getIcon().IsEmpty() && showicon)
		tex = theMapEditor->textureManager().getEditorImage(S_FMT("thing/square/%s", CHR(tt->getIcon())));

	// Otherwise, no icon
	int tc_start = 0;
	if (!tex) {
		tex = theMapEditor->textureManager().getEditorImage("thing/square/normal_n");

		if (tt->isAngled() && showicon) {
			tex = theMapEditor->textureManager().getEditorImage("thing/square/normal_d1");

			// Setup variables depending on angle
			switch ((int)angle) {
			case 0:		// East: normal, texcoord 0
				break;
			case 45:	// Northeast: diagonal, texcoord 0
				tex = theMapEditor->textureManager().getEditorImage("thing/square/normal_d2");
				break;
			case 90:	// North: normal, texcoord 2
				tc_start = 2;
				break;
			case 135:	// Northwest: diagonal, texcoord 2
				tex = theMapEditor->textureManager().getEditorImage("thing/square/normal_d2");
				tc_start = 2;
				break;
			case 180:	// West: normal, texcoord 4
				tc_start = 4;
				break;
			case 225:	// Southwest: diagonal, texcoord 4
				tex = theMapEditor->textureManager().getEditorImage("thing/square/normal_d2");
				tc_start = 4;
				break;
			case 270:	// South: normal, texcoord 6
				tc_start = 6;
				break;
			case 315:	// Southeast: diagonal, texcoord 6
				tex = theMapEditor->textureManager().getEditorImage("thing/square/normal_d2");
				tc_start = 6;
				break;
			default:	// Unsupported angle, don't draw arrow
				tex = theMapEditor->textureManager().getEditorImage("thing/square/normal_n");
				break;
			};
		}
	}

	// If for whatever reason the thing texture doesn't exist, just draw a basic, square thing
	if (!tex) {
		renderSimpleSquareThing(x, y, angle, tt, alpha);
		return false;
	}

	// Bind texture
	if (tex_last != tex) {
		tex->bind();
		tex_last = tex;
	}

	// Draw thing
	double radius = tt->getRadius();
	if (tt->shrinkOnZoom()) radius = scaledRadius(radius);
	glBegin(GL_QUADS);
	int tc = tc_start;
	glTexCoord2f(sq_thing_tc[tc], sq_thing_tc[tc+1]);
	tc += 2;
	if (tc == 8) tc = 0;
	glVertex2d(x-radius, y-radius);
	glTexCoord2f(sq_thing_tc[tc], sq_thing_tc[tc+1]);
	tc += 2;
	if (tc == 8) tc = 0;
	glVertex2d(x-radius, y+radius);
	glTexCoord2f(sq_thing_tc[tc], sq_thing_tc[tc+1]);
	tc += 2;
	if (tc == 8) tc = 0;
	glVertex2d(x+radius, y+radius);
	glTexCoord2f(sq_thing_tc[tc], sq_thing_tc[tc+1]);
	glVertex2d(x+radius, y-radius);
	glEnd();

	return false;
}

void MapRenderer2D::renderSimpleSquareThing(double x, double y, double angle, ThingType* tt, float alpha) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return;

	// Get thing info
	double radius = tt->getRadius();
	if (tt->shrinkOnZoom()) radius = scaledRadius(radius);
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
	if (tt->isAngled() || thing_force_dir) {
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glRotated(angle, 0, 0, 1);
		glBegin(GL_LINES);
		glVertex2d(0, 0);
		glVertex2d(radius, 0);
		glEnd();
	}

	// Restore previous matrix
	glPopMatrix();
}

void MapRenderer2D::renderThings(float alpha) {
	renderThingsImmediate(alpha);
}

void MapRenderer2D::renderThingsImmediate(float alpha) {
	// Display lists aren't really good for this, better to check for
	// visibility and just render things in immediate mode

	// Enable textures
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, alpha);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	tex_last = NULL;

	// Go through things
	MapThing* thing = NULL;
	double x, y, angle;
	vector<int> things_arrows;

	// Draw thing shadows if needed
	if (thing_shadow > 0.01f && alpha >= 0.9 && thing_drawtype != 2) {
		glEnable(GL_TEXTURE_2D);
		GLTexture* tex_shadow = theMapEditor->textureManager().getEditorImage("thing/shadow");
		if (thing_drawtype == 0 || thing_drawtype == 3)
			tex_shadow = theMapEditor->textureManager().getEditorImage("thing/square/shadow");
		if (tex_shadow) {
			tex_shadow->bind();
			glColor4f(0.0f, 0.0f, 0.0f, alpha*thing_shadow);

			// Setup point sprites if supported
			bool point = false;
			if (OpenGL::pointSpriteSupport()) {
				glEnable(GL_POINT_SPRITE);
				glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
				point = true;
			}

			for (unsigned a = 0; a < map->nThings(); a++) {
				if (vis_t[a] > 0)
					continue;

				// No shadow if filtered
				thing = map->getThing(a);
				if (thing->isFiltered())
					continue;

				// Get thing info
				ThingType* tt = theGameConfiguration->thingType(thing->getType());
				double radius = (tt->getRadius()+1);
				if (tt->shrinkOnZoom()) radius = scaledRadius(radius);
				radius *= 1.3;
				x = thing->xPos();
				y = thing->yPos();

				// Draw shadow
				if (point && radius*2*view_scale <= OpenGL::maxPointSize()) {
					// Point sprite
					glPointSize(radius*2*view_scale);
					glBegin(GL_POINTS);
					glVertex2d(x, y);
					glEnd();
				}
				else {
					// Textured quad
					if (point) glDisable(GL_POINT_SPRITE);
					glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 1.0f);	glVertex2d(x-radius, y-radius);
					glTexCoord2f(0.0f, 0.0f);	glVertex2d(x-radius, y+radius);
					glTexCoord2f(1.0f, 0.0f);	glVertex2d(x+radius, y+radius);
					glTexCoord2f(1.0f, 1.0f);	glVertex2d(x+radius, y-radius);
					glEnd();
					if (point) glEnable(GL_POINT_SPRITE);
				}
			}

			if (point)
				glDisable(GL_POINT_SPRITE);
		}
	}

	// Draw things
	double talpha;
	for (unsigned a = 0; a < map->nThings(); a++) {
		if (vis_t[a] > 0)
			continue;

		// Get thing info
		thing = map->getThing(a);
		x = thing->xPos();
		y = thing->yPos();
		angle = thing->prop("angle").getFloatValue();

		// Set alpha
		if (thing->isFiltered())
			talpha = alpha*0.25;
		else
			talpha = alpha;

		// Get thing type properties from game configuration
		ThingType* tt = theGameConfiguration->thingType(thing->getType());

		// Draw thing depending on 'things_drawtype' cvar
		if (thing_drawtype == 2) {		// Drawtype 2: Sprites
			// Check if we need to draw the direction arrow for this thing
			if (renderSpriteThing(x, y, angle, tt, talpha))
				things_arrows.push_back(a);
		}
		else if (thing_drawtype == 1)	// Drawtype 1: Round
			renderRoundThing(x, y, angle, tt, talpha);
		else {							// Drawtype 0 (or other): Square
			if (renderSquareThing(x, y, angle, tt, talpha, thing_drawtype != 3))
				things_arrows.push_back(a);
		}
	}

	// Draw thing sprites within squares if that drawtype is set
	if (thing_drawtype > 2) {
		glEnable(GL_TEXTURE_2D);

		for (unsigned a = 0; a < map->nThings(); a++) {
			if (vis_t[a] > 0)
				continue;

			// Get thing info
			thing = map->getThing(a);
			ThingType* tt = theGameConfiguration->thingType(thing->getType());
			x = thing->xPos();
			y = thing->yPos();
			angle = thing->prop("angle").getFloatValue();

			// Set alpha
			if (thing->isFiltered())
				talpha = alpha*0.25;
			else
				talpha = alpha;

			if (renderSpriteThing(x, y, angle, tt, talpha, true))
				things_arrows.push_back(a);
		}
	}

	// Draw any thing direction arrows needed
	if (things_arrows.size() > 0) {
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		GLTexture* tex_arrow = theMapEditor->textureManager().getEditorImage("arrow");
		if (tex_arrow) {
			glEnable(GL_TEXTURE_2D);
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
	glDisable(GL_TEXTURE_2D);
}

void MapRenderer2D::renderThingHilight(int index, float fade) {
	// Check hilight
	if (index < 0)
		return;

	// Reset fade if hilight animation is disabled
	if (!map_animate_hilight)
		fade = 1.0f;

	// Set hilight colour
	rgba_t col = ColourConfiguration::getColour("map_hilight");
	col.a *= fade;
	col.set_gl();

	// Get thing info
	MapThing* thing = map->getThing(index);
	ThingType* tt = theGameConfiguration->thingType(thing->getType());
	double x = thing->xPos();
	double y = thing->yPos();

	// Get thing radius
	double radius = tt->getRadius();

	// Check if we want square overlays
	if (thing_overlay_square) {
		glDisable(GL_TEXTURE_2D);
		glLineWidth(3.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2d(x - radius, y - radius);
		glVertex2d(x - radius, y + radius);
		glVertex2d(x + radius, y + radius);
		glVertex2d(x + radius, y - radius);
		glEnd();
		col.a *= 0.5;
		col.set_gl(false);
		glBegin(GL_QUADS);
		glVertex2d(x - radius, y - radius);
		glVertex2d(x - radius, y + radius);
		glVertex2d(x + radius, y + radius);
		glVertex2d(x + radius, y - radius);
		glEnd();

		return;
	}

	// Shrink if needed
	if (tt->shrinkOnZoom()) radius = scaledRadius(radius);

	// Adjust radius
	if (thing_drawtype == 0 || thing_drawtype > 2)
		radius += 6;
	else
		radius *= 1.1 + (0.2*fade);

	// Setup hilight thing texture
	GLTexture* tex = NULL;
	if (thing_drawtype == 0 || thing_drawtype == 3)
		tex = theMapEditor->textureManager().getEditorImage("thing/square/hilight");
	else
		tex = theMapEditor->textureManager().getEditorImage("thing/hilight");
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

void MapRenderer2D::renderThingSelection(vector<int>& selection, float fade) {
	// Check anything is selected
	if (selection.size() == 0)
		return;

	// Reset fade if selection animation is disabled
	if (!map_animate_selection)
		fade = 1.0f;

	// Set selection colour
	rgba_t col = ColourConfiguration::getColour("map_selection");
	col.a *= fade;
	col.set_gl();

	// Setup overlay rendering
	bool point = setupThingOverlay();

	// Draw all selection overlays
	for (unsigned a = 0; a < selection.size(); a++) {
		MapThing* thing = map->getThing(selection[a]);
		ThingType* tt = theGameConfiguration->thingType(thing->getType());
		double radius = tt->getRadius();
		if (tt->shrinkOnZoom()) radius = scaledRadius(radius);

		// Adjust radius if the overlay isn't square
		if (!thing_overlay_square)
			radius += 8;

		// Draw it
		renderThingOverlay(thing->xPos(), thing->yPos(), radius*(0.8+(0.2*fade)), point);
	}

	// Clean up gl state
	if (point)
		glDisable(GL_POINT_SPRITE);
	glDisable(GL_TEXTURE_2D);
}

void MapRenderer2D::renderTaggedThings(vector<MapThing*>& things, float fade) {
	// Reset fade if tagged animation is disabled
	if (!map_animate_tagged)
		fade = 1.0f;

	// Set hilight colour
	rgba_t col = ColourConfiguration::getColour("map_tagged");
	col.a *= fade;
	col.set_gl();

	// Setup overlay rendering
	bool point = setupThingOverlay();

	// Draw all tagged overlays
	for (unsigned a = 0; a < things.size(); a++) {
		MapThing* thing = things[a];
		ThingType* tt = theGameConfiguration->thingType(thing->getType());
		double radius = tt->getRadius();
		if (tt->shrinkOnZoom()) radius = scaledRadius(radius);

		// Adjust radius if the overlay isn't square
		if (!thing_overlay_square)
			radius += 8;

		// Draw it
		renderThingOverlay(thing->xPos(), thing->yPos(), radius, point);
	}

	// Clean up gl state
	if (point)
		glDisable(GL_POINT_SPRITE);
	glDisable(GL_TEXTURE_2D);
}

void MapRenderer2D::renderFlats(int type) {
	if (GLEW_ARB_vertex_buffer_object && !FORCE_NO_VBO)
		renderFlatsVBO(type);
	else
		renderFlatsImmediate(type);
}

bool sortPolyByTex(Polygon2D* left, Polygon2D* right) {
	return left->getTexture()->glId() < right->getTexture()->glId();
}

void MapRenderer2D::renderFlatsImmediate(int type) {
	if (type > 0)
		glEnable(GL_TEXTURE_2D);

	if (flat_ignore_light)
		glColor4f(flat_brightness, flat_brightness, flat_brightness, 1.0f);

	// Go through sectors
	GLTexture* tex_last = NULL;
	GLTexture* tex = NULL;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		MapSector* sector = map->getSector(a);

		// Skip if sector is out of view
		if (vis_s[a] > 0)
			continue;

		if (type > 0) {
			// Get the sector texture
			if (type == 1)
				tex = theMapEditor->textureManager().getFlat(sector->floorTexture());
			else
				tex = theMapEditor->textureManager().getFlat(sector->ceilingTexture());

			// Bind the texture if needed
			if (tex) {
				if (!tex_last)
					glEnable(GL_TEXTURE_2D);
				if (tex != tex_last)
					tex->bind();
			}
			else if (tex_last)
				glDisable(GL_TEXTURE_2D);
			tex_last = tex;
		}

		// Setup polygon texture info if needed
		Polygon2D* poly = sector->getPolygon();
		if (type > 0 && poly->getTexture() != tex) {
			poly->setTexture(tex);
			poly->updateTextureCoords();
		}

		// Render the polygon
		if (!flat_ignore_light) {
			float light = ((int)sector->prop("lightlevel") / 255.0f) * flat_brightness;
			glColor4f(light, light, light, 1.0f);
		}
		poly->render();
	}

	if (type > 0)
		glDisable(GL_TEXTURE_2D);
}

void MapRenderer2D::renderFlatsVBO(int type) {
	bool vbo_updated = false;

	if (flat_ignore_light)
		glColor4f(flat_brightness, flat_brightness, flat_brightness, 1.0f);

	// First, check if any polygon vertex data has changed (in this case we need to refresh the entire vbo)
	for (unsigned a = 0; a < map->nSectors(); a++) {
		Polygon2D* poly = map->getSector(a)->getPolygon();
		if (poly && poly->vboUpdate() > 1) {
			wxLogMessage("Updating sector %d polygon vbo data", a);
			updateFlatsVBO();
			vbo_updated = true;
		}
	}

	// Create VBO if necessary
	if (!vbo_updated && vbo_flats == 0) {
		updateFlatsVBO();
		vbo_updated = true;
	}

	if (vbo_updated)
		wxLogMessage("Updated vbo");

	// Setup opengl state
	if (type > 0) glEnable(GL_TEXTURE_2D);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_flats);

	// Setup VBO pointers
	glVertexPointer(3, GL_FLOAT, 36, 0);
	glTexCoordPointer(2, GL_FLOAT, 36, ((char*)NULL + 12));
	//glColorPointer(4, GL_FLOAT, 36, ((char*)NULL + 20));
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Go through sectors
	GLTexture* tex_last = NULL;
	GLTexture* tex = NULL;
	bool first = true;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		MapSector* sector = map->getSector(a);

		// Skip if sector is out of view
		if (vis_s[a] > 0)
			continue;

		first = false;
		if (type > 0) {
			// Get the sector texture
			if (type == 1)
				tex = theMapEditor->textureManager().getFlat(sector->floorTexture());
			else
				tex = theMapEditor->textureManager().getFlat(sector->ceilingTexture());
		}

		// Setup polygon texture info if needed
		Polygon2D* poly = sector->getPolygon();
		if (type > 0 && poly->getTexture() != tex) {
			poly->setTexture(tex);			// Set polygon texture
			poly->updateTextureCoords();	// Update polygon texture coordinates
		}

		// Update polygon VBO data if needed
		if (poly->vboUpdate() > 0)
			poly->updateVBOData();

		// Bind the texture if needed
		if (tex) {
			if (!tex_last || first)
				glEnable(GL_TEXTURE_2D);
			if (tex != tex_last)
				tex->bind();
		}
		else if (!tex_last || first)
			glDisable(GL_TEXTURE_2D);
		tex_last = tex;

		// Render the polygon
		if (!flat_ignore_light) {
			float light = ((int)sector->prop("lightlevel") / 255.0f) * flat_brightness;
			glColor4f(light, light, light, 1.0f);
		}
		poly->renderVBO(false);
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Clean up opengl state
	if (type > 0) glDisable(GL_TEXTURE_2D);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MapRenderer2D::renderFlatHilight(int index, float fade) {
	// Check hilight
	if (index < 0)
		return;

	// Reset fade if hilight animation is disabled
	if (!map_animate_hilight)
		fade = 1.0f;

	// Set hilight colour
	rgba_t col = ColourConfiguration::getColour("map_hilight");
	col.a *= fade;
	col.set_gl();

	// Fill if cvar is set
	if (sector_hilight_fill) {
		glColor4f(col.fr(), col.fg(), col.fb(), col.fa()*0.5f);
		map->getSector(index)->getPolygon()->render();
	}

	// Get all lines belonging to the hilighted sector
	vector<MapLine*> lines;
	map->getLinesOfSector(index, lines);

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

	// Draw sector split lines
	if (test_ssplit) {
		glColor4f(col.fr(), col.fg(), col.fb(), col.fa() * 0.5f);
		glLineWidth(1.0f);
		map->getSector(index)->getPolygon()->renderWireframe();
	}
}

void MapRenderer2D::renderFlatSelection(vector<int>& selection, float fade) {
	// Check anything is selected
	if (selection.size() == 0)
		return;

	// Reset fade if selection animation is disabled
	if (!map_animate_selection)
		fade = 1.0f;

	// Set selection colour
	rgba_t col = ColourConfiguration::getColour("map_selection");
	col.a *= fade;
	col.set_gl();

	// Draw selection
	glColor4f(col.fr(), col.fg(), col.fb(), col.fa() * 0.75f);
	vector<MapSide*> sides_selected;
	for (unsigned a = 0; a < selection.size(); a++) {
		// Don't draw if outside screen (but still draw if it's small)
		if (vis_s[selection[a]] > 0 && vis_s[selection[a]] != VIS_SMALL)
			continue;

		// Get the sector's polygon
		Polygon2D* poly = map->getSector(selection[a])->getPolygon();
		vector<MapSide*>& sides = map->getSector(selection[a])->connectedSides();

		if (poly->hasPolygon()) {
			map->getSector(selection[a])->getPolygon()->render();
			for (unsigned a = 0; a < sides.size(); a++)
				sides_selected.push_back(sides[a]);
		}
		else {
			// Something went wrong with the polygon, just draw sector outline instead
			glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
			glBegin(GL_LINES);
			for (unsigned s = 0; s < sides.size(); s++) {
				MapLine* line = sides[s]->getParentLine();
				glVertex2d(line->v1()->xPos(), line->v1()->yPos());
				glVertex2d(line->v2()->xPos(), line->v2()->yPos());
			}
			glEnd();

			glColor4f(col.fr(), col.fg(), col.fb(), col.fa() * 0.6f);
		}
	}

	// Draw selection outline
	glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
	glLineWidth(line_width * 2);
	bool* lines_drawn = new bool[map->nLines()];
	memset(lines_drawn, 0, map->nLines());
	glBegin(GL_LINES);
	for (unsigned a = 0; a < sides_selected.size(); a++) {
		MapLine* line = sides_selected[a]->getParentLine();
		if (lines_drawn[line->getIndex()])
			continue;

		glVertex2d(line->v1()->xPos(), line->v1()->yPos());
		glVertex2d(line->v2()->xPos(), line->v2()->yPos());
		lines_drawn[line->getIndex()] = true;
	}
	glEnd();
	delete[] lines_drawn;
}

void MapRenderer2D::renderTaggedFlats(vector<MapSector*>& sectors, float fade) {
	// Reset fade if tagged animation is disabled
	if (!map_animate_tagged)
		fade = 1.0f;

	// Set colour
	rgba_t col = ColourConfiguration::getColour("map_tagged");
	col.a *= fade;
	col.set_gl();

	// Render each sector polygon
	glDisable(GL_TEXTURE_2D);
	for (unsigned a = 0; a < sectors.size(); a++) {
		sectors[a]->getPolygon()->render();

		// Get all lines belonging to the tagged sector
		vector<MapLine*> lines;
		map->getLinesOfSector(sectors[a], lines);

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
}

void MapRenderer2D::renderMovingVertices(vector<int>& vertices, fpoint2_t move_vec) {
	uint8_t* lines_drawn = new uint8_t[map->nLines()];
	memset(lines_drawn, 0, map->nLines());

	// Determine what lines need drawing (and which of their vertices are being moved)
	for (unsigned a = 0; a < vertices.size(); a++) {
		MapVertex* v = map->getVertex(vertices[a]);
		for (unsigned l = 0; l < v->nConnectedLines(); l++) {
			MapLine* line = v->connectedLine(l);

			if (line->v1() == v) lines_drawn[map->lineIndex(line)] |= 1;
			if (line->v2() == v) lines_drawn[map->lineIndex(line)] |= 2;
		}
	}

	// Draw any lines attached to the moving vertices
	glLineWidth(line_width);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_LINES);
	for (unsigned a = 0; a < map->nLines(); a++) {
		MapLine* line = map->getLine(a);
		uint8_t drawn = lines_drawn[map->lineIndex(line)];

		// Skip if not attached to any moving vertices
		if (drawn == 0)
			continue;

		// Set line colour
		lineColour(line, true).set_gl(false);

		// First vertex
		if (drawn & 1)
			glVertex2d(line->x1() + move_vec.x, line->y1() + move_vec.y);
		else
			glVertex2d(line->x1(), line->y1());

		// Second vertex
		if (drawn & 2)
			glVertex2d(line->x2() + move_vec.x, line->y2() + move_vec.y);
		else
			glVertex2d(line->x2(), line->y2());
	}
	glEnd();

	// Set 'moving' colour
	ColourConfiguration::getColour("map_moving").set_gl();

	// Draw moving vertex overlays
	bool point = setupVertexRendering(1.5f);
	glBegin(GL_POINTS);
	for (unsigned a = 0; a < vertices.size(); a++) {
		glVertex2d(map->getVertex(vertices[a])->xPos() + move_vec.x,
					map->getVertex(vertices[a])->yPos() + move_vec.y);
	}
	glEnd();

	// Clean up
	delete[] lines_drawn;
	if (point) {
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_TEXTURE_2D);
	}
}

void MapRenderer2D::renderMovingLines(vector<int>& lines, fpoint2_t move_vec) {
	uint8_t* lines_drawn = new uint8_t[map->nLines()];
	memset(lines_drawn, 0, map->nLines());

	// Determine what lines need drawing (and which of their vertices are being moved)
	for (unsigned a = 0; a < lines.size(); a++) {
		// Check first vertex
		MapVertex* v = map->getLine(lines[a])->v1();
		for (unsigned l = 0; l < v->nConnectedLines(); l++) {
			MapLine* line = v->connectedLine(l);

			if (line->v1() == v) lines_drawn[map->lineIndex(line)] |= 1;
			if (line->v2() == v) lines_drawn[map->lineIndex(line)] |= 2;
		}

		// Check second vertex
		v = map->getLine(lines[a])->v2();
		for (unsigned l = 0; l < v->nConnectedLines(); l++) {
			MapLine* line = v->connectedLine(l);

			if (line->v1() == v) lines_drawn[map->lineIndex(line)] |= 1;
			if (line->v2() == v) lines_drawn[map->lineIndex(line)] |= 2;
		}
	}

	// Draw any lines attached to the moving vertices
	glLineWidth(line_width);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_LINES);
	for (unsigned a = 0; a < map->nLines(); a++) {
		MapLine* line = map->getLine(a);
		uint8_t drawn = lines_drawn[map->lineIndex(line)];

		// Skip if not attached to any moving vertices
		if (drawn == 0)
			continue;

		// Set line colour
		lineColour(line, true).set_gl(false);

		// First vertex
		if (drawn & 1)
			glVertex2d(line->x1() + move_vec.x, line->y1() + move_vec.y);
		else
			glVertex2d(line->x1(), line->y1());

		// Second vertex
		if (drawn & 2)
			glVertex2d(line->x2() + move_vec.x, line->y2() + move_vec.y);
		else
			glVertex2d(line->x2(), line->y2());
	}
	glEnd();

	// Set 'moving' colour
	ColourConfiguration::getColour("map_moving").set_gl();

	// Draw moving line overlays
	glLineWidth(line_width*3);
	glBegin(GL_LINES);
	for (unsigned a = 0; a < lines.size(); a++) {
		MapLine* line = map->getLine(lines[a]);
		glVertex2d(line->x1() + move_vec.x, line->y1() + move_vec.y);
		glVertex2d(line->x2() + move_vec.x, line->y2() + move_vec.y);
	}
	glEnd();

	// Clean up
	delete[] lines_drawn;
}

void MapRenderer2D::renderMovingSectors(vector<int>& sectors, fpoint2_t move_vec) {
	// Determine what lines are being moved
	uint8_t* lines_moved = new uint8_t[map->nLines()];
	memset(lines_moved, 0, map->nLines());
	for (unsigned a = 0; a < sectors.size(); a++) {
		// Go through connected sides
		vector<MapSide*>& sides = map->getSector(sectors[a])->connectedSides();
		for (unsigned s = 0; s < sides.size(); s++)
			lines_moved[map->lineIndex(sides[s]->getParentLine())] = 1;	// Mark parent line as moved
	}

	// Build list of moving lines
	vector<int> lines;
	for (unsigned a = 0; a < map->nLines(); a++) {
		if (lines_moved[a] > 0)
			lines.push_back(a);
	}

	// Draw moving lines
	renderMovingLines(lines, move_vec);

	// Clean up
	delete[] lines_moved;
}

void MapRenderer2D::renderMovingThings(vector<int>& things, fpoint2_t move_vec) {
	// Enable textures
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	tex_last = NULL;

	// Draw things
	MapThing* thing = NULL;
	double x, y, angle;
	for (unsigned a = 0; a < things.size(); a++) {
		// Get thing info
		thing = map->getThing(things[a]);
		x = thing->xPos() + move_vec.x;
		y = thing->yPos() + move_vec.y;
		angle = thing->prop("angle").getFloatValue();

		// Get thing type properties from game configuration
		ThingType* tt = theGameConfiguration->thingType(thing->getType());

		// Draw thing depending on 'things_drawtype' cvar
		if (thing_drawtype == 2)		// Drawtype 2: Sprites
			renderSpriteThing(x, y, angle, tt, 1.0f);
		else if (thing_drawtype == 1)	// Drawtype 1: Round
			renderRoundThing(x, y, angle, tt, 1.0f);
		else							// Drawtype 0 (or other): Square
			renderSquareThing(x, y, angle, tt, 1.0f, thing_drawtype != 3);
	}

	// Draw thing sprites within squares if that drawtype is set
	if (thing_drawtype > 2) {
		glEnable(GL_TEXTURE_2D);

		for (unsigned a = 0; a < things.size(); a++) {
			// Get thing info
			thing = map->getThing(things[a]);
			ThingType* tt = theGameConfiguration->thingType(thing->getType());
			x = thing->xPos() + move_vec.x;
			y = thing->yPos() + move_vec.y;
			angle = thing->prop("angle").getFloatValue();

			renderSpriteThing(x, y, angle, tt, 1.0f, true);
		}
	}

	// Set 'moving' colour
	ColourConfiguration::getColour("map_moving").set_gl();

	// Draw moving thing overlays
	bool point = setupThingOverlay();
	for (unsigned a = 0; a < things.size(); a++) {
		MapThing* thing = map->getThing(things[a]);
		ThingType* tt = theGameConfiguration->thingType(thing->getType());
		double radius = tt->getRadius();
		if (tt->shrinkOnZoom()) radius = scaledRadius(radius);

		// Adjust radius if the overlay isn't square
		if (!thing_overlay_square)
			radius += 8;

		renderThingOverlay(thing->xPos() + move_vec.x, thing->yPos() + move_vec.y, radius, point);
	}

	// Clean up gl state
	if (point) {
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_TEXTURE_2D);
	}
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

	n_vertices = map->nVertices();
	vertices_updated = theApp->runTimer();
}

void MapRenderer2D::updateLinesVBO(bool show_direction) {
	// Create VBO if needed
	if (vbo_lines == 0)
		glGenBuffers(1, &vbo_lines);

	// Determine the number of vertices per line
	int vpl = 2;
	if (show_direction) vpl = 4;

	// Fill lines VBO
	int nverts = map->nLines()*vpl;
	glvert_t* lines = new glvert_t[nverts];
	unsigned v = 0;
	rgba_t col;
	float alpha;
	for (unsigned a = 0; a < map->nLines(); a++) {
		MapLine* line = map->getLine(a);

		// Get line colour
		col = lineColour(line);
		alpha = col.fa();

		// Set line vertices
		lines[v].x = line->v1()->xPos();
		lines[v].y = line->v1()->yPos();
		lines[v+1].x = line->v2()->xPos();
		lines[v+1].y = line->v2()->yPos();

		// Set line colour(s)
		lines[v].r = lines[v+1].r = col.fr();
		lines[v].g = lines[v+1].g = col.fg();
		lines[v].b = lines[v+1].b = col.fb();
		lines[v].a = lines[v+1].a = alpha;

		// Direction tab if needed
		if (show_direction) {
			fpoint2_t mid = line->midPoint();
			fpoint2_t tab = line->dirTabPoint();
			lines[v+2].x = mid.x;
			lines[v+2].y = mid.y;
			lines[v+3].x = tab.x;
			lines[v+3].y = tab.y;

			// Colours
			lines[v+2].r = lines[v+3].r = col.fr();
			lines[v+2].g = lines[v+3].g = col.fg();
			lines[v+2].b = lines[v+3].b = col.fb();
			lines[v+2].a = lines[v+3].a = alpha*0.6f;
		}

		// Next line
		v += vpl;
	}
	glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glvert_t)*nverts, lines, GL_STATIC_DRAW);

	// Clean up
	delete[] lines;
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	n_lines = map->nLines();
	lines_updated = theApp->runTimer();
}

void MapRenderer2D::updateFlatsVBO() {
	// Create VBO if needed
	if (vbo_flats == 0)
		glGenBuffers(1, &vbo_flats);

	// Get total size needed
	unsigned totalsize = 0;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		Polygon2D* poly = map->getSector(a)->getPolygon();
		totalsize += poly->vboDataSize();
	}

	// Allocate buffer data
	glBindBuffer(GL_ARRAY_BUFFER, vbo_flats);
	glBufferData(GL_ARRAY_BUFFER, totalsize, NULL, GL_STATIC_DRAW);

	// Write polygon data to VBO
	unsigned offset = 0;
	unsigned index = 0;
	for (unsigned a = 0; a < map->nSectors(); a++) {
		Polygon2D* poly = map->getSector(a)->getPolygon();
		offset = poly->writeToVBO(offset, index);
		index += poly->totalVertices();
	}

	// Clean up
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	flats_updated = theApp->runTimer();
}

void MapRenderer2D::updateVisibility(fpoint2_t view_tl, fpoint2_t view_br) {
	// Sector visibility
	if (map->nSectors() != vis_s.size()) {
		// Number of sectors changed, reset array
		vis_s.clear();
		for (unsigned a = 0; a < map->nSectors(); a++)
			vis_s.push_back(0);
	}
	for (unsigned a = 0; a < map->nSectors(); a++) {
		// Check against sector bounding box
		bbox_t bbox = map->getSector(a)->boundingBox();
		vis_s[a] = 0;
		if (bbox.max.x < view_tl.x) vis_s[a] = VIS_LEFT;
		if (bbox.max.y < view_tl.y) vis_s[a] = VIS_ABOVE;
		if (bbox.min.x > view_br.x) vis_s[a] = VIS_RIGHT;
		if (bbox.min.y > view_br.y) vis_s[a] = VIS_BELOW;

		// Check if the sector is worth drawing
		if ((bbox.max.x - bbox.min.x) * view_scale < 4 ||
			(bbox.max.y - bbox.min.y) * view_scale < 4)
			vis_s[a] = VIS_SMALL;
	}

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
		radius = scaledRadius(tt->getRadius());

		// Ignore if outside of screen
		if (x+radius < view_tl.x || x-radius > view_br.x || y+radius < view_tl.y || y-radius > view_br.y)
			vis_t[a] = 1;

		// Check if the thing is worth drawing
		else if (radius*view_scale < 2)
			vis_t[a] = VIS_SMALL;
	}
}


void MapRenderer2D::forceUpdate() {
	// Update variables
	this->view_scale = view_scale;
	this->view_scale_inv = 1.0 / view_scale;

	if (GLEW_ARB_vertex_buffer_object && !FORCE_NO_VBO) {
		updateVerticesVBO();
		updateLinesVBO(lines_dirs);
	} else {
		if (list_lines > 0) {
			glDeleteLists(list_lines, 1);
			list_lines = 0;
		}
		if (list_vertices > 0) {
			glDeleteLists(list_vertices, 1);
			list_vertices = 0;
		}
	}
	renderVertices(view_scale);
	renderLines(lines_dirs);
}

double MapRenderer2D::scaledRadius(int radius) {
	if (radius > 16)
		radius = 16;

	if (view_scale > 1.0)
		return radius * view_scale_inv;
	else
		return (double)radius;
}
