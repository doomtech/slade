
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
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
#include "MapEditorWindow.h"
#include "MapRenderer2D.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, things_always, 1, CVAR_SAVE)
CVAR(Bool, vertices_always, 0, CVAR_SAVE)
CVAR(Bool, line_tabs_always, 0, CVAR_SAVE)
CVAR(Bool, grid_dashed, false, CVAR_SAVE)
CVAR(Bool, scroll_smooth, true, CVAR_SAVE)
CVAR(Int, flat_drawtype, 2, CVAR_SAVE)
CVAR(Bool, selection_clear_click, false, CVAR_SAVE)
CVAR(Bool, map_showfps, false, CVAR_SAVE)
PolygonSplitter splitter;	// for testing


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(Int, vertex_size)


/* MapCanvas::MapCanvas
 * MapCanvas class constructor
 *******************************************************************/
MapCanvas::MapCanvas(wxWindow *parent, int id, MapEditor* editor)
: OGLCanvas(parent, id) {
	// Init variables
	this->editor = editor;
	editor->setCanvas(this);
	view_xoff = 0;
	view_yoff = 0;
	view_scale = 1;
	last_hilight = -1;
	anim_flash_level = 0.5f;
	anim_flash_inc = true;
	anim_info_fade = 0.0f;
	mouse_state = MSTATE_NORMAL;
	mouse_downpos.set(-1, -1);
	fr_idle = 0;
	last_time = 0;
	renderer_2d = new MapRenderer2D(&editor->getMap());
	view_xoff_inter = 0;
	view_yoff_inter = 0;
	view_scale_inter = 1;
	anim_view_speed = 0.05;
	zooming_cursor = false;
	mouse_selbegin = false;

	timer.Start(2);
#ifdef USE_SFML_RENDERWINDOW
	UseVerticalSync(false);
#endif

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
	Bind(wxEVT_LEAVE_WINDOW, &MapCanvas::onMouseLeave, this);
	Bind(wxEVT_ENTER_WINDOW, &MapCanvas::onMouseEnter, this);
	Bind(wxEVT_TIMER, &MapCanvas::onTimer, this);
#ifdef USE_SFML_RENDERWINDOW
	Bind(wxEVT_IDLE, &MapCanvas::onIdle, this);
#endif
}

/* MapCanvas::~MapCanvas
 * MapCanvas class destructor
 *******************************************************************/
MapCanvas::~MapCanvas() {
	delete renderer_2d;
}

/* MapCanvas::translateX
 * Translates an x position on the canvas to the corresponding x
 * position on the map itself
 *******************************************************************/
double MapCanvas::translateX(double x, bool inter) {
	if (inter)
		return double(x / view_scale_inter) + view_xoff_inter - (double(GetSize().x * 0.5) / view_scale_inter);
	else
		return double(x / view_scale) + view_xoff - (double(GetSize().x * 0.5) / view_scale);
}

/* MapCanvas::translateY
 * Translates a y position on the canvas to the corresponding y
 * position on the map itself
 *******************************************************************/
double MapCanvas::translateY(double y, bool inter) {
	if (inter)
		return double(-y / view_scale_inter) + view_yoff_inter + (double(GetSize().y * 0.5) / view_scale_inter);
	else
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

	// Update object visibility
	renderer_2d->updateVisibility(view_tl, view_br);
}

void MapCanvas::pan(double x, double y) {
	// Pan the view
	setView(view_xoff + x, view_yoff + y);
}

void MapCanvas::zoom(double amount, bool toward_cursor) {
	// Get current mouse map coordinates
	double mx = translateX(mouse_pos.x);
	double my = translateY(mouse_pos.y);

	// Zoom view
	view_scale = view_scale * amount;

	// Check for zoom limits
	if (view_scale < 0.005)	// Min scale
		view_scale = 0.005;
	if (view_scale > 10.0) // Max scale
		view_scale = 10.0;

	// Do 'zoom towards cursor' stuff
	if (toward_cursor) {
		view_xoff += mx - translateX(mouse_pos.x);
		view_yoff += my - translateY(mouse_pos.y);
	}

	// Update screen limits
	view_tl.x = translateX(0);
	view_tl.y = translateY(GetSize().y);
	view_br.x = translateX(GetSize().x);
	view_br.y = translateY(0);

	// Update object visibility
	renderer_2d->updateVisibility(view_tl, view_br);
}

