
#ifndef __PALETTEDIALOG_H__
#define	__PALETTEDIALOG_H__

#include "PaletteCanvas.h"

class PaletteDialog : public wxDialog {
private:
	PaletteCanvas*	pal_canvas;

public:
	PaletteDialog(Palette8bit* palette);
	~PaletteDialog();

	rgba_t	getSelectedColour();
};

#endif//__PALETTEDIALOG_H__
