
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


/* OGLCanvas::OGLCanvas
 * OGLCanvas class constructor
 *******************************************************************/
OGLCanvas::OGLCanvas(wxWindow* parent, int id)
: wxGLCanvas(parent, id, NULL, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE) {
	context = NULL;
	init_done = false;
}

/* OGLCanvas::OGLCanvas
 * OGLCanvas class constructor
 *******************************************************************/
OGLCanvas::~OGLCanvas() {
	if (context)
		delete context;
}

/* OGLCanvas::setContext
 * Sets the current gl context to the canvas' context, and creates
 * it if it doesn't exist. Returns true if the context is valid,
 * false otherwise
 *******************************************************************/
bool OGLCanvas::setContext() {
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

/* OGLCanvas::init
 * Initialises OpenGL settings for the GL canvas
 *******************************************************************/
void OGLCanvas::init() {
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


BEGIN_EVENT_TABLE(OGLCanvas, wxGLCanvas)
	EVT_PAINT(OGLCanvas::paint)
	EVT_ERASE_BACKGROUND(OGLCanvas::onEraseBackground)
END_EVENT_TABLE()

/* OGLCanvas::paint
 * Called when the gfx canvas has to be redrawn
 *******************************************************************/
void OGLCanvas::paint(wxPaintEvent& e) {
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