void MapCanvas::viewFitToMap(bool snap) {
	// Get the map bbox
	bbox_t map_bbox = editor->getMap().getMapBBox();

	// Reset zoom and set offsets to the middle of the map
	view_scale = 2;
	view_xoff = map_bbox.min.x + ((map_bbox.max.x - map_bbox.min.x) * 0.5);
	view_yoff = map_bbox.min.y + ((map_bbox.max.y - map_bbox.min.y) * 0.5);

	// Now just keep zooming out until we fit the whole map in the view
	bool done = false;
	while (!done) {
		// Update screen limits
		view_tl.x = translateX(0);
		view_tl.y = translateY(GetSize().y);
		view_br.x = translateX(GetSize().x);
		view_br.y = translateY(0);

		if (map_bbox.min.x >= view_tl.x &&
			map_bbox.max.x <= view_br.x &&
			map_bbox.min.y >= view_tl.y &&
			map_bbox.max.y <= view_br.y)
			done = true;
		else
			view_scale *= 0.8;
	}

	// Don't animate if specified
	if (snap) {
		view_xoff_inter = view_xoff;
		view_yoff_inter = view_yoff;
		view_scale_inter = view_scale;
	}

	// Update object visibility
	renderer_2d->forceUpdate();
	renderer_2d->updateVisibility(view_tl, view_br);
}

void MapCanvas::viewShowObject() {
	// Determine bbox of hilighted or selected object(s)
	bbox_t bbox;
	SLADEMap& map = editor->getMap();

	// Create list of objects
	vector<int> objects;
	vector<int>& selection = editor->getSelection();
	if (selection.size() > 0) {
		for (unsigned a = 0; a < selection.size(); a++)
			objects.push_back(selection[a]);
	}
	else if (editor->hilightItem() >= 0)
		objects.push_back(editor->hilightItem());
	else
		return;	// Nothing selected or hilighted

	// Generate bbox (depending on object type)
	if (editor->editMode() == MapEditor::MODE_VERTICES) {
		for (unsigned a = 0; a < objects.size(); a++)
			bbox.extend(map.getVertex(objects[a])->xPos(), map.getVertex(objects[a])->yPos());
	}
	else if (editor->editMode() == MapEditor::MODE_LINES) {
		MapLine* line;
		for (unsigned a = 0; a < objects.size(); a++) {
			line = map.getLine(objects[a]);
			bbox.extend(line->v1()->xPos(), line->v1()->yPos());
			bbox.extend(line->v2()->xPos(), line->v2()->yPos());
		}
	}
	else if (editor->editMode() == MapEditor::MODE_SECTORS) {
		bbox = map.getSector(objects[0])->boundingBox();
		for (unsigned a = 1; a < objects.size(); a++) {
			bbox_t sbb = map.getSector(objects[a])->boundingBox();
			if (sbb.min.x < bbox.min.x)
				bbox.min.x = sbb.min.x;
			if (sbb.min.y < bbox.min.y)
				bbox.min.y = sbb.min.y;
			if (sbb.max.x > bbox.max.x)
				bbox.max.x = sbb.max.x;
			if (sbb.max.y > bbox.max.y)
				bbox.max.y = sbb.max.y;
		}
	}
	else if (editor->editMode() == MapEditor::MODE_THINGS) {
		for (unsigned a = 0; a < objects.size(); a++)
			bbox.extend(map.getThing(objects[a])->xPos(), map.getThing(objects[a])->yPos());
	}

	// Reset zoom and set offsets to the middle of the object(s)
	view_scale = 2;
	view_xoff = bbox.min.x + ((bbox.max.x - bbox.min.x) * 0.5);
	view_yoff = bbox.min.y + ((bbox.max.y - bbox.min.y) * 0.5);

	// Now just keep zooming out until we fit the whole map in the view
	bool done = false;
	while (!done) {
		// Update screen limits
		view_tl.x = translateX(0);
		view_tl.y = translateY(GetSize().y);
		view_br.x = translateX(GetSize().x);
		view_br.y = translateY(0);

		if (bbox.min.x >= view_tl.x &&
			bbox.max.x <= view_br.x &&
			bbox.min.y >= view_tl.y &&
			bbox.max.y <= view_br.y)
			done = true;
		else
			view_scale *= 0.8;
	}

	// Update object visibility
	renderer_2d->updateVisibility(view_tl, view_br);
}

void MapCanvas::viewMatchSpot(double mx, double my, double sx, double sy) {
	setView(view_xoff_inter + mx - translateX(sx, true), view_yoff_inter + my - translateY(sy, true));
}

