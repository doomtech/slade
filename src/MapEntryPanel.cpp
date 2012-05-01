
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
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
#include "Parser.h"
#include "SImage.h"
#include "SIFormat.h"
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Int, map_image_width,  800, CVAR_SAVE)
CVAR(Int, map_image_height, 600, CVAR_SAVE)


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(String, dir_last)


/*******************************************************************
 * MEPCANVAS CLASS FUNCTIONS
 *******************************************************************/

/* MEPCanvas::MEPCanvas
 * MEPCanvas class constructor
 *******************************************************************/
//MEPCanvas::MEPCanvas(wxWindow* parent) : OGLCanvas(parent, -1) {
//	zoom = 1;
//	offset_x = 0;
//	offset_y = 0;
//}

/* MEPCanvas::~MEPCanvas
 * MEPCanvas class destructor
 *******************************************************************/
//MEPCanvas::~MEPCanvas() {
//}

/* MEPCanvas::addVertex
 * Adds a vertex to the map data
 *******************************************************************/
//void MEPCanvas::addVertex(double x, double y) {
//	verts.push_back(mep_vertex_t(x, y));
//}

/* MEPCanvas::addLine
 * Adds a line to the map data
 *******************************************************************/
//void MEPCanvas::addLine(unsigned v1, unsigned v2, bool twosided, bool special, bool macro) {
//	mep_line_t line(v1, v2);
//	line.twosided = twosided;
//	line.special = special;
//	line.macro = macro;
//	lines.push_back(line);
//}

/* MEPCanvas::clearMap
 * Clears map data
 *******************************************************************/
//void MEPCanvas::clearMap() {
//	verts.clear();
//	lines.clear();
//}

/* MEPCanvas::showMap
 * Adjusts zoom and offset to show the whole map
 *******************************************************************/
//void MEPCanvas::showMap() {
//	// Find extents of map
//	mep_vertex_t m_min(999999.0, 999999.0);
//	mep_vertex_t m_max(-999999.0, -999999.0);
//	for (unsigned a = 0; a < verts.size(); a++) {
//		if (verts[a].x < m_min.x)
//			m_min.x = verts[a].x;
//		if (verts[a].x > m_max.x)
//			m_max.x = verts[a].x;
//		if (verts[a].y < m_min.y)
//			m_min.y = verts[a].y;
//		if (verts[a].y > m_max.y)
//			m_max.y = verts[a].y;
//	}
//
//	// Offset to center of map
//	double width = m_max.x - m_min.x;
//	double height = m_max.y - m_min.y;
//	offset_x = m_min.x + (width * 0.5);
//	offset_y = m_min.y + (height * 0.5);
//
//	// Zoom to fit whole map
//	double x_scale = ((double)GetClientSize().x) / width;
//	double y_scale = ((double)GetClientSize().y) / height;
//	zoom = MIN(x_scale, y_scale);
//	zoom *= 0.95;
//}

/* MEPCanvas::draw
 * Draws the map
 *******************************************************************/
//void MEPCanvas::draw() {
//	// Setup colours
//	wxColour wxc;
//	wxc.Set(map_view_col_background);	rgba_t col_view_background(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//	wxc.Set(map_view_col_line_1s);		rgba_t col_view_line_1s(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//	wxc.Set(map_view_col_line_2s);		rgba_t col_view_line_2s(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//	wxc.Set(map_view_col_line_special);	rgba_t col_view_line_special(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//	wxc.Set(map_view_col_line_macro);	rgba_t col_view_line_macro(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//
//	// Setup the viewport
//	glViewport(0, 0, GetSize().x, GetSize().y);
//
//	// Setup the screen projection
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	glOrtho(0, GetSize().x, 0, GetSize().y, -1, 1);
//
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	// Clear
//	glClearColor(((double)col_view_background.r)/255.f, ((double)col_view_background.g)/255.f,
//				 ((double)col_view_background.b)/255.f, ((double)col_view_background.a)/255.f);
//	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//
//	// Translate to inside of pixel (otherwise inaccuracies can occur on certain gl implementations)
//	glTranslatef(0.375f, 0.375f, 0);
//
//	// Zoom/offset to show full map
//	showMap();
//
//	// Translate to middle of canvas
//	glTranslated(GetSize().x * 0.5, GetSize().y * 0.5, 0);
//
//	// Zoom
//	glScaled(zoom, zoom, 1);
//
//	// Translate to offset
//	glTranslated(-offset_x, -offset_y, 0);
//
//	// Setup drawing
//	glDisable(GL_TEXTURE_2D);
//	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//	glLineWidth(1.5f);
//	glEnable(GL_LINE_SMOOTH);
//
//	// Draw lines
//	for (unsigned a = 0; a < lines.size(); a++) {
//		mep_line_t line = lines[a];
//
//		// Check ends
//		if (line.v1 >= verts.size() || line.v2 >= verts.size())
//			continue;
//
//		// Get vertices
//		mep_vertex_t v1 = verts[lines[a].v1];
//		mep_vertex_t v2 = verts[lines[a].v2];
//
//		// Set colour
//		if (line.special)
//			col_view_line_special.set_gl();
//		else if (line.macro)
//			col_view_line_macro.set_gl();
//		else if (line.twosided)
//			col_view_line_2s.set_gl();
//		else
//			col_view_line_1s.set_gl();
//
//		// Draw line
//		glBegin(GL_LINES);
//		glVertex2d(v1.x, v1.y);
//		glVertex2d(v2.x, v2.y);
//		glEnd();
//	}
//
//	glLineWidth(1.0f);
//	glDisable(GL_LINE_SMOOTH);
//
//	// Swap buffers (ie show what was drawn)
//	SwapBuffers();
//}


