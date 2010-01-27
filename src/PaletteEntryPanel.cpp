
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    PaletteEntryPanel.cpp
 * Description: PaletteEntryPanel class. The UI for editing
 *              palette (PLAYPAL) entries
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
#include "PaletteEntryPanel.h"


/*******************************************************************
 * PALETTEENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* PaletteEntryPanel::PaletteEntryPanel
 * PaletteEntryPanel class constructor
 *******************************************************************/
PaletteEntryPanel::PaletteEntryPanel(wxWindow* parent)
: EntryPanel(parent) {
	// Get the sizer
	//wxSizer* sizer = GetSizer();

	// Add palette canvas
	pal_canvas = new PaletteCanvas(this, -1);
	sizer_main->Add(pal_canvas, 1, wxEXPAND|wxALL, 4);

	Layout();
}

/* DefaultEntryPanel::~DefaultEntryPanel
 * DefaultEntryPanel class destructor
 *******************************************************************/
PaletteEntryPanel::~PaletteEntryPanel() {
}

/* PaletteEntryPanel::loadEntry
 * Loads an entry into the entry panel if it is a valid image format
 *******************************************************************/
bool PaletteEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Copy current entry content
	entry_data.clear();
	entry_data.importMem(entry->getData(true), entry->getSize());

	MemChunk mc;
	mc.importMem(entry->getData(true), entry->getSize());
	pal_canvas->getPalette().loadMem(mc);

	return true;
}
