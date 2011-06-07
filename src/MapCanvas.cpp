
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
#include "MapEditor.h"
#include "ColourConfiguration.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, vertex_round, true, CVAR_SAVE)
CVAR(Bool, line_smooth, true, CVAR_SAVE)
CVAR(Int, vertex_size, 6, CVAR_SAVE)


/* MapCanvas::MapCanvas
 * MapCanvas class constructor
 *******************************************************************/
MapCanvas::MapCanvas(wxWindow *parent, int id, MapEditor* editor)
: OGLCanvas(parent, id) {
	// Init variables
	this->editor = editor;
	view_xoff = 0;
	view_yoff = 0;
	view_scale = 1;

	// Bind Events
	Bind(wxEVT_KEY_DOWN, &MapCanvas::onKeyDown, this);
	Bind(wxEVT_LEFT_DOWN, &MapCanvas::onMouseDown, this);
	Bind(wxEVT_RIGHT_DOWN, &MapCanvas::onMouseDown, this);
	Bind(wxEVT_MIDDLE_DOWN, &MapCanvas::onMouseDown, this);
	Bind(wxEVT_AUX1_DOWN, &MapCanvas::onMouseDown, this);
	Bind(wxEVT_AUX2_DOWN, &MapCanvas::onMouseDown, this);
	Bind(wxEVT_LEFT_UP, &MapCanvas::onMouseUp, this);
	Bind(wxEVT_RIGHT_UP, &MapCanvas::onMouseUp, this);
	Bind(wxEVT_MIDDLE_UP, &MapCanvas::onMouseUp, this);
	Bind(wxEVT_AUX1_UP, &MapCanvas::onMouseUp, this);
	Bind(wxEVT_AUX2_UP, &MapCanvas::onMouseUp, this);
	Bind(wxEVT_MOTION, &MapCanvas::onMouseMotion, this);
	Bind(wxEVT_MOUSEWHEEL, &MapCanvas::onMouseWheel, this);
}

/* MapCanvas::~MapCanvas
 * MapCanvas class destructor
 *******************************************************************/
MapCanvas::~MapCanvas() {
}

/* MapCanvas::translateX
 * Translates an x position on the canvas to the corresponding x
 * position on the map itself
 *******************************************************************/
double MapCanvas::translateX(double x) {
	return double(x / view_scale) + view_xoff - (double(GetSize().x * 0.5) / view_scale);
}

/* MapCanvas::translateY
 * Translates a y position on the canvas to the corresponding y
 * position on the map itself
 *******************************************************************/
double MapCanvas::translateY(double y) {
	return double(-y / view_scale) + view_yoff + (double(GetSize().y * 0.5) / view_scale);
}

void MapCanvas::pan(double x, double y) {
	// Pan the view
	view_xoff += x/view_scale;
	view_yoff += y/view_scale;

	// Update screen limits
	view_tl.x = translateX(0);
	view_tl.y = translateY(GetSize().y);
	view_br.x = translateX(GetSize().x);
	view_br.y = translateY(0);

	// Refresh
	Update();
	Refresh();
}

void MapCanvas::zoom(double amount, bool toward_cursor) {
	// Zoom view
	view_scale = view_scale * amount;

	// Check for zoom limits
	bool limit = false;
	if (view_scale < 0.005) {
		// Min scale
		view_scale = 0.005;
		limit = true;
	}
	if (view_scale > 10.0) {
		// Max scale
		view_scale = 10.0;
		limit = true;
	}

	if (!limit && toward_cursor) {
		double mx = editor->mousePos().x;
		double my = editor->mousePos().y;
		view_xoff = mx + (double(view_xoff - mx) / amount);
		view_yoff = my + (double(view_yoff - my) / amount);
	}

	// Update screen limits
	view_tl.x = translateX(0);
	view_tl.y = translateY(GetSize().y);
	view_br.x = translateX(GetSize().x);
	view_br.y = translateY(0);

	// Refresh
	Update();
	Refresh();
}

