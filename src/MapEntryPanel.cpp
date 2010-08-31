
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MapEntryPanel.cpp
 * Description: MapEntryPanel class. Shows a basic (lines-only)
 *              preview of a map entry
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
#include "MapEntryPanel.h"
#include "Archive.h"
#include "MapLine.h"
#include "MapVertex.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
// Some temporary colours (these will eventually be setup in some kind of global colour configuration)
rgba_t col_line_1s(255, 255, 255, 255);
rgba_t col_line_2s(170, 170, 170, 255);
rgba_t col_line_special(130, 140, 255, 255);


/*******************************************************************
 * MEPCANVAS CLASS FUNCTIONS
 *******************************************************************/

/* MEPCanvas::MEPCanvas
 * MEPCanvas class constructor
 *******************************************************************/
MEPCanvas::MEPCanvas(wxWindow* parent) : OGLCanvas(parent, -1) {
	zoom = 1;
	offset_x = 0;
	offset_y = 0;
}

/* MEPCanvas::~MEPCanvas
 * MEPCanvas class destructor
 *******************************************************************/
MEPCanvas::~MEPCanvas() {
}

/* MEPCanvas::addVertex
 * Adds a vertex to the map data
 *******************************************************************/
void MEPCanvas::addVertex(double x, double y) {
	verts.push_back(mep_vertex_t(x, y));
}

/* MEPCanvas::addLine
 * Adds a line to the map data
 *******************************************************************/
void MEPCanvas::addLine(unsigned v1, unsigned v2, bool twosided, bool special) {
	mep_line_t line(v1, v2);
	line.twosided = twosided;
	line.special = special;
	lines.push_back(line);
}

/* MEPCanvas::clearMap
 * Clears map data
 *******************************************************************/
void MEPCanvas::clearMap() {
	verts.clear();
	lines.clear();
}

/* MEPCanvas::showMap
 * Adjusts zoom and offset to show the whole map
 *******************************************************************/
void MEPCanvas::showMap() {
	// Find extents of map
	mep_vertex_t m_min(999999.0, 999999.0);
	mep_vertex_t m_max(-999999.0, -999999.0);
	for (unsigned a = 0; a < verts.size(); a++) {
		if (verts[a].x < m_min.x)
			m_min.x = verts[a].x;
		if (verts[a].x > m_max.x)
			m_max.x = verts[a].x;
		if (verts[a].y < m_min.y)
			m_min.y = verts[a].y;
		if (verts[a].y > m_max.y)
			m_max.y = verts[a].y;
	}

	// Offset to center of map
	double width = m_max.x - m_min.x;
	double height = m_max.y - m_min.y;
	offset_x = m_min.x + (width * 0.5);
	offset_y = m_min.y + (height * 0.5);

	// Zoom to fit whole map
	double x_scale = ((double)GetClientSize().x) / width;
	double y_scale = ((double)GetClientSize().y) / height;
	zoom = min(x_scale, y_scale);
	zoom *= 0.95;
}

/* MEPCanvas::draw
 * Draws the map
 *******************************************************************/
void MEPCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetSize().x, 0, GetSize().y, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Zoom/offset to show full map
	showMap();

	// Translate to middle of canvas
	glTranslated(GetSize().x * 0.5, GetSize().y * 0.5, 0);

	// Zoom
	glScaled(zoom, zoom, 1);

	// Translate to offset
	glTranslated(-offset_x, -offset_y, 0);

	// Setup drawing
	glDisable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(1.5f);
	glEnable(GL_LINE_SMOOTH);

	// Draw lines
	for (unsigned a = 0; a < lines.size(); a++) {
		mep_line_t line = lines[a];

		// Check ends
		if (line.v1 >= verts.size() || line.v2 >= verts.size())
			continue;

		// Get vertices
		mep_vertex_t v1 = verts[lines[a].v1];
		mep_vertex_t v2 = verts[lines[a].v2];

		// Set colour
		if (line.special)
			col_line_special.set_gl();
		else if (line.twosided)
			col_line_2s.set_gl();
		else
			col_line_1s.set_gl();

		// Draw line
		glBegin(GL_LINES);
		glVertex2d(v1.x, v1.y);
		glVertex2d(v2.x, v2.y);
		glEnd();
	}

	glLineWidth(1.0f);
	glDisable(GL_LINE_SMOOTH);

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}


/*******************************************************************
 * MAPENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* MapEntryPanel::MapEntryPanel
 * MapEntryPanel class constructor
 *******************************************************************/
