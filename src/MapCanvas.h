
#ifndef __MAPCANVAS_H__
#define __MAPCANVAS_H__

#include "OGLCanvas.h"
#include "VertexInfoOverlay.h"
#include "LineInfoOverlay.h"
#include "SectorInfoOverlay.h"
#include "ThingInfoOverlay.h"
#include "KeyBind.h"
#include "MainApp.h"
#include <SFML/System.hpp>

class MapEditor;
class MCAnimation;
class MapSide;
class ThingType;
class GLTexture;
class MapCanvas : public OGLCanvas, public KeyBindHandler, public SActionHandler {
private:
	MapEditor*				editor;
	vector<MCAnimation*>	animations;
	int						frametime_last;
	vector<int>				fps_avg;
	int						fr_idle;
	sf::Clock				sfclock;

	// Mouse stuff
	enum {
		MSTATE_NORMAL,
		MSTATE_SELECTION,
		MSTATE_PANNING,
		MSTATE_MOVE,
	};
	point2_t	mouse_pos;			// 'Raw' mouse position
	point2_t	mouse_downpos;
	fpoint2_t	mouse_pos_m;		// 'Map' mouse position (translated)
	fpoint2_t	mouse_downpos_m;
	uint8_t		mouse_state;

	// Info overlays
	int					last_hilight;
	VertexInfoOverlay	info_vertex;
	LineInfoOverlay		info_line;
	SectorInfoOverlay	info_sector;
	ThingInfoOverlay	info_thing;

	// View properties
	double		view_xoff;
	double		view_yoff;
	double		view_scale;
	fpoint2_t	view_tl;
	fpoint2_t	view_br;

	// Visibility
	enum {
		VIS_LEFT	= 1,
		VIS_RIGHT	= 2,
		VIS_ABOVE	= 4,
		VIS_BELOW	= 8,
	};
	vector<uint8_t>	vis_v;
	vector<uint8_t>	vis_l;
	vector<uint8_t>	vis_t;

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
	void	drawVertices();
	void	drawLines(bool show_direction = false);
	void	drawRoundThing(double x, double y, double angle, ThingType* type);
	bool	drawSpriteThing(double x, double y, double angle, ThingType* type);
	void	drawSquareThing(double x, double y, double angle, ThingType* type);
	void	drawThings();
	void	drawHilight();
	void	drawSelection();
	void	draw();
	void	update(long frametime);
	void	updateVisibility();

	// Keybind handling
	void	onKeyBindPress(string name);
	void	onKeyBindRelease(string name);

	// SAction handler
	bool	handleAction(string id);

	// Events
	void	onSize(wxSizeEvent& e);
	void	onKeyDown(wxKeyEvent& e);
	void	onKeyUp(wxKeyEvent& e);
	void	onMouseDown(wxMouseEvent& e);
	void	onMouseUp(wxMouseEvent& e);
	void	onMouseMotion(wxMouseEvent& e);
	void	onMouseWheel(wxMouseEvent& e);
	void	onMouseLeave(wxMouseEvent& e);
	void	onMouseEnter(wxMouseEvent& e);
	void	onIdle(wxIdleEvent& e);
	void	onTimer(wxTimerEvent& e);
};

#endif //__MAPCANVAS_H__
