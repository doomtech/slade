
#ifndef __MAP_EDITOR_H__
#define __MAP_EDITOR_H__

#include "SLADEMap.h"
#include "GameConfiguration.h"

struct selection_3d_t {
	int		index;
	uint8_t	type;

	selection_3d_t(int index = -1, uint8_t type = 0) {
		this->index = index;
		this->type = type;
	}
};

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
	bool		link_3d_light;
	bool		link_3d_offset;

	// Tagged items
	vector<MapSector*>	tagged_sectors;
	vector<MapLine*>	tagged_lines;
	vector<MapThing*>	tagged_things;

	// Moving
	fpoint2_t	move_origin;
	fpoint2_t	move_vec;
	vector<int>	move_items;
	int			move_item_closest;

	// Object properties
	MapThing*	copy_thing;
	MapSector*	copy_sector;

	// Drawing
	vector<fpoint2_t>	draw_points;
	fpoint2_t			draw_origin;

	// Editor messages
	struct editor_msg_t {
		string	message;
		long	act_time;
	};
	vector<editor_msg_t>	editor_messages;

	// 3d mode
	selection_3d_t			hilight_3d;
	vector<selection_3d_t>	selection_3d;

	// Helper for selectAdjacent3d
	bool wallMatches(MapSide* side, uint8_t part, string tex);
	void getAdjacentWalls3d(selection_3d_t item, vector<selection_3d_t>& list);

	// Helper for autoAlignX3d
	void doAlignX3d(MapSide* side, int offset, string tex, vector<selection_3d_t>& walls_done);

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

		// Selection
		DESELECT = 0,
		SELECT,
		TOGGLE,

		// 3d mode selection type
		SEL_SIDE_TOP = 0,
		SEL_SIDE_MIDDLE,
		SEL_SIDE_BOTTOM,
		SEL_FLOOR,
		SEL_CEILING,
		SEL_THING,
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

	vector<selection_3d_t>&	get3dSelection() { return selection_3d; }
	void					set3dHilight(selection_3d_t hl) { hilight_3d = hl; }
	selection_3d_t			hilightItem3d() { return hilight_3d; }

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
	void		selectItem3d(selection_3d_t item, int sel = TOGGLE);

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
	void	copyProperties();
	void	pasteProperties();
	void	splitLine(double x, double y, double min_dist = 64);
	void	flipLines(bool sides = true);
	void	changeSectorHeight(int amount, bool floor = true, bool ceiling = true);
	void	changeSectorLight(int amount);
	void	changeThingType(int newtype);
	void	thingQuickAngle(fpoint2_t mouse_pos);
	
	// Object creation/deletion
	void	createObject(double x, double y);
	void	createVertex(double x, double y);
	void	createThing(double x, double y);
	void	createSector(double x, double y);
	void	deleteObject();

	// Line drawing
	unsigned	nLineDrawPoints() { return draw_points.size(); }
	fpoint2_t	lineDrawPoint(unsigned index);
	bool		addLineDrawPoint(fpoint2_t point, bool nearest = false);
	void		removeLineDrawPoint();
	void		setShapeDrawOrigin(fpoint2_t point, bool nearest = false);
	void		updateShapeDraw(fpoint2_t point);
	void		endLineDraw(bool apply = true);

	// 3d mode
	void	selectAdjacent3d(selection_3d_t item);
	void	changeSectorLight3d(int amount);
	void	changeWallOffset3d(int amount, bool x);
	void	changeSectorHeight3d(int amount);
	void	autoAlignX3d(selection_3d_t start);
	void	resetWall3d();
	void	toggleUnpegged3d(bool lower);

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
