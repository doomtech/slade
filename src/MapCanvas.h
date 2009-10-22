
#ifndef __MAPCANVAS_H__
#define __MAPCANVAS_H__

#include "OGLCanvas.h"
#include "SLADEMap.h"

class MapCanvas : public OGLCanvas {
private:
	SLADEMap*	map;

public:
	MapCanvas(wxWindow *parent, int id, SLADEMap* map);
	~MapCanvas();

	void	draw();
};

#endif //__MAPCANVAS_H__
