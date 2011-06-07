
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
#include "PaletteManager.h"
#include "Icons.h"
#include "SFileDialog.h"


/*******************************************************************
 * PALETTEENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* PaletteEntryPanel::PaletteEntryPanel
 * PaletteEntryPanel class constructor
 *******************************************************************/
PaletteEntryPanel::PaletteEntryPanel(wxWindow* parent)
: EntryPanel(parent, "palette") {
	// Add palette canvas
	pal_canvas = new PaletteCanvas(this, -1);
	pal_canvas->allowSelection(1);
	sizer_main->Add(pal_canvas->toPanel(this), 1, wxEXPAND, 0);

	// Disable default entry buttons
	btn_save->Enable(false);
	btn_revert->Enable(false);
	btn_edit_ext->Enable(false);

	// Add palette selection buttons
	btn_nextpal = new wxBitmapButton(this, -1, getIcon("t_right"));
	btn_prevpal = new wxBitmapButton(this, -1, getIcon("t_left"));
	text_curpal = new wxStaticText(this, -1, "Palette XX/XX");
	sizer_bottom->Add(btn_prevpal, 0, wxEXPAND|wxRIGHT, 4);
	sizer_bottom->Add(btn_nextpal, 0, wxEXPAND|wxRIGHT, 4);
	sizer_bottom->Add(text_curpal, 0, wxALIGN_CENTER_VERTICAL, 4);

	// Setup custom menu
	menu_custom = new wxMenu();
	theApp->getAction("ppal_addcustom")->addToMenu(menu_custom);
	theApp->getAction("ppal_exportas")->addToMenu(menu_custom);
	custom_menu_name = "Palette";

	// Bind events
	btn_nextpal->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PaletteEntryPanel::onBtnNextPal, this);
	btn_prevpal->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PaletteEntryPanel::onBtnPrevPal, this);
	pal_canvas->Bind(wxEVT_LEFT_DOWN, &PaletteEntryPanel::onPalCanvasMouseEvent, this);

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

/* GfxEntryPanel::statusString
 * Returns a string with extended editing/entry info for the status
 * bar
 *******************************************************************/
string PaletteEntryPanel::statusString() {
	// Get current colour
	rgba_t col = pal_canvas->getSelectedColour();

	return S_FMT("R %d, G %d, B %d", col.r, col.g, col.b);
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
	text_curpal->SetLabel(S_FMT("Palette %d/%d", index+1, palettes.size()));

	// Refresh
	Layout();
	pal_canvas->Refresh();

	return true;
}

/* PaletteEntryPanel::addCustomPalette
 * Adds the current palette to the custom user palettes folder, so
 * it can be selected via the palette selector
 *******************************************************************/
bool PaletteEntryPanel::addCustomPalette() {
	// Get name to export as
	string name = wxGetTextFromUser("Enter name for Palette:", "Add to Palettes");
	if (name.IsEmpty())
		return false;

	// Write current palette to the user palettes directory
	string path = appPath(S_FMT("palettes/%s.pal", CHR(name)), DIR_USER);
	palettes[cur_palette]->saveFile(path);

	// Add to palette manager
	Palette8bit* pal = new Palette8bit();
	pal->copyPalette(palettes[cur_palette]);
	thePaletteManager->addPalette(pal, name);

	return true;
}

/* PaletteEntryPanel::exportAs
 * Exports the current palette to a file, in the selected format
 *******************************************************************/
bool PaletteEntryPanel::exportAs() {
	// Run save file dialog
	SFileDialog::fd_info_t info;
	string extensions = "Raw Palette (*.pal)|*.pal|PNG File (*.png)|*.png|CSV Palette (*.csv)|*.csv|JASC Palette (*.pal)|*.pal";
	if (SFileDialog::saveFile(info, "Export Palette As", extensions, this))
		return palettes[cur_palette]->saveFile(info.filenames[0], info.ext_index);

	return false;
}

/* PaletteEntryPanel::handleAction
 * Handles the action [id]. Returns true if the action was handled,
 * false otherwise
 *******************************************************************/
bool PaletteEntryPanel::handleAction(string id) {
	// Ignore if hidden
	if (!IsShown())
		return false;

	// Only interested in "ppal_" events
	if (!id.StartsWith("ppal_"))
		return false;

	// Add to custom palettes
	if (id == "ppal_addcustom") {
		addCustomPalette();
		return true;
	}

	// Export As
	else if (id == "ppal_exportas") {
		exportAs();
		return true;
	}

	return false;
}


/*******************************************************************
 * PALETTEENTRYPANEL CLASS EVENTS
 *******************************************************************/

/* PaletteEntryPanel::onBtnNextPal
 * Called when the 'next palette' button is clicked
 *******************************************************************/
void PaletteEntryPanel::onBtnNextPal(wxCommandEvent& e) {
	if (showPalette(cur_palette + 1))
		cur_palette++;
}

/* PaletteEntryPanel::onBtnPrevPal
 * Called when the 'previous palette' button is clicked
 *******************************************************************/
void PaletteEntryPanel::onBtnPrevPal(wxCommandEvent& e) {
	if (showPalette(cur_palette - 1))
		cur_palette--;
}

/* PaletteEntryPanel::onPalCanvasMouseEvent
 * Called when a mouse event happens within the palette canvas (eg.
 * button clicked, pointer moved, etc)
 *******************************************************************/
void PaletteEntryPanel::onPalCanvasMouseEvent(wxMouseEvent& e) {
	// Update colour info label with selected colour (if any)
	if (e.LeftDown()) {
		// Send to palette canvas
		pal_canvas->onMouseLeftDown(e);

		// Update status bar
		updateStatus();
	}
}
