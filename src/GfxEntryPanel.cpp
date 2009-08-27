
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
#include "Palette.h"


/* GfxEntryPanel::GfxEntryPanel
 * GfxEntryPanel class constructor
 *******************************************************************/
GfxEntryPanel::GfxEntryPanel(wxWindow* parent)
: EntryPanel(parent) {
	// Get the sizer
	wxSizer* sizer = GetSizer();

	// Add gfx canvas
	gfx_canvas = new GfxCanvas(this, -1);
	sizer->Add(gfx_canvas, 1, wxEXPAND|wxALL, 4);

	Layout();
}

/* GfxEntryPanel::~GfxEntryPanel
 * GfxEntryPanel class destructor
 *******************************************************************/
GfxEntryPanel::~GfxEntryPanel() {
}

/* GfxEntryPanel::loadEntry
 * Loads an entry into the entry panel if it is a valid image format
 *******************************************************************/
bool GfxEntryPanel::loadEntry(ArchiveEntry* entry) {
	if (entry->getParent())
		gfx_canvas->getImage()->getPalette().loadFromArchive(entry->getParent());

	if (entry->getType() == ETYPE_PNG)
		gfx_canvas->getImage()->loadImage(entry->getData(true), entry->getSize());
	else if (entry->getType() == ETYPE_SPRITE || // TODO: Really should change this stuff (merge all these types into a single 'ETYPE_DOOMGFX' type, or something)
			entry->getType() == ETYPE_PATCH ||
			entry->getType() == ETYPE_GFX)
		gfx_canvas->getImage()->loadDoomGfx(entry->getData(true), entry->getSize());
	else if (entry->getType() == ETYPE_FLAT)
		gfx_canvas->getImage()->loadDoomFlat(entry->getData(true), entry->getSize());

	gfx_canvas->Refresh();

	return true;
}
