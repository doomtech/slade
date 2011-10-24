
#include "Main.h"
#include "WxStuff.h"
#include "MCAnimations.h"
#include "ColourConfiguration.h"
#include "MapEditorWindow.h"
#include "MapLine.h"
#include "MapVertex.h"

EXTERN_CVAR(Bool, thing_overlay_square)
EXTERN_CVAR(Int, thing_drawtype)
EXTERN_CVAR(Bool, vertex_round)
EXTERN_CVAR(Float, line_width)

MCASelboxFader::MCASelboxFader(long start, fpoint2_t tl, fpoint2_t br) : MCAnimation(start) {
	// Init variables
	this->tl = tl;
	this->br = br;
	fade = 1.0f;
}

MCASelboxFader::~MCASelboxFader() {
}

bool MCASelboxFader::update(long time) {
	// Determine fade amount (1.0-0.0 over 150ms)
	fade = 1.0f - ((time - starttime) * 0.006f);

	// Check if animation is finished
	if (fade < 0.0f)
		return false;
	else
		return true;
}

void MCASelboxFader::draw() {
	rgba_t col;

	// Outline
	col.set(ColourConfiguration::getColour("map_selbox_outline"));
	col.a *= fade;
	col.set_gl();
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2d(tl.x, tl.y);
	glVertex2d(tl.x, br.y);
	glVertex2d(br.x, br.y);
	glVertex2d(br.x, tl.y);
	glEnd();

	// Fill
	col.set(ColourConfiguration::getColour("map_selbox_fill"));
	col.a *= fade;
	col.set_gl();
	glBegin(GL_QUADS);
	glVertex2d(tl.x, tl.y);
	glVertex2d(tl.x, br.y);
	glVertex2d(br.x, br.y);
	glVertex2d(br.x, tl.y);
	glEnd();
}


MCAThingSelection::MCAThingSelection(long start, double x, double y, double radius, bool select) : MCAnimation(start) {
	// Init variables
	this->x = x;
	this->y = y;
	this->radius = radius;
	this->select = select;
	this->fade = 1.0f;

	// Adjust radius
	if (!thing_overlay_square && thing_drawtype > 0 && thing_drawtype < 3)
		this->radius += 8;
}

MCAThingSelection::~MCAThingSelection() {
}

bool MCAThingSelection::update(long time) {
	// Determine fade amount (0.0-1.0 over 150ms)
	fade = 1.0f - ((time - starttime) * 0.004f);

	// Check if animation is finished
	if (fade < 0.0f || fade > 1.0f)
		return false;
	else
		return true;
}

void MCAThingSelection::draw() {
	// Setup colour
	rgba_t col;
	if (select)
		col.set(255, 255, 255, 255*fade, 1);
	else {
		col = ColourConfiguration::getColour("map_selection");
		col.a *= fade;
	}
	col.set_gl();

	// Get texture if needed
	if (!thing_overlay_square && thing_drawtype > 0 && thing_drawtype < 3) {
		// Get thing selection texture
		GLTexture* tex = theMapEditor->textureManager().getEditorImage("thing/hilight");
		if (!tex)
			return;

		// Bind the texture
		glEnable(GL_TEXTURE_2D);
		tex->bind();
	}

	double r = radius;
	if (select) r += radius*0.2*fade;

	// Draw
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(x - r, y - r);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(x - r, y + r);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(x + r, y + r);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(x + r, y - r);
	glEnd();
}



MCALineSelection::MCALineSelection(long start, vector<MapLine*>& lines, bool select) : MCAnimation(start) {
	// Init variables
	this->select = select;
	this->fade = 1.0f;

	// Go through list of lines
	double xmid, ymid;
	for (unsigned a = 0; a < lines.size(); a++) {
		if (!lines[a]) continue;

		// Add line
		this->lines.push_back(frect_t(lines[a]->x1(), lines[a]->y1(), lines[a]->x2(), lines[a]->y2()));

		// Calculate line direction tab
		frect_t& rect = this->lines.back();
		xmid = rect.tl.x + ((rect.br.x - rect.tl.x) * 0.5);
		ymid = rect.tl.y + ((rect.br.y - rect.tl.y) * 0.5);
		double tablen = rect.length() * 0.2;
		if (tablen > 8) tablen = 8;
		if (tablen < 2) tablen = 2;
		fpoint2_t invdir(-(rect.br.y - rect.tl.y), rect.br.x - rect.tl.x);
		invdir.normalize();

		this->tabs.push_back(frect_t(xmid, ymid, xmid - invdir.x*tablen, ymid - invdir.y*tablen));
	}
}

