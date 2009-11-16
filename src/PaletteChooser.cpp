

#include "Main.h"
#include "WxStuff.h"
#include "PaletteChooser.h"
#include "PaletteManager.h"

PaletteChooser::PaletteChooser(wxWindow* parent, int id)
: wxComboBox(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY|wxCB_DROPDOWN) {
	// Add first 'existing' item
	Append(_T("Existing Palette"));

	// Add palette names from palette manager
	for (int a = 0; a < thePaletteManager->numPalettes(); a++)
		Append(thePaletteManager->getPalName(a));

	// Select first item
	SetSelection(0);
}

PaletteChooser::~PaletteChooser() {
}

Palette8bit* PaletteChooser::getSelectedPalette() {
	if (GetSelection() > 0)
		return thePaletteManager->getPalette(GetSelection() - 1);
	else
		return thePaletteManager->defaultPalette();
}
