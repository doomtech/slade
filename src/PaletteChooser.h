
#ifndef __PALETTECHOOSER_H__
#define	__PALETTECHOOSER_H__

#include "Palette.h"
#include "Archive.h"

class PaletteChooser : public wxComboBox {
private:
	Palette8bit	pal_global;

public:
	PaletteChooser(wxWindow* parent, int id);
	~PaletteChooser();

	void			setGlobalFromArchive(Archive* archive);
	Palette8bit*	getSelectedPalette();
	bool			globalSelected();
};

#endif//__PALETTECHOOSER_H__
