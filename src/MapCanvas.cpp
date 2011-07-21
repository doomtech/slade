
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
#include "MCAnimations.h"
#include "ArchiveManager.h"
#include "Drawing.h"
#include "MathStuff.h"
#include "MainApp.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, vertex_round, true, CVAR_SAVE)
CVAR(Bool, line_smooth, true, CVAR_SAVE)
CVAR(Int, vertex_size, 7, CVAR_SAVE)


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
	last_hilight = -1;
	anim_flash_level = 0.5f;
	anim_flash_inc = true;
	anim_info_fade = 0.0f;
	timer.Start(10);
	panning = false;

	// Bind Events
	Bind(wxEVT_SIZE, &MapCanvas::onSize, this);
	Bind(wxEVT_KEY_DOWN, &MapCanvas::onKeyDown, this);
	Bind(wxEVT_KEY_UP, &MapCanvas::onKeyUp, this);
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

void MapCanvas::setView(double x, double y) {
	// Set new view
	view_xoff = x;
	view_yoff = y;

	// Update screen limits
	view_tl.x = translateX(0);
	view_tl.y = translateY(GetSize().y);
	view_br.x = translateX(GetSize().x);
	view_br.y = translateY(0);
}

void MapCanvas::pan(double x, double y) {
	// Pan the view
	setView(view_xoff + x, view_yoff + y);
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
}

