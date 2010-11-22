
#ifndef __MAPCANVAS_H__
#define __MAPCANVAS_H__

#include "OGLCanvas.h"
#include "SLADEMap.h"

class MapCanvas : public OGLCanvas {
private:
	SLADEMap*	map;

	// View properties
	double	view_xoff;
	double	view_yoff;
	double	view_scale;

	// Mouse position info
	fpoint2_t	mouse;
	fpoint2_t	mouse_down;

public:
	MapCanvas(wxWindow *parent, int id, SLADEMap* map);
	~MapCanvas();

	double	translateX(double x);
	double	translateY(double y);

	void	pan(double x, double y);
	void	zoom(double amount);

	void	drawGrid();
	void	draw();

	// Events
	void	onKeyDown(wxKeyEvent& e);
	void	onMouseDown(wxMouseEvent& e);
	void	onMouseUp(wxMouseEvent& e);
	void	onMouseMotion(wxMouseEvent& e);
	void	onMouseWheel(wxMouseEvent& e);
};

#endif //__MAPCANVAS_H__
