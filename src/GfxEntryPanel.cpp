
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GfxEntryPanel.cpp
 * Description: GfxEntryPanel class. The UI for editing gfx entries.
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
#include "GfxEntryPanel.h"


/* GfxCanvas::GfxCanvas
 * GfxCanvas class constructor
 *******************************************************************/
GfxCanvas::GfxCanvas(wxWindow* parent, int id)
: wxGLCanvas(parent, id, NULL, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN) {
	context = NULL;
	image = new SImage();
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

/* GfxCanvas::draw
 * Draws the image/background/etc
 *******************************************************************/
void GfxCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_BLEND);

	// Draw the background
	drawChequeredBackground();

	// Draw the image
	drawImage();

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
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
	glScaled(s_size, s_size, 1.0f);

	// Draw a grid of squares
	for (int x = 0; x < cols; x++) {
		for (int y = 0; y < rows; y++) {
			// Set square colour
			rgba_t col(128, 128, 144, 255);
			if (x%2 != y%2)
				col.set(160, 160, 176, 255);
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

void GfxCanvas::drawImage() {
	// Save current matrix
	glPushMatrix();

	// Zoom
	glScaled(1, 1, 1);

	for (int x = 0; x < image->getWidth(); x++) {
		for (int y = 0; y < image->getHeight(); y++) {
			int a = (y*image->getWidth() + x) * 4;
			uint8_t* data = image->getRGBAData();

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

	// Restore previous matrix
	glPopMatrix();
}


BEGIN_EVENT_TABLE(GfxCanvas, wxGLCanvas)
	EVT_PAINT(GfxCanvas::paint)
	EVT_SIZE(GfxCanvas::resize)
END_EVENT_TABLE()

/* GfxCanvas::paint
 * Called when the gfx canvas has to be redrawn
 *******************************************************************/
void GfxCanvas::paint(wxPaintEvent& e) {
	if (!setContext())
		return;

	wxPaintDC(this);

	draw();
}

/* GfxCanvas::resize
 * Called when the gfx canvas is resized
 *******************************************************************/
void GfxCanvas::resize(wxSizeEvent& e) {
	OnSize(e);

	if (!context)
		return;

	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);
}



/* GfxEntryPanel::GfxEntryPanel
 * GfxEntryPanel class constructor
 *******************************************************************/
GfxEntryPanel::GfxEntryPanel(wxWindow* parent)
: EntryPanel(parent) {
	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Entry Contents"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);

	// Add gfx canvas
	gfx_canvas = new GfxCanvas(this, -1);
	framesizer->Add(gfx_canvas, 1, wxEXPAND|wxALL, 4);

	Layout();
}

/* GfxEntryPanel::~GfxEntryPanel
 * GfxEntryPanel class destructor
 *******************************************************************/
GfxEntryPanel::~GfxEntryPanel() {
}

/* GfxEntryPanel::loadEntry
 * Loads an entry into the entry panel if it is a valid image format
 *******************************************************************/
bool GfxEntryPanel::loadEntry(ArchiveEntry* entry) {
	if (entry->getType() == ETYPE_PNG)
		gfx_canvas->getImage()->loadPNG(entry->getData(true), entry->getSize());

	Refresh();

	return true;
}
