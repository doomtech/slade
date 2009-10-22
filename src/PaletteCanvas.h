
#ifndef __PALETTECANVAS_H__
#define	__PALETTECANVAS_H__

#include "OGLCanvas.h"
#include "Palette.h"

class PaletteCanvas : public OGLCanvas {
private:
	Palette8bit palette;

public:
	PaletteCanvas(wxWindow* parent, int id);
	~PaletteCanvas();

	Palette8bit& getPalette() { return palette; }

	bool	setContext();
	void	draw();
};

#endif //__PALETTECANVAS_H__
