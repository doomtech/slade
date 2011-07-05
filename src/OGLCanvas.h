
#ifndef __OGLCANVAS_H__
#define	__OGLCANVAS_H__

#include "OpenGL.h"
#include "Palette.h"
#include <wx/control.h>
#include <wx/stopwatch.h>
#undef None
#include <SFML/Graphics.hpp>

class wxPanel;
class OGLCanvas : public wxControl, sf::RenderWindow {
protected:
	bool		init_done;
	Palette8bit	palette;
	wxStopWatch	timer;

public:
	OGLCanvas(wxWindow* parent, int id);
	~OGLCanvas();

	Palette8bit*	getPalette() { return &palette; }
	void			setPalette(Palette8bit* pal) { palette.copyPalette(pal); }
	bool			setContext();
	void			init();
	virtual void	draw() = 0;
	void			drawCheckeredBackground();
	wxPanel*		toPanel(wxWindow* parent);

	void	SwapBuffers() { Display(); }

	void	onPaint(wxPaintEvent& e);
	void	onEraseBackground(wxEraseEvent& e);
	void	onIdle(wxIdleEvent& e);
};

#endif //__OGLCANVAS_H__
