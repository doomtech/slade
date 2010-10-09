
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ANSIEntryPanel.cpp
 * Description: ANSIEntryPanel class. Views ANSI entry data content
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
#include "CodePages.h"
#include "ANSIEntryPanel.h"

/*******************************************************************
 * ANSIENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* ANSIEntryPanel::ANSIEntryPanel
 * ANSIEntryPanel class constructor
 *******************************************************************/
ANSIEntryPanel::ANSIEntryPanel(wxWindow* parent) : EntryPanel(parent, "ansi") {

	// Create and setup ANSI text grid
	grid_ansi = new wxGrid(this, -1, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS|wxBORDER_SUNKEN);
	grid_ansi->SetDefaultRowSize(14, true);
	grid_ansi->SetDefaultColSize(5, true);
	grid_ansi->HideColLabels();
	grid_ansi->HideRowLabels();
	grid_ansi->CreateGrid(25, 80);
	grid_ansi->EnableEditing(false);
	grid_ansi->DisableDragGridSize();
	grid_ansi->EnableGridLines(false);
	grid_ansi->SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
	grid_ansi->SetInitialSize(wxSize(5*80+8, -1));
	sizer_main->Add(grid_ansi, 1, wxEXPAND|wxALL);

	// Disable save/revert buttons
	btn_save->Enable(false);
	btn_revert->Enable(false);

//	SetInitialSize(wxDefaultSize);
	Layout();
}

/* ANSIEntryPanel::~ANSIEntryPanel
 * ANSIEntryPanel class destructor
 *******************************************************************/
ANSIEntryPanel::~ANSIEntryPanel() {
}

/* ANSIEntryPanel::loadEntry
 * Loads an entry to the panel
 *******************************************************************/
bool ANSIEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Check entry exists
	if (!entry)
		return false;

	MemChunk& mc = entry->getMCData();
	if (mc.getSize() == 4000) {
		for (size_t i = 0; i < 2000; i++) {
			grid_ansi->SetCellValue(i/80, i%80, CP::fromCP437(mc[i<<1]));
			grid_ansi->SetCellTextColour(i/80, i%80, CP::ansiColor(mc[(i<<1)+1]&15));
			grid_ansi->SetCellBackgroundColour(i/80, i%80, CP::ansiColor(mc[(i<<1)+1]&112));
		}
		Layout();
		return true;
	}

	return false;
}

/* ANSIEntryPanel::saveEntry
 * Saves changes to the entry
 *******************************************************************/
bool ANSIEntryPanel::saveEntry() {
	return true;
}
