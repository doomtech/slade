
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    HexEntryPanel.cpp
 * Description: HexEntryPanel class. Views entry data content in a
 *              hex grid (read-only)
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
#include "HexEntryPanel.h"


/*******************************************************************
 * HEXENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* HexEntryPanel::HexEntryPanel
 * HexEntryPanel class constructor
 *******************************************************************/
HexEntryPanel::HexEntryPanel(wxWindow* parent) : EntryPanel(parent, "hex") {
	// Create hex editor
	hex_editor = new HexEditorPanel(this);
	sizer_main->Add(hex_editor, 1, wxEXPAND);

	// Disable save/revert buttons
	btn_save->Enable(false);
	btn_revert->Enable(false);

	Layout();
}

/* HexEntryPanel::~HexEntryPanel
 * HexEntryPanel class destructor
 *******************************************************************/
HexEntryPanel::~HexEntryPanel() {
}

/* HexEntryPanel::loadEntry
 * Loads an entry to the panel
 *******************************************************************/
bool HexEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Check entry exists
	if (!entry)
		return false;

	// Load entry data to hex editor
	return hex_editor->loadData(entry->getMCData());
}

/* HexEntryPanel::saveEntry
 * Saves changes to the entry
 *******************************************************************/
bool HexEntryPanel::saveEntry() {
	return true;
}
