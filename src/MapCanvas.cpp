
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
: OGLCanvas(parent, id), timer(this) {
	// Init variables
	this->editor = editor;
	redraw = true;
	view_xoff = 0;
	view_yoff = 0;
	view_scale = 1;
	timer.Start(10);
	anim_flash_level = 0.5f;
	anim_flash_inc = true;

	// Load small font
	ArchiveEntry* entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans.ttf");
	if (entry)
		font_small.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	PreserveOpenGLStates(true);

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
	Bind(wxEVT_TIMER, &MapCanvas::onTimer, this);
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
	// Set the GL context to point to this window
	if (!setContext())
		return;

	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);
	SetView(sf::View(sf::FloatRect(0.0f, 0.0f, GetSize().x, GetSize().y)));

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

	// Draw overlay
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (editor->editMode() == MapEditor::MODE_VERTICES)
		drawVertexOverlay();
	else if (editor->editMode() == MapEditor::MODE_LINES)
		drawLineOverlay();

	SwapBuffers();
	redraw = false;
}

void MapCanvas::drawVertexOverlay() {
	// Setup text string
	sf::String str_vinfo;
	str_vinfo.SetFont(font_small);
	str_vinfo.SetSize(12);
	str_vinfo.SetPosition(2, GetSize().y - 16);

	// Get hilighted vertex
	MapVertex* vert = editor->getHilightedVertex();
	if (vert)
		str_vinfo.SetText(CHR(S_FMT("Vertex #%d: (%d, %d)", vert->getIndex(), (int)vert->xPos(), (int)vert->yPos())));
	else if (editor->selectionSize() > 0)
		str_vinfo.SetText(CHR(S_FMT("%d Vertices selected", editor->selectionSize())));
	else
		str_vinfo.SetText("No Vertices selected");

	// Draw overlay background
	rgba_t(0, 0, 0, 190, 0).set_gl();
	Drawing::drawFilledRect(0, GetSize().y - 16, GetSize().x, GetSize().y);
	glLineWidth(2.0f);
	rgba_t(255, 255, 255, 70, 0).set_gl();
	Drawing::drawLine(0, GetSize().y-16, GetSize().x, GetSize().y-16);

	// Draw text
	Draw(str_vinfo);
}

void MapCanvas::drawLineOverlay() {
	int bottom = GetSize().y;

	// General line info
	sf::String str_linfo("", font_small, 12);
	str_linfo.SetPosition(2, bottom - 64);

	// Draw overlay background
	rgba_t(0, 0, 0, 190, 0).set_gl();
	Drawing::drawFilledRect(0, bottom - 64, GetSize().x, bottom);
	rgba_t(255, 255, 255, 70, 0).set_gl();
	glLineWidth(2.0f);
	Drawing::drawLine(0, bottom-64, GetSize().x, bottom-64);

	// Get hilighted line
	MapLine* line = editor->getHilightedLine();
	if (!line) {
		// No line is hilighted, just display simple info (# selected, if any)
		if (editor->selectionSize() > 0)
			str_linfo.SetText(CHR(S_FMT("%d Lines selected", editor->selectionSize())));
		else
			str_linfo.SetText("No Lines selected");

		Draw(str_linfo);
		return;
	}

	// Line index
	str_linfo.SetText(CHR(S_FMT("Line #%d", line->getIndex())));
	Draw(str_linfo);

	// Line length
	int len = MathStuff::round(line->getLength());
	str_linfo.SetText(CHR(S_FMT("Length: %d", len)));
	str_linfo.SetPosition(2, bottom - 48);
	Draw(str_linfo);

	// Line special
	int special = line->prop("special");
	str_linfo.SetText(CHR(S_FMT("Special: %d", special)));
	str_linfo.SetPosition(2, bottom - 32);
	Draw(str_linfo);

	// Line sector tag
	int tag = line->prop("arg0");
	str_linfo.SetText(CHR(S_FMT("Sector Tag: %d", tag)));
	str_linfo.SetPosition(2, bottom - 16);
	Draw(str_linfo);

	
	// Side info
	int x = GetSize().x - 256;
	if (line->s1()) {
		drawSideOverlay(line->s1(), "Front", x);
		x -= 256;
	}
	if (line->s2())
		drawSideOverlay(line->s2(), "Back", x);
}

void MapCanvas::drawSideOverlay(MapSide* side, string side_name, int xstart) {
	int bottom = GetSize().y;

	if (!side)
		return;

	// Side info
	sf::String str_sinfo("", font_small, 12);
	str_sinfo.SetColor(sf::Color(255, 255, 255, 255));
	str_sinfo.SetPosition(xstart + 2, bottom - 32);

	// Index and sector index
	int sec = -1;
	if (side->getSector())
		sec = side->getSector()->getIndex();
	int index = side->getIndex();
	str_sinfo.SetText(CHR(S_FMT("%s Side #%d (Sector %d)", CHR(side_name), index, sec)));
	Draw(str_sinfo);

	// Texture offsets
	int xoff = side->prop("offsetx");
	int yoff = side->prop("offsety");
	str_sinfo.SetText(CHR(S_FMT("Offsets: (%d, %d)", xoff, yoff)));
	str_sinfo.SetPosition(xstart + 2, bottom - 16);
	Draw(str_sinfo);

	// Textures
	drawSideTexOverlay(xstart + 4, bottom - 36, side->prop("texturetop"));
	drawSideTexOverlay(xstart + 88, bottom - 36, side->prop("texturemiddle"), "Middle");
	drawSideTexOverlay(xstart + 92 + 80, bottom - 36, side->prop("texturebottom"), "Lower");
}

