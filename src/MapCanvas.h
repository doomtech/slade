
#ifndef __MAPCANVAS_H__
#define __MAPCANVAS_H__

#include "OGLCanvas.h"
#include "VertexInfoOverlay.h"
#include "LineInfoOverlay.h"
#include "SectorInfoOverlay.h"

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

	// Info overlays
	int						last_hilight;
	VertexInfoOverlay		info_vertex;
	LineInfoOverlay			info_line;
	SectorInfoOverlay		info_sector;

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
	float	anim_info_fade;
	bool	anim_info_show;

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

	// Events
	void	onKeyDown(wxKeyEvent& e);
	void	onMouseDown(wxMouseEvent& e);
	void	onMouseUp(wxMouseEvent& e);
	void	onMouseMotion(wxMouseEvent& e);
	void	onMouseWheel(wxMouseEvent& e);
	void	onTimer(wxTimerEvent& e);
};

#endif //__MAPCANVAS_H__
