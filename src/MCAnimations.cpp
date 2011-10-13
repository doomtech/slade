
#include "Main.h"
#include "WxStuff.h"
#include "MCAnimations.h"
#include "ColourConfiguration.h"
#include "MapEditorWindow.h"

EXTERN_CVAR(Bool, thing_overlay_square)

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
	if (!thing_overlay_square)
		this->radius += 8;
}

MCAThingSelection::~MCAThingSelection() {
}

bool MCAThingSelection::update(long time) {
	// Determine fade amount (0.0-1.0 over 150ms)
	fade = ((time - starttime) * 0.006f);

	// Reverse if deselecting
	if (!select) fade = 1.0f - fade;

	// Check if animation is finished
	if (fade < 0.0f || fade > 1.0f)
		return false;
	else
		return true;
}

void MCAThingSelection::draw() {
	rgba_t col = ColourConfiguration::getColour("map_selection");
	col.set_gl();
	glColor4f(col.fr(), col.fg(), col.fb(), fade);

	if (!thing_overlay_square) {
		// Get thing selection texture
		GLTexture* tex = theMapEditor->textureManager().getEditorImage("thing/hilight");
		if (!tex)
			return;

		// Bind the texture
		glEnable(GL_TEXTURE_2D);
		tex->bind();
	}

	double r = radius + (radius * 0.5 * (1.0-fade));

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(x - r, y - r);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(x - r, y + r);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(x + r, y + r);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(x + r, y - r);
	glEnd();
}
