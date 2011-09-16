
#include "Main.h"
#include "VertexInfoOverlay.h"
#include "MapVertex.h"
#include "Drawing.h"
#include "MathStuff.h"
#include "ColourConfiguration.h"

VertexInfoOverlay::VertexInfoOverlay() {
	// Init variables
	pos_frac = false;
}

VertexInfoOverlay::~VertexInfoOverlay() {
}

void VertexInfoOverlay::update(MapVertex* vertex) {
	if (!vertex)
		return;

	// Update info string
	if (pos_frac)
		info = S_FMT("Vertex %d: (%1.4f, %1.4f)", vertex->getIndex(), vertex->xPos(), vertex->yPos());
	else
		info = S_FMT("Vertex %d: (%d, %d)", vertex->getIndex(), (int)vertex->xPos(), (int)vertex->yPos());
}

void VertexInfoOverlay::draw(int bottom, int right, float alpha) {
	// Don't bother if completely faded
	if (alpha <= 0.0f)
		return;

	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	col_fg.a = col_fg.a*alpha;

	// Slide in/out animation
	float alpha_inv = 1.0f - alpha;
	bottom += 16*alpha_inv*alpha_inv;

	// Draw overlay background
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(col_bg.fr(), col_bg.fg(), col_bg.fb(), col_bg.fa()*alpha);
	Drawing::drawFilledRect(0, bottom - 16, right, bottom);
	glBegin(GL_QUADS);
	glVertex2d(0, bottom - 16);
	glVertex2d(right, bottom - 16);
	glColor4f(col_bg.fr(), col_bg.fg(), col_bg.fb(), 0.0f);
	glVertex2d(right, bottom - 32);
	glVertex2d(0, bottom - 32);
	glEnd();

	// Draw text
	Drawing::drawText(info, 2, bottom - 16, col_fg);
}
