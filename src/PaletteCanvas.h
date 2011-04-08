
#ifndef __PALETTECANVAS_H__
#define	__PALETTECANVAS_H__

#include "OGLCanvas.h"
#include "Palette.h"
#include <wx/colordlg.h>

class PaletteCanvas : public OGLCanvas {
private:
	int				sel_begin;
	int				sel_end;
	bool			double_width;

public:
	PaletteCanvas(wxWindow* parent, int id);
	~PaletteCanvas();

	Palette8bit&	getPalette() { return palette; }
	bool			doubleWidth() { return double_width; }
	int				getSelectionStart() { return sel_begin; }
	int				getSelectionEnd() { return sel_end; }

	void	doubleWidth(bool dw) { double_width = dw; }
	void	setSelection(int begin, int end = -1);

	void	draw();
	rgba_t	getSelectedColour();

	// Events
	void	onMouseLeftDown(wxMouseEvent& e);
	void	onMouseRightDown(wxMouseEvent& e);
	void	onMouseMotion(wxMouseEvent& e);
};

#endif //__PALETTECANVAS_H__
