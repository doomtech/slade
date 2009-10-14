
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MapCanvas.cpp
 * Description: MapCanvas class, the OpenGL canvas widget that the
 *              2d map view is drawn on
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
#include "MapCanvas.h"
#include "SLADEMap.h"
#include <wx/log.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
wxGLContext *glcontext_map;


/* MapCanvas::MapCanvas
 * MapCanvas class constructor
 *******************************************************************/
MapCanvas::MapCanvas(wxWindow *parent, int id, SLADEMap* map)
: wxGLCanvas(parent, id, NULL) {
	init_done = false;
	this->map = map;
}

/* MapCanvas::~MapCanvas
 * MapCanvas class destructor
 *******************************************************************/
MapCanvas::~MapCanvas() {
}

/* MapCanvas::init
 * Initialises OpenGL settings for the map canvas
 *******************************************************************/
void MapCanvas::init()
{
	if (!setContext())
		return;

	glViewport(0, 0, GetSize().x, GetSize().y);
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

	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	init_done = true;
}

/* MapCanvas::draw
 * Draw the 2d map on the map gl canvas
 *******************************************************************/
void MapCanvas::draw() {
	if (!init_done)
		init();

	// Set the GL context to point to this window
	if (!setContext())
		return;

	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//rgba_t col_background(0, 0, 0);
	//glClearColor(col_background.fr(), col_background.fg(), col_background.fb(), 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	map->drawLines();
	map->drawVertices();

	SwapBuffers();
}

/* MapCanvas::setContext
 * Sets the program GL context to draw to the map canvas, creates
 * the context if it doesn't yet exist
 *******************************************************************/
bool MapCanvas::setContext() {
	if (!glcontext_map) {
		if (IsShown())
			glcontext_map = new wxGLContext(this);
	}

	if (glcontext_map) {
		glcontext_map->SetCurrent(*this);
		return true;
	}
	else
		return false;
}

BEGIN_EVENT_TABLE(MapCanvas, wxGLCanvas)
	EVT_PAINT(MapCanvas::paint)
	//EVT_SIZE(MapCanvas::resize)
END_EVENT_TABLE()

/* MapCanvas::paint
 * Called when the map canvas has to be redrawn
 *******************************************************************/
void MapCanvas::paint(wxPaintEvent& event) {
	//wxLogMessage(_T("MapCanvas paint"));
	wxPaintDC(this);
	draw();
}

/* MapCanvas::resize
 * Called when the map canvas is resized
 *******************************************************************/
void MapCanvas::resize(wxSizeEvent& event)
{
	// Set the GL context to point to this window
	if (!setContext())
		return;

	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	draw();
	//OnSize(event);
}
