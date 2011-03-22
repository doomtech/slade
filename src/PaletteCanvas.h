
#ifndef __PALETTECANVAS_H__
#define	__PALETTECANVAS_H__

#include "OGLCanvas.h"
#include "Palette.h"
#include <wx/colordlg.h>

class PaletteCanvas : public OGLCanvas {
private:
	Palette8bit*	palette;
	int				sel_begin;
	int				sel_end;

public:
	PaletteCanvas(wxWindow* parent, int id);
	~PaletteCanvas();

	Palette8bit& getPalette() { return *palette; }

	void	draw();
	rgba_t	getSelectedColour();
	void	setSelection(int begin, int end = -1);

	// Events
	void	onMouseLeftDown(wxMouseEvent& e);
	void	onMouseRightDown(wxMouseEvent& e);
	void	onMouseMotion(wxMouseEvent& e);
};

#endif //__PALETTECANVAS_H__
