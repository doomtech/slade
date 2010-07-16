
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    PaletteChooser.cpp
 * Description: PaletteChooser class. A wxComboBox that contains a
 *              list of palettes available (from the PaletteManager)
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
#include "PaletteChooser.h"
#include "PaletteManager.h"
#include "Misc.h"


/*******************************************************************
 * PALETTECHOOSER CLASS FUNCTIONS
 *******************************************************************/

/* PaletteChooser::PaletteChooser
 * PaletteChooser class constructor
 *******************************************************************/
PaletteChooser::PaletteChooser(wxWindow* parent, int id)
: wxChoice(parent, id) { //, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY|wxCB_DROPDOWN) {
	// Init variables
	pal_global.copyPalette(thePaletteManager->globalPalette());

	// Add first 'existing' item
	Append("Existing/Global");

	// Add palette names from palette manager
	for (int a = 0; a < thePaletteManager->numPalettes(); a++)
		Append(thePaletteManager->getPalName(a));

	// Add greyscale palette
	Append("Greyscale");

	// Select first item
	SetSelection(0);
}

/* PaletteChooser::~PaletteChooser
 * PaletteChooser class destructor
 *******************************************************************/
PaletteChooser::~PaletteChooser() {
}

/* PaletteChooser::setGlobalFromArchive
 * Sets the chooser's 'global' palette to the palette contained in
 * [archive], or if it doesn't exist, the PaletteManager's global
 * palette
 *******************************************************************/
void PaletteChooser::setGlobalFromArchive(Archive* archive, int lump) {
	if (!Misc::loadPaletteFromArchive(&pal_global, archive, lump) &&
		!(archive->getParentArchive() && Misc::loadPaletteFromArchive(&pal_global, archive->getParentArchive(), lump)))
		pal_global.copyPalette(thePaletteManager->globalPalette());
}

/* PaletteChooser::getSelectedPalette
 * Returns the selected palette (from the PaletteManager)
 *******************************************************************/
Palette8bit* PaletteChooser::getSelectedPalette(ArchiveEntry* entry) {
	if (GetSelection() > 0)
		return thePaletteManager->getPalette(GetSelection() - 1);
	else if (entry)
		Misc::loadPaletteFromArchive(&pal_global, entry->getParent(), Misc::detectPaletteHack(entry));
	return &pal_global;
}

/* PaletteChooser::globalSelected
 * Returns true if the 'Archive/Global Palette' entry is selected
 *******************************************************************/
bool PaletteChooser::globalSelected() {
	return (GetSelection() == 0);
}