void MapCanvas::drawSideTexOverlay(int x, int y, string texture, string pos) {
	// Init string object
	sf::String str(CHR(pos), font_small, 12);
	float diff = 40 - (str.GetRect().GetWidth() * 0.5);

	// Check texture isn't blank
	if (!(S_CMPNOCASE(texture, "-"))) {
		// Draw background
		rgba_t(0, 0, 0, 190, 0).set_gl();
		Drawing::drawFilledRect(x, y, x + 80, y - 80);

		// Draw texture
		rgba_t(150, 150, 150, 255, 0).set_gl();
		Drawing::drawFilledRect(x, y, x + 80, y - 80);

		// Draw position text
		str.SetPosition(x + diff, y - 80);
		rgba_t(0, 0, 0, 140, 0).set_gl();
		Drawing::drawFilledRect(x, y - 80, x + 80, y - 64);
		Draw(str);

		// Draw border
		rgba_t(0, 0, 0, 255, 0).set_gl();
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.5f);
		Drawing::drawRect(x, y, x+80, y-80);
		glDisable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);

		// Draw texture name background
		rgba_t(0, 0, 0, 140, 0).set_gl();
		Drawing::drawFilledRect(x, y - 16, x + 80, y);
	}

	// Draw texture name (even if texture is blank)
	str.SetText(CHR(texture));
	diff = 40 - (str.GetRect().GetWidth() * 0.5);
	str.SetPosition(x + diff, y - 16);
	Draw(str);
}



void MapCanvas::onKeyDown(wxKeyEvent& e) {
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

	// Increment grid
	if (e.GetKeyCode() == ']') {
		editor->incrementGrid();
		redraw = true;
	}

	// Decrement grid
	if (e.GetKeyCode() == '[') {
		editor->decrementGrid();
		redraw = true;
	}

	// Vertices mode
	if (e.GetKeyCode() == 'V') {
		editor->setEditMode(MapEditor::MODE_VERTICES);
		redraw = true;
	}

	// Lines mode
	if (e.GetKeyCode() == 'L') {
		editor->setEditMode(MapEditor::MODE_LINES);
		redraw = true;
	}

	// Sectors mode
	if (e.GetKeyCode() == 'S') {
		editor->setEditMode(MapEditor::MODE_SECTORS);
		redraw = true;
	}

	// Things mode
	if (e.GetKeyCode() == 'T') {
		editor->setEditMode(MapEditor::MODE_THINGS);
		redraw = true;
	}

	e.Skip();
}

void MapCanvas::onMouseDown(wxMouseEvent& e) {
	// Get map coordinates of cursor
	double x = translateX(e.GetX());
	double y = translateY(e.GetY());

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

	e.Skip();
}

void MapCanvas::onMouseUp(wxMouseEvent& e) {
	// Left button up
	if (e.LeftUp()) {
		if (sel_active) {
			sel_active = false;
			editor->selectWithin(min(sel_origin.x, sel_end.x), min(sel_origin.y, sel_end.y),
								max(sel_origin.x, sel_end.x), max(sel_origin.y, sel_end.y));
			animations.push_back(new MCASelboxFader(stopwatch.Time(), sel_origin, sel_end));
		}
		
		editor->updateHilight();
		Refresh();
	}

	e.Skip();
}

void MapCanvas::onMouseMotion(wxMouseEvent& e) {
	// Get map coordinates of cursor
	double x = translateX(e.GetX());
	double y = translateY(e.GetY());

	// Update editor mouse tracking
	editor->setMousePos(x, y);

	// If dragging left mouse
	if (e.Dragging() && e.LeftIsDown()) {
		sel_end.set(x, y);
		redraw = true;
	}
	else {
		sel_active = false;

		// Update editor mouse hilight
		if (editor->updateHilight())
			redraw = true;
	}

	e.Skip();
}

void MapCanvas::onMouseWheel(wxMouseEvent& e) {
	if (e.GetWheelRotation() > 0)
		zoom(1.2, true);
	else if (e.GetWheelRotation() < 0)
		zoom(0.8, true);
}

void MapCanvas::onTimer(wxTimerEvent& e) {
	// Flashing animation for hilight
	// Pulsates between 0.5-1.0f (multiplied with hilight alpha)
	if (anim_flash_inc) {
		anim_flash_level += 0.03f;
		if (anim_flash_level >= 1.0f) {
			anim_flash_inc = false;
			anim_flash_level = 1.0f;
		}
	}
	else {
		anim_flash_level -= 0.03f;
		if (anim_flash_level <= 0.5f) {
			anim_flash_inc = true;
			anim_flash_level = 0.6f;
		}
	}

	// Update animations
	for (unsigned a = 0; a < animations.size(); a++) {
		if (!animations[a]->update(stopwatch.Time())) {
			// If animation is finished, delete and remove from the list
			delete animations[a];
			animations.erase(animations.begin() + a);
			a--;
		}
	}

	Update();
	Refresh();
}
