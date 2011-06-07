
#ifndef __MAP_EDITOR_H__
#define __MAP_EDITOR_H__

#include "SLADEMap.h"

class MapEditor {
private:
	SLADEMap	map;
	
	// Editor state
	uint8_t		edit_mode;
	int			hilight_item;
	vector<int>	selection;

	// Mouse pointer state
	fpoint2_t	mouse_pos;
	fpoint2_t	mouse_downpos;
	
public:
	enum {
		// Editor modes
		MODE_VERTICES,
		MODE_LINES,
		MODE_SECTORS,
		MODE_THINGS,
		MODE_3D,
	};
	
	MapEditor();
	~MapEditor();

	fpoint2_t	mousePos() { return mouse_pos; }
	fpoint2_t	mouseDownPos() { return mouse_downpos; }

	void	setEditMode(int mode) { edit_mode = mode; }
	void	setMousePos(double x, double y) { mouse_pos.set(x, y); }
	void	setMouseDownPos(double x, double y) { mouse_downpos.set(x, y); }
	
	bool	openMap(Archive::mapdesc_t map);
	void	clearMap() { map.clearMap(); }
	
	// Drawing
	void	drawVertices(double xmin, double ymin, double xmax, double ymax);
	void	drawLines(double xmin, double ymin, double xmax, double ymax, bool show_direction = false);
	void	drawThings(double xmin, double ymin, double xmax, double ymax);
	void	drawMap(double xmin, double ymin, double xmax, double ymax);

	// Selection/hilight
	bool	updateHilight();
};

#endif//__MAP_EDITOR_H__
