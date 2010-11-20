

#include "Main.h"
#include "MapLine.h"
#include "MapVertex.h"

rgba_t col_line_normal(255, 255, 255, 255);
rgba_t col_line_special(100, 120, 255, 255);
rgba_t col_line_invalid(255, 0, 0, 255);

MapLine::MapLine(MapVertex* v1, MapVertex* v2, MapSide* s1, MapSide* s2) {
	// Init variables
	vertex1 = v1;
	vertex2 = v2;
	side1 = s1;
	side2 = s2;

	// Connect to vertices
	v1->connectLine(this);
	v2->connectLine(this);
}

bool MapLine::parseUDMF(Tokenizer& tz) {
	// Skip opening {
	tz.getToken();

	// TODO: Init required line attributes

	// TODO: Read line definition
	string token = tz.getToken();
	while (token != "}") {

		token = tz.getToken();
	}

	// TODO: Check that all required attributes were defined

	return true;
}

void MapLine::draw(bool side_indicator) {
	// Check line is valid
	if (!vertex1 || !vertex2)
		return;

	// Set line thickness
	glLineWidth(1.5f);

	// Determine line colour
	rgba_t col = col_line_normal;
	if ((int)udmf_props["special"] > 0)
		col = col_line_special;
	if (!side1)
		col = col_line_invalid;

	// Darken line if 2s
	if (side2) {
		col = col.ampf(0.8f, 0.8f, 0.8f, 0.7f);
		glLineWidth(1.2f);
	}

	// Set colour
	col.set_gl();

	// Draw the line
	glBegin(GL_LINES);
	glVertex2d(vertex1->xPos(), vertex1->yPos());
	glVertex2d(vertex2->xPos(), vertex2->yPos());
	glEnd();
}
