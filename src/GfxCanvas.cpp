
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GfxCanvas.cpp
 * Description: GfxCanvas class. A wxWidgets GL canvas that displays
 *              an image and can take offsets into account etc
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
#include "GfxCanvas.h"


/* GfxCanvas::GfxCanvas
 * GfxCanvas class constructor
 *******************************************************************/
GfxCanvas::GfxCanvas(wxWindow* parent, int id)
: wxGLCanvas(parent, id, NULL, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN) {
	context = NULL;
	image = new SImage();
	init_done = false;
	offset_type = 1;
}

/* GfxCanvas::~GfxCanvas
 * GfxCanvas class destructor
 *******************************************************************/
GfxCanvas::~GfxCanvas() {
}

/* GfxCanvas::setContext
 * Sets the current gl context to the canvas' context, and creates
 * it if it doesn't exist. Returns true if the context is valid,
 * false otherwise
 *******************************************************************/
bool GfxCanvas::setContext() {
	if (!context) {
		if (IsShown())
			context = new wxGLContext(this);
	}

	if (context) {
		context->SetCurrent(*this);
		return true;
	}
	else
		return false;
}

/* GfxCanvas::init
 * Initialises OpenGL settings for the gfx canvas
 *******************************************************************/
void GfxCanvas::init()
{
	if (!setContext())
		return;

	glViewport(0, 0, GetClientSize().x, GetClientSize().y);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_NONE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	init_done = true;
}

/* GfxCanvas::draw
 * Draws the image/background/etc
 *******************************************************************/
void GfxCanvas::draw() {
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

	// Draw the background
	drawChequeredBackground();

	// Pan if offsets
	if (offset_type > 0) {
		int mid_x = GetSize().x / 2;
		int mid_y = GetSize().y / 2;
		glTranslated(mid_x, mid_y, 0);
	}

	// Draw offset lines
	drawOffsetLines();

	// Draw the image
	drawImage();

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}

void GfxCanvas::drawOffsetLines() {
	if (offset_type == 1) {
		COL_BLACK.set_gl();

		glBegin(GL_LINES);
		glVertex2d(-9999, 0);
		glVertex2d(9999, 0);
		glEnd();

		glBegin(GL_LINES);
		glVertex2d(0, -9999);
		glVertex2d(0, 9999);
		glEnd();
	}
}

/* GfxCanvas::drawChequeredBackground
 * Fills the canvas with a chequered pattern (used as the
 * 'background' - to indicate transparency)
 *******************************************************************/
void GfxCanvas::drawChequeredBackground() {
	// Save current matrix
	glPushMatrix();

	// Determine the number of rows and columns
	double s_size = 8.0;
	int cols = int((double)GetSize().x / s_size) + 1;
	int rows = int((double)GetSize().y / s_size) + 1;

	// Scale to square size
	glScaled(s_size, s_size, 1);

	// Draw a grid of squares
	for (int x = 0; x < cols; x++) {
		for (int y = 0; y < rows; y++) {
			// Set square colour
			rgba_t col(64, 64, 80, 255);
			if (x%2 != y%2)
				col.set(80, 80, 96, 255);
			col.set_gl();

			// Draw the square
			glBegin(GL_QUADS);
			glVertex2d(x, y);
			glVertex2d(x, y+1);
			glVertex2d(x+1, y+1);
			glVertex2d(x+1, y);
			glEnd();
		}
	}

	// Restore previous matrix
	glPopMatrix();
}

/* GfxCanvas::drawImage
 * Draws the image (as a quad per pixel for now, will change this to
 * use textures later, probably)
 *******************************************************************/
void GfxCanvas::drawImage() {
	// Save current matrix
	glPushMatrix();

	// Zoom
	glScaled(scale, scale, 1);

	// Pan (offsets)
	if (offset_type > 0)
		glTranslated(-image->offset().x, -image->offset().y, 0);

	uint8_t* data = image->getRGBAData();

	for (int x = 0; x < image->getWidth(); x++) {
		for (int y = 0; y < image->getHeight(); y++) {
			int a = (y*image->getWidth() + x) * 4;

			rgba_t col(data[a], data[a+1], data[a+2], data[a+3], 0);
			col.set_gl();

			// Draw the pixel
			glBegin(GL_QUADS);
			glVertex2d(x, y);
			glVertex2d(x, y+1);
			glVertex2d(x+1, y+1);
			glVertex2d(x+1, y);
			glEnd();
		}
	}

	delete[] data;

	// Restore previous matrix
	glPopMatrix();
}

void GfxCanvas::zoomToFit() {
	if (image->getWidth() > image->getHeight()) {
		// Get maximum dimension
		double x_dim = image->getWidth();

		// Increase a bit to add a border
		x_dim += (x_dim * 0.1);

		// Set scale accordingly
		scale = (double)GetSize().x / x_dim;
	}
	else {
		// Get maximum dimension
		double y_dim = image->getHeight();

		// Increase a bit to add a border
		y_dim += (y_dim * 0.1);

		// Set scale accordingly
		scale = (double)GetSize().y / y_dim;
	}
}

BEGIN_EVENT_TABLE(GfxCanvas, wxGLCanvas)
	EVT_PAINT(GfxCanvas::paint)
	EVT_ERASE_BACKGROUND(GfxCanvas::onEraseBackground)
END_EVENT_TABLE()

/* GfxCanvas::paint
 * Called when the gfx canvas has to be redrawn
 *******************************************************************/
void GfxCanvas::paint(wxPaintEvent& e) {
	wxPaintDC(this);

	if (!init_done)
		init();

	if (!setContext())
		return;

	draw();
}

/* GfxCanvas::onEraseBackground
 * Called when the gfx canvas background is to be erased (need to
 * override this to do nothing or the canvas will flicker in wxMSW)
 *******************************************************************/
void GfxCanvas::onEraseBackground(wxEraseEvent& e) {
	// Do nothing
}
