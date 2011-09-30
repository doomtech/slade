
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
#include "MapEditorWindow.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, vertex_round, true, CVAR_SAVE)
CVAR(Int, vertex_size, 7, CVAR_SAVE)
CVAR(Float, line_width, 1.5f, CVAR_SAVE)
CVAR(Bool, line_smooth, true, CVAR_SAVE)
CVAR(Int, things_drawtype, 1, CVAR_SAVE)
CVAR(Bool, things_force_dir, false, CVAR_SAVE)
CVAR(Bool, grid_dashed, false, CVAR_SAVE)


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
	mouse_state = MSTATE_NORMAL;
	mouse_downpos.set(-1, -1);
	fr_idle = 0;
	last_time = 0;

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

	// Update object visibility
	updateVisibility();
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
		double mx = translateX(mouse_pos.x);
		double my = translateY(mouse_pos.y);
		view_xoff = mx + (double(view_xoff - mx) / amount);
		view_yoff = my + (double(view_yoff - my) / amount);
	}

	// Update screen limits
	view_tl.x = translateX(0);
	view_tl.y = translateY(GetSize().y);
	view_br.x = translateX(GetSize().x);
	view_br.y = translateY(0);

	// Update object visibility
	updateVisibility();
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

	glDisable(GL_LINE_STIPPLE);
}

void MapCanvas::drawVertices() {
	// Set to vertex colour
	ColourConfiguration::getColour("map_vertex").set_gl();

	// Go through vertices
	glBegin(GL_POINTS);
	for (unsigned a = 0; a < editor->getMap().nVertices(); a++) {
		// Skip if outside of screen
		if (vis_v[a] > 0)
			continue;

		// Draw vertex
		glVertex2d(editor->getMap().getVertex(a)->xPos(), editor->getMap().getVertex(a)->yPos());
	}
	glEnd();
}

void MapCanvas::drawLines(bool show_direction) {
	// Temp for now
	float fade_coeff = 0.5f;

	// Get line colours
	rgba_t col_background = ColourConfiguration::getColour("map_background");
	rgba_t col_line_normal = ColourConfiguration::getColour("map_line_normal");
	rgba_t col_line_special = ColourConfiguration::getColour("map_line_special");

	// Set line width
	glLineWidth(line_width);

	// Draw all lines
	rgba_t col;
	MapLine* line = NULL;
	double x1, y1, x2, y2;
	for (unsigned a = 0; a < editor->getMap().nLines(); a++) {
		// Skip if outside of screen
		if (vis_l[a] > 0)
			continue;

		// Get line info
		line = editor->getMap().getLine(a);
		x1 = line->v1()->xPos();
		y1 = line->v1()->yPos();
		x2 = line->v2()->xPos();
		y2 = line->v2()->yPos();

		// Check for special line
		if ((int)line->prop("special") > 0)
			col.set(col_line_special);
		else
			col.set(col_line_normal);

		// Check for two-sided line
		if (line->s2())
			col.set(col.r*fade_coeff+col_background.r*(1.0-fade_coeff),
					col.g*fade_coeff+col_background.g*(1.0-fade_coeff),
					col.b*fade_coeff+col_background.b*(1.0-fade_coeff), 255, 0);

		// Set line colour
		col.set_gl();

		// Draw the line
		glBegin(GL_LINES);
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		glEnd();

		// Direction tab
		if (show_direction) {
			double xmid = x1 + ((x2 - x1) * 0.5);
			double ymid = y1 + ((y2 - y1) * 0.5);
			double x = (-(y2 - y1)) * 0.125;
			double y = (x2 - x1) * 0.125;

			glBegin(GL_LINES);
			glVertex2d(xmid, ymid);
			glVertex2d(xmid - x, ymid - y);
			glEnd();
		}
	}
}

