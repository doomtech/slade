
#ifndef __MAPCANVAS_H__
#define __MAPCANVAS_H__

#include "OGLCanvas.h"

class MapEditor;
class MCAnimation;
class MapSide;
class MapCanvas : public OGLCanvas {
private:
	MapEditor*				editor;
	wxTimer					timer;
	wxStopWatch				stopwatch;
	vector<MCAnimation*>	animations;
	sf::Font				font_small;

	// Rendering
	bool	redraw;

	// View properties
	double		view_xoff;
	double		view_yoff;
	double		view_scale;
	fpoint2_t	view_tl;
	fpoint2_t	view_br;

	// Mass selection
	bool		sel_active;
	fpoint2_t	sel_origin;
	fpoint2_t	sel_end;

	// Animation
	float	anim_flash_level;
	bool	anim_flash_inc;

public:
	MapCanvas(wxWindow *parent, int id, MapEditor* editor);
	~MapCanvas();

	double	translateX(double x);
	double	translateY(double y);

	void	pan(double x, double y);
	void	zoom(double amount, bool toward_cursor = false);

	// Drawing
	void	drawGrid();
	void	draw();
	void	drawVertexOverlay();
	void	drawLineOverlay();
	void	drawSideOverlay(MapSide* side, string side_name = "Front", int xstart = 0);
	void	drawSideTexOverlay(int x, int y, string texture, string pos = "Upper");

	// Events
	void	onKeyDown(wxKeyEvent& e);
	void	onMouseDown(wxMouseEvent& e);
	void	onMouseUp(wxMouseEvent& e);
	void	onMouseMotion(wxMouseEvent& e);
	void	onMouseWheel(wxMouseEvent& e);
	void	onTimer(wxTimerEvent& e);
};

#endif //__MAPCANVAS_H__
