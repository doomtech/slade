
#ifndef __PALETTECANVAS_H__
#define	__PALETTECANVAS_H__

#include "OGLCanvas.h"
#include "Palette.h"

class PaletteCanvas : public OGLCanvas {
private:
	Palette8bit*	palette;
	int				selected;

public:
	PaletteCanvas(wxWindow* parent, int id);
	~PaletteCanvas();

	Palette8bit& getPalette() { return *palette; }

	void	draw();

	// Events
	void	leftClick(wxMouseEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__PALETTECANVAS_H__