void MapCanvas::drawRoundThing(double x, double y, double angle, ThingType* tt) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return;

	// --- Determine texture to use ---
	GLTexture* tex = NULL;
	bool rotate = false;

	// Check for unknown type
	if (tt->getName() == "Unknown") {
		tex = theMapEditor->textureManager().getThingImage("unknown");
		COL_WHITE.set_gl();
	}

	else {
		// Otherwise, normal thing image (for now)
		tt->getColour().set_gl();

		// Check if we want an angle indicator
		if (tt->isAngled() || things_force_dir) {
			rotate = true;	// Also rotate to angle
			tex = theMapEditor->textureManager().getThingImage("normal_d");
		}
		else
			tex = theMapEditor->textureManager().getThingImage("normal_n");
	}

	// If for whatever reason the thing texture doesn't exist, just draw a basic, square thing
	if (!tex) {
		drawSquareThing(x, y, angle, tt);
		return;
	}

	// Bind texture
	tex->bind();

	// Move to thing position
	glPushMatrix();
	glTranslated(x, y, 0);

	// Rotate if needed
	if (rotate)
		glRotated(angle, 0, 0, 1);

	// Draw thing
	double radius = tt->getRadius();
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(-radius, -radius);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(-radius, radius);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(radius, radius);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(radius, -radius);
	glEnd();

	// Restore previous matrix
	glPopMatrix();
}

bool MapCanvas::drawSpriteThing(double x, double y, double angle, ThingType* tt) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return false;

	// --- Determine texture to use ---
	bool show_angle = false;
	GLTexture* tex = NULL;

	// Check for unknown type
	if (tt->getName() == "Unknown")
		tex = theMapEditor->textureManager().getThingImage("unknown");
	else {
		// Attempt to get sprite texture
		tex = theMapEditor->textureManager().getSprite(tt->getSprite());

		// If sprite not found, just draw as a normal, round thing
		if (!tex) {
			drawRoundThing(x, y, angle, tt);
			return false;
		}

		// Check if we have to draw the angle arrow later
		if (tt->isAngled() || things_force_dir)
			show_angle = true;
	}

	// If for whatever reason the thing texture doesn't exist, just draw a basic, square thing
	if (!tex) {
		drawSquareThing(x, y, angle, tt);
		return false;
	}

	// Move to thing position
	glPushMatrix();
	glTranslated(x, y, 0);

	// Bind texture
	tex->bind();

	// Draw thing
	double hw = tex->getWidth()*0.5;
	double hh = tex->getHeight()*0.5;
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(-hw, -hh);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(-hw, hh);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(hw, hh);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(hw, -hh);
	glEnd();

	// Restore previous matrix
	glPopMatrix();

	return show_angle;
}

void MapCanvas::drawSquareThing(double x, double y, double angle, ThingType* tt) {
	// Ignore if no type given (shouldn't happen)
	if (!tt)
		return;

	// Get thing info
	double radius = tt->getRadius();
	double radius2 = radius * 0.1;

	// Move to thing position
	glPushMatrix();
	glTranslated(x, y, 0);

	// Draw background
	COL_BLACK.set_gl();
	glBegin(GL_QUADS);
	glVertex2d(-radius, -radius);
	glVertex2d(-radius, radius);
	glVertex2d(radius, radius);
	glVertex2d(radius, -radius);
	glEnd();

	// Draw base
	tt->getColour().set_gl();
	glBegin(GL_QUADS);
	glVertex2d(-radius+radius2, -radius+radius2);
	glVertex2d(-radius+radius2, radius-radius2);
	glVertex2d(radius-radius2, radius-radius2);
	glVertex2d(radius-radius2, -radius+radius2);
	glEnd();

	// Draw angle indicator (if needed)
	if (tt->isAngled() || things_force_dir) {
		COL_BLACK.set_gl();
		glRotated(angle, 0, 0, 1);
		glBegin(GL_LINES);
		glVertex2d(0, 0);
		glVertex2d(radius, 0);
		glEnd();
	}

	// Restore previous matrix
	glPopMatrix();
}

