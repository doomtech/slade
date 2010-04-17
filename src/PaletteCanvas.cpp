
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    PaletteCanvas.cpp
 * Description: PaletteCanvas class. An OpenGL canvas that displays
 *              a palette (256 colours max)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WxStuff.h"
#include "PaletteCanvas.h"


/*******************************************************************
 * PALETTECANVAS CLASS FUNCTIONS
 *******************************************************************/

/* PaletteCanvas::PaletteCanvas
 * PaletteCanvas class constructor
 *******************************************************************/
PaletteCanvas::PaletteCanvas(wxWindow* parent, int id)
: OGLCanvas(parent, id) {
	palette = new Palette8bit();
	selected = -1;

	// Bind events
	Bind(wxEVT_LEFT_DOWN,  &PaletteCanvas::onMouseLeftDown,  this);
	Bind(wxEVT_RIGHT_DOWN, &PaletteCanvas::onMouseRightDown, this);
}

/* PaletteCanvas::~PaletteCanvas
 * PaletteCanvas class destructor
 *******************************************************************/
PaletteCanvas::~PaletteCanvas() {
	if (palette)
		delete palette;
}

/* PaletteCanvas::draw
 * Draws the palette as 16x16 coloured squares
 *******************************************************************/
void PaletteCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetClientSize().x, GetClientSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetClientSize().x, GetClientSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Setup some variables
	float size = float(min(GetSize().x, GetSize().y)) / 16.0f;

	// Scale to size
	glScalef(size, size, 1.0f);

	// Draw palette
	int c = 0;
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			// Set colour
			palette->colour(c++).set_gl();

			// Draw square
			glBegin(GL_QUADS);
			glVertex2d(x, y);
			glVertex2d(x, y+1);
			glVertex2d(x+1, y+1);
			glVertex2d(x+1, y);
			glEnd();
		}
	}

	// Draw selection rectangle if this is selected
	if (selected >= 0) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// Determine x and y coordinates of selected colour
		int x = selected % 16;
		int y = selected / 16;

		// Draw outline (thick black outline underneath thin white outline)
		glLineWidth(2.0f);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex2d(x, y);
		glVertex2d(x, y + 1);
		glVertex2d(x + 1, y + 1);
		glVertex2d(x + 1, y);
		glEnd();
		glLineWidth(1.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex2d(x, y);
		glVertex2d(x, y + 1);
		glVertex2d(x + 1, y + 1);
		glVertex2d(x + 1, y);
		glEnd();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}

/* PaletteCanvas::getSelectedColour
 * Returns the currently selected colour, or a completely black +
 * transparent colour if nothing is selected
 *******************************************************************/
rgba_t PaletteCanvas::getSelectedColour() {
	if (selected >= 0)
		return palette->colour(selected);
	else
		return rgba_t(0, 0, 0, 0);
}


/*******************************************************************
 * PALETTECANVAS EVENTS
 *******************************************************************/

/* PaletteCanvas::onMouseLeftDown
 * Called when the palette canvas is left clicked
 *******************************************************************/
void PaletteCanvas::onMouseLeftDown(wxMouseEvent& e) {
	// Figure out what 'grid' position was clicked
	float size = float(min(GetSize().x, GetSize().y)) / 16.0f;
	int x = int((float)e.GetX() / size);
	int y = int((float)e.GetY() / size);

	// If it was within the palette box, select the cell
	if (x >= 0 && x < 16 && y >= 0 && y < 16)
		selected = y * 16 + x;
	else
		selected = -1;

	// Redraw
	Refresh();

	// Do normal left click stuff
	e.Skip();
}

/* PaletteCanvas::onMouseRightDown
 * Called when the palette canvas is right clicked
 *******************************************************************/
void PaletteCanvas::onMouseRightDown(wxMouseEvent& e) {
	// Figure out what 'grid' position was clicked
	float size = float(min(GetSize().x, GetSize().y)) / 16.0f;
	int x = int((float)e.GetX() / size);
	int y = int((float)e.GetY() / size);

	// If it was within the palette box, select the cell
	if (x >= 0 && x < 16 && y >= 0 && y < 16)
		selected = y * 16 + x;
	else
		selected = -1;

	// TODO: Summon a dialog box to select a color or something;
	// Gotta investigate wxColourDialog. In the meantime: 
	// no colors anymore I want them to turn black
	//wxColourDialog coldial;
	palette->setColour(selected, rgba_t(0, 0, 0, 0));

	// Redraw
	Refresh();

	// Do normal right click stuff
	e.Skip();
}
