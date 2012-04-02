
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
	bool		hilight_locked;
	vector<int>	selection;
	int			gridsize;
	int			sector_mode;
	bool		grid_snap;

	// Tagged items
	vector<MapSector*>	tagged_sectors;
	vector<MapLine*>	tagged_lines;
	vector<MapThing*>	tagged_things;

	// Moving
	fpoint2_t	move_origin;
	fpoint2_t	move_vec;
	vector<int>	move_items;
	int			move_item_closest;

	// Drawing
	vector<fpoint2_t>	draw_points;

	// Editor messages
	struct editor_msg_t {
		string	message;
		long	act_time;
	};
	vector<editor_msg_t>	editor_messages;

public:
	enum {
		// Editor modes
		MODE_VERTICES = 0,
		MODE_LINES,
		MODE_SECTORS,
		MODE_THINGS,
		MODE_3D,

		// Sector edit modes (for shortcut keys, mostly)
		SECTOR_BOTH = 0,
		SECTOR_FLOOR,
		SECTOR_CEILING,
	};

	MapEditor();
	~MapEditor();

	SLADEMap&			getMap() { return map; }
	uint8_t				editMode() { return edit_mode; }
	int					sectorEditMode() { return sector_mode; }
	double				gridSize();
	unsigned			selectionSize() { return selection.size(); }
	vector<int>&		getSelection() { return selection; }
	int					hilightItem() { return hilight_item; }
	vector<MapSector*>&	taggedSectors() { return tagged_sectors; }
	vector<MapLine*>&	taggedLines() { return tagged_lines; }
	vector<MapThing*>&	taggedThings() { return tagged_things; }
	bool				hilightLocked() { return hilight_locked; }
	void				lockHilight(bool lock = true) { hilight_locked = lock; }
	bool				gridSnap() { return grid_snap; }

	void	setEditMode(int mode);
	void	setSectorEditMode(int mode);
	void	setCanvas(MapCanvas* canvas) { this->canvas = canvas; }

	// Map loading
	bool	openMap(Archive::mapdesc_t map);
	void	clearMap();

	// Selection/hilight
	void		clearHilight() { if (!hilight_locked) hilight_item = -1; }
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
	bool		isHilightOrSelection() { return selection.size() > 0 || hilight_item != -1; }

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
	void	changeSectorHeight(int amount, bool floor = true, bool ceiling = true);
	void	changeSectorLight(int amount);
	void	changeThingType(int newtype);
	void	thingQuickAngle(fpoint2_t mouse_pos);
	
	// Object creation/deletion
	void	createObject(double x, double y);
	void	createVertex(double x, double y);
	void	createThing(double x, double y);
	void	deleteObject();

	// Line drawing
	unsigned	nLineDrawPoints() { return draw_points.size(); }
	fpoint2_t	lineDrawPoint(unsigned index);
	bool		addLineDrawPoint(fpoint2_t point, bool nearest = false);
	void		removeLineDrawPoint();
	void		endLineDraw(bool apply = true);

	// Editor messages
	unsigned	numEditorMessages();
	string		getEditorMessage(int index);
	long		getEditorMessageTime(int index);
	void		addEditorMessage(string message);

	// Misc
	string	getModeString();
	bool	handleKeyBind(string key, fpoint2_t position);
	void	updateDisplay();
};

#endif//__MAP_EDITOR_H__