void MapCanvas::drawGrid() {
	// Temp
	int gridsize = 32;

	// Disable line smoothing (not needed for straight, 1.0-sized lines)
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	ColourConfiguration::getColour("map_grid").set_gl();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Determine smallest grid size to bother drawing
	int grid_hidelevel = 2.0 / view_scale;

	// Determine canvas edges in map coordinates
	int start_x = translateX(0);
	int end_x = translateX(GetSize().x);
	int start_y = translateY(GetSize().y);
	int end_y = translateY(0);

	// Draw regular grid if it's not too small
	if (gridsize > grid_hidelevel) {
		// Vertical
		int ofs = start_x % gridsize;
		for (int x = start_x-ofs; x <= end_x; x += gridsize) {
			glBegin(GL_LINES);
			glVertex2d(x, start_y);
			glVertex2d(x, end_y);
			glEnd();
		}

		// Horizontal
		ofs = start_y % gridsize;
		for (int y = start_y-ofs; y <= end_y; y += gridsize) {
			glBegin(GL_LINES);
			glVertex2d(start_x, y);
			glVertex2d(end_x, y);
			glEnd();
		}
	}
}

/* MapCanvas::draw
 * Draw the 2d map on the map gl canvas
 *******************************************************************/
void MapCanvas::draw() {
	// Set the GL context to point to this window
	if (!setContext())
		return;

	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GetSize().x, 0.0f, GetSize().y, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Translate to middle of pixel (otherwise inaccuracies can occur on certain gl implemenataions)
	glTranslatef(0.375f, 0.375f, 0);

	// Translate to middle of screen
	glTranslated(GetSize().x * 0.5, GetSize().y * 0.5, 0);

	// Zoom
	glScaled(view_scale, view_scale, 1);

	// Translate to offsets
	glTranslated(-view_xoff, -view_yoff, 0);

	// Draw grid
	drawGrid();

	// Setup options
	if (vertex_round)
		glEnable(GL_POINT_SMOOTH);
	else
		glDisable(GL_POINT_SMOOTH);
	if (line_smooth)
		glEnable(GL_LINE_SMOOTH);
	else
		glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.5f);
	glPointSize((float)vertex_size);

	// Draw map
	editor->drawMap(view_tl.x, view_tl.y, view_br.x, view_br.y);

	SwapBuffers();
}



void MapCanvas::onKeyDown(wxKeyEvent& e) {
	bool refresh = false;

	// Pan left
	if (e.GetKeyCode() == WXK_LEFT)
		pan(-128, 0);

	// Pan right
	if (e.GetKeyCode() == WXK_RIGHT)
		pan(128, 0);

	// Pan up
	if (e.GetKeyCode() == WXK_UP)
		pan(0, 128);

	// Pan down
	if (e.GetKeyCode() == WXK_DOWN)
		pan(0, -128);

	// Zoom out
	if (e.GetKeyCode() == '-')
		zoom(0.8);

	// Zoom in
	if (e.GetKeyCode() == '=')
		zoom(1.2);

	// Vertices mode
	if (e.GetKeyCode() == 'V') {
		editor->setEditMode(MapEditor::MODE_VERTICES);
		refresh = true;
	}

	// Lines mode
	if (e.GetKeyCode() == 'L') {
		editor->setEditMode(MapEditor::MODE_LINES);
		refresh = true;
	}

	// Sectors mode
	if (e.GetKeyCode() == 'S') {
		editor->setEditMode(MapEditor::MODE_SECTORS);
		refresh = true;
	}

	// Things mode
	if (e.GetKeyCode() == 'T') {
		editor->setEditMode(MapEditor::MODE_THINGS);
		refresh = true;
	}

	if (refresh)
		Refresh();

	e.Skip();
}

void MapCanvas::onMouseDown(wxMouseEvent& e) {
	e.Skip();
}

void MapCanvas::onMouseUp(wxMouseEvent& e) {
	e.Skip();
}

void MapCanvas::onMouseMotion(wxMouseEvent& e) {
	e.Skip();

	// Update editor mouse tracking and hilight
	editor->setMousePos(translateX(e.GetPosition().x), translateY(e.GetPosition().y));
	if (editor->updateHilight())
		Refresh();
}

void MapCanvas::onMouseWheel(wxMouseEvent& e) {
	if (e.GetWheelRotation() > 0)
		zoom(1.2, true);
	else if (e.GetWheelRotation() < 0)
		zoom(0.8, true);
}
