
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


/* PaletteCanvas::PaletteCanvas
 * PaletteCanvas class constructor
 *******************************************************************/
PaletteCanvas::PaletteCanvas(wxWindow* parent, int id)
: OGLCanvas(parent, id) {
	palette = new Palette8bit();
}

/* PaletteCanvas::~PaletteCanvas
 * PaletteCanvas class destructor
 *******************************************************************/
PaletteCanvas::~PaletteCanvas() {
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
	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

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

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}