/* MEPCanvas::createImage
 * Draws the map in an image
 * TODO: Factorize code with normal draw() and showMap() functions.
 * TODO: Find a way to generate an arbitrary-sized image through
 * tiled rendering.
 *******************************************************************/
//void MEPCanvas::createImage(ArchiveEntry& ae, int width, int height) {
//	// Setup colours
//	wxColour wxc;
//	wxc.Set(map_image_col_background);	rgba_t col_save_background(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//	wxc.Set(map_image_col_line_1s);		rgba_t col_save_line_1s(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//	wxc.Set(map_image_col_line_2s);		rgba_t col_save_line_2s(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//	wxc.Set(map_image_col_line_special);rgba_t col_save_line_special(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//	wxc.Set(map_image_col_line_macro);	rgba_t col_save_line_macro(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
//	col_save_background.a = map_image_alpha_background;
//
//	// Setup OpenGL rigmarole
//	GLuint texID, fboID;
//	if (GLEW_ARB_framebuffer_object) {
//		glGenTextures(1, &texID);
//		glBindTexture(GL_TEXTURE_2D, texID);
//		// We don't use mipmaps, but OpenGL will refuse to attach
//		// the texture to the framebuffer if they are not present
//		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
//		glBindTexture(GL_TEXTURE_2D, 0);
//		glGenFramebuffersEXT(1, &fboID);
//		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);
//		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texID, 0);
//		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
//	}
//
//	glViewport(0, 0, width, height);
//
//	// Setup the screen projection
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	glOrtho(0, width, 0, height, -1, 1);
//
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	// Clear
//	glClearColor(((double)col_save_background.r)/255.f, ((double)col_save_background.g)/255.f,
//				 ((double)col_save_background.b)/255.f, ((double)col_save_background.a)/255.f);
//	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//
//	// Translate to inside of pixel (otherwise inaccuracies can occur on certain gl implementations)
//	glTranslatef(0.375f, 0.375f, 0);
//
//	// Zoom/offset to show full map
//	// Find extents of map
//	mep_vertex_t m_min(999999.0, 999999.0);
//	mep_vertex_t m_max(-999999.0, -999999.0);
//	for (unsigned a = 0; a < verts.size(); a++) {
//		if (verts[a].x < m_min.x)
//			m_min.x = verts[a].x;
//		if (verts[a].x > m_max.x)
//			m_max.x = verts[a].x;
//		if (verts[a].y < m_min.y)
//			m_min.y = verts[a].y;
//		if (verts[a].y > m_max.y)
//			m_max.y = verts[a].y;
//	}
//
//	// Offset to center of map
//	double mapwidth = m_max.x - m_min.x;
//	double mapheight = m_max.y - m_min.y;
//	offset_x = m_min.x + (mapwidth * 0.5);
//	offset_y = m_min.y + (mapheight * 0.5);
//
//	// Zoom to fit whole map
//	double x_scale = ((double)width) / mapwidth;
//	double y_scale = ((double)height) / mapheight;
//	zoom = MIN(x_scale, y_scale);
//	zoom *= 0.95;
//
//	// Translate to middle of canvas
//	glTranslated(width>>1, height>>1, 0);
//
//	// Zoom
//	glScaled(zoom, zoom, 1);
//
//	// Translate to offset
//	glTranslated(-offset_x, -offset_y, 0);
//
//	// Setup drawing
//	glDisable(GL_TEXTURE_2D);
//	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//	glLineWidth(map_image_thickness);
//	glEnable(GL_LINE_SMOOTH);
//
//	// Draw lines
//	for (unsigned a = 0; a < lines.size(); a++) {
//		mep_line_t line = lines[a];
//
//		// Check ends
//		if (line.v1 >= verts.size() || line.v2 >= verts.size())
//			continue;
//
//		// Get vertices
//		mep_vertex_t v1 = verts[lines[a].v1];
//		mep_vertex_t v2 = verts[lines[a].v2];
//
//		// Set colour
//		if (line.special)
//			col_save_line_special.set_gl();
//		else if (line.macro)
//			col_save_line_macro.set_gl();
//		else if (line.twosided)
//			col_save_line_2s.set_gl();
//		else
//			col_save_line_1s.set_gl();
//
//		// Draw line
//		glBegin(GL_LINES);
//		glVertex2d(v1.x, v1.y);
//		glVertex2d(v2.x, v2.y);
//		glEnd();
//	}
//
//	glLineWidth(1.0f);
//	glDisable(GL_LINE_SMOOTH);
//
//	uint8_t * ImageBuffer = new uint8_t[width * height * 4];
//	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, ImageBuffer);
//
//	if (GLEW_ARB_framebuffer_object) {
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//		glDeleteTextures( 1, &texID );
//		glDeleteFramebuffersEXT( 1, &fboID );
//	}
//	SImage img;
//	img.setImageData(ImageBuffer, width, height, RGBA);
//	img.mirror(true);
//	MemChunk mc;
//	SIFormat::getFormat("png")->saveImage(img, mc);
//	ae.importMemChunk(mc);
//
//}

