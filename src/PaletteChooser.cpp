
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



/* PaletteChooser::PaletteChooser
 * PaletteChooser class constructor
 *******************************************************************/
PaletteChooser::PaletteChooser(wxWindow* parent, int id)
: wxComboBox(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY|wxCB_DROPDOWN) {
	// Add first 'existing' item
	Append(_T("Existing/Global"));

	// Add palette names from palette manager
	for (int a = 0; a < thePaletteManager->numPalettes(); a++)
		Append(thePaletteManager->getPalName(a));

	// Add greyscale palette
	Append(_T("Greyscale"));

	// Select first item
	SetSelection(0);
}

/* PaletteChooser::~PaletteChooser
 * PaletteChooser class destructor
 *******************************************************************/
PaletteChooser::~PaletteChooser() {
}

/* PaletteChooser::getSelectedPalette
 * Returns the selected palette (from the PaletteManager)
 *******************************************************************/
Palette8bit* PaletteChooser::getSelectedPalette() {
	if (GetSelection() > 0)
		return thePaletteManager->getPalette(GetSelection() - 1);
	else
		return thePaletteManager->defaultPalette();
}

/* PaletteChooser::globalSelected
 * Returns true if the 'Archive/Global Palette' entry is selected
 *******************************************************************/
bool PaletteChooser::globalSelected() {
	return (GetSelection() == 0);
}
