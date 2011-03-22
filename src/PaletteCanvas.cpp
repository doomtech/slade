
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
	sel_begin = -1;
	sel_end = -1;

	// Bind events
	Bind(wxEVT_LEFT_DOWN,  &PaletteCanvas::onMouseLeftDown,  this);
	Bind(wxEVT_RIGHT_DOWN, &PaletteCanvas::onMouseRightDown, this);
	Bind(wxEVT_MOTION, &PaletteCanvas::onMouseMotion, this);
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

	// Translate to middle of pixel (otherwise inaccuracies can occur on certain gl implemenataions)
	glTranslatef(0.375f, 0.375f, 0);

	// Setup some variables
	int size = (MIN(GetSize().x, GetSize().y) - 17) / 16;

	// Draw palette
	int c = 0;
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			// Draw selection outline if needed
			if (c >= sel_begin && c <= sel_end) {
				COL_WHITE.set_gl();
				glBegin(GL_LINES);
				glVertex2d(x*size, y*size);
				glVertex2d(x*size+size, y*size);
				glVertex2d(x*size, y*size+size);
				glVertex2d(x*size+size, y*size+size);
				glEnd();

				// Selection beginning
				if (c == sel_begin) {
					glBegin(GL_LINES);
					glVertex2d(x*size, y*size);
					glVertex2d(x*size, y*size+size);
					glEnd();
				}

				// Selection ending
				if (c == sel_end) {
					glBegin(GL_LINES);
					glVertex2d(x*size+size, y*size+size);
					glVertex2d(x*size+size, y*size);
					glEnd();
				}
			}

			// Set colour
			palette->colour(c++).set_gl();

			// Draw square
			glBegin(GL_QUADS);
			glVertex2d(x*size+1, y*size+1);
			glVertex2d(x*size+1, y*size+size);
			glVertex2d(x*size+size, y*size+size);
			glVertex2d(x*size+size, y*size+1);
			glEnd();
		}
	}

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}

/* PaletteCanvas::getSelectedColour
 * Returns the currently selected colour, or a completely black +
 * transparent colour if nothing is selected
 *******************************************************************/
rgba_t PaletteCanvas::getSelectedColour() {
	if (sel_begin >= 0)
		return palette->colour(sel_begin);
	else
		return rgba_t(0, 0, 0, 0);
}

/* PaletteCanvas::setSelection
 * Sets the selection range. If [end] is -1 the range is set to a
 * single index
 *******************************************************************/
void PaletteCanvas::setSelection(int begin, int end) {
	sel_begin = begin;
	if (end == -1)
		sel_end = begin;
	else
		sel_end = end;
}


/*******************************************************************
 * PALETTECANVAS EVENTS
 *******************************************************************/

/* PaletteCanvas::onMouseLeftDown
 * Called when the palette canvas is left clicked
 *******************************************************************/
void PaletteCanvas::onMouseLeftDown(wxMouseEvent& e) {
	// Figure out what 'grid' position was clicked
	int size = (MIN(GetSize().x, GetSize().y) - 17) / 16;
	int x = e.GetX() / size;
	int y = e.GetY() / size;

	// If it was within the palette box, select the cell
	if (x >= 0 && x < 16 && y >= 0 && y < 16)
		setSelection(y * 16 + x);
	else
		setSelection(-1);

	// Redraw
	Refresh();

	// Do normal left click stuff
	e.Skip();
}

/* PaletteCanvas::onMouseRightDown
 * Called when the palette canvas is right clicked
 *******************************************************************/
void PaletteCanvas::onMouseRightDown(wxMouseEvent& e) {
	// Do normal right click stuff
	e.Skip();
}

/* PaletteCanvas::onMouseMotion
 * Called when the mouse cursor is moved over the palette canvas
 *******************************************************************/
void PaletteCanvas::onMouseMotion(wxMouseEvent& e) {
	// Check for dragging selection
	if (e.LeftIsDown()) {
		// Figure out what 'grid' position the cursor is over
		int size = (MIN(GetSize().x, GetSize().y) - 17) / 16;
		int x = e.GetX() / size;
		int y = e.GetY() / size;

		// Set selection accordingly
		if (x >= 0 && x < 16 && y >= 0 && y < 16) {
			setSelection(sel_begin, y * 16 + x);
			Refresh();
		}
	}
}
