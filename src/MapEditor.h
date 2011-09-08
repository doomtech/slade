
#ifndef __MAP_EDITOR_H__
#define __MAP_EDITOR_H__

#include "SLADEMap.h"
#include "GLTexture.h"

class MapEditor {
private:
	SLADEMap	map;

	// Editor state
	uint8_t		edit_mode;
	int			hilight_item;
	vector<int>	selection;
	int			gridsize;

	// Mouse pointer state
	fpoint2_t	mouse_pos;
	fpoint2_t	mouse_downpos;

	// Temporary
	GLTexture	tex_thing;

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

	uint8_t		editMode() { return edit_mode; }
	double		gridSize();
	fpoint2_t	mousePos() { return mouse_pos; }
	fpoint2_t	mouseDownPos() { return mouse_downpos; }
	unsigned	selectionSize() { return selection.size(); }
	int			hilightItem() { return hilight_item; }

	void	setEditMode(int mode);
	void	setMousePos(double x, double y) { mouse_pos.set(x, y); }
	void	setMouseDownPos(double x, double y) { mouse_downpos.set(x, y); }

	bool	openMap(Archive::mapdesc_t map);
	void	clearMap();

	// Drawing
	void	drawVertices(double xmin, double ymin, double xmax, double ymax);
	void	drawLines(double xmin, double ymin, double xmax, double ymax, bool show_direction = false);
	void	drawThings(double xmin, double ymin, double xmax, double ymax, double view_scale = 1.0);
	void	drawMap(double xmin, double ymin, double xmax, double ymax, double view_scale = 1.0);
	void	drawHilight(float flash_level);
	void	drawSelection(double xmin, double ymin, double xmax, double ymax);

	// Selection/hilight
	void		clearHilight() { hilight_item = -1; }
	bool		updateHilight(double dist_scale = 1.0);
	void		clearSelection() { selection.clear(); }
	void		selectAll();
	bool		selectCurrent(bool clear_none = true);
	bool		selectWithin(double xmin, double ymin, double xmax, double ymax);
	MapVertex*	getHilightedVertex();
	MapLine*	getHilightedLine();
	MapSector*	getHilightedSector();
	MapThing*	getHilightedThing();
	void		getSelectedVertices(vector<MapVertex*>& list);
	void		getSelectedLines(vector<MapLine*>& list);
	void		getSelectedSectors(vector<MapSector*>& list);
	void		getSelectedThings(vector<MapThing*>& list);

	// Grid
	void	incrementGrid();
	void	decrementGrid();
};

#endif//__MAP_EDITOR_H__