void MapCanvas::drawThings() {
	// Enable textures
	if (things_drawtype > 0)
		glEnable(GL_TEXTURE_2D);
	rgba_t(255, 255, 255, 255, 0).set_gl();
	glLineWidth(2.0f);

	// Go through things
	MapThing* thing = NULL;
	double x, y;
	vector<int> things_arrows;
	for (unsigned a = 0; a < editor->getMap().nThings(); a++) {
		// Skip if outside of screen
		if (vis_t[a] > 0)
			continue;

		// Get thing info
		thing = editor->getMap().getThing(a);
		x = thing->xPos();
		y = thing->yPos();

		// Get thing type properties from game configuration
		ThingType* tt = theGameConfiguration->thingType(thing->getType());

		// Draw thing depending on 'things_drawtype' cvar
		if (things_drawtype == 2) {		// Drawtype 2: Sprites
			// Check if we need to draw the direction arrow for this thing
			if (drawSpriteThing(x, y, thing->prop("angle").getFloatValue(), tt))
				things_arrows.push_back(a);
		}
		else if (things_drawtype == 1)	// Drawtype 1: Round
			drawRoundThing(x, y, thing->prop("angle").getFloatValue(), tt);
		else							// Drawtype 0 (or other): Square
			drawSquareThing(x, y, thing->prop("angle").getFloatValue(), tt);
	}

	// Draw any thing direction arrows needed
	if (things_drawtype == 2) {
		GLTexture* tex_arrow = theMapEditor->textureManager().getThingImage("arrow");
		if (tex_arrow) {
			tex_arrow->bind();

			for (unsigned a = 0; a < things_arrows.size(); a++) {
				MapThing* thing = editor->getMap().getThing(things_arrows[a]);
				x = thing->xPos();
				y = thing->yPos();

				glPushMatrix();
				glTranslated(x, y, 0);
				glRotated(thing->prop("angle").getIntValue(), 0, 0, 1);

				COL_WHITE.set_gl();
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 1.0f);	glVertex2d(-32, -32);
				glTexCoord2f(0.0f, 0.0f);	glVertex2d(-32, 32);
				glTexCoord2f(1.0f, 0.0f);	glVertex2d(32, 32);
				glTexCoord2f(1.0f, 1.0f);	glVertex2d(32, -32);
				glEnd();

				glPopMatrix();
			}
		}
	}

	// Disable textures
	if (things_drawtype > 0)
		glDisable(GL_TEXTURE_2D);
}

void MapCanvas::drawHilight() {
	int hilight_item = editor->hilightItem();

	// Check anything is hilighted
	if (hilight_item < 0)
		return;

	// Set hilight colour
	rgba_t col = ColourConfiguration::getColour("map_hilight");
	col.a *= anim_flash_level;
	col.set_gl();

	glLineWidth(line_width*3);
	glPointSize(vertex_size*1.5f);

	// Draw depending on mode
	if (editor->editMode() == MapEditor::MODE_VERTICES) {
		// Vertex
		glBegin(GL_POINTS);
		glVertex2d(editor->getMap().getVertex(hilight_item)->xPos(),
					editor->getMap().getVertex(hilight_item)->yPos());
		glEnd();
	}
	else if (editor->editMode() == MapEditor::MODE_LINES) {
		// Set line width
		glLineWidth(line_width*3);

		// Line
		MapLine* line = editor->getMap().getLine(hilight_item);
		double x1 = line->v1()->xPos();
		double y1 = line->v1()->yPos();
		double x2 = line->v2()->xPos();
		double y2 = line->v2()->yPos();
		glBegin(GL_LINES);
		glVertex2d(x1, y1);
		glVertex2d(x2, y2);
		glEnd();

		// Direction tab
		double xmid = x1 + ((x2 - x1) * 0.5);
		double ymid = y1 + ((y2 - y1) * 0.5);
		double x = (-(y2 - y1)) * 0.125;
		double y = (x2 - x1) * 0.125;
		glBegin(GL_LINES);
		glVertex2d(xmid, ymid);
		glVertex2d(xmid - x, ymid - y);
		glEnd();
	}
	else if (editor->editMode() == MapEditor::MODE_SECTORS) {
		// Sector

		// Set line width
		glLineWidth(line_width*3);

		// Get all lines belonging to the hilighted sector
		vector<MapLine*> lines;
		editor->getMap().getLinesOfSector(hilight_item, lines);

		// Draw hilight
		MapLine* line = NULL;
		for (unsigned a = 0; a < lines.size(); a++) {
			line = lines[a];
			if (!line) continue;

			// Draw line
			glBegin(GL_LINES);
			glVertex2d(line->v1()->xPos(), line->v1()->yPos());
			glVertex2d(line->v2()->xPos(), line->v2()->yPos());
			glEnd();
		}
	}
	else if (editor->editMode() == MapEditor::MODE_THINGS) {
		// Thing
		MapThing* thing = editor->getMap().getThing(hilight_item);
		double x = thing->xPos();
		double y = thing->yPos();

		// Get thing radius
		double radius = theGameConfiguration->thingType(thing->getType())->getRadius();
		if (things_drawtype == 0)
			radius += 6;
		else
			radius *= 1.1 + (0.2*anim_flash_level);

		if (things_drawtype == 0) {
			glBegin(GL_QUADS);
			glVertex2d(x - radius, y - radius);
			glVertex2d(x - radius, y + radius);
			glVertex2d(x + radius, y + radius);
			glVertex2d(x + radius, y - radius);
			glEnd();
		}
		else {
			// Setup hilight thing texture
			GLTexture* tex = theMapEditor->textureManager().getThingImage("hilight");
			if (tex) {
				glEnable(GL_TEXTURE_2D);
				tex->bind();
			}

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex2d(x - radius, y - radius);
			glTexCoord2f(0.0f, 1.0f);	glVertex2d(x - radius, y + radius);
			glTexCoord2f(1.0f, 1.0f);	glVertex2d(x + radius, y + radius);
			glTexCoord2f(1.0f, 0.0f);	glVertex2d(x + radius, y - radius);
			glEnd();

			glDisable(GL_TEXTURE_2D);
		}
	}

	glPointSize(vertex_size);
}

