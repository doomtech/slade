
#ifndef __MAPCANVAS_H__
#define __MAPCANVAS_H__

#include <wx/glcanvas.h>

class MapCanvas : public wxGLCanvas {
private:

public:
	MapCanvas(wxWindow *parent, int id);
	~MapCanvas();
};

#endif //__MAPCANVAS_H__