void MapCanvas::drawGrid() {
	// Get grid size
	int gridsize = editor->gridSize();

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

	// Draw 64 grid if it's not too small and we're not on a larger grid size
	if (64 > grid_hidelevel && gridsize < 64) {
		ColourConfiguration::getColour("map_64grid").set_gl();

		// Vertical
		int ofs = start_x % 64;
		for (int x = start_x-ofs; x <= end_x; x += 64) {
			glBegin(GL_LINES);
			glVertex2d(x, start_y);
			glVertex2d(x, end_y);
			glEnd();
		}

		// Horizontal
		ofs = start_y % 64;
		for (int y = start_y-ofs; y <= end_y; y += 64) {
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

	// Draw overlays (hilight etc)
	editor->drawSelection(view_tl.x, view_tl.y, view_br.x, view_br.y);
	if (!sel_active) editor->drawHilight(anim_flash_level);

	// Draw selection box if active
	if (sel_active) {
		// Get selection box map coordinates
		rgba_t col;

		// Outline
		ColourConfiguration::getColour("map_selbox_outline").set_gl();
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2d(sel_origin.x, sel_origin.y);
		glVertex2d(sel_origin.x, sel_end.y);
		glVertex2d(sel_end.x, sel_end.y);
		glVertex2d(sel_end.x, sel_origin.y);
		glEnd();

		// Fill
		ColourConfiguration::getColour("map_selbox_fill").set_gl();
		glBegin(GL_QUADS);
		glVertex2d(sel_origin.x, sel_origin.y);
		glVertex2d(sel_origin.x, sel_end.y);
		glVertex2d(sel_end.x, sel_end.y);
		glVertex2d(sel_end.x, sel_origin.y);
		glEnd();
	}

	// Draw animations
	for (unsigned a = 0; a < animations.size(); a++)
		animations[a]->draw();

	// Draw info overlay
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Check if we have to update the info
	if (editor->hilightItem() != last_hilight) {
		// Update hilight index
		last_hilight = editor->hilightItem();
		anim_info_show = (last_hilight != -1);

		// Update info overlay depending on edit mode
		switch (editor->editMode()) {
		case MapEditor::MODE_VERTICES:	info_vertex.update(editor->getHilightedVertex()); break;
		case MapEditor::MODE_LINES:		info_line.update(editor->getHilightedLine()); break;
		case MapEditor::MODE_SECTORS:	info_sector.update(editor->getHilightedSector()); break;
		}
	}

	// Draw current info overlay
	if (editor->editMode() == MapEditor::MODE_VERTICES)
		info_vertex.draw(GetSize().y, GetSize().x, anim_info_fade);
	else if (editor->editMode() == MapEditor::MODE_LINES)
		info_line.draw(GetSize().y, GetSize().x, anim_info_fade);
	else if (editor->editMode() == MapEditor::MODE_SECTORS)
		info_sector.draw(GetSize().y, GetSize().x, anim_info_fade);

	SwapBuffers();
}

void MapCanvas::update(long frametime) {
	// Get frame time multiplier
	float mult = (float)frametime / 10.0f;

	// Flashing animation for hilight
	// Pulsates between 0.5-1.0f (multiplied with hilight alpha)
	if (anim_flash_inc) {
		anim_flash_level += 0.015f*mult;
		if (anim_flash_level >= 1.0f) {
			anim_flash_inc = false;
			anim_flash_level = 1.0f;
		}
	}
	else {
		anim_flash_level -= 0.015f*mult;
		if (anim_flash_level <= 0.5f) {
			anim_flash_inc = true;
			anim_flash_level = 0.6f;
		}
	}

	// Fader for info overlay
	if (anim_info_show) {
		anim_info_fade += 0.1f*mult;
		if (anim_info_fade > 1.0f)
			anim_info_fade = 1.0f;
	}
	else {
		anim_info_fade -= 0.05f*mult;
		if (anim_info_fade < 0.0f)
			anim_info_fade = 0.0f;
	}

	// Update animations
	for (unsigned a = 0; a < animations.size(); a++) {
		if (!animations[a]->update(theApp->runTimer())) {
			// If animation is finished, delete and remove from the list
			delete animations[a];
			animations.erase(animations.begin() + a);
			a--;
		}
	}
}

void MapCanvas::onKeyBindPress(string name) {
	// Pan left
	if (name == "me2d_left")
		pan(-128/view_scale, 0);

	// Pan right
	else if (name == "me2d_right")
		pan(128/view_scale, 0);

	// Pan up
	else if (name == "me2d_up")
		pan(0, 128/view_scale);

	// Pan down
	else if (name == "me2d_down")
		pan(0, -128/view_scale);

	// Zoom out
	else if (name == "me2d_zoom_out")
		zoom(0.8);

	// Zoom in
	else if (name == "me2d_zoom_in")
		zoom(1.2);

	// Zoom out (follow mouse)
	else if (name == "me2d_zoom_out_m")
		zoom(0.8, true);

	// Zoom in (follow mouse)
	else if (name == "me2d_zoom_in_m")
		zoom(1.2, true);

	// Increment grid
	else if (name == "me2d_grid_inc")
		editor->incrementGrid();

	// Decrement grid
	else if (name == "me2d_grid_dec")
		editor->decrementGrid();

	// Vertices mode
	else if (name == "me2d_mode_vertices")
		editor->setEditMode(MapEditor::MODE_VERTICES);

	// Lines mode
	else if (name == "me2d_mode_lines")
		editor->setEditMode(MapEditor::MODE_LINES);

	// Sectors mode
	else if (name == "me2d_mode_sectors")
		editor->setEditMode(MapEditor::MODE_SECTORS);

	// Things mode
	else if (name == "me2d_mode_things")
		editor->setEditMode(MapEditor::MODE_THINGS);

	// Pan view
	else if (name == "me2d_pan_view") {
		pan_origin.set(mouse_relpos.x, mouse_relpos.y);
		panning = true;
		editor->clearHilight();
		SetCursor(wxCURSOR_SIZING);
	}
}

void MapCanvas::onKeyBindRelease(string name) {
	if (name == "me2d_pan_view") {
		panning = false;
		editor->updateHilight();
		SetCursor(wxNullCursor);
	}
}




void MapCanvas::onSize(wxSizeEvent& e) {
	// Update screen limits
	view_tl.x = translateX(0);
	view_tl.y = translateY(GetSize().y);
	view_br.x = translateX(GetSize().x);
	view_br.y = translateY(0);
}

void MapCanvas::onKeyDown(wxKeyEvent& e) {
	// Let keybind system handle it
	KeyBind::keyPressed(KeyBind::asKeyPress(e.GetKeyCode(), e.GetModifiers()));

	e.Skip();
}

void MapCanvas::onKeyUp(wxKeyEvent& e) {
	// Let keybind system handle it
	KeyBind::keyReleased(KeyBind::keyName(e.GetKeyCode()));

	e.Skip();
}

void MapCanvas::onMouseDown(wxMouseEvent& e) {
	// Get map coordinates of cursor
	double x = translateX(e.GetX());
	double y = translateY(e.GetY());
	editor->setMouseDownPos(x, y);

	// Left button down
	if (e.LeftDown()) {
		if (e.GetModifiers() == wxMOD_SHIFT) {
			// Shift held, begin box selection
			sel_origin.set(x, y);
			sel_end.set(x, y);
			sel_active = true;
		}
		else {
			// No shift, select any current hilight
			editor->selectCurrent();
		}
	}

	// Not left or right button (both reserved), let keybind system handle it
	else if (!e.RightDown())
		KeyBind::keyPressed(keypress_t(KeyBind::mbName(e.GetButton()), e.AltDown(), e.CmdDown(), e.ShiftDown()));

	e.Skip();
}

void MapCanvas::onMouseUp(wxMouseEvent& e) {
	// Left button up
	if (e.LeftUp()) {
		if (sel_active) {
			sel_active = false;
			editor->selectWithin(min(sel_origin.x, sel_end.x), min(sel_origin.y, sel_end.y),
								max(sel_origin.x, sel_end.x), max(sel_origin.y, sel_end.y));
			animations.push_back(new MCASelboxFader(theApp->runTimer(), sel_origin, sel_end));
		}

		editor->updateHilight();
	}

	// Not left or right button (both reserved), let keybind system handle it
	else if (!e.RightUp())
		KeyBind::keyReleased(KeyBind::mbName(e.GetButton()));

	e.Skip();
}

void MapCanvas::onMouseMotion(wxMouseEvent& e) {
	// Set mouse cursor position
	mouse_relpos.set(e.GetX(), e.GetY());

	// Get map coordinates of cursor
	double x = translateX(e.GetX());
	double y = translateY(e.GetY());

	// Update editor mouse tracking
	editor->setMousePos(x, y);

	// If dragging left mouse
	if (e.Dragging() && e.LeftIsDown())
		sel_end.set(x, y);
	else if (KeyBind::isPressed("me2d_pan_view")) {
		pan((pan_origin.x - e.GetX()) / view_scale, -((pan_origin.y - e.GetY()) / view_scale));
		pan_origin.set(e.GetX(), e.GetY());
	}
	else {
		sel_active = false;

		// Update editor mouse hilight
		editor->updateHilight();
	}

	e.Skip();
}

void MapCanvas::onMouseWheel(wxMouseEvent& e) {
	if (e.GetWheelRotation() > 0) {
		KeyBind::keyPressed(keypress_t("mwheelup", e.AltDown(), e.CmdDown(), e.ShiftDown()));
		KeyBind::keyReleased("mwheelup");
	}
	else if (e.GetWheelRotation() < 0) {
		KeyBind::keyPressed(keypress_t("mwheeldown", e.AltDown(), e.CmdDown(), e.ShiftDown()));
		KeyBind::keyReleased("mwheeldown");
	}
}
