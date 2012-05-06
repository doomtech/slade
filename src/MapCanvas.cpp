
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
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
#include <SFML/Window.hpp>
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
#include "MapRenderer3D.h"
#include "ThingTypeBrowser.h"
#include "MapTextureBrowser.h"
#include "MapObjectPropsPanel.h"
#include "SectorTextureOverlay.h"
#include "LineTextureOverlay.h"
#include "SectorBuilder.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Int, things_always, 2, CVAR_SAVE)
CVAR(Int, vertices_always, 0, CVAR_SAVE)
CVAR(Bool, line_tabs_always, 0, CVAR_SAVE)
CVAR(Bool, flat_fade, 1, CVAR_SAVE)
CVAR(Bool, line_fade, 0, CVAR_SAVE)
CVAR(Bool, grid_dashed, false, CVAR_SAVE)
CVAR(Bool, scroll_smooth, true, CVAR_SAVE)
CVAR(Int, flat_drawtype, 2, CVAR_SAVE)
CVAR(Bool, selection_clear_click, false, CVAR_SAVE)
CVAR(Bool, map_showfps, false, CVAR_SAVE)
CVAR(Bool, camera_3d_gravity, true, CVAR_SAVE)
CVAR(Int, camera_3d_crosshair_size, 6, CVAR_SAVE)
CVAR(Bool, camera_3d_show_distance, false, CVAR_SAVE)

// for testing
PolygonSplitter splitter;
SectorBuilder sbuilder;


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(Int, vertex_size)
EXTERN_CVAR(Bool, vertex_round)
EXTERN_CVAR(Float, render_max_dist)
EXTERN_CVAR(Int, render_3d_things)
EXTERN_CVAR(Int, render_3d_hilight)


/* MapCanvas::MapCanvas
 * MapCanvas class constructor
 *******************************************************************/
MapCanvas::MapCanvas(wxWindow *parent, int id, MapEditor* editor)
: OGLCanvas(parent, id, false) {
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
	anim_overlay_fade = 0.0f;
	fade_vertices = 1.0f;
	fade_lines = 1.0f;
	fade_flats = 1.0f;
	fade_things = 1.0f;
	mouse_state = MSTATE_NORMAL;
	mouse_downpos.set(-1, -1);
	fr_idle = 0;
	last_time = 0;
	renderer_2d = new MapRenderer2D(&editor->getMap());
	renderer_3d = new MapRenderer3D(&editor->getMap());
	view_xoff_inter = 0;
	view_yoff_inter = 0;
	view_scale_inter = 1;
	anim_view_speed = 0.05;
	zooming_cursor = false;
	mouse_selbegin = false;
	mouse_movebegin = false;
	overlay_current = NULL;
	mouse_locked = false;
	mouse_warp = false;
	
#ifdef USE_SFML_RENDERWINDOW
#if SFML_VERSION_MAJOR < 2
	UseVerticalSync(false);
#else
	setVerticalSyncEnabled(false);
#endif
#else
	timer.Start(2);
#endif

	// Bind Events
	Bind(wxEVT_SIZE, &MapCanvas::onSize, this);
	Bind(wxEVT_KEY_DOWN, &MapCanvas::onKeyDown, this);
	Bind(wxEVT_KEY_UP, &MapCanvas::onKeyUp, this);
	Bind(wxEVT_LEFT_DOWN, &MapCanvas::onMouseDown, this);
	Bind(wxEVT_LEFT_DCLICK, &MapCanvas::onMouseDown, this);
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
	Bind(wxEVT_TIMER, &MapCanvas::onRTimer, this);
	Bind(wxEVT_SET_FOCUS, &MapCanvas::onFocus, this);
	Bind(wxEVT_KILL_FOCUS, &MapCanvas::onFocus, this);
#ifdef USE_SFML_RENDERWINDOW
	Bind(wxEVT_IDLE, &MapCanvas::onIdle, this);
#endif
}

/* MapCanvas::~MapCanvas
 * MapCanvas class destructor
 *******************************************************************/
MapCanvas::~MapCanvas() {
	delete renderer_2d;
	delete renderer_3d;
}

/* MapCanvas::overlayActive
 * Returns true if there is currently a full-screen overlay active
 *******************************************************************/