void MapCanvas::drawSelection() {
	vector<int>& selection = editor->getSelection();
	SLADEMap& map = editor->getMap();

	// Check anything is selected
	if (selection.size() == 0)
		return;

	// Set selection colour
	ColourConfiguration::getColour("map_selection").set_gl();

	glLineWidth(line_width*4);
	glPointSize(vertex_size*1.5f);

	// Draw depending on mode
	if (editor->editMode() == MapEditor::MODE_VERTICES) {
		// Vertices
		double x, y;
		glBegin(GL_POINTS);
		for (unsigned a = 0; a < selection.size(); a++) {
			// Skip if not on screen
			if (vis_v[selection[a]] > 0)
				continue;

			// Draw vertex selection
			glVertex2d(map.getVertex(selection[a])->xPos(), map.getVertex(selection[a])->yPos());
		}
		glEnd();
	}
	else if (editor->editMode() == MapEditor::MODE_LINES) {
		// Set line width
		glLineWidth(line_width*4);

		// Lines
		MapLine* line;
		double x1, y1, x2, y2;
		glBegin(GL_LINES);
		for (unsigned a = 0; a < selection.size(); a++) {
			// Skip if not on screen
			if (vis_l[selection[a]] > 0)
				continue;

			line = map.getLine(selection[a]);
			x1 = line->v1()->xPos();
			y1 = line->v1()->yPos();
			x2 = line->v2()->xPos();
			y2 = line->v2()->yPos();

			// Draw line
			glVertex2d(x1, y1);
			glVertex2d(x2, y2);

			// Direction tab
			double xmid = x1 + ((x2 - x1) * 0.5);
			double ymid = y1 + ((y2 - y1) * 0.5);
			double x = (-(y2 - y1)) * 0.125;
			double y = (x2 - x1) * 0.125;
			glVertex2d(xmid, ymid);
			glVertex2d(xmid - x, ymid - y);
		}
		glEnd();
	}
	else if (editor->editMode() == MapEditor::MODE_SECTORS) {
		// Sectors

		// Set line width
		glLineWidth(line_width*4);

		// Get selected sector(s)
		vector<MapSector*> selected_sectors;
		for (unsigned a = 0; a < selection.size(); a++)
			selected_sectors.push_back(map.getSector(selection[a]));

		// Go through map lines
		MapLine* line;
		double x1, y1, x2, y2;
		glBegin(GL_LINES);
		for (unsigned a = 0; a < map.nLines(); a++) {
			// Skip if not on screen
			if (vis_l[a] > 0)
				continue;

			// Check if line is in a selected sector
			line = map.getLine(a);
			bool selected = false;
			for (unsigned b = 0; b < selected_sectors.size(); b++) {
				if (map.lineInSector(line, selected_sectors[b])) {
					selected = true;
					break;
				}
			}

			// Skip if not selected
			if (!selected)
				continue;

			// Draw line
			glVertex2d(line->v1()->xPos(), line->v1()->yPos());
			glVertex2d(line->v2()->xPos(), line->v2()->yPos());
		}
		glEnd();
	}
	else if (editor->editMode() == MapEditor::MODE_THINGS) {
		// Things
		double x, y;
		MapThing* thing = NULL;
		for (unsigned a = 0; a < selection.size(); a++) {
			// Skip if not on screen
			if (vis_t[selection[a]] > 0)
				continue;

			thing = map.getThing(selection[a]);
			x = thing->xPos();
			y = thing->yPos();

			// Get thing radius
			double radius = theGameConfiguration->thingType(thing->getType())->getRadius() + 8;

			if (things_drawtype == 0) {
				glBegin(GL_QUADS);
				glVertex2d(x - radius, y - radius);
				glVertex2d(x - radius, y + radius);
				glVertex2d(x + radius, y + radius);
				glVertex2d(x + radius, y - radius);
				glEnd();
			}
			else {
				// Setup hilight thing texture
				GLTexture* tex = theMapEditor->textureManager().getThingImage("hilight");
				if (tex) {
					glEnable(GL_TEXTURE_2D);
					tex->bind();
				}

				// Draw thing selection if on screen
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);	glVertex2d(x - radius, y - radius);
				glTexCoord2f(0.0f, 1.0f);	glVertex2d(x - radius, y + radius);
				glTexCoord2f(1.0f, 1.0f);	glVertex2d(x + radius, y + radius);
				glTexCoord2f(1.0f, 0.0f);	glVertex2d(x + radius, y - radius);
				glEnd();

				glDisable(GL_TEXTURE_2D);
			}
		}
	}

	glPointSize(vertex_size);
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

	// Translate to inside of pixel (otherwise inaccuracies can occur on certain gl implemenataions)
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

	// --- Draw map ---

	// Check if we need to update visibility
	if (vis_v.size() != editor->getMap().nVertices() ||
		vis_l.size() != editor->getMap().nLines() ||
		vis_t.size() != editor->getMap().nThings())
		updateVisibility();

	// Lines always (show direction only in lines mode)
	drawLines(editor->editMode() == MapEditor::MODE_LINES);

	// Vertices if in vertex mode
	if (editor->editMode() == MapEditor::MODE_VERTICES)
		drawVertices();

	// Things if in things mode
	if (editor->editMode() == MapEditor::MODE_THINGS)
		drawThings();

	// Draw overlays (hilight etc)
	if (mouse_state == MSTATE_NORMAL) drawHilight();
	drawSelection();

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
	if (editor->editMode() == MapEditor::MODE_VERTICES)
		info_vertex.draw(GetSize().y, GetSize().x, anim_info_fade);
	else if (editor->editMode() == MapEditor::MODE_LINES)
		info_line.draw(GetSize().y, GetSize().x, anim_info_fade);
	else if (editor->editMode() == MapEditor::MODE_SECTORS)
		info_sector.draw(GetSize().y, GetSize().x, anim_info_fade);
	else if (editor->editMode() == MapEditor::MODE_THINGS)
		info_thing.draw(GetSize().y, GetSize().x, anim_info_fade);

	// FPS counter
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

	SwapBuffers();
}

