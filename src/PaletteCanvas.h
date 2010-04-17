
#ifndef __PALETTECANVAS_H__
#define	__PALETTECANVAS_H__

#include "OGLCanvas.h"
#include "Palette.h"
#include <wx/colordlg.h>

class PaletteCanvas : public OGLCanvas {
private:
	Palette8bit*	palette;
	int				selected;

public:
	PaletteCanvas(wxWindow* parent, int id);
	~PaletteCanvas();

	Palette8bit& getPalette() { return *palette; }

	void	draw();
	rgba_t	getSelectedColour();

	// Events
	void	onMouseLeftDown(wxMouseEvent &e);
	void	onMouseRightDown(wxMouseEvent &e);
};

#endif //__PALETTECANVAS_H__