/*******************************************************************
 * MAPENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* MapEntryPanel::MapEntryPanel
 * MapEntryPanel class constructor
 *******************************************************************/
MapEntryPanel::MapEntryPanel(wxWindow* parent) : EntryPanel(parent, "map") {
	// Setup map canvas
	map_canvas = new MapPreviewCanvas(this);
	sizer_main->Add(map_canvas->toPanel(this), 1, wxEXPAND, 0);

	// Add 'Save Map Image' button
	btn_saveimg = new wxButton(this, -1, "Save Map Image");
	sizer_top->Add(btn_saveimg, 0, wxEXPAND|wxRIGHT, 4);

	// Add 'Edit Level Script' button
	btn_script = new wxButton(this, -1, "Edit Level Script");
	sizer_top->Add(btn_script, 0, wxEXPAND|wxRIGHT, 4);

	// Bind events
	btn_saveimg->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MapEntryPanel::onBtnSaveImage, this);

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
	bool found = false;
	for (unsigned a = 0; a < maps.size(); a++) {
		if (maps[a].head == entry) {
			thismap = maps[a];
			found = true;
			break;
		}
	}

	// All errors = invalid map
	Global::error = "Invalid map";

	// There is no map entry for the map marker.
	// This may happen if a map marker lump is copy/pasted without the rest of the map lumps.
	if (!found) {
		entry->setType(EntryType::unknownType());
		EntryType::detectEntryType(entry);
		return false;
	}

	// Load map into preview canvas
	return map_canvas->openMap(thismap);

	/*
	// Parse UDMF map
	if (thismap.format == MAP_UDMF) {
		ArchiveEntry* udmfdata = NULL;
		for (ArchiveEntry* mapentry = thismap.head; mapentry != thismap.end; mapentry = mapentry->nextEntry()) {
			// Check entry type
			if (mapentry->getType() == EntryType::getType("udmf_textmap")) {
				udmfdata = mapentry;
				break;
			}
		}
		if (udmfdata == NULL)
			return false;

		// Start parsing
		Tokenizer tz;
		tz.openMem(udmfdata->getData(), udmfdata->getSize(), thismap.head->getName());

		// Get first token
		string token = tz.getToken();
		size_t vertcounter = 0, linecounter = 0;
		while (!token.IsEmpty()) {
			if (!token.CmpNoCase("namespace")) {
				//  skip till we reach the ';'
				do { token = tz.getToken(); } while (token.Cmp(";"));
			} else if (!token.CmpNoCase("vertex")) {
				// Get X and Y properties
				bool gotx = false;
				bool goty = false;
				double x = 0.;
				double y = 0.;
				do {
					token = tz.getToken();
					if (!token.CmpNoCase("x") || !token.CmpNoCase("y")) {
						bool isx = !token.CmpNoCase("x");
						token = tz.getToken();
						if (token.Cmp("=")) {
							wxLogMessage("Bad syntax for vertex %i in UDMF map data", vertcounter);
							return false;
						}
						if (isx) x = tz.getDouble(), gotx = true;
						else y = tz.getDouble(), goty = true;
						// skip to end of declaration after each key
						do { token = tz.getToken(); } while (token.Cmp(";"));
					}
				} while (token.Cmp("}"));
				if (gotx && goty)
					map_canvas->addVertex(x, y);
				else {
					wxLogMessage("Wrong vertex %i in UDMF map data", vertcounter);
					return false;
				}
				vertcounter++;
			} else if (!token.CmpNoCase("linedef")) {
				bool special = false;
				bool twosided = false;
				bool gotv1 = false, gotv2 = false;
				size_t v1 = 0, v2 = 0;
				do {
					token = tz.getToken();
					if (!token.CmpNoCase("v1") || !token.CmpNoCase("v2")) {
						bool isv1 = !token.CmpNoCase("v1");
						token = tz.getToken();
						if (token.Cmp("=")) {
							wxLogMessage("Bad syntax for linedef %i in UDMF map data", linecounter);
							return false;
						}
						if (isv1) v1 = tz.getInteger(), gotv1 = true;
						else v2 = tz.getInteger(), gotv2 = true;
						// skip to end of declaration after each key
						do { token = tz.getToken(); } while (token.Cmp(";"));
					} else if (!token.CmpNoCase("special")) {
						special = true;
						// skip to end of declaration after each key
						do { token = tz.getToken(); } while (token.Cmp(";"));
					} else if (!token.CmpNoCase("sideback")) {
						twosided = true;
						// skip to end of declaration after each key
						do { token = tz.getToken(); } while (token.Cmp(";"));
					}
				} while (token.Cmp("}"));
				if (gotv1 && gotv2)
					map_canvas->addLine(v1, v2, twosided, special);
				else {
					wxLogMessage("Wrong line %i in UDMF map data", linecounter);
					return false;
				}
				linecounter++;
			} else {
				// map preview ignores things, sidedefs, sectors, comments,
				// unknown fields, etc. so skip to end of block
				do { token = tz.getToken(); } while (token.Cmp("}"));
			}
			// Iterate to next token
			token = tz.getToken();
		}
	}

	// Read vertices
	if (thismap.format == MAP_DOOM || thismap.format == MAP_HEXEN || thismap.format == MAP_DOOM64) {
		// Find VERTEXES entry
		ArchiveEntry* mapentry = thismap.head;
		ArchiveEntry* vertexes = NULL;
		while (mapentry) {
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
		while (mapentry) {
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

		// Can't open a map without linedefs
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
				if (l.side2 > 0)
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
				bool macro = false;
				bool special = false;
				bool twosided = false;
				if (l.side2 > 0)
					twosided = true;
				if (l.type > 0) {
					if (l.type & 0x100)
						macro = true;
					else special = true;
				}

				// Add line
				map_canvas->addLine(l.vertex1, l.vertex2, twosided, special, macro);
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
				if (l.side2 > 0)
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
	*/
}