void MapCanvas::update(long frametime) {
	// Get frame time multiplier
	float mult = (float)frametime / 10.0f;

	// Update hilight if needed
	if (mouse_state == MSTATE_NORMAL)
		editor->updateHilight(mouse_pos_m, view_scale);

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
		//anim_info_fade -= 0.05f*mult;
		anim_info_fade -= 0.04f*mult;
		if (anim_info_fade < 0.0f) {
			anim_info_fade = 0.0f;
			fade_anim = false;
		}
	}

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
	if (mouse_state == MSTATE_SELECTION || mouse_state == MSTATE_PANNING || anim_running || fade_anim) {
#ifdef USE_SFML_RENDERWINDOW
		fr_idle = 2;	// SFML RenderWindow can handle high framerates better than wxGLCanvas, or something like that
#else
		fr_idle = 5;
#endif
	}
	else {
		// No high-priority animations running, throttle framerate
		fr_idle = 30;
	}

	frametime_last = frametime;
}

void MapCanvas::updateVisibility() {
	SLADEMap& map = editor->getMap();

	// Vertex visibility
	if (map.nVertices() != vis_v.size()) {
		// Number of vertices changed, reset array
		vis_v.clear();
		for (unsigned a = 0; a < map.nVertices(); a++)
			vis_v.push_back(0);
	}
	double x, y;
	for (unsigned a = 0; a < vis_v.size(); a++) {
		x = map.getVertex(a)->xPos();
		y = map.getVertex(a)->yPos();
		vis_v[a] = 0;
		if (x < view_tl.x)
			vis_v[a] |= VIS_LEFT;
		if (x > view_br.x)
			vis_v[a] |= VIS_RIGHT;
		if (y < view_tl.y)
			vis_v[a] |= VIS_BELOW;
		if (y > view_br.y)
			vis_v[a] |= VIS_ABOVE;
	}

	// Line visibility
	if (map.nLines() != vis_l.size()) {
		// Number of lines changed, reset array
		vis_l.clear();
		for (unsigned a = 0; a < map.nLines(); a++)
			vis_l.push_back(0);
	}
	uint8_t vis_v1, vis_v2;
	for (unsigned a = 0; a < vis_l.size(); a++) {
		vis_v1 = vis_v[map.getLine(a)->v1()->getIndex()];
		vis_v2 = vis_v[map.getLine(a)->v2()->getIndex()];
		vis_l[a] = 0;
		if (vis_v1 & VIS_LEFT && vis_v2 & VIS_LEFT)
			vis_l[a] |= VIS_LEFT;
		if (vis_v1 & VIS_RIGHT && vis_v2 & VIS_RIGHT)
			vis_l[a] |= VIS_RIGHT;
		if (vis_v1 & VIS_BELOW && vis_v2 & VIS_BELOW)
			vis_l[a] |= VIS_BELOW;
		if (vis_v1 & VIS_ABOVE && vis_v2 & VIS_ABOVE)
			vis_l[a] |= VIS_ABOVE;
	}

	// Thing visibility
	if (map.nThings() != vis_t.size()) {
		// Number of things changed, reset array
		vis_t.clear();
		for (unsigned a = 0; a < map.nThings(); a++)
			vis_t.push_back(0);
	}
	double radius;
	for (unsigned a = 0; a < vis_t.size(); a++) {
		vis_t[a] = 0;
		x = map.getThing(a)->xPos();
		y = map.getThing(a)->yPos();

		// Get thing type properties from game configuration
		ThingType* tt = theGameConfiguration->thingType(map.getThing(a)->getType());
		radius = tt->getRadius();

		// Ignore if outside of screen
		if (x+radius < view_tl.x || x-radius > view_br.x || y+radius < view_tl.y || y-radius > view_br.y)
			vis_t[a] = 1;

		// Check if the thing is worth drawing
		else if (radius*view_scale < 1)
			vis_t[a] = 1;
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
		mouse_downpos.set(mouse_pos);
		mouse_state = MSTATE_PANNING;
		editor->clearHilight();
		SetCursor(wxCURSOR_SIZING);
	}

	// Select all
	else if (name == "select_all")
		editor->selectAll();

	// Clear selection
	else if (name == "me2d_clear_selection")
		editor->clearSelection();
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
	updateVisibility();
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
		// Try to select hilighted object
		if (!editor->selectCurrent(!e.ShiftDown()))
			mouse_state = MSTATE_SELECTION;	// Nothing hilighted, begin box selection
	}

	// Right button
	else if (e.RightDown()) {
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
		// If we're ending a box selection
		if (mouse_state == MSTATE_SELECTION) {
			// Reset mouse state
			mouse_state = MSTATE_NORMAL;

			// Select
			editor->selectWithin(min(mouse_downpos_m.x, mouse_pos_m.x), min(mouse_downpos_m.y, mouse_pos_m.y),
								max(mouse_downpos_m.x, mouse_pos_m.x), max(mouse_downpos_m.y, mouse_pos_m.y));

			// Begin selection box fade animation
			animations.push_back(new MCASelboxFader(theApp->runTimer(), mouse_downpos_m, mouse_pos_m));
		}
	}

	// Right button
	else if (e.RightUp()) {
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
	SetFocus();

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
