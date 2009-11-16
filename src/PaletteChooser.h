
#ifndef __PALETTECHOOSER_H__
#define	__PALETTECHOOSER_H__

#include "Palette.h"

class PaletteChooser : public wxComboBox {
private:

public:
	PaletteChooser(wxWindow* parent, int id);
	~PaletteChooser();

	Palette8bit*	getSelectedPalette();
};

#endif//__PALETTECHOOSER_H__
