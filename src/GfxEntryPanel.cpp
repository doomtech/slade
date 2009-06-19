
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GfxEntryPanel.cpp
 * Description: GfxEntryPanel class. The UI for editing gfx entries.
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
#include "GfxEntryPanel.h"


GfxCanvas::GfxCanvas(wxWindow* parent, int id)
: wxGLCanvas(parent, id, NULL, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN) {
}

GfxCanvas::~GfxCanvas() {
}

/* GfxCanvas::setContext
 * Sets the current gl context to the canvas' context, and creates
 * it if it doesn't exist. Returns true if the context is valid,
 * false otherwise
 *******************************************************************/
bool GfxCanvas::setContext() {
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

BEGIN_EVENT_TABLE(GfxCanvas, wxGLCanvas)
	EVT_PAINT(GfxCanvas::paint)
END_EVENT_TABLE()

void GfxCanvas::paint(wxPaintEvent& e) {
	wxPaintDC(this);

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

	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SwapBuffers();
}


GfxEntryPanel::GfxEntryPanel(wxWindow* parent)
: EntryPanel(parent) {
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);
	
	gfx_canvas = new GfxCanvas(this, -1);
	vbox->Add(gfx_canvas, 1, wxEXPAND|wxALL, 4);

	Layout();
}

GfxEntryPanel::~GfxEntryPanel() {
}

bool GfxEntryPanel::loadEntry(ArchiveEntry* entry) {
	return true;
}
