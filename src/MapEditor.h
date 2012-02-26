
#ifndef __MAP_EDITOR_H__
#define __MAP_EDITOR_H__

#include "SLADEMap.h"
#include "GameConfiguration.h"

class MapCanvas;
class MapEditor {
private:
	SLADEMap			map;
	MapCanvas*			canvas;

	// Editor state
	uint8_t		edit_mode;
	int			hilight_item;
	vector<int>	selection;
	int			gridsize;

	// Tagged items
	vector<MapSector*>	tagged_sectors;
	vector<MapLine*>	tagged_lines;
	vector<MapThing*>	tagged_things;

	// Moving
	fpoint2_t	move_origin;
	fpoint2_t	move_vec;
	vector<int>	move_items;
	int			move_item_closest;

	// Editor messages
	struct editor_msg_t {
		string	message;
		long	act_time;
	};
	vector<editor_msg_t>	editor_messages;

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

	SLADEMap&			getMap() { return map; }
	uint8_t				editMode() { return edit_mode; }
	double				gridSize();
	unsigned			selectionSize() { return selection.size(); }
	vector<int>&		getSelection() { return selection; }
	int					hilightItem() { return hilight_item; }
	vector<MapSector*>&	taggedSectors() { return tagged_sectors; }
	vector<MapLine*>&	taggedLines() { return tagged_lines; }
	vector<MapThing*>&	taggedThings() { return tagged_things; }

	void	setEditMode(int mode);
	void	setCanvas(MapCanvas* canvas) { this->canvas = canvas; }

	// Map loading
	bool	openMap(Archive::mapdesc_t map);
	void	clearMap();

	// Selection/hilight
	void		clearHilight() { hilight_item = -1; }
	bool		updateHilight(fpoint2_t mouse_pos, double dist_scale = 1.0);
	void		selectionUpdated();
	void		clearSelection();
	void		selectAll();
	bool		selectCurrent(bool clear_none = true);
	bool		selectWithin(double xmin, double ymin, double xmax, double ymax, bool add = false);
	MapVertex*	getHilightedVertex();
	MapLine*	getHilightedLine();
	MapSector*	getHilightedSector();
	MapThing*	getHilightedThing();
	MapObject*	getHilightedObject();
	void		getSelectedVertices(vector<MapVertex*>& list);
	void		getSelectedLines(vector<MapLine*>& list);
	void		getSelectedSectors(vector<MapSector*>& list);
	void		getSelectedThings(vector<MapThing*>& list);
	void		getSelectedObjects(vector<MapObject*>& list);
	void		showItem(int index);

	// Grid
	void	incrementGrid();
	void	decrementGrid();
	double	snapToGrid(double position);

	// Item moving
	vector<int>&	movingItems() { return move_items; }
	fpoint2_t		moveVector() { return move_vec; }
	bool			beginMove(fpoint2_t mouse_pos);
	void			doMove(fpoint2_t mouse_pos);
	void			endMove();

	// Editing
	void	splitLine(double x, double y, double min_dist = 64);

	// Editor messages
	unsigned	numEditorMessages();
	string		getEditorMessage(int index);
	long		getEditorMessageTime(int index);
	void		addEditorMessage(string message);

	// Misc
	string	getModeString();
};

#endif//__MAP_EDITOR_H__
