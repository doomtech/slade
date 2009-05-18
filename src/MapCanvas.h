
#ifndef __MAPCANVAS_H__
#define __MAPCANVAS_H__

#include <wx/glcanvas.h>

class MapCanvas : public wxGLCanvas {
private:
	bool	init_done;

public:
	MapCanvas(wxWindow *parent, int id);
	~MapCanvas();

	void init();
	void draw();

	void paint(wxPaintEvent &event);
	void resize(wxSizeEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif //__MAPCANVAS_H__
