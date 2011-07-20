
#ifndef __MAPCANVAS_H__
#define __MAPCANVAS_H__

#include "OGLCanvas.h"
#include "VertexInfoOverlay.h"
#include "LineInfoOverlay.h"
#include "SectorInfoOverlay.h"
#include "KeyBind.h"

class MapEditor;
class MCAnimation;
class MapSide;
class MapCanvas : public OGLCanvas, public KeyBindHandler {
private:
	MapEditor*				editor;
	vector<MCAnimation*>	animations;
	point2_t				mouse_relpos;

	// Info overlays
	int						last_hilight;
	VertexInfoOverlay		info_vertex;
	LineInfoOverlay			info_line;
	SectorInfoOverlay		info_sector;

	// View properties
	double		view_xoff;
	double		view_yoff;
	double		view_scale;
	fpoint2_t	view_tl;
	fpoint2_t	view_br;

	// Panning
	fpoint2_t	pan_origin;
	bool		panning;

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

	void	setView(double x, double y);
	void	pan(double x, double y);
	void	zoom(double amount, bool toward_cursor = false);

	// Drawing
	void	drawGrid();
	void	draw();
	void	update(long frametime);

	// Keybind handling
	void onKeyBindPress(string name);
	void onKeyBindRelease(string name);

	// Events
	void	onSize(wxSizeEvent& e);
	void	onKeyDown(wxKeyEvent& e);
	void	onKeyUp(wxKeyEvent& e);
	void	onMouseDown(wxMouseEvent& e);
	void	onMouseUp(wxMouseEvent& e);
	void	onMouseMotion(wxMouseEvent& e);
	void	onMouseWheel(wxMouseEvent& e);
	void	onTimer(wxTimerEvent& e);
};

#endif //__MAPCANVAS_H__
