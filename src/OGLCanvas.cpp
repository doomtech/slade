
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    OGLCanvas.cpp
 * Description: OGLCanvas class. Abstract base class for all SLADE
 *              wxGLCanvas-based UI elements
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
#include "OGLCanvas.h"
#include "GLTexture.h"


/*******************************************************************
 * OGLCANVAS CLASS FUNCTIONS
 *******************************************************************/

/* OGLCanvas::OGLCanvas
 * OGLCanvas class constructor
 *******************************************************************/
OGLCanvas::OGLCanvas(wxWindow* parent, int id)
: wxGLCanvas(parent, id, NULL, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE|wxWANTS_CHARS) {
	init_done = false;

	// Bind events
	Bind(wxEVT_PAINT, &OGLCanvas::onPaint, this);
	Bind(wxEVT_ERASE_BACKGROUND, &OGLCanvas::onEraseBackground, this);
}

/* OGLCanvas::OGLCanvas
 * OGLCanvas class constructor
 *******************************************************************/
OGLCanvas::~OGLCanvas() {
}

/* OGLCanvas::setContext
 * Sets the current gl context to the canvas' context, and creates
 * it if it doesn't exist. Returns true if the context is valid,
 * false otherwise
 *******************************************************************/
bool OGLCanvas::setContext() {
	wxGLContext* context = OpenGL::getContext(this);

	if (context) {
		context->SetCurrent(*this);
		return true;
	}
	else
		return false;
}

/* OGLCanvas::init
 * Initialises OpenGL settings for the GL canvas
 *******************************************************************/
void OGLCanvas::init() {
	if (!setContext())
		return;

#ifdef _WINDOWS
	if (glewInit() != GLEW_OK)
		wxLogMessage("GLEW init failed");
#endif

	glViewport(0, 0, GetSize().x, GetSize().y);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_NONE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	init_done = true;
}

/* OGLCanvas::drawCheckeredBackground
 * Fills the canvas with a checkered pattern (generally used as the
 * 'background' - to indicate transparency)
 *******************************************************************/
void OGLCanvas::drawCheckeredBackground() {
	// Save current matrix
	glPushMatrix();

	// Enable textures
	glEnable(GL_TEXTURE_2D);

	// Bind background texture
	GLTexture::bgTex().bind();

	// Draw background
	frect_t rect(0, 0, GetSize().x, GetSize().y);
	COL_WHITE.set_gl();
	glBegin(GL_QUADS);
	glTexCoord2d(rect.x1()*0.0625, rect.y1()*0.0625);	glVertex2d(rect.x1(), rect.y1());
	glTexCoord2d(rect.x1()*0.0625, rect.y2()*0.0625);	glVertex2d(rect.x1(), rect.y2());
	glTexCoord2d(rect.x2()*0.0625, rect.y2()*0.0625);	glVertex2d(rect.x2(), rect.y2());
	glTexCoord2d(rect.x2()*0.0625, rect.y1()*0.0625);	glVertex2d(rect.x2(), rect.y1());
	glEnd();

	// Disable textures
	glDisable(GL_TEXTURE_2D);

	// Restore previous matrix
	glPopMatrix();
}

/* OGLCanvas::toPanel
 * Places the canvas on top of a new wxPanel and returns the panel.
 * This is sometimes needed to fix redraw problems in Windows XP
 *******************************************************************/
wxPanel* OGLCanvas::toPanel(wxWindow* parent) {
	// Create panel
	wxPanel* panel = new wxPanel(parent, -1);

	// Reparent
	Reparent(panel);

	// Create sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	panel->SetSizer(sizer);

	// Add to sizer
	sizer->Add(this, 1, wxEXPAND);

	return panel;
}


/*******************************************************************
 * OGLCANVAS EVENTS
 *******************************************************************/

/* OGLCanvas::paint
 * Called when the gfx canvas has to be redrawn
 *******************************************************************/
void OGLCanvas::onPaint(wxPaintEvent& e) {
	wxPaintDC(this);

	if (!init_done)
		init();

	if (!setContext())
		return;

	draw();
}

/* OGLCanvas::onEraseBackground
 * Called when the gfx canvas background is to be erased (need to
 * override this to do nothing or the canvas will flicker in wxMSW)
 *******************************************************************/
void OGLCanvas::onEraseBackground(wxEraseEvent& e) {
	// Do nothing
}