void MapCanvas::drawGrid() {
	// Get grid size
	int gridsize = editor->gridSize();

	// Disable line smoothing (not needed for straight, 1.0-sized lines)
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	// Enable dashed lines if needed
	if (grid_dashed) {
		glLineStipple(2, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
	}

	ColourConfiguration::getColour("map_grid").set_gl(false);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Determine smallest grid size to bother drawing
	int grid_hidelevel = 2.0 / view_scale;

	// Determine canvas edges in map coordinates
	int start_x = translateX(0, true);
	int end_x = translateX(GetSize().x, true);
	int start_y = translateY(GetSize().y, true);
	int end_y = translateY(0, true);

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

	glDisable(GL_LINE_STIPPLE);
}

void MapCanvas::drawEditorMessages() {
	// Go through editor messages
	int yoff = 0;
	if (map_showfps) yoff = 16;
	for (unsigned a = 0; a < editor->numEditorMessages(); a++) {
		// Check message time
		long time = editor->getEditorMessageTime(a);
		if (time > 2000)
			continue;

		// Setup message colour
		rgba_t col = ColourConfiguration::getColour("map_editor_message");
		if (time < 200) {
			float flash = 1.0f - (time / 200.0f);
			col.r += (255-col.r)*flash;
			col.g += (255-col.g)*flash;
			col.b += (255-col.b)*flash;
		}

		// Setup message alpha
		if (time > 1500)
			col.a = 255 - (double((time - 1500) / 500.0) * 255);

		// Draw message 'shadow'
		Drawing::drawText(editor->getEditorMessage(a), 1, yoff+1, rgba_t(0, 0, 0, col.a), Drawing::FONT_BOLD);

		// Draw message
		Drawing::drawText(editor->getEditorMessage(a), 0, yoff, col, Drawing::FONT_BOLD);
		yoff += 16;
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

	rgba_t col_bg = ColourConfiguration::getColour("map_background");
	glClearColor(col_bg.fr(), col_bg.fg(), col_bg.fb(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_COLOR_MATERIAL);

	// Translate to inside of pixel (otherwise inaccuracies can occur on certain gl implemenataions)
	glTranslatef(0.375f, 0.375f, 0);

	// Translate to middle of screen
	glTranslated(GetSize().x * 0.5, GetSize().y * 0.5, 0);

	// Zoom
	glScaled(view_scale_inter, view_scale_inter, 1);

	// Translate to offsets
	glTranslated(-view_xoff_inter, -view_yoff_inter, 0);

	// Draw flats if needed
	if (flat_drawtype > 0) {
		COL_WHITE.set_gl();
		renderer_2d->renderFlats(flat_drawtype-1);
	}

	// Draw grid
	drawGrid();

	// --- Draw map (depending on mode) ---

	if (editor->editMode() == MapEditor::MODE_VERTICES) {
		// Vertices mode
		if (things_always) renderer_2d->renderThings(0.5f);	// Things (faded)
		renderer_2d->renderLines(line_tabs_always);			// Lines (no direction tabs)

		// Vertices
		if (mouse_state == MSTATE_MOVE)
			renderer_2d->renderVertices(0.25f);
		else
			renderer_2d->renderVertices();

		// Selection if needed
		if (mouse_state != MSTATE_MOVE)
			renderer_2d->renderVertexSelection(editor->getSelection());

		// Hilight if needed
		if (mouse_state == MSTATE_NORMAL)
			renderer_2d->renderVertexHilight(editor->hilightItem(), anim_flash_level);
	}
	else if (editor->editMode() == MapEditor::MODE_LINES) {
		// Lines mode
		if (things_always) renderer_2d->renderThings(0.5f);			// Things (faded)
		if (vertices_always) renderer_2d->renderVertices(0.5f);		// Vertices (faded)
		renderer_2d->renderLines(true);								// Lines

		// Selection if needed
		if (mouse_state != MSTATE_MOVE)
			renderer_2d->renderLineSelection(editor->getSelection());

		// Hilight if needed
		if (mouse_state == MSTATE_NORMAL)
			renderer_2d->renderLineHilight(editor->hilightItem(), anim_flash_level);
	}
	else if (editor->editMode() == MapEditor::MODE_SECTORS) {
		// Sectors mode
		if (things_always) renderer_2d->renderThings(0.5f);			// Things (faded)
		if (vertices_always) renderer_2d->renderVertices(0.5f);		// Vertices (faded)
		renderer_2d->renderLines(line_tabs_always);					// Lines (no direction tabs)

		// Selection if needed
		if (mouse_state != MSTATE_MOVE)
			renderer_2d->renderFlatSelection(editor->getSelection());

		splitter.testRender();	// Testing

		// Hilight if needed
		if (mouse_state == MSTATE_NORMAL)
			renderer_2d->renderFlatHilight(editor->hilightItem(), anim_flash_level);
	}
	else if (editor->editMode() == MapEditor::MODE_THINGS) {
		// Things mode
		if (vertices_always) renderer_2d->renderVertices(0.5f);		// Vertices (faded)
		renderer_2d->renderLines(line_tabs_always);					// Lines (no direction tabs)
		renderer_2d->renderThings();								// Things

		// Selection if needed
		if (mouse_state != MSTATE_MOVE)
			renderer_2d->renderThingSelection(editor->getSelection());

		// Hilight if needed
		if (mouse_state == MSTATE_NORMAL)
			renderer_2d->renderThingHilight(editor->hilightItem(), anim_flash_level);
	}


	// Draw tagged sectors/lines/things if needed
	if (editor->taggedSectors().size() > 0 && mouse_state == MSTATE_NORMAL)
		renderer_2d->renderTaggedFlats(editor->taggedSectors(), anim_flash_level);
	else if (editor->taggedLines().size() > 0 && mouse_state == MSTATE_NORMAL)
		renderer_2d->renderTaggedLines(editor->taggedLines(), anim_flash_level);
	else if (editor->taggedThings().size() > 0 && mouse_state == MSTATE_NORMAL)
		renderer_2d->renderTaggedThings(editor->taggedThings(), anim_flash_level);


	// Draw selection box if active
	if (mouse_state == MSTATE_SELECTION) {
		// Outline
		ColourConfiguration::getColour("map_selbox_outline").set_gl();
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2d(mouse_downpos_m.x, mouse_downpos_m.y);
		glVertex2d(mouse_downpos_m.x, mouse_pos_m.y);
		glVertex2d(mouse_pos_m.x, mouse_pos_m.y);
		glVertex2d(mouse_pos_m.x, mouse_downpos_m.y);
		glEnd();

		// Fill
		ColourConfiguration::getColour("map_selbox_fill").set_gl();
		glBegin(GL_QUADS);
		glVertex2d(mouse_downpos_m.x, mouse_downpos_m.y);
		glVertex2d(mouse_downpos_m.x, mouse_pos_m.y);
		glVertex2d(mouse_pos_m.x, mouse_pos_m.y);
		glVertex2d(mouse_pos_m.x, mouse_downpos_m.y);
		glEnd();
	}

	// Draw animations
	for (unsigned a = 0; a < animations.size(); a++)
		animations[a]->draw();

	// Draw moving stuff if needed
	if (mouse_state == MSTATE_MOVE) {
		switch (editor->editMode()) {
		case MapEditor::MODE_VERTICES:
			renderer_2d->renderMovingVertices(editor->movingItems(), editor->moveVector()); break;
		case MapEditor::MODE_LINES:
			renderer_2d->renderMovingLines(editor->movingItems(), editor->moveVector()); break;
		case MapEditor::MODE_SECTORS:
			renderer_2d->renderMovingSectors(editor->movingItems(), editor->moveVector()); break;
		case MapEditor::MODE_THINGS:
			renderer_2d->renderMovingThings(editor->movingItems(), editor->moveVector()); break;
		default: break;
		};
	}

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
		case MapEditor::MODE_THINGS:	info_thing.update(editor->getHilightedThing()); break;
		}
	}

	// Draw current info overlay
	glDisable(GL_TEXTURE_2D);
	if (editor->editMode() == MapEditor::MODE_VERTICES)
		info_vertex.draw(GetSize().y, GetSize().x, anim_info_fade);
	else if (editor->editMode() == MapEditor::MODE_LINES)
		info_line.draw(GetSize().y, GetSize().x, anim_info_fade);
	else if (editor->editMode() == MapEditor::MODE_SECTORS)
		info_sector.draw(GetSize().y, GetSize().x, anim_info_fade);
	else if (editor->editMode() == MapEditor::MODE_THINGS)
		info_thing.draw(GetSize().y, GetSize().x, anim_info_fade);

	// FPS counter
	if (map_showfps) {
		if (frametime_last > 0) {
			int fps = 1.0 / (frametime_last/1000.0);
			fps_avg.push_back(fps);
			if (fps_avg.size() > 20) fps_avg.erase(fps_avg.begin());
		}
		int afps = 0;
		for (unsigned a = 0; a < fps_avg.size(); a++)
			afps += fps_avg[a];
		if (fps_avg.size() > 0) afps /= fps_avg.size();
		Drawing::drawText(S_FMT("FPS: %d", afps));
	}

	// Editor messages
	drawEditorMessages();

	SwapBuffers();
}

void MapCanvas::update(long frametime) {
	// Get frame time multiplier
	float mult = (float)frametime / 10.0f;

	// Update hilight if needed
	if (mouse_state == MSTATE_NORMAL)
		editor->updateHilight(mouse_pos_m, view_scale);

	// Do item moving if needed
	if (mouse_state == MSTATE_MOVE)
		editor->doMove(mouse_pos_m);

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
	bool fade_anim = true;
	if (anim_info_show) {
		anim_info_fade += 0.1f*mult;
		if (anim_info_fade > 1.0f) {
			anim_info_fade = 1.0f;
			fade_anim = false;
		}
	}
	else {
		anim_info_fade -= 0.04f*mult;
		if (anim_info_fade < 0.0f) {
			anim_info_fade = 0.0f;
			fade_anim = false;
		}
	}

	// View pan/zoom animation
	bool view_anim = false;
	if (scroll_smooth) {
		// Scale
		double diff_scale = view_scale - view_scale_inter;
		if (diff_scale < -0.0000001 || diff_scale > 0.0000001) {
			// Get current mouse position in map coordinates (for zdooming towards cursor)
			double mx = translateX(mouse_pos.x, true);
			double my = translateY(mouse_pos.y, true);

			// Interpolate zoom
			view_scale_inter += diff_scale*anim_view_speed*mult;

			// Check for zoom finish
			if (diff_scale < 0 && view_scale_inter < view_scale ||
				diff_scale > 0 && view_scale_inter > view_scale)
				view_scale_inter = view_scale;
			else
				view_anim = true;

			if (zooming_cursor) {
				viewMatchSpot(mx, my, mouse_pos.x, mouse_pos.y);
				view_xoff_inter = view_xoff;
				view_yoff_inter = view_yoff;
			}
		}
		else
			view_scale_inter = view_scale;

		if (!zooming_cursor) {
			// X offset
			double diff_xoff = view_xoff - view_xoff_inter;
			if (diff_xoff < -0.05 || diff_xoff > 0.05) {
				// Interpolate offset
				view_xoff_inter += diff_xoff*anim_view_speed*mult;

				// Check stuff
				if (diff_xoff < 0 && view_xoff_inter < view_xoff ||
					diff_xoff > 0 && view_xoff_inter > view_xoff)
					view_xoff_inter = view_xoff;
				else
					view_anim = true;
			}
			else
				view_xoff_inter = view_xoff;

			// Y offset
			double diff_yoff = view_yoff - view_yoff_inter;
			if (diff_yoff < -0.05 || diff_yoff > 0.05) {
				// Interpolate offset
				view_yoff_inter += diff_yoff*anim_view_speed*mult;

				if (diff_yoff < 0 && view_yoff_inter < view_yoff ||
					diff_yoff > 0 && view_yoff_inter > view_yoff)
					view_yoff_inter = view_yoff;
				else
					view_anim = true;
			}
			else
				view_yoff_inter = view_yoff;
		}

		if (!view_anim)
			anim_view_speed = 0.05;
		else {
			anim_view_speed += 0.05*mult;
			if (anim_view_speed > 0.4)
				anim_view_speed = 0.4;
		}
	}
	else {
		view_xoff_inter = view_xoff;
		view_yoff_inter = view_yoff;
		view_scale_inter = view_scale;
	}

	if (!view_anim)
		zooming_cursor = false;

	// Update renderer scale
	renderer_2d->setScale(view_scale_inter);

	// Update animations
	bool anim_running = false;
	for (unsigned a = 0; a < animations.size(); a++) {
		if (!animations[a]->update(theApp->runTimer())) {
			// If animation is finished, delete and remove from the list
			delete animations[a];
			animations.erase(animations.begin() + a);
			a--;
		}
		else
			anim_running = true;
	}

	// Determine the framerate limit
#ifdef USE_SFML_RENDERWINDOW
	// SFML RenderWindow can handle high framerates better than wxGLCanvas, or something like that
	if (mouse_state == MSTATE_SELECTION || mouse_state == MSTATE_PANNING || anim_running || fade_anim || view_anim)
		fr_idle = 2;
	else	// No high-priority animations running, throttle framerate
		fr_idle = 25;
#else
	if (mouse_state == MSTATE_SELECTION || mouse_state == MSTATE_PANNING || anim_running || fade_anim || view_anim)
		fr_idle = 5;
	else	// No high-priority animations running, throttle framerate
		fr_idle = 30;
#endif

	frametime_last = frametime;
}

void MapCanvas::itemSelected(int index, bool selected) {
	// Things mode
	if (editor->editMode() == MapEditor::MODE_THINGS) {
		// Get thing
		MapThing* t = editor->getMap().getThing(index);
		if (!t) return;

		// Get thing type
		ThingType* tt = theGameConfiguration->thingType(t->getType());

		// Start animation
		double radius = tt->getRadius();
		if (tt->shrinkOnZoom()) radius = renderer_2d->scaledRadius(radius);
		animations.push_back(new MCAThingSelection(theApp->runTimer(), t->xPos(), t->yPos(), radius, selected));
	}

	// Lines mode
	else if (editor->editMode() == MapEditor::MODE_LINES) {
		// Get line
		vector<MapLine*> vec;
		vec.push_back(editor->getMap().getLine(index));

		// Start animation
		animations.push_back(new MCALineSelection(theApp->runTimer(), vec, selected));
	}

	// Vertices mode
	else if (editor->editMode() == MapEditor::MODE_VERTICES) {
		// Get vertex
		vector<MapVertex*> verts;
		verts.push_back(editor->getMap().getVertex(index));

		// Determine current vertex size
		float vs = vertex_size;
		if (view_scale < 1.0) vs *= view_scale;
		if (vs < 2.0) vs = 2.0;

		// Start animation
		animations.push_back(new MCAVertexSelection(theApp->runTimer(), verts, vs, selected));
	}

	// Sectors mode
	else if (editor->editMode() == MapEditor::MODE_SECTORS) {
		// Get sector polygon
		vector<Polygon2D*> polys;
		polys.push_back(editor->getMap().getSector(index)->getPolygon());

		// Start animation
		animations.push_back(new MCASectorSelection(theApp->runTimer(), polys, selected));
	}
}

void MapCanvas::itemsSelected(vector<int>& items, bool selected) {
	// Things mode
	if (editor->editMode() == MapEditor::MODE_THINGS) {
		// Go through selection
		for (unsigned a = 0; a < items.size(); a++)
			itemSelected(items[a], selected);
	}

	// Lines mode
	else if (editor->editMode() == MapEditor::MODE_LINES) {
		vector<MapLine*> lines;
		for (unsigned a = 0; a < items.size(); a++)
			lines.push_back(editor->getMap().getLine(items[a]));

		// Start animation
		animations.push_back(new MCALineSelection(theApp->runTimer(), lines, selected));
	}

	// Vertices mode
	else if (editor->editMode() == MapEditor::MODE_VERTICES) {
		// Get list of vertices
		vector<MapVertex*> verts;
		for (unsigned a = 0; a < items.size(); a++)
			verts.push_back(editor->getMap().getVertex(items[a]));

		// Determine current vertex size
		float vs = vertex_size;
		if (view_scale < 1.0) vs *= view_scale;
		if (vs < 2.0) vs = 2.0;

		// Start animation
		animations.push_back(new MCAVertexSelection(theApp->runTimer(), verts, vs, selected));
	}

	// Sectors mode
	else if (editor->editMode() == MapEditor::MODE_SECTORS) {
		// Get list of sector polygons
		vector<Polygon2D*> polys;
		for (unsigned a = 0; a < items.size(); a++)
			polys.push_back(editor->getMap().getSector(items[a])->getPolygon());

		// Start animation
		animations.push_back(new MCASectorSelection(theApp->runTimer(), polys, selected));
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
	else if (name == "me2d_zoom_out_m") {
		zoom(0.8, true);
		zooming_cursor = true;
	}

	// Zoom in (follow mouse)
	else if (name == "me2d_zoom_in_m") {
		zoom(1.2, true);
		zooming_cursor = true;
	}

	// Zoom in (show object)
	else if (name == "me2d_show_object")
		viewShowObject();

	// Zoom out (full map)
	else if (name == "me2d_show_all")
		viewFitToMap();

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
		mouse_downpos.set(mouse_pos);
		mouse_state = MSTATE_PANNING;
		editor->clearHilight();
		SetCursor(wxCURSOR_SIZING);
	}

	// Select all
	else if (name == "select_all")
		editor->selectAll();

	// Clear selection
	else if (name == "me2d_clear_selection") {
		editor->clearSelection();
		editor->addEditorMessage("Selection cleared");
	}

	// Cycle flat type
	else if (name == "me2d_flat_type") {
		flat_drawtype = flat_drawtype + 1;
		if (flat_drawtype > 3)
			flat_drawtype = 0;

		// Editor message
		switch (flat_drawtype) {
		case 0: editor->addEditorMessage("Flats: None"); break;
		case 1: editor->addEditorMessage("Flats: Light Level"); break;
		case 2: editor->addEditorMessage("Flats: Floor Texture"); break;
		case 3: editor->addEditorMessage("Flats: Ceiling Texture"); break;
		default: break;
		};
	}

	// Move items (toggle)
	else if (name == "me2d_move") {
		if (mouse_state == MSTATE_NORMAL) {
			if (editor->beginMove(mouse_pos_m)) {
				mouse_state = MSTATE_MOVE;
				renderer_2d->forceUpdate();
			}
		}
		else if (mouse_state == MSTATE_MOVE) {
			editor->endMove();
			mouse_state = MSTATE_NORMAL;
			renderer_2d->forceUpdate();
		}
	}

	// Split line
	else if (name == "me2d_line_split" && editor->editMode() == MapEditor::MODE_LINES)
		editor->splitLine(mouse_pos_m.x, mouse_pos_m.y);
}

void MapCanvas::onKeyBindRelease(string name) {
	if (name == "me2d_pan_view") {
		mouse_state = MSTATE_NORMAL;
		editor->updateHilight(mouse_pos_m);
		SetCursor(wxNullCursor);
	}
}

bool MapCanvas::handleAction(string id) {
	// Skip if not shown
	if (!IsShown())
		return false;

	// Vertices mode
	if (id == "mapw_mode_vertices") {
		editor->setEditMode(MapEditor::MODE_VERTICES);
		return true;
	}

	// Lines mode
	else if (id == "mapw_mode_lines") {
		editor->setEditMode(MapEditor::MODE_LINES);
		return true;
	}

	// Sectors mode
	else if (id == "mapw_mode_sectors") {
		editor->setEditMode(MapEditor::MODE_SECTORS);
		return true;
	}

	// Things mode
	else if (id == "mapw_mode_things") {
		editor->setEditMode(MapEditor::MODE_THINGS);
		return true;
	}

	// Not handled
	return false;
}




void MapCanvas::onSize(wxSizeEvent& e) {
	// Update screen limits
	view_tl.x = translateX(0);
	view_tl.y = translateY(GetSize().y);
	view_br.x = translateX(GetSize().x);
	view_br.y = translateY(0);

	// Update map item visibility
	renderer_2d->updateVisibility(view_tl, view_br);
}

void MapCanvas::onKeyDown(wxKeyEvent& e) {
	// Let keybind system handle it
	KeyBind::keyPressed(KeyBind::asKeyPress(e.GetKeyCode(), e.GetModifiers()));

	// Testing
	if (e.GetKeyCode() == WXK_F6) {
		Polygon2D poly;
		sf::Clock clock;
		wxLogMessage("Generating polygons...");
		for (unsigned a = 0; a < editor->getMap().nSectors(); a++) {
			if (!poly.openSector(editor->getMap().getSector(a)))
				wxLogMessage("Splitting failed for sector %d", a);
		}
		int ms = clock.GetElapsedTime() * 1000;
		wxLogMessage("Polygon generation took %dms", ms);
	}

	if (e.GetKeyCode() == WXK_F5 && editor->editMode() == MapEditor::MODE_SECTORS) {
		splitter.setVerbose(true);
		splitter.clear();
		splitter.openSector(editor->getHilightedSector());
		Polygon2D temp;
		splitter.doSplitting(&temp);
	}

	e.Skip();
}

void MapCanvas::onKeyUp(wxKeyEvent& e) {
	// Let keybind system handle it
	KeyBind::keyReleased(KeyBind::keyName(e.GetKeyCode()));

	e.Skip();
}

void MapCanvas::onMouseDown(wxMouseEvent& e) {
	// Do nothing if the cursor isn't in the normal state
	if (mouse_state != MSTATE_NORMAL) {
		e.Skip();
		return;
	}

	// Update mouse variables
	mouse_downpos.set(e.GetX(), e.GetY());
	mouse_downpos_m.set(translateX(e.GetX()), translateY(e.GetY()));

	// Left button
	if (e.LeftDown()) {
		// Begin box selection if shift is held down, otherwise toggle selection on hilighted object
		if (e.ShiftDown())
			mouse_state = MSTATE_SELECTION;
		else
			mouse_selbegin = !editor->selectCurrent(selection_clear_click);
	}

	// Right button
	else if (e.RightDown()) {
		if (editor->beginMove(mouse_downpos_m)) {
			mouse_state = MSTATE_MOVE;
			renderer_2d->forceUpdate();
		}
		else if (editor->editMode() == MapEditor::MODE_VERTICES)
			editor->splitLine(mouse_pos_m.x, mouse_pos_m.y, 16/view_scale);
	}

	// Any other mouse button (let keybind system handle it)
	else
		KeyBind::keyPressed(keypress_t(KeyBind::mbName(e.GetButton()), e.AltDown(), e.CmdDown(), e.ShiftDown()));

	e.Skip();
}

void MapCanvas::onMouseUp(wxMouseEvent& e) {
	// Clear mouse down position
	mouse_downpos.set(-1, -1);

	// Left button
	if (e.LeftUp()) {
		mouse_selbegin = false;

		// If we're ending a box selection
		if (mouse_state == MSTATE_SELECTION) {
			// Reset mouse state
			mouse_state = MSTATE_NORMAL;

			// Select
			editor->selectWithin(min(mouse_downpos_m.x, mouse_pos_m.x), min(mouse_downpos_m.y, mouse_pos_m.y),
								max(mouse_downpos_m.x, mouse_pos_m.x), max(mouse_downpos_m.y, mouse_pos_m.y), e.ShiftDown());

			// Begin selection box fade animation
			animations.push_back(new MCASelboxFader(theApp->runTimer(), mouse_downpos_m, mouse_pos_m));
		}
	}

	// Right button
	else if (e.RightUp()) {
		if (mouse_state == MSTATE_MOVE) {
			editor->endMove();
			mouse_state = MSTATE_NORMAL;
			renderer_2d->forceUpdate();
		}
	}

	// Any other mouse button (let keybind system handle it)
	else if (mouse_state != MSTATE_SELECTION)
		KeyBind::keyReleased(KeyBind::mbName(e.GetButton()));

	e.Skip();
}

void MapCanvas::onMouseMotion(wxMouseEvent& e) {
	// Panning
	if (mouse_state == MSTATE_PANNING)
		pan((mouse_pos.x - e.GetX()) / view_scale, -((mouse_pos.y - e.GetY()) / view_scale));

	// Update mouse variables
	mouse_pos.set(e.GetX(), e.GetY());
	mouse_pos_m.set(translateX(e.GetX()), translateY(e.GetY()));

	// Check if we want to start a selection box
	if (mouse_selbegin && fpoint2_t(mouse_pos.x - mouse_downpos.x, mouse_pos.y - mouse_downpos.y).magnitude() > 16)
		mouse_state = MSTATE_SELECTION;

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

void MapCanvas::onMouseLeave(wxMouseEvent& e) {
	// Stop panning
	if (mouse_state == MSTATE_PANNING) {
		mouse_state = MSTATE_NORMAL;
		SetCursor(wxNullCursor);
	}

	e.Skip();
}

void MapCanvas::onMouseEnter(wxMouseEvent& e) {
	// Set focus
	//SetFocus();

	e.Skip();
}

void MapCanvas::onIdle(wxIdleEvent& e) {
	// Get time since last redraw
	long frametime = (sfclock.GetElapsedTime() * 1000) - last_time;

	if (frametime < fr_idle)
		return;

	last_time = (sfclock.GetElapsedTime() * 1000);
	update(frametime);
	Refresh();
}

void MapCanvas::onTimer(wxTimerEvent& e) {
	// Get time since last redraw
	long frametime = (sfclock.GetElapsedTime() * 1000) - last_time;

	if (frametime < fr_idle)
		return;

	last_time = (sfclock.GetElapsedTime() * 1000);
	update(frametime);
	Refresh();
}