bool MapCanvas::overlayActive() {
	if (!overlay_current)
		return false;
	else
		return overlay_current->isActive();
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

int MapCanvas::screenX(double x) {
	return MathStuff::round((GetSize().x * 0.5) + ((x - view_xoff_inter) * view_scale_inter));
}

int MapCanvas::screenY(double y) {
	return MathStuff::round((GetSize().y * 0.5) - ((y - view_yoff_inter) * view_scale_inter));
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
	renderer_2d->setScale(view_scale_inter);
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
	renderer_2d->setScale(view_scale_inter);
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

void MapCanvas::set3dCameraThing(MapThing* thing) {
	// Determine position
	fpoint3_t pos(thing->xPos(), thing->yPos(), 40);
	int sector = editor->getMap().sectorAt(pos.x, pos.y);
	if (sector >= 0)
		pos.z += editor->getMap().getSector(sector)->intProperty("heightfloor");

	// Determine direction
	fpoint2_t dir = MathStuff::vectorAngle(MathStuff::degToRad(thing->intProperty("angle")));

	renderer_3d->cameraSet(pos, dir);
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

void MapCanvas::drawSelectionNumbers() {
	// Check if any selection exists
	vector<MapObject*> selection;
	editor->getSelectedObjects(selection);
	if (selection.size() == 0)
		return;

	// Get editor message text colour
	rgba_t col = ColourConfiguration::getColour("map_editor_message");

	// Go through selection
	for (unsigned a = 0; a < selection.size(); a++) {
		fpoint2_t tp = selection[a]->textPoint();
		tp.x = screenX(tp.x);
		tp.y = screenY(tp.y);

		// Draw text 'shadow'
		Drawing::drawText(S_FMT("%d", a+1), tp.x+3, tp.y+3, rgba_t(0, 0, 0, 255), Drawing::FONT_BOLD);

		// Draw text
		Drawing::drawText(S_FMT("%d", a+1), tp.x+2, tp.y+2, col, Drawing::FONT_BOLD);
	}
}

void MapCanvas::drawThingQuickAngleLines() {
	// Check if any selection exists
	vector<MapThing*> selection;
	editor->getSelectedThings(selection);
	if (selection.size() == 0)
		return;

	// Get moving colour
	rgba_t col = ColourConfiguration::getColour("map_moving");
	col.set_gl();

	// Draw lines
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	for (unsigned a = 0; a < selection.size(); a++) {
		glVertex2d(selection[a]->xPos(), selection[a]->yPos());
		glVertex2d(mouse_pos_m.x, mouse_pos_m.y);
	}
	glEnd();
}

void MapCanvas::drawLineDrawLines() {	// Best function name ever
	// Get line draw colour
	rgba_t col = ColourConfiguration::getColour("map_linedraw");
	col.set_gl();

	// Determine end point
	fpoint2_t end = mouse_pos_m;
	if (modifiers_current & wxMOD_SHIFT) {
		// If shift is held down, snap to the nearest vertex (if any)
		int vertex = editor->getMap().nearestVertex(end.x, end.y);
		if (vertex >= 0) {
			end.x = editor->getMap().getVertex(vertex)->xPos();
			end.y = editor->getMap().getVertex(vertex)->yPos();
		}
		else if (editor->gridSnap()) {
			// No nearest vertex, snap to grid if needed
			end.x = editor->snapToGrid(end.x);
			end.y = editor->snapToGrid(end.y);
		}
	}
	else if (editor->gridSnap()) {
		// Otherwise, snap to grid if needed
		end.x = editor->snapToGrid(end.x);
		end.y = editor->snapToGrid(end.y);
	}

	// Draw lines
	glLineWidth(2.0f);
	glBegin(GL_LINE_STRIP);
	for (unsigned a = 0; a < editor->nLineDrawPoints(); a++) {
		fpoint2_t point = editor->lineDrawPoint(a);
		glVertex2d(point.x, point.y);
	}
	if (draw_state == DSTATE_LINE)
		glVertex2d(end.x, end.y);
	glEnd();

	// Draw points
	glPointSize(vertex_size);
	if (vertex_round) glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	for (unsigned a = 0; a < editor->nLineDrawPoints(); a++) {
		fpoint2_t point = editor->lineDrawPoint(a);
		glVertex2d(point.x, point.y);
	}
	if (draw_state == DSTATE_LINE || draw_state == DSTATE_SHAPE_ORIGIN)
		glVertex2d(end.x, end.y);
	glEnd();
}

/* MapCanvas::draw
 * Draw the 2d map
 *******************************************************************/
void MapCanvas::drawMap2d() {
	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GetSize().x, 0.0f, GetSize().y, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// Translate to inside of pixel (otherwise inaccuracies can occur on certain gl implemenataions)
	if (OpenGL::accuracyTweak())
		glTranslatef(0.375f, 0.375f, 0);

	// Translate to middle of screen
	glTranslated(GetSize().x * 0.5, GetSize().y * 0.5, 0);

	// Zoom
	glScaled(view_scale_inter, view_scale_inter, 1);

	// Translate to offsets
	glTranslated(-view_xoff_inter, -view_yoff_inter, 0);

	// Update visibility info if needed
	if (!renderer_2d->visOK())
		renderer_2d->updateVisibility(view_tl, view_br);


	// Draw flats if needed
	COL_WHITE.set_gl();
	if (flat_drawtype > 0) {
		bool texture = false;
		if (flat_drawtype > 1)
			texture = true;

		// Adjust flat type depending on sector mode
		int drawtype = 0;
		if (editor->editMode() == MapEditor::MODE_SECTORS) {
			if (editor->sectorEditMode() == MapEditor::SECTOR_FLOOR)
				drawtype = 1;
			else if (editor->sectorEditMode() == MapEditor::SECTOR_CEILING)
				drawtype = 2;
		}

		renderer_2d->renderFlats(drawtype, texture, fade_flats);
	}

	// Draw grid
	drawGrid();

	// --- Draw map (depending on mode) ---
	if (editor->editMode() == MapEditor::MODE_VERTICES) {
		// Vertices mode
		renderer_2d->renderThings(fade_things);						// Things
		renderer_2d->renderLines(line_tabs_always, fade_lines);		// Lines

		// Vertices
		if (mouse_state == MSTATE_MOVE)
			renderer_2d->renderVertices(0.25f);
		else
			renderer_2d->renderVertices(fade_vertices);

		// Selection if needed
		if (mouse_state != MSTATE_MOVE && !overlayActive())
			renderer_2d->renderVertexSelection(editor->getSelection(), anim_flash_level);

		// Hilight if needed
		if (mouse_state == MSTATE_NORMAL && !overlayActive())
			renderer_2d->renderVertexHilight(editor->hilightItem(), anim_flash_level);
	}
	else if (editor->editMode() == MapEditor::MODE_LINES) {
		// Lines mode
		renderer_2d->renderThings(fade_things);		// Things
		renderer_2d->renderVertices(fade_vertices);	// Vertices
		renderer_2d->renderLines(true);				// Lines

		// Selection if needed
		if (mouse_state != MSTATE_MOVE && !overlayActive())
			renderer_2d->renderLineSelection(editor->getSelection(), anim_flash_level);

		// Hilight if needed
		if (mouse_state == MSTATE_NORMAL && !overlayActive())
			renderer_2d->renderLineHilight(editor->hilightItem(), anim_flash_level);
	}
	else if (editor->editMode() == MapEditor::MODE_SECTORS) {
		// Sectors mode
		renderer_2d->renderThings(fade_things);					// Things
		renderer_2d->renderVertices(fade_vertices);				// Vertices
		renderer_2d->renderLines(line_tabs_always, fade_lines);	// Lines

		// Selection if needed
		if (mouse_state != MSTATE_MOVE && !overlayActive())
			renderer_2d->renderFlatSelection(editor->getSelection(), anim_flash_level);

		splitter.testRender();	// Testing

		// Hilight if needed
		if (mouse_state == MSTATE_NORMAL && !overlayActive())
			renderer_2d->renderFlatHilight(editor->hilightItem(), anim_flash_level);
	}
	else if (editor->editMode() == MapEditor::MODE_THINGS) {
		// Check if we should force thing angles visible
		bool force_dir = false;
		if (mouse_state == MSTATE_THING_ANGLE)
			force_dir = true;

		// Things mode
		renderer_2d->renderVertices(fade_vertices);				// Vertices
		renderer_2d->renderLines(line_tabs_always, fade_lines);	// Lines
		renderer_2d->renderThings(fade_things, force_dir);		// Things

		// Selection if needed
		if (mouse_state != MSTATE_MOVE && !overlayActive())
			renderer_2d->renderThingSelection(editor->getSelection(), anim_flash_level);

		// Hilight if needed
		if (mouse_state == MSTATE_NORMAL && !overlayActive())
			renderer_2d->renderThingHilight(editor->hilightItem(), anim_flash_level);
	}


	// Draw tagged sectors/lines/things if needed
	if (!overlayActive()) {
		if (editor->taggedSectors().size() > 0 && mouse_state == MSTATE_NORMAL)
			renderer_2d->renderTaggedFlats(editor->taggedSectors(), anim_flash_level);
		else if (editor->taggedLines().size() > 0 && mouse_state == MSTATE_NORMAL)
			renderer_2d->renderTaggedLines(editor->taggedLines(), anim_flash_level);
		else if (editor->taggedThings().size() > 0 && mouse_state == MSTATE_NORMAL)
			renderer_2d->renderTaggedThings(editor->taggedThings(), anim_flash_level);
	}

	// Draw selection numbers if needed
	//if (editor->selectionSize() > 0)
	//	drawSelectionNumbers();

	// Draw thing quick angle lines if needed
	if (mouse_state == MSTATE_THING_ANGLE)
		drawThingQuickAngleLines();

	// Draw line drawing lines if needed
	if (mouse_state == MSTATE_LINE_DRAW)
		drawLineDrawLines();

	// Draw sectorbuilder test stuff
	//sbuilder.drawResult();


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
	for (unsigned a = 0; a < animations.size(); a++) {
		if (!animations[a]->mode3d())
			animations[a]->draw();
	}

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
}

void MapCanvas::drawMap3d() {
	// Setup 3d renderer view
	renderer_3d->setupView(GetSize().x, GetSize().y);

	// Render 3d map
	renderer_3d->renderMap();

	// Determine hilight
	selection_3d_t hl = renderer_3d->determineHilight();
	editor->set3dHilight(hl);

	// Draw hilight if any
	if (hl.index >= 0)
		renderer_3d->renderHilight(hl, anim_flash_level);

	// Draw selection if any
	vector<selection_3d_t> selection = editor->get3dSelection();
	renderer_3d->renderFlatSelection(selection);
	renderer_3d->renderWallSelection(selection);
	renderer_3d->renderThingSelection(selection);

	// Draw animations
	for (unsigned a = 0; a < animations.size(); a++) {
		if (animations[a]->mode3d())
			animations[a]->draw();
	}
}

/* MapCanvas::draw
 * Draw the current map (2d or 3d) and any overlays etc
 *******************************************************************/
void MapCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup GL state
	rgba_t col_bg = ColourConfiguration::getColour("map_background");
	glClearColor(col_bg.fr(), col_bg.fg(), col_bg.fb(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisable(GL_TEXTURE_2D);

	// Draw 2d or 3d map depending on mode
	if (editor->editMode() == MapEditor::MODE_3D)
		drawMap3d();
	else
		drawMap2d();

	// Draw info overlay
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Translate to inside of pixel (otherwise inaccuracies can occur on certain gl implemenataions)
	if (OpenGL::accuracyTweak())
		glTranslatef(0.375f, 0.375f, 0);

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

	// Draw current fullscreen overlay
	if (overlay_current)
		overlay_current->draw(GetSize().x, GetSize().y, anim_overlay_fade);

	// Draw crosshair if 3d mode
	if (editor->editMode() == MapEditor::MODE_3D) {
		// Get crosshair colour
		rgba_t col = ColourConfiguration::getColour("map_3d_crosshair");
		col.set_gl();

		glDisable(GL_TEXTURE_2D);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.5f);

		double midx = GetSize().x * 0.5;
		double midy = GetSize().y * 0.5;
		int size = camera_3d_crosshair_size;

		glBegin(GL_LINES);
		// Right
		col.set_gl(false);
		glVertex2d(midx+1, midy);
		glColor4f(col.fr(), col.fg(), col.fb(), 0.0f);
		glVertex2d(midx+size, midy);

		// Left
		col.set_gl(false);
		glVertex2d(midx-1, midy);
		glColor4f(col.fr(), col.fg(), col.fb(), 0.0f);
		glVertex2d(midx-size, midy);

		// Bottom
		col.set_gl(false);
		glVertex2d(midx, midy+1);
		glColor4f(col.fr(), col.fg(), col.fb(), 0.0f);
		glVertex2d(midx, midy+size);

		// Top
		col.set_gl(false);
		glVertex2d(midx, midy-1);
		glColor4f(col.fr(), col.fg(), col.fb(), 0.0f);
		glVertex2d(midx, midy-size);
		glEnd();

		// Draw item distance (if any)
		if (renderer_3d->itemDistance() >= 0 && camera_3d_show_distance) {
			glEnable(GL_TEXTURE_2D);
			col.set_gl(true);
			Drawing::drawText(S_FMT("%d", renderer_3d->itemDistance()), midx+5, midy+5, rgba_t(255, 255, 255, 200), Drawing::FONT_SMALL);
		}
	}

	// FPS counter
	if (map_showfps) {
		glEnable(GL_TEXTURE_2D);
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

	// test
	//Drawing::drawText(S_FMT("Render distance: %1.2f", (double)render_max_dist), 0, 100);

	// Editor messages
	drawEditorMessages();

	SwapBuffers();
}

bool MapCanvas::update2d(double mult) {
	// Update hilight if needed
	if (mouse_state == MSTATE_NORMAL && !mouse_movebegin)
		editor->updateHilight(mouse_pos_m, view_scale);

	// Do item moving if needed
	if (mouse_state == MSTATE_MOVE)
		editor->doMove(mouse_pos_m);

	// --- Fade map objects depending on mode ---

	// Determine fade levels
	float fa_vertices, fa_lines, fa_flats, fa_things;

	// Vertices
	if (vertices_always == 0)		fa_vertices = 0.0f;
	else if (vertices_always == 1)	fa_vertices = 1.0f;
	else							fa_vertices = 0.5f;

	// Things
	if (things_always == 0)			fa_things = 0.0f;
	else if (things_always == 1)	fa_things = 1.0f;
	else							fa_things = 0.5f;

	// Lines
	if (line_fade)	fa_lines = 0.5f;
	else			fa_lines = 1.0f;

	// Flats
	if (flat_fade)	fa_flats = 0.7f;
	else			fa_flats = 1.0f;

	// Interpolate
	bool anim_mode_crossfade = false;
	float mcs_speed = 0.08f;
	if (editor->editMode() == MapEditor::MODE_VERTICES) {
		if (fade_vertices < 1.0f)  { fade_vertices += mcs_speed*(1.0f-fa_vertices)*mult; anim_mode_crossfade = true; }
		fade_lines = fa_lines;
		if (fade_flats > fa_flats) { fade_flats -= mcs_speed*(1.0f-fa_flats)*mult; anim_mode_crossfade = true; }
		if (fade_things > fa_things) { fade_things -= mcs_speed*(1.0f-fa_things)*mult; anim_mode_crossfade = true; }
	}
	else if (editor->editMode() == MapEditor::MODE_LINES) {
		if (fade_vertices > fa_vertices) { fade_vertices -= mcs_speed*(1.0f-fa_vertices)*mult; anim_mode_crossfade = true; }
		fade_lines = 1.0f;
		if (fade_flats > fa_flats) { fade_flats -= mcs_speed*(1.0f-fa_flats)*mult; anim_mode_crossfade = true; }
		if (fade_things > fa_things) { fade_things -= mcs_speed*(1.0f-fa_things)*mult; anim_mode_crossfade = true; }
	}
	else if (editor->editMode() == MapEditor::MODE_SECTORS) {
		if (fade_vertices > fa_vertices) { fade_vertices -= mcs_speed*(1.0f-fa_vertices)*mult; anim_mode_crossfade = true; }
		fade_lines = fa_lines;
		if (fade_flats < 1.0f) { fade_flats += mcs_speed*(1.0f-fa_flats)*mult; anim_mode_crossfade = true; }
		if (fade_things > fa_things) { fade_things -= mcs_speed*(1.0f-fa_things)*mult; anim_mode_crossfade = true; }
	}
	else if (editor->editMode() == MapEditor::MODE_THINGS) {
		if (fade_vertices > fa_vertices) { fade_vertices -= mcs_speed*(1.0f-fa_vertices)*mult; anim_mode_crossfade = true; }
		fade_lines = fa_lines;
		if (fade_flats > fa_flats) { fade_flats -= mcs_speed*(1.0f-fa_flats)*mult; anim_mode_crossfade = true; }
		if (fade_things < 1.0f) { fade_things += mcs_speed*(1.0f-fa_things)*mult; anim_mode_crossfade = true; }
	}

	// Clamp
	if (fade_vertices < fa_vertices) fade_vertices = fa_vertices;
	if (fade_vertices > 1.0f) fade_vertices = 1.0f;
	if (fade_lines < fa_lines) fade_lines = fa_lines;
	if (fade_lines > 1.0f) fade_lines = 1.0f;
	if (fade_flats < fa_flats) fade_flats = fa_flats;
	if (fade_flats > 1.0f) fade_flats = 1.0f;
	if (fade_things < fa_things) fade_things = fa_things;
	if (fade_things > 1.0f) fade_things = 1.0f;

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

	// Check if framerate shouldn't be throttled
	if (mouse_state == MSTATE_SELECTION || mouse_state == MSTATE_PANNING || view_anim || anim_mode_crossfade)
		return true;
	else
		return false;
}

bool MapCanvas::update3d(double mult) {
	// --- Check for held-down keys ---
	bool moving = false;

	// Camera forward
	if (KeyBind::isPressed("me3d_camera_forward")) {
		renderer_3d->cameraMove(mult*4, !camera_3d_gravity);
		moving = true;
	}

	// Camera backward
	if (KeyBind::isPressed("me3d_camera_back")) {
		renderer_3d->cameraMove(-mult*4, !camera_3d_gravity);
		moving = true;
	}

	// Camera left (strafe)
	if (KeyBind::isPressed("me3d_camera_left")) {
		renderer_3d->cameraStrafe(-mult*4);
		moving = true;
	}

	// Camera right (strafe)
	if (KeyBind::isPressed("me3d_camera_right")) {
		renderer_3d->cameraStrafe(mult*4);
		moving = true;
	}

	// Camera up
	if (KeyBind::isPressed("me3d_camera_up")) {
		renderer_3d->cameraMoveUp(mult*4);
		moving = true;
	}

	// Camera down
	if (KeyBind::isPressed("me3d_camera_down")) {
		renderer_3d->cameraMoveUp(-mult*4);
		moving = true;
	}

	// Camera turn left
	if (KeyBind::isPressed("me3d_camera_turn_left")) {
		renderer_3d->cameraTurn(-mult);
		moving = true;
	}

	// Camera turn right
	if (KeyBind::isPressed("me3d_camera_turn_right")) {
		renderer_3d->cameraTurn(mult);
		moving = true;
	}

	// Apply gravity to camera if needed
	if (camera_3d_gravity)
		renderer_3d->cameraApplyGravity(mult);

	return moving;
}

void MapCanvas::update(long frametime) {
	// Get frame time multiplier
	float mult = (float)frametime / 10.0f;

	// Update stuff depending on (2d/3d) mode
	bool mode_anim = false;
	if (editor->editMode() == MapEditor::MODE_3D)
		mode_anim = update3d(mult);
	else
		mode_anim = update2d(mult);

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
	if (anim_info_show && !overlayActive()) {
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

	// Fader for fullscreen overlay
	bool overlay_fade_anim = true;
	if (overlayActive()) {
		anim_overlay_fade += 0.1f*mult;
		if (anim_overlay_fade > 1.0f) {
			anim_overlay_fade = 1.0f;
			overlay_fade_anim = false;
		}
	}
	else {
		anim_overlay_fade -= 0.05f*mult;
		if (anim_overlay_fade < 0.0f) {
			anim_overlay_fade = 0.0f;
			overlay_fade_anim = false;
		}
	}

	// Update overlay animation (if active)
	if (overlayActive())
		overlay_current->update(frametime);

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
	if (mode_anim || fade_anim || overlay_fade_anim || anim_running)
		fr_idle = 0;
	else	// No high-priority animations running, throttle framerate
		fr_idle = 25;
#else
	if (mode_anim || fade_anim || overlay_fade_anim || anim_running)
		fr_idle = 5;
	else	// No high-priority animations running, throttle framerate
		fr_idle = 30;
#endif

	frametime_last = frametime;
}

void MapCanvas::mouseToCenter() {
	mouse_warp = true;
	WarpPointer(GetSize().x * 0.5, GetSize().y * 0.5);
	/*
	wxRect rect = GetScreenRect();
#if SFML_VERSION_MAJOR < 2
	sf::Window::SetCursorPosition(sf::Window::GetWidth()/2, sf::Window::GetHeight()/2);
#else
	sf::Mouse::setPosition(sf::Vector2i(rect.x + rect.width*0.5, rect.y + rect.height*0.5));
#endif
	*/
}

void MapCanvas::lockMouse(bool lock) {
	mouse_locked = lock;
	if (lock) {
		// Center mouse
		mouseToCenter();

		// Hide cursor
		wxImage img(32, 32, true);
		img.SetMask(true);
		img.SetMaskColour(0, 0, 0);
		SetCursor(wxCursor(img));

#ifdef USE_SFML_RENDERWINDOW
#if SFML_VERSION_MAJOR < 2
		ShowMouseCursor(false);
#else
		setMouseCursorVisible(false);
#endif
#endif
	}
	else {
		// Show cursor
		SetCursor(wxNullCursor);

#ifdef USE_SFML_RENDERWINDOW
#if SFML_VERSION_MAJOR < 2
		ShowMouseCursor(false);
#else
		setMouseCursorVisible(false);
#endif
#endif
	}
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

void MapCanvas::itemSelected3d(selection_3d_t item, bool selected) {
	// Wall selected
	if (item.type == MapEditor::SEL_SIDE_BOTTOM ||
		item.type == MapEditor::SEL_SIDE_TOP ||
		item.type == MapEditor::SEL_SIDE_MIDDLE) {
		// Get quad
		MapRenderer3D::quad_3d_t* quad = renderer_3d->getQuad(item);

		if (quad) {
			// Get quad points
			fpoint3_t points[4];
			for (unsigned a = 0; a < 4; a++)
				points[a].set(quad->points[a].x, quad->points[a].y, quad->points[a].z);

			// Start animation
			animations.push_back(new MCA3dWallSelection(theApp->runTimer(), points, selected));
		}
	}

	// Flat selected
	if (item.type == MapEditor::SEL_CEILING || item.type == MapEditor::SEL_FLOOR) {
		// Get flat
		MapRenderer3D::flat_3d_t* flat = renderer_3d->getFlat(item);

		// Start animation
		if (flat)
			animations.push_back(new MCA3dFlatSelection(theApp->runTimer(), flat->sector, flat->plane, selected));
	}
}

void MapCanvas::itemsSelected3d(vector<selection_3d_t>& items, bool selected) {
	// Just do one animation per item in 3d mode
	for (unsigned a = 0; a < items.size(); a++)
		itemSelected3d(items[a], selected);
}

void MapCanvas::updateInfoOverlay() {
	// Update info overlay depending on edit mode
	switch (editor->editMode()) {
	case MapEditor::MODE_VERTICES:	info_vertex.update(editor->getHilightedVertex()); break;
	case MapEditor::MODE_LINES:		info_line.update(editor->getHilightedLine()); break;
	case MapEditor::MODE_SECTORS:	info_sector.update(editor->getHilightedSector()); break;
	case MapEditor::MODE_THINGS:	info_thing.update(editor->getHilightedThing()); break;
	}
}

void MapCanvas::forceRefreshRenderer() {
	renderer_2d->forceUpdate();
	renderer_3d->clearData();
}

void MapCanvas::changeEditMode(int mode) {
	// Set edit mode
	int mode_prev = editor->editMode();
	editor->setEditMode(mode);

	// Unlock mouse
	lockMouse(false);

	// Update toolbar
	if (mode != mode_prev) theMapEditor->removeAllCustomToolBars();
	if (mode == MapEditor::MODE_VERTICES)
		theApp->toggleAction("mapw_mode_vertices");
	else if (mode == MapEditor::MODE_LINES)
		theApp->toggleAction("mapw_mode_lines");
	else if (mode == MapEditor::MODE_SECTORS) {
		theApp->toggleAction("mapw_mode_sectors");

		// Sector mode toolbar
		if (mode_prev != MapEditor::MODE_SECTORS) {
			wxArrayString actions;
			actions.Add("mapw_sectormode_normal");
			actions.Add("mapw_sectormode_floor");
			actions.Add("mapw_sectormode_ceiling");
			theMapEditor->addCustomToolBar("Sector Mode", actions);
		}

		// Toggle current sector mode
		if (editor->sectorEditMode() == MapEditor::SECTOR_BOTH)
			theApp->toggleAction("mapw_sectormode_normal");
		else if (editor->sectorEditMode() == MapEditor::SECTOR_FLOOR)
			theApp->toggleAction("mapw_sectormode_floor");
		else if (editor->sectorEditMode() == MapEditor::SECTOR_CEILING)
			theApp->toggleAction("mapw_sectormode_ceiling");
	}
	else if (mode == MapEditor::MODE_THINGS)
		theApp->toggleAction("mapw_mode_things");
	else if (mode == MapEditor::MODE_3D) {
		lockMouse(true);
		renderer_3d->refresh();
	}
	theMapEditor->refreshToolBar();

	// Refresh
	//if (mode != MapEditor::MODE_LINES)
	//	renderer_2d->forceUpdate(fade_lines);
	//else
	//	renderer_2d->forceUpdate(1.0f);
}

void MapCanvas::changeThingType() {
	// Determine the initial type
	int type = -1;
	vector<MapThing*> selection;
	editor->getSelectedThings(selection);
	if (selection.size() > 0)
		type = selection[0]->intProperty("type");
	else
		return;

	// Lock hilight in the editor
	bool hl_lock = editor->hilightLocked();
	editor->lockHilight();

	// Open type browser
	ThingTypeBrowser browser(theMapEditor, type);
	if (browser.ShowModal() == wxID_OK)
		editor->changeThingType(browser.getSelectedType());

	// Unlock hilight if needed
	editor->lockHilight(hl_lock);
}

void MapCanvas::changeSectorTexture() {
	// Determine the initial texture
	string texture = "";
	string browser_title;
	vector<MapSector*> selection;
	editor->getSelectedSectors(selection);
	if (selection.size() > 0) {
		// Check edit mode
		if (editor->sectorEditMode() == MapEditor::SECTOR_FLOOR) {
			texture = selection[0]->stringProperty("texturefloor");
			browser_title = "Browse Floor Texture";
		}
		else if (editor->sectorEditMode() == MapEditor::SECTOR_CEILING) {
			texture = selection[0]->stringProperty("textureceiling");
			browser_title = "Browse Ceiling Texture";
		}
		else {
			if (overlay_current) delete overlay_current;
			SectorTextureOverlay* sto = new SectorTextureOverlay();
			sto->openSectors(selection);
			overlay_current = sto;
			return;
		}
	}
	else
		return;

	// Lock hilight in the editor
	bool hl_lock = editor->hilightLocked();
	editor->lockHilight();

	// Open texture browser
	MapTextureBrowser browser(theMapEditor, 1, texture);
	browser.SetTitle(browser_title);
	if (browser.ShowModal() == wxID_OK) {
		// Set texture depending on edit mode
		for (unsigned a = 0; a < selection.size(); a++) {
			if (editor->sectorEditMode() == MapEditor::SECTOR_FLOOR)
				selection[a]->setStringProperty("texturefloor", browser.getSelectedItem()->getName());
			else if (editor->sectorEditMode() == MapEditor::SECTOR_CEILING)
				selection[a]->setStringProperty("textureceiling", browser.getSelectedItem()->getName());
		}
	}

	// Unlock hilight if needed
	editor->lockHilight(hl_lock);
}

void MapCanvas::changeThingType3d(selection_3d_t first) {
	// Get first selected thing
	MapThing* thing = editor->getMap().getThing(first.index);

	// Do nothing if no things selected/hilighted
	if (!thing)
		return;

	// Open type browser
	ThingTypeBrowser browser(theMapEditor, thing->getType());
	if (browser.ShowModal() == wxID_OK)
		editor->changeThingType(browser.getSelectedType());
}

void MapCanvas::changeTexture3d(selection_3d_t first) {
	// Check index
	if (first.index < 0)
		return;

	// Get initial texture
	string tex;
	int type = 0;
	if (first.type == MapEditor::SEL_FLOOR) {
		tex = editor->getMap().getSector(first.index)->floorTexture();
		type = 1;
	}
	else if (first.type == MapEditor::SEL_CEILING) {
		tex = editor->getMap().getSector(first.index)->ceilingTexture();
		type = 1;
	}
	else if (first.type == MapEditor::SEL_SIDE_BOTTOM)
		tex = editor->getMap().getSide(first.index)->stringProperty("texturebottom");
	else if (first.type == MapEditor::SEL_SIDE_MIDDLE)
		tex = editor->getMap().getSide(first.index)->stringProperty("texturemiddle");
	else if (first.type == MapEditor::SEL_SIDE_TOP)
		tex = editor->getMap().getSide(first.index)->stringProperty("texturetop");

	// Open texture browser
	MapTextureBrowser browser(theMapEditor, type, tex);
	browser.SetTitle("Browse Texture");
	if (browser.ShowModal() == wxID_OK) {
		bool mix = theGameConfiguration->mixTexFlats();
		tex = browser.getSelectedItem()->getName();
		selection_3d_t hl = editor->hilightItem3d();
		
		// Apply to flats
		vector<selection_3d_t>& selection = editor->get3dSelection();
		if (mix || type == 1) {
			// Selection
			if (selection.size() > 0) {
				for (unsigned a = 0; a < selection.size(); a++) {
					if (selection[a].type == MapEditor::SEL_FLOOR)
						editor->getMap().getSector(selection[a].index)->setStringProperty("texturefloor", tex);
					else if (selection[a].type == MapEditor::SEL_CEILING)
						editor->getMap().getSector(selection[a].index)->setStringProperty("textureceiling", tex);
				}
			}
			else if (hl.index >= 0) {
				// Hilight if no selection
				if (hl.type == MapEditor::SEL_FLOOR)
					editor->getMap().getSector(hl.index)->setStringProperty("texturefloor", tex);
				else if (hl.type == MapEditor::SEL_CEILING)
					editor->getMap().getSector(hl.index)->setStringProperty("textureceiling", tex);
			}
		}

		// Apply to walls
		if (mix || type == 0) {
			// Selection
			if (selection.size() > 0) {
				for (unsigned a = 0; a < selection.size(); a++) {
					if (selection[a].type == MapEditor::SEL_SIDE_BOTTOM)
						editor->getMap().getSide(selection[a].index)->setStringProperty("texturebottom", tex);
					else if (selection[a].type == MapEditor::SEL_SIDE_MIDDLE)
						editor->getMap().getSide(selection[a].index)->setStringProperty("texturemiddle", tex);
					else if (selection[a].type == MapEditor::SEL_SIDE_TOP)
						editor->getMap().getSide(selection[a].index)->setStringProperty("texturetop", tex);
				}
			}
			else if (hl.index >= 0) {
				// Hilight if no selection
				if (hl.type == MapEditor::SEL_SIDE_BOTTOM)
					editor->getMap().getSide(hl.index)->setStringProperty("texturebottom", tex);
				else if (hl.type == MapEditor::SEL_SIDE_MIDDLE)
					editor->getMap().getSide(hl.index)->setStringProperty("texturemiddle", tex);
				else if (hl.type == MapEditor::SEL_SIDE_TOP)
					editor->getMap().getSide(hl.index)->setStringProperty("texturetop", tex);
			}
		}
	}
}

void MapCanvas::onKeyBindPress(string name) {
	// Check if an overlay is active
	if (overlayActive()) {
		// Accept edit
		if (name == "map_edit_accept")
			overlay_current->close();

		// Cancel edit
		else if (name == "map_edit_cancel")
			overlay_current->close(true);

		// Nothing else
		return;
	}

	// Toggle 3d mode
	if (name == "map_toggle_3d") {
		if (editor->editMode() == MapEditor::MODE_3D)
			changeEditMode(mode_last);
		else {
			mode_last = editor->editMode();
			changeEditMode(MapEditor::MODE_3D);
		}
	}

	// Handle keybinds depending on mode
	if (editor->editMode() == MapEditor::MODE_3D)
		keyBinds3d(name);
	else
		keyBinds2d(name);
}

void MapCanvas::keyBinds2d(string name) {
	// Check if line drawing mode is active
	if (mouse_state == MSTATE_LINE_DRAW) {
		// Accept line draw
		if (name == "map_edit_accept") {
			mouse_state = MSTATE_NORMAL;
			editor->endLineDraw();
		}

		// Cancel line draw
		else if (name == "map_edit_cancel") {
			mouse_state = MSTATE_NORMAL;
			editor->endLineDraw(false);
		}
	}

	// Vertices mode
	if (name == "me2d_mode_vertices" && mouse_state == MSTATE_NORMAL)
		changeEditMode(MapEditor::MODE_VERTICES);

	// Lines mode
	else if (name == "me2d_mode_lines" && mouse_state == MSTATE_NORMAL)
		changeEditMode(MapEditor::MODE_LINES);

	// Sectors mode
	else if (name == "me2d_mode_sectors" && mouse_state == MSTATE_NORMAL)
		changeEditMode(MapEditor::MODE_SECTORS);

	// Things mode
	else if (name == "me2d_mode_things" && mouse_state == MSTATE_NORMAL)
		changeEditMode(MapEditor::MODE_THINGS);

	// 3d mode
	else if (name == "me2d_mode_3d" && mouse_state == MSTATE_NORMAL)
		changeEditMode(MapEditor::MODE_3D);

	// Pan left
	else if (name == "me2d_left")
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
		zoom(1.25);

	// Zoom out (follow mouse)
	else if (name == "me2d_zoom_out_m") {
		zoom(0.8, true);
		zooming_cursor = true;
	}

	// Zoom in (follow mouse)
	else if (name == "me2d_zoom_in_m") {
		zoom(1.25, true);
		zooming_cursor = true;
	}

	// Zoom in (show object)
	else if (name == "me2d_show_object")
		viewShowObject();

	// Zoom out (full map)
	else if (name == "me2d_show_all")
		viewFitToMap();

	// Pan view
	else if (name == "me2d_pan_view" && mouse_state == MSTATE_NORMAL) {
		mouse_downpos.set(mouse_pos);
		mouse_state = MSTATE_PANNING;
		editor->clearHilight();
		SetCursor(wxCURSOR_SIZING);
	}

	// Cycle flat type
	else if (name == "me2d_flat_type") {
		flat_drawtype = flat_drawtype + 1;
		if (flat_drawtype > 2)
			flat_drawtype = 0;

		// Editor message and toolbar update
		switch (flat_drawtype) {
		case 0: editor->addEditorMessage("Flats: None"); theApp->toggleAction("mapw_flat_none"); break;
		case 1: editor->addEditorMessage("Flats: Untextured"); theApp->toggleAction("mapw_flat_untextured"); break;
		case 2: editor->addEditorMessage("Flats: Textured"); theApp->toggleAction("mapw_flat_textured"); break;
		default: break;
		};

		theMapEditor->refreshToolBar();
	}

	// Move items (toggle)
	else if (name == "me2d_move" && mouse_state == MSTATE_NORMAL) {
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
	else if (name == "me2d_split_line" && mouse_state == MSTATE_NORMAL)
		editor->splitLine(mouse_pos_m.x, mouse_pos_m.y, 16/view_scale);

	// Change line texture
	else if (name == "me2d_line_change_texture" && editor->editMode() == MapEditor::MODE_LINES && mouse_state == MSTATE_NORMAL) {
		// Get selection
		vector<MapLine*> selection;
		editor->getSelectedLines(selection);

		// Open line texture overlay if anything is selected
		if (selection.size() > 0) {
			if (overlay_current) delete overlay_current;
			LineTextureOverlay* lto = new LineTextureOverlay();
			lto->openLines(selection);
			overlay_current = lto;
		}
	}

	// Flip line
	else if (name == "me2d_line_flip" && mouse_state == MSTATE_NORMAL)
		editor->flipLines();

	// Flip line (no sides)
	else if (name == "me2d_line_flip_nosides" && mouse_state == MSTATE_NORMAL)
		editor->flipLines(false);

	// Begin line drawing
	else if (name == "me2d_begin_linedraw" && mouse_state == MSTATE_NORMAL) {
		draw_state = DSTATE_LINE;
		mouse_state = MSTATE_LINE_DRAW;
	}

	// Begin shape drawing
	else if (name == "me2d_begin_shapedraw" && mouse_state == MSTATE_NORMAL) {
		draw_state = DSTATE_SHAPE_ORIGIN;
		mouse_state = MSTATE_LINE_DRAW;
	}

	// Create object
	else if (name == "me2d_create_object" && mouse_state == MSTATE_NORMAL) {
		// If in lines mode, begin line drawing
		if (editor->editMode() == MapEditor::MODE_LINES)
			mouse_state = MSTATE_LINE_DRAW;
		else
			editor->createObject(mouse_pos_m.x, mouse_pos_m.y);
	}

	// Delete object
	else if (name == "me2d_delete_object" && mouse_state == MSTATE_NORMAL)
		editor->deleteObject();

	// Copy properties
	else if (name == "me2d_copy_properties" && mouse_state == MSTATE_NORMAL)
		editor->copyProperties();

	// Paste properties
	else if (name == "me2d_paste_properties" && mouse_state == MSTATE_NORMAL)
		editor->pasteProperties();


	// --- Things mode keys ---

	// Change type
	else if (name == "me2d_thing_change_type" && editor->editMode() == MapEditor::MODE_THINGS && mouse_state == MSTATE_NORMAL)
		changeThingType();

	// Quick angle
	else if (name == "me2d_thing_quick_angle" && editor->editMode() == MapEditor::MODE_THINGS && mouse_state == MSTATE_NORMAL) {
		if (mouse_state == MSTATE_NORMAL)
			mouse_state = MSTATE_THING_ANGLE;
	}

	// Change sector texture
	else if (name == "me2d_sector_change_texture" && editor->editMode() == MapEditor::MODE_SECTORS && mouse_state == MSTATE_NORMAL)
		changeSectorTexture();

	// Not handled here, send to editor
	else if (mouse_state == MSTATE_NORMAL)
		editor->handleKeyBind(name, mouse_pos_m);
}

void MapCanvas::keyBinds3d(string name) {
	// Toggle fog
	if (name == "me3d_toggle_fog") {
		bool fog = renderer_3d->fogEnabled();
		renderer_3d->enableFog(!fog);
		if (fog)
			editor->addEditorMessage("Fog disabled");
		else
			editor->addEditorMessage("Fog enabled");
	}

	// Toggle fullbright
	else if (name == "me3d_toggle_fullbright") {
		bool fb = renderer_3d->fullbrightEnabled();
		renderer_3d->enableFullbright(!fb);
		if (fb)
			editor->addEditorMessage("Fullbright disabled");
		else
			editor->addEditorMessage("Fullbright enabled");
	}

	// Toggle gravity
	else if (name == "me3d_toggle_gravity") {
		camera_3d_gravity = !camera_3d_gravity;
		if (!camera_3d_gravity)
			editor->addEditorMessage("Gravity disabled");
		else
			editor->addEditorMessage("Gravity enabled");
	}

	// Release mouse cursor
	else if (name == "me3d_release_mouse")
		lockMouse(false);

	// Toggle things
	else if (name == "me3d_toggle_things") {
		// Change thing display type
		render_3d_things = render_3d_things + 1;
		if (render_3d_things > 1)
			render_3d_things = 0;

		// Editor message
		if (render_3d_things == 0)
			editor->addEditorMessage("Things disabled");
		else
			editor->addEditorMessage("Things enabled: Sprites");
	}

	// Toggle hilight
	else if (name == "me3d_toggle_hilight") {
		// Change hilight type
		render_3d_hilight = render_3d_hilight + 1;
		if (render_3d_hilight > 2)
			render_3d_hilight = 0;

		// Editor message
		if (render_3d_hilight == 0)
			editor->addEditorMessage("Hilight disabled");
		else if (render_3d_hilight == 1)
			editor->addEditorMessage("Hilight enabled: Outline");
		else if (render_3d_hilight == 2)
			editor->addEditorMessage("Hilight enabled: Solid");
	}

	// Send to map editor
	else
		editor->handleKeyBind(name, mouse_pos_m);
}

void MapCanvas::onKeyBindRelease(string name) {
	if (name == "me2d_pan_view" && mouse_state == MSTATE_PANNING) {
		mouse_state = MSTATE_NORMAL;
		editor->updateHilight(mouse_pos_m);
		SetCursor(wxNullCursor);
	}

	else if (name == "me2d_thing_quick_angle" && mouse_state == MSTATE_THING_ANGLE) {
		mouse_state = MSTATE_NORMAL;
		editor->updateHilight(mouse_pos_m);
	}
}

bool MapCanvas::handleAction(string id) {
	// Skip if not shown
	if (!IsShown())
		return false;

	// Skip if overlay is active
	if (overlayActive())
		return false;

	// Vertices mode
	if (id == "mapw_mode_vertices") {
		changeEditMode(MapEditor::MODE_VERTICES);
		return true;
	}

	// Lines mode
	else if (id == "mapw_mode_lines") {
		changeEditMode(MapEditor::MODE_LINES);
		return true;
	}

	// Sectors mode
	else if (id == "mapw_mode_sectors") {
		changeEditMode(MapEditor::MODE_SECTORS);
		return true;
	}

	// Things mode
	else if (id == "mapw_mode_things") {
		changeEditMode(MapEditor::MODE_THINGS);
		return true;
	}

	// 'None' (wireframe) flat type
	else if (id == "mapw_flat_none")
		flat_drawtype = 0;

	// 'Untextured' flat type
	else if (id == "mapw_flat_untextured")
		flat_drawtype = 1;

	// 'Textured' flat type
	else if (id == "mapw_flat_textured")
		flat_drawtype = 2;
		
	// Normal sector edit mode
	else if (id == "mapw_sectormode_normal")
		editor->setSectorEditMode(MapEditor::SECTOR_BOTH);
	
	// Floors sector edit mode
	else if (id == "mapw_sectormode_floor")
		editor->setSectorEditMode(MapEditor::SECTOR_FLOOR);
		
	// Ceilings sector edit mode
	else if (id == "mapw_sectormode_ceiling")
		editor->setSectorEditMode(MapEditor::SECTOR_CEILING);

	// --- Context menu ---

	// Edit item properties
	else if (id == "mapw_item_properties") {
		// Create dialog for properties panel
		wxDialog dlg(theMapEditor, -1, "Properties", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
		dlg.SetInitialSize(wxSize(500, 500));
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		dlg.SetSizer(sizer);

		// Create properties panel
		MapObjectPropsPanel* panel_props = new MapObjectPropsPanel(&dlg);
		panel_props->showApplyButton(false);
		sizer->Add(panel_props, 1, wxEXPAND|wxALL, 4);

		// Add dialog buttons
		sizer->Add(dlg.CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

		// Open current selection
		vector<MapObject*> list;
		editor->getSelectedObjects(list);
		panel_props->openObjects(list);

		// Open the dialog and apply changes if OK was clicked
		dlg.CenterOnParent();
		if (dlg.ShowModal() == wxID_OK) {
			panel_props->applyChanges();
			renderer_2d->forceUpdate(fade_lines);
			Refresh();
		}
		return true;
	}

	// --- Line context menu ---

	// Change line texture
	else if (id == "mapw_line_changetexture") {
		// Get selection
		vector<MapLine*> selection;
		editor->getSelectedLines(selection);

		// Open line texture overlay if anything is selected
		if (selection.size() > 0) {
			if (overlay_current) delete overlay_current;
			LineTextureOverlay* lto = new LineTextureOverlay();
			lto->openLines(selection);
			overlay_current = lto;
		}
		return true;
	}

	// --- Thing context menu ---

	// Change thing type
	else if (id == "mapw_thing_changetype") {
		changeThingType();
		return true;
	}

	// --- Sector context menu ---

	// Change sector texture
	else if (id == "mapw_sector_changetexture") {
		changeSectorTexture();
		return true;
	}

	// Not handled here
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
	// Set current modifiers
	modifiers_current = e.GetModifiers();

	// Let keybind system handle it
	KeyBind::keyPressed(KeyBind::asKeyPress(e.GetKeyCode(), e.GetModifiers()));

	// Send to overlay if active
	if (overlayActive())
		overlay_current->keyDown(KeyBind::keyName(e.GetKeyCode()));

	// Testing
	if (e.GetKeyCode() == WXK_F6) {
		Polygon2D poly;
		sf::Clock clock;
		wxLogMessage("Generating polygons...");
		for (unsigned a = 0; a < editor->getMap().nSectors(); a++) {
			if (!poly.openSector(editor->getMap().getSector(a)))
				wxLogMessage("Splitting failed for sector %d", a);
		}
		//int ms = clock.GetElapsedTime() * 1000;
		//wxLogMessage("Polygon generation took %dms", ms);
	}
	if (e.GetKeyCode() == WXK_F7) {
		// Get nearest line
		int nearest = editor->getMap().nearestLine(mouse_pos_m.x, mouse_pos_m.y, 999999);
		MapLine* line = editor->getMap().getLine(nearest);
		if (line) {
			// Determine line side
			double side = MathStuff::lineSide(mouse_pos_m.x, mouse_pos_m.y, line->x1(), line->y1(), line->x2(), line->y2());
			if (side >= 0)
				sbuilder.traceSector(&(editor->getMap()), line, true);
			else
				sbuilder.traceSector(&(editor->getMap()), line, false);
		}
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
	// Set current modifiers
	modifiers_current = e.GetModifiers();

	// Let keybind system handle it
	KeyBind::keyReleased(KeyBind::keyName(e.GetKeyCode()));

	e.Skip();
}

void MapCanvas::onMouseDown(wxMouseEvent& e) {
	// Update mouse variables
	mouse_downpos.set(e.GetX(), e.GetY());
	mouse_downpos_m.set(translateX(e.GetX()), translateY(e.GetY()));

	// Check if a full screen overlay is active
	if (overlayActive()) {
		// Left click
		if (e.LeftDown())
			overlay_current->mouseLeftClick();

		// Right click
		else if (e.RightDown())
			overlay_current->mouseRightClick();

		return;
	}

	// Left button
	if (e.LeftDown() || e.LeftDClick()) {
		// 3d mode
		if (editor->editMode() == MapEditor::MODE_3D) {
			// If the mouse is unlocked, lock the mouse
			if (!mouse_locked) {
				mouseToCenter();
				lockMouse(true);
			}
			else {
				if (e.ShiftDown())	// Shift down, select all matching adjacent structures
					editor->selectAdjacent3d(editor->hilightItem3d());
				else	// Toggle selection
					editor->selectCurrent();
			}

			return;
		}

		// Line drawing state, add line draw point
		if (mouse_state == MSTATE_LINE_DRAW) {
			// Snap point to nearest vertex if shift is held down
			bool nearest_vertex = false;
			if (e.GetModifiers() & wxMOD_SHIFT)
				nearest_vertex = true;

			// Line drawing
			if (draw_state == DSTATE_LINE) {
				if (editor->addLineDrawPoint(mouse_downpos_m, nearest_vertex)) {
					// If line drawing finished, revert to normal state
					mouse_state = MSTATE_NORMAL;
				}
			}
			
			// Shape drawing
			else {
				if (draw_state == DSTATE_SHAPE_ORIGIN) {
					// Set shape origin
					editor->setShapeDrawOrigin(mouse_downpos_m, nearest_vertex);
					draw_state = DSTATE_SHAPE_EDGE;
				}
				else {
					// Finish shape draw
					editor->endLineDraw(true);
					mouse_state = MSTATE_NORMAL;
				}
			}
		}
		else if (mouse_state == MSTATE_NORMAL) {
			// Begin box selection if shift is held down, otherwise toggle selection on hilighted object
			if (e.ShiftDown())
				mouse_state = MSTATE_SELECTION;
			else
				mouse_selbegin = !editor->selectCurrent(selection_clear_click);
		}
	}

	// Right button
	else if (e.RightDown()) {
		// 3d mode
		if (editor->editMode() == MapEditor::MODE_3D) {
			// Get first selected item
			selection_3d_t first = editor->hilightItem3d();
			if (editor->get3dSelection().size() > 0)
				first = editor->get3dSelection()[0];

			// Check type
			if (first.type == MapEditor::SEL_THING)
				changeThingType3d(first);
			else
				changeTexture3d(first);
		}

		// Remove line draw point if in line drawing state
		if (mouse_state == MSTATE_LINE_DRAW) {
			// Line drawing
			if (draw_state == DSTATE_LINE)
				editor->removeLineDrawPoint();

			// Shape drawing
			else if (draw_state == DSTATE_SHAPE_EDGE) {
				editor->endLineDraw(false);
				draw_state = DSTATE_SHAPE_ORIGIN;
			}
		}

		// Normal state
		else if (mouse_state == MSTATE_NORMAL) {
			// Begin move if something is selected/hilighted
			if (editor->isHilightOrSelection())
				mouse_movebegin = true;
			//else if (editor->editMode() == MapEditor::MODE_VERTICES)
			//	editor->splitLine(mouse_pos_m.x, mouse_pos_m.y, 16/view_scale);
		}
	}

	// Any other mouse button (let keybind system handle it)
	else if (mouse_state == MSTATE_NORMAL)
		KeyBind::keyPressed(keypress_t(KeyBind::mbName(e.GetButton()), e.AltDown(), e.CmdDown(), e.ShiftDown()));

	// Set focus
	SetFocus();

	e.Skip();
}

void MapCanvas::onMouseUp(wxMouseEvent& e) {
	// Clear mouse down position
	mouse_downpos.set(-1, -1);

	// Check if a full screen overlay is active
	if (overlayActive()) {
		return;
	}

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
		mouse_movebegin = false;

		if (mouse_state == MSTATE_MOVE) {
			editor->endMove();
			mouse_state = MSTATE_NORMAL;
			renderer_2d->forceUpdate();
		}

		else if (mouse_state == MSTATE_NORMAL && (editor->selectionSize() > 0 || editor->hilightItem() >= 0)) {
			// Context menu
			wxMenu menu_context;

			// Mode-specific items
			if (editor->editMode() == MapEditor::MODE_LINES)
				theApp->getAction("mapw_line_changetexture")->addToMenu(&menu_context);
			else if (editor->editMode() == MapEditor::MODE_THINGS)
				theApp->getAction("mapw_thing_changetype")->addToMenu(&menu_context);
			else if (editor->editMode() == MapEditor::MODE_SECTORS)
				theApp->getAction("mapw_sector_changetexture")->addToMenu(&menu_context);

			// Properties
			theApp->getAction("mapw_item_properties")->addToMenu(&menu_context);

			PopupMenu(&menu_context);
		}
	}

	// Any other mouse button (let keybind system handle it)
	else if (mouse_state != MSTATE_SELECTION)
		KeyBind::keyReleased(KeyBind::mbName(e.GetButton()));

	e.Skip();
}

void MapCanvas::onMouseMotion(wxMouseEvent& e) {
	// Ignore if it was generated by a mouse pointer warp
	if (mouse_warp) {
		mouse_warp = false;
		e.Skip();
		return;
	}

	// Check if a full screen overlay is active
	if (overlayActive()) {
		overlay_current->mouseMotion(e.GetX(), e.GetY());
		return;
	}

	// Check for 3d mode
	if (editor->editMode() == MapEditor::MODE_3D && mouse_locked) {
		// Get relative mouse movement
		double xrel = e.GetX() - int(GetSize().x * 0.5);
		double yrel = e.GetY() - int(GetSize().y * 0.5);

		renderer_3d->cameraTurn(xrel*0.1);
		renderer_3d->cameraPitch(-yrel*0.003);

		mouseToCenter();
		fr_idle = 0;
		return;
	}

	// Panning
	if (mouse_state == MSTATE_PANNING)
		pan((mouse_pos.x - e.GetX()) / view_scale, -((mouse_pos.y - e.GetY()) / view_scale));

	// Update mouse variables
	mouse_pos.set(e.GetX(), e.GetY());
	mouse_pos_m.set(translateX(e.GetX()), translateY(e.GetY()));

	// Check if we want to start a selection box
	if (mouse_selbegin && fpoint2_t(mouse_pos.x - mouse_downpos.x, mouse_pos.y - mouse_downpos.y).magnitude() > 16)
		mouse_state = MSTATE_SELECTION;

	// Check if we want to start moving
	if (mouse_movebegin && fpoint2_t(mouse_pos.x - mouse_downpos.x, mouse_pos.y - mouse_downpos.y).magnitude() > 4) {
		mouse_movebegin = false;
		editor->beginMove(mouse_downpos_m);
		mouse_state = MSTATE_MOVE;
		renderer_2d->forceUpdate();
	}

	// Check if we are in thing quick angle state
	if (mouse_state == MSTATE_THING_ANGLE)
		editor->thingQuickAngle(mouse_pos_m);

	// Update shape drawing if needed
	if (mouse_state == MSTATE_LINE_DRAW && draw_state == DSTATE_SHAPE_EDGE)
		editor->updateShapeDraw(mouse_pos_m);

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

#if SFML_VERSION_MAJOR < 2
void MapCanvas::onIdle(wxIdleEvent& e) {
	// Get time since last redraw
	long frametime = (sfclock.GetElapsedTime() * 1000) - last_time;

	if (frametime < fr_idle)
		return;

	last_time = (sfclock.GetElapsedTime() * 1000);
	update(frametime);
	Refresh();
}

void MapCanvas::onRTimer(wxTimerEvent& e) {
	// Get time since last redraw
	long frametime = (sfclock.GetElapsedTime() * 1000) - last_time;

	if (frametime < fr_idle)
		return;

	last_time = (sfclock.GetElapsedTime() * 1000);
	update(frametime);
	Refresh();
}
#else
void MapCanvas::onIdle(wxIdleEvent& e) {
	// Get time since last redraw
	long frametime = (sfclock.getElapsedTime().asMilliseconds()) - last_time;

	if (frametime < fr_idle)
		return;

	last_time = (sfclock.getElapsedTime().asMilliseconds());
	update(frametime);
	Refresh();
}

void MapCanvas::onRTimer(wxTimerEvent& e) {
	// Get time since last redraw
	long frametime = (sfclock.getElapsedTime().asMilliseconds()) - last_time;

	//if (frametime < fr_idle)
	//	return;

	last_time = (sfclock.getElapsedTime().asMilliseconds());
	update(frametime);
	Refresh();
}
#endif

void MapCanvas::onFocus(wxFocusEvent& e) {
	if (e.GetEventType() == wxEVT_SET_FOCUS) {
		if (editor->editMode() == MapEditor::MODE_3D)
			lockMouse(true);
	}
	else if (e.GetEventType() == wxEVT_KILL_FOCUS)
		lockMouse(false);
}
