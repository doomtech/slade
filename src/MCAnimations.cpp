
#include "Main.h"
#include "MCAnimations.h"
#include "ColourConfiguration.h"

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
