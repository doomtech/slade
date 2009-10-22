
#ifndef __OGLCANVAS_H__
#define	__OGLCANVAS_H__

#include <wx/glcanvas.h>

class OGLCanvas : public wxGLCanvas {
protected:
	wxGLContext*	context;
	bool			init_done;

public:
	OGLCanvas(wxWindow* parent, int id);
	~OGLCanvas();

	bool			setContext();
	void			init();
	virtual void	draw() = 0;

	void	paint(wxPaintEvent &e);
	void	onEraseBackground(wxEraseEvent &e);

	DECLARE_EVENT_TABLE()
};

#endif //__OGLCANVAS_H__
