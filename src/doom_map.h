
#ifndef __DM_MAP_H__
#define __DM_MAP_H__

#include "structs.h"
#include "wad.h"

/*
#include "dm_vertex.h"
#include "dm_line.h"
#include "dm_side.h"
#include "dm_sector.h"
#include "dm_thing.h"
*/
class Vertex;
class Line;
class Side;
class Sector;
class Thing;

#define MAP_OPEN	0x01
#define MAP_HEXEN	0x02

#define MISTYLE_NORMAL		0
#define MISTYLE_HILIGHTED	1
#define MISTYLE_SELECTED	2
#define MISTYLE_MOVING		3
#define MISTYLE_TAGGED		4

#define MTYPE_VERTEX		0x01
#define MTYPE_LINE			0x02
#define MTYPE_SECTOR		0x04
#define MTYPE_THING			0x08
#define MTYPE_SIDE			0x10

#define MC_SAVE_NEEDED	0x01
#define MC_NODE_REBUILD	0x02
#define MC_LINES		0x04
#define MC_SSECTS		0x08
#define MC_SECTORS		0x10
#define MC_THINGS		0x20

class DoomMap
{
private:
	vector<Vertex*>	vertices;
	vector<Sector*>	sectors;
	vector<Line*>	lines;
	vector<Side*>	sides;
	vector<Thing*>	things;

	Vertex*	def_vertex;
	Sector*	def_sector;
	Line*	def_line;
	Side*	def_side;
	Thing*	def_thing;

	string	name;
	BYTE	flags;

	Lump	*scripts, *behavior;

	vector<int>	selected_items;
	vector<int>	tagged_items;
	vector<int>	moving_items;
	int			hilight_item;

	BYTE	changed;

public:
	DoomMap();
	~DoomMap();

	Vertex* vertex(int index);
	Line*	line(int index);
	Side*	side(int index);
	Sector*	sector(int index);
	Thing*	thing(int index);

	unsigned int	n_verts();
	unsigned int	n_lines();
	unsigned int	n_sides();
	unsigned int	n_sectors();
	unsigned int	n_things();

	string	map_name() { return name; }

	void	add_vertex(Vertex* vertex);
	void	add_line(Line* line);
	void	add_side(Side* side);
	void	add_sector(Sector* sector);
	void	add_thing(Thing* thing);

	void	delete_selection(int type);
	void	delete_vertex(Vertex* vertex);
	void	delete_vertex(int index);
	void	delete_line(Line* line);
	void	delete_line(int index, Vertex *vertex = NULL);
	void	delete_side(Side* side);
	void	delete_side(int index);
	void	delete_sector(Sector* sector, bool check = true);
	void	delete_sector(int index, bool check = true);
	void	delete_thing(Thing* thing);
	void	delete_thing(int index);

	int		index(Vertex* i, bool force_search = false);
	int		index(Line* i, bool force_search = false);
	int		index(Side* i, bool force_search = false);
	int		index(Sector* i, bool force_search = false);
	int		index(Thing* i, bool force_search = false);
	void	update_indices(BYTE types =  255);

	bool	valid(Vertex* i, bool fullcheck = false);
	bool	valid(Line* i, bool fullcheck = false);
	bool	valid(Side* i, bool fullcheck = false);
	bool	valid(Sector* i, bool fullcheck = false);
	bool	valid(Thing* i, bool fullcheck = false);

	bool	hexen() { return !!(flags & MAP_HEXEN); }
	bool	opened() { return !!(flags & MAP_OPEN); }

	vector<int>&	selection() { return selected_items; }
	int				hilight()	{ return hilight_item; }

	Vertex*	hilight_vertex();
	Line*	hilight_line();
	Sector*	hilight_sector();
	Thing*	hilight_thing();

	void	get_selection(vector<Vertex*> &list, bool hilight = false);
	void	get_selection(vector<Line*> &list, bool hilight = false);
	void	get_selection(vector<Sector*> &list, bool hilight = false);
	void	get_selection(vector<Thing*> &list, bool hilight = false);

	void	set_hexen(bool b);
	void	set_name(string name);

	void	create(string name);
	bool	open(Wad* wad, string mapname);
	void	close();

	void	select_deselect();
	void	select_item(int index);
	void	clear_selection();
	void	update_tagged(int type);

	void	draw(rect_t vis_area, BYTE type = 1);
	void	draw_lines(rect_t vis_area, bool side = false);

	int		get_hilight_line(point2_t mouse, int m_d = 64);
	void	select_lines_box(rect_t box);

	int		get_hilight_vertex(point2_t mouse);
	void	select_vertices_box(rect_t box);

	int		get_hilight_sector(point2_t mouse);
	void	select_sectors_box(rect_t box);

	int		get_hilight_thing(point2_t mouse);
	void	select_things_box(rect_t box);

	bool	add_to_wad(Wad *wad);

	rect_t		bounds();
	point2_t	middle();

	void change_level(BYTE flags);
	bool check_changed(BYTE flag) { return changed & flag; }
	void clear_change(BYTE flag) { changed = (changed & ~flag); }

	void move_items(point2_t mouse);
	void clear_move_items();

	void get_lines_to_vert(int vertex, vector<int> &list, bool allowdup = true);
	void update_vertex_refs();

	// Checks
	int		get_free_tag();
	int		remove_unused_vertices();
	int		remove_unused_sectors();
	int		check_vertex_spot(point2_t pos);
	int		remove_zerolength_lines();
	void	remove_overlapping_lines(vector<Line*> &list, bool merge = false);
	void	check_split(Vertex* vert);
	void	check_split(Vertex* vert, vector<Line*> &list);
	void	merge_vertices(Vertex* v1, Vertex* v2);
	void	merge_under_vertex(Vertex* v);
	void	merge_like_sectors();
	void	merge_vertices_spot(point2_t pos);
};

#endif