MCALineSelection::~MCALineSelection() {
}

bool MCALineSelection::update(long time) {
	// Determine fade amount (0.0-1.0 over 150ms)
	fade = 1.0f - ((time - starttime) * 0.004f);

	// Check if animation is finished
	if (fade < 0.0f || fade > 1.0f)
		return false;
	else
		return true;
}

void MCALineSelection::draw() {
	// Setup colour
	rgba_t col;
	if (select)
		col.set(255, 255, 255, 255*fade, 1);
	else {
		col = ColourConfiguration::getColour("map_selection");
		col.a *= fade;
	}
	col.set_gl();

	// Draw lines
	glLineWidth(line_width*5);
	glBegin(GL_LINES);
	for (unsigned a = 0; a < lines.size(); a++) {
		glVertex2d(lines[a].tl.x, lines[a].tl.y);
		glVertex2d(lines[a].br.x, lines[a].br.y);
		glVertex2d(tabs[a].tl.x, tabs[a].tl.y);
		glVertex2d(tabs[a].br.x, tabs[a].br.y);
	}
	glEnd();
}



MCAVertexSelection::MCAVertexSelection(long start, vector<MapVertex*>& verts, double size, bool select) : MCAnimation(start) {
	// Init variables
	this->size = size;
	this->select = select;
	this->fade = 1.0f;

	// Setup vertices list
	for (unsigned a = 0; a < verts.size(); a++) {
		if (!verts[a]) continue;
		vertices.push_back(fpoint2_t(verts[a]->xPos(), verts[a]->yPos()));
	}
}

MCAVertexSelection::~MCAVertexSelection() {
}

bool MCAVertexSelection::update(long time) {
	// Determine fade amount (0.0-1.0 over 150ms)
	fade = 1.0f - ((time - starttime) * 0.004f);

	// Check if animation is finished
	if (fade < 0.0f || fade > 1.0f)
		return false;
	else
		return true;
}

void MCAVertexSelection::draw() {
	// Setup colour
	rgba_t col;
	if (select)
		col.set(255, 255, 255, 255*fade, 1);
	else {
		col = ColourConfiguration::getColour("map_selection");
		col.a *= fade;
	}
	col.set_gl();

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

	// Draw points
	if (select)
		glPointSize(size+(size*fade));
	else
		glPointSize(size);
	glBegin(GL_POINTS);
	for (unsigned a = 0; a < vertices.size(); a++)
		glVertex2d(vertices[a].x, vertices[a].y);
	glEnd();

	if (point) {
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_TEXTURE_2D);
	}
}




MCASectorSelection::MCASectorSelection(long start, vector<Polygon2D*>& polys, bool select) : MCAnimation(start) {
	// Init variables
	this->select = select;
	this->fade = 1.0f;

	// Copy polygon list
	for (unsigned a = 0; a < polys.size(); a++)
		this->polygons.push_back(polys[a]);
}

MCASectorSelection::~MCASectorSelection() {
}

bool MCASectorSelection::update(long time) {
	// Determine fade amount (0.0-1.0 over 150ms)
	fade = 1.0f - ((time - starttime) * 0.004f);

	// Check if animation is finished
	if (fade < 0.0f || fade > 1.0f)
		return false;
	else
		return true;
}

void MCASectorSelection::draw() {
	// Setup colour
	rgba_t col;
	if (select)
		col.set(255, 255, 255, 180*fade, 1);
	else {
		col = ColourConfiguration::getColour("map_selection");
		col.a *= fade*0.75;
	}
	col.set_gl();

	// Draw polygons
	for (unsigned a = 0; a < polygons.size(); a++)
		polygons[a]->render();
}
