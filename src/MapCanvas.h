
#ifndef __MAPCANVAS_H__
#define __MAPCANVAS_H__

#include <wx/glcanvas.h>
#include "SLADEMap.h"

class MapCanvas : public wxGLCanvas {
private:
	bool		init_done;
	SLADEMap*	map;

public:
	MapCanvas(wxWindow *parent, int id, SLADEMap* map);
	~MapCanvas();

	void init();
	void draw();
	bool setContext();

	void paint(wxPaintEvent &event);
	void resize(wxSizeEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif //__MAPCANVAS_H__
