
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

void VertexInfoOverlay::draw(sf::RenderWindow* rw, sf::Font& font, float alpha) {
	// Don't bother if completely faded
	if (alpha <= 0.0f)
		return;

	int bottom = rw->GetHeight();

	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");

	// Setup text string
	sf::String str_vinfo;
	str_vinfo.SetFont(font);
	str_vinfo.SetSize(12);
	str_vinfo.SetPosition(2, bottom - 16);
	str_vinfo.SetText(CHR(info));
	str_vinfo.SetColor(sf::Color(col_fg.r, col_fg.g, col_fg.b, 255*alpha));

	// Draw overlay background
	rgba_t(col_bg.r, col_bg.g, col_bg.b, alpha*60, 0).set_gl();
	Drawing::drawFilledRect(0, bottom - 16, rw->GetWidth(), bottom);
	Drawing::drawFilledRect(0, bottom - 18, rw->GetWidth(), bottom);
	Drawing::drawFilledRect(0, bottom - 20, rw->GetWidth(), bottom);

	// Draw text
	rw->Draw(str_vinfo);
}