MapEntryPanel::MapEntryPanel(wxWindow* parent) : EntryPanel(parent, "map") {
	// Setup map canvas
	map_canvas = new MEPCanvas(this);
	sizer_main->Add(map_canvas, 1, wxEXPAND|wxALL, 4);

	// Layout
	Layout();
}

/* MapEntryPanel::~MapEntryPanel
 * MapEntryPanel class destructor
 *******************************************************************/
MapEntryPanel::~MapEntryPanel() {
}

/* MapEntryPanel::loadEntry
 * Loads [entry] into the EntryPanel. Returns false if the map was
 * invalid, true otherwise
 *******************************************************************/
bool MapEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Clear current map data
	map_canvas->clearMap();

	// Find map definition for entry
	vector<Archive::mapdesc_t> maps = entry->getParent()->detectMaps();
	Archive::mapdesc_t thismap;
	for (unsigned a = 0; a < maps.size(); a++) {
		if (maps[a].head == entry) {
			thismap = maps[a];
			break;
		}
	}

	// Read vertices
	if (thismap.format == MAP_DOOM || thismap.format == MAP_HEXEN || thismap.format == MAP_DOOM64) {
		// Find VERTEXES entry
		ArchiveEntry* mapentry = thismap.head;
		ArchiveEntry* vertexes = NULL;
		while (1) {
			// Check entry type
			if (mapentry->getType() == EntryType::getType("map_vertexes")) {
				vertexes = mapentry;
				break;
			}

			// Exit loop if we've reached the end of the map entries
			if (mapentry == thismap.end)
				break;
			else
				mapentry = mapentry->nextEntry();
		}

		// Can't open a map without vertices
		if (!vertexes)
			return false;

		// Read vertex data
		MemChunk& mc = vertexes->getMCData();
		mc.seek(0, SEEK_SET);

		if (thismap.format == MAP_DOOM64) {
			doom64vertex_t v;
			while (1) {
				// Read vertex
				if (!mc.read(&v, 8))
					break;

				// Add vertex
				map_canvas->addVertex((double)v.x/65536, (double)v.y/65536);
			}
		} else {
			doomvertex_t v;
			while (1) {
				// Read vertex
				if (!mc.read(&v, 4))
					break;

				// Add vertex
				map_canvas->addVertex((double)v.x, (double)v.y);
			}
		}
	}


	// Read linedefs
	if (thismap.format == MAP_DOOM || thismap.format == MAP_HEXEN || thismap.format == MAP_DOOM64) {
		// Find LINEDEFS entry
		ArchiveEntry* mapentry = thismap.head;
		ArchiveEntry* linedefs = NULL;
		while (1) {
			// Check entry type
			if (mapentry->getType() == EntryType::getType("map_linedefs")) {
				linedefs = mapentry;
				break;
			}

			// Exit loop if we've reached the end of the map entries
			if (mapentry == thismap.end)
				break;
			else
				mapentry = mapentry->nextEntry();
		}

		// Can't open a map without vertices
		if (!linedefs)
			return false;

		// Read line data
		MemChunk& mc = linedefs->getMCData();
		mc.seek(0, SEEK_SET);
		if (thismap.format == MAP_DOOM) {
			while (1) {
				// Read line
				doomline_t l;
				if (!mc.read(&l, sizeof(doomline_t)))
					break;

				// Check properties
				bool special = false;
				bool twosided = false;
				if (l.side2 >= 0)
					twosided = true;
				if (l.type > 0)
					special = true;

				// Add line
				map_canvas->addLine(l.vertex1, l.vertex2, twosided, special);
			}
		}
		else if (thismap.format == MAP_DOOM64) {
			while (1) {
				// Read line
				doom64line_t l;
				if (!mc.read(&l, sizeof(doom64line_t)))
					break;

				// Check properties
				bool special = false;
				bool twosided = false;
				if (l.side2 >= 0)
					twosided = true;
				if (l.type > 0)
					special = true;

				// Add line
				map_canvas->addLine(l.vertex1, l.vertex2, twosided, special);
			}
		}
		else if (thismap.format == MAP_HEXEN) {
			while (1) {
				// Read line
				hexenline_t l;
				if (!mc.read(&l, sizeof(hexenline_t)))
					break;

				// Check properties
				bool special = false;
				bool twosided = false;
				if (l.side2 >= 0)
					twosided = true;
				if (l.type > 0)
					special = true;

				// Add line
				map_canvas->addLine(l.vertex1, l.vertex2, twosided, special);
			}
		}
	}

	// Refresh map
	map_canvas->Refresh();

	return true;
}

/* MapEntryPanel::saveEntry
 * Saves any changes to the entry (does nothing in map viewer)
 *******************************************************************/
bool MapEntryPanel::saveEntry() {
	return true;
}