/* MapEntryPanel::saveEntry
 * Saves any changes to the entry (does nothing in map viewer)
 *******************************************************************/
bool MapEntryPanel::saveEntry() {
	return true;
}

/* MapEntryPanel::createImage
 * Creates a PNG file of the map preview
 * TODO: Preference panel for background and line colors,
 * as well as for image size
 *******************************************************************/
bool MapEntryPanel::createImage() {
	if (entry == NULL)
		return false;

	ArchiveEntry temp;
	// Stupid OpenGL grumble grumble grumble
	if (GLEW_ARB_framebuffer_object)
		map_canvas->createImage(temp, map_image_width, map_image_height);
	else
		map_canvas->createImage(temp, min<int>(map_image_width, map_canvas->GetSize().x), 
									  min<int>(map_image_height, map_canvas->GetSize().y));
	string name = S_FMT("%s_%s", CHR(entry->getParent()->getFilename(false)), CHR(entry->getName()));
	wxFileName fn(name);

	// Create save file dialog
	wxFileDialog dialog_save(this, S_FMT("Save Map Preview \"%s\"", name.c_str()),
								dir_last, fn.GetFullName(), "PNG (*.PNG)|*.png",
								wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_save.ShowModal() == wxID_OK) {
		// If a filename was selected, export it
		bool ret = temp.exportFile(dialog_save.GetPath());

		// Save 'dir_last'
		dir_last = dialog_save.GetDirectory();

		return ret;
	}
	return true;
}

/* MapEntryPanel::onBtnSaveImage
 * Called when the 'Save Map Image' button is clicked
 *******************************************************************/
void MapEntryPanel::onBtnSaveImage(wxCommandEvent& e) {
	createImage();
}
