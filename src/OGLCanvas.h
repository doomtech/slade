
#ifndef __OGLCANVAS_H__
#define	__OGLCANVAS_H__

#include "OpenGL.h"
#include "Palette.h"

class OGLCanvas : public wxGLCanvas {
protected:
	bool		init_done;
	Palette8bit	palette;

public:
	OGLCanvas(wxWindow* parent, int id);
	~OGLCanvas();

	Palette8bit*	getPalette() { return &palette; }
	void			setPalette(Palette8bit* pal) { palette.copyPalette(pal); }
	bool			setContext();
	void			init();
	virtual void	draw() = 0;

	void	onPaint(wxPaintEvent &e);
	void	onEraseBackground(wxEraseEvent &e);
};

#endif //__OGLCANVAS_H__
