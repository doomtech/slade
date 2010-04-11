
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
: EntryPanel(parent, _T("palette")) {
	// Add palette canvas
	pal_canvas = new PaletteCanvas(this, -1);
	sizer_main->Add(pal_canvas->toPanel(this), 1, wxEXPAND|wxALL, 4);

	// Disable default entry buttons
	btn_save->Enable(false);
	btn_revert->Enable(false);
	btn_edit_ext->Enable(false);

	// Add palette selection buttons
	btn_nextpal = new wxButton(this, -1, _T("Next >"));
	btn_prevpal = new wxButton(this, -1, _T("< Prev"));
	text_curpal = new wxStaticText(this, -1, _T("Palette XX/XX"));
	sizer_bottom->Add(btn_prevpal, 0, wxEXPAND|wxRIGHT|wxLEFT, 4);
	sizer_bottom->Add(btn_nextpal, 0, wxEXPAND|wxRIGHT, 4);
	sizer_bottom->Add(text_curpal, 0, wxALIGN_CENTER_VERTICAL, 4);

	// Bind events
	btn_nextpal->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PaletteEntryPanel::onBtnNextPal, this);
	btn_prevpal->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PaletteEntryPanel::onBtnPrevPal, this);

	// Init variables
	cur_palette = 0;

	Layout();
}

/* DefaultEntryPanel::~DefaultEntryPanel
 * DefaultEntryPanel class destructor
 *******************************************************************/
PaletteEntryPanel::~PaletteEntryPanel() {
	for (size_t a = 0; a < palettes.size(); a++)
		delete palettes[a];
}

/* PaletteEntryPanel::loadEntry
 * Reads all palettes in the PLAYPAL entry and shows the first one
 *******************************************************************/
bool PaletteEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Clear any existing palettes
	for (size_t a = 0; a < palettes.size(); a++)
		delete palettes[a];
	palettes.clear();

	// Determine how many palettes are in the entry
	int n_palettes = entry->getSize() / 768;

	// Load each palette
	entry->seek(0, SEEK_SET);
	uint8_t pal_data[768];
	for (int a = 0; a < n_palettes; a++) {
		// Read palette data
		entry->read(&pal_data, 768);

		// Create palette
		Palette8bit* pal = new Palette8bit();
		pal->loadMem(pal_data, 768);

		// Add palette
		palettes.push_back(pal);
	}

	// Show first palette
	cur_palette = 0;
	showPalette(0);

	return true;
}

/* PaletteEntryPanel::showPalette
 * Shows the palette at [index]. Returns false if [index] is out of
 * bounds, true otherwise
 *******************************************************************/
bool PaletteEntryPanel::showPalette(uint32_t index) {
	// Check index range
	if (index >= palettes.size())
		return false;

	// Copy palette at index into canvas
	pal_canvas->getPalette().copyPalette(palettes[index]);

	// Set current palette text
	text_curpal->SetLabel(s_fmt(_T("Palette %d/%d"), index+1, palettes.size()));

	// Refresh
	Layout();
	pal_canvas->Refresh();

	return true;
}




void PaletteEntryPanel::onBtnNextPal(wxCommandEvent& e) {
	if (showPalette(cur_palette + 1))
		cur_palette++;
}

void PaletteEntryPanel::onBtnPrevPal(wxCommandEvent& e) {
	if (showPalette(cur_palette - 1))
		cur_palette--;
}
