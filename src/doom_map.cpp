
#include "main.h"
#include "doom_map.h"
#include "splash.h"
#include "byteswap.h"
#include "mathstuff.h"
#include "edit.h"
#include "edit_misc.h"
#include "3dmode.h"
#include "editor_window.h"

#include "dm_vertex.h"
#include "dm_line.h"
#include "dm_side.h"
#include "dm_sector.h"
#include "dm_thing.h"

DoomMap d_map;

CVAR(Float, edit_split_dist, 2, CVAR_SAVE)

extern int edit_mode;
extern EditorWindow* editor_window;

EXTERN_CVAR(Bool, edit_auto_merge)
EXTERN_CVAR(Bool, edit_auto_split)

string map_lumps[12] =
{
	_T("THINGS"),
	_T("VERTEXES"),
	_T("LINEDEFS"),
	_T("SIDEDEFS"),
	_T("SECTORS"),
	_T("SEGS"),
	_T("SSECTORS"),
	_T("NODES"),
	_T("BLOCKMAP"),
	_T("REJECT"),
	_T("SCRIPTS"),
	_T("BEHAVIOR")
};

DoomMap::DoomMap()
{
	def_vertex = new Vertex();
	def_vertex->set_parent(this);

	def_line = new Line();
	def_line->set_parent(this);

	def_sector = new Sector();
	def_sector->set_parent(this);

	def_side = new Side();
	def_side->set_parent(this);

	def_thing = new Thing();
	def_thing->set_parent(this);

	def_side->set_sector(def_sector);
	def_line->set_side1(def_side);
	def_line->set_side2(def_side);
}

DoomMap::~DoomMap()
{
}

void DoomMap::create(string name)
{
	this->name = name;
	this->flags |= MAP_OPEN;

	if (scripts)
		delete scripts;

	if (behavior)
		delete behavior;

	scripts = new Lump(0, 0, _T("SCRIPTS"));
	behavior = new Lump(0, 0, _T("BEHAVIOR"));
	changed = 255;
}

void DoomMap::close()
{
	for (int a = 0; a < vertices.size(); a++)
		delete vertices[a];
	vertices.clear();

	for (int a = 0; a < lines.size(); a++)
		delete lines[a];
	lines.clear();

	for (int a = 0; a < sides.size(); a++)
		delete sides[a];
	sides.clear();

	for (int a = 0; a < sectors.size(); a++)
		delete sectors[a];
	sectors.clear();

	for (int a = 0; a < things.size(); a++)
		delete things[a];
	things.clear();

	flags = (flags & ~MAP_OPEN);
	changed = 0;
}

void DoomMap::set_hexen(bool b)
{
	if (b)
		flags |= MAP_HEXEN;
	else
		flags = (flags & ~MAP_HEXEN);
}

void DoomMap::set_name(string name)
{
	this->name = name;
}

Vertex* DoomMap::vertex(int index)
{
	if (index < 0 || index >= vertices.size())
	{
		log_message(s_fmt(_T("Vertex %d doesn't exist"), index));
		return def_vertex;
	}
	else
		return vertices[index];
}

Line* DoomMap::line(int index)
{
	if (index < 0 || index >= lines.size())
	{
		if (index != 65535)
			log_message(s_fmt(_T("Line %d doesn't exist"), index));
		return def_line;
	}
	else
		return lines[index];
}

Side* DoomMap::side(int index)
{
	if (index < 0 || index >= sides.size())
	{
		if (index != -1 && index != 65535)
			log_message(s_fmt(_T("Side %d doesn't exist"), index));

		return def_side;
	}
	else
		return sides[index];
}

Sector* DoomMap::sector(int index)
{
	if (index < 0 || index >= sectors.size())
	{
		if (index != -1)
			log_message(s_fmt(_T("Sector %d doesn't exist"), index));
		return def_sector;
	}
	else
		return sectors[index];
}

Thing* DoomMap::thing(int index)
{
	if (index < 0 || index >= things.size())
	{
		log_message(s_fmt(_T("Thing %d doesn't exist"), index));
		return def_thing;
	}
	else
		return things[index];
}

int	DoomMap::n_verts()
{
	return vertices.size();
}

int	DoomMap::n_lines()
{
	return lines.size();
}

int	DoomMap::n_sides()
{
	return sides.size();
}

int	DoomMap::n_sectors()
{
	return sectors.size();
}

int	DoomMap::n_things()
{
	return things.size();
}

void DoomMap::add_vertex(Vertex* vertex)
{
	vertices.push_back(vertex);
	vertex->set_parent(this);
	vertex->set_index(vertices.size() - 1);
	change_level(MC_SAVE_NEEDED);
}

void DoomMap::add_line(Line *line)
{
	lines.push_back(line);
	line->set_parent(this);
	line->set_index(lines.size() - 1);
	change_level(MC_NODE_REBUILD);
}

void DoomMap::add_side(Side* side)
{
	sides.push_back(side);
	side->set_index(sides.size() - 1);
	side->set_parent(this);
}

void DoomMap::add_sector(Sector* sector)
{
	sectors.push_back(sector);
	sector->set_index(sectors.size() - 1);
	sector->set_parent(this);
}

void DoomMap::add_thing(Thing* thing)
{
	things.push_back(thing);
	thing->set_parent(this);
	thing->set_index(things.size() - 1);
	change_level(MC_THINGS);
}

void DoomMap::delete_selection(int type)
{
	if (selected_items.size() == 0)
	{
		if (hilight_item == -1)
			return;

		if (type == 0)
		{
			delete_vertex(hilight_item);
			update_indices(MTYPE_LINE|MTYPE_SIDE|MTYPE_VERTEX);
			remove_unused_vertices();
		}

		if (type == 1)
		{
			delete_line(hilight_item);
			update_indices(MTYPE_LINE|MTYPE_SIDE|MTYPE_VERTEX);
			remove_unused_vertices();
		}

		if (type == 2)
		{
			delete_sector(hilight_item);
			update_indices(MTYPE_SECTOR);
		}

		if (type == 3)
		{
			delete_thing(hilight_item);
			update_indices(MTYPE_THING);
		}

		hilight_item = -1;
	}
	else
	{
		if (type == 0)
		{
			vector<Vertex*> delete_verts;

			for (int a = 0; a < selected_items.size(); a++)
				delete_verts.push_back(vertex(selected_items[a]));
			
			for (int a = 0; a < delete_verts.size(); a++)
				delete_vertex(delete_verts[a]);

			update_indices(MTYPE_LINE|MTYPE_SIDE|MTYPE_VERTEX);
			remove_unused_vertices();
		}

		if (type == 1)
		{
			vector<Line*> delete_lines;

			for (int a = 0; a < selected_items.size(); a++)
				delete_lines.push_back(line(selected_items[a]));
			
			for (int a = 0; a < delete_lines.size(); a++)
				delete_line(delete_lines[a]);

			update_indices(MTYPE_LINE|MTYPE_VERTEX|MTYPE_SIDE);
			remove_unused_vertices();
		}

		if (type == 2)
		{
			vector<Sector*> delete_sectors;

			for (int a = 0; a < selected_items.size(); a++)
				delete_sectors.push_back(sector(selected_items[a]));
			
			for (int a = 0; a < delete_sectors.size(); a++)
				delete_sector(delete_sectors[a]);

			update_indices(MTYPE_SECTOR);
		}

		if (type == 3)
		{
			vector<Thing*> delete_things;

			for (int a = 0; a < selected_items.size(); a++)
				delete_things.push_back(thing(selected_items[a]));
			
			for (int a = 0; a < delete_things.size(); a++)
				delete_thing(delete_things[a]);

			update_indices(MTYPE_THING);
		}

		clear_selection();
	}
}

void DoomMap::delete_vertex(Vertex* vertex)
{
	delete_vertex(index(vertex, true));
}

void DoomMap::delete_vertex(int index)
{
	Vertex* vert = vertex(index);

	if (!valid(vert))
		return;

	// Delete any attached lines before deleting the vertex
	if (vertex(index)->refs() > 0)
	{
		for (int a = 0; a < lines.size(); a++)
		{
			if (lines[a]->has_vertex(vert))
			{
				delete_line(a, vert);
				a--;
			}
		}
	}

	delete vert;
	vertices[index] = vertices.back();
	vertices.pop_back();
}

void DoomMap::delete_line(Line* line)
{
	delete_line(index(line, true));
}

void DoomMap::delete_line(int index, Vertex *vertex)
{
	if (!valid(line(index)))
		return;

	/*
	if (lines[index]->vertex1()->rem_ref() &&
		lines[index]->vertex1() != vertex)
		//delete_vertex(lines[index]->vertex1());

	if (lines[index]->vertex2()->rem_ref() &&
		lines[index]->vertex2() != vertex)
		//delete_vertex(lines[index]->vertex2());
		*/

	if (lines[index]->vertex1() != vertex)
		lines[index]->vertex1()->rem_ref();

	if (lines[index]->vertex2() != vertex)
		lines[index]->vertex2()->rem_ref();

	delete lines[index];
	lines[index] = lines.back();
	lines.pop_back();
	change_level(MC_NODE_REBUILD);
}

void DoomMap::delete_side(Side* side)
{
	delete_side(index(side, true));
}

void DoomMap::delete_side(int index)
{
	if (!valid(side(index)))
		return;

	Side* s = side(index);

	for (int a = 0; a < n_lines(); a++)
	{
		if (line(a)->side1() == s)
			line(a)->remove_side1();

		if (line(a)->side2() == s)
			line(a)->remove_side2();
	}

	delete sides[index];
	sides[index] = sides.back();
	sides.pop_back();
	change_level(MC_SAVE_NEEDED);
}

void DoomMap::delete_sector(Sector* sector, bool check)
{
	delete_sector(index(sector, true), check);
}

void DoomMap::delete_sector(int index, bool check)
{
	if (!valid(sector(index)))
		return;

	Sector* s = sector(index);

	if (check)
	{
		for (int a = 0; a < n_sides(); a++)
		{
			if (sides[a]->get_sector() == s)
			{
				delete_side(a);
				a--;
			}
		}
	}

	delete sectors[index];
	sectors[index] = sectors.back();
	sectors.pop_back();
	change_level(MC_SECTORS);
}

void DoomMap::delete_thing(Thing* thing)
{
	delete_thing(index(thing, true));
}

void DoomMap::delete_thing(int index)
{
	if (!valid(thing(index)))
		return;

	delete things[index];
	things[index] = things.back();
	things.pop_back();
	change_level(MC_THINGS);
}

int	DoomMap::index(Vertex* i, bool force_search)
{
	if (!i)
		return -1;

	if (i->get_index() != -1 && !force_search)
		return i->get_index();

	if (i == def_vertex)
		return -1;

	for (int a = 0; a < vertices.size(); a++)
	{
		if (vertices[a] == i)
		{
			vertices[a]->set_index(a);
			return a;
		}
	}

	return -1;
}

int	DoomMap::index(Line* i, bool force_search)
{
	if (!i)
		return -1;

	if (i->get_index() != -1 && !force_search)
		return i->get_index();

	if (i == def_line)
		return -1;

	for (int a = 0; a < lines.size(); a++)
	{
		if (lines[a] == i)
		{
			lines[a]->set_index(a);
			return a;
		}
	}

	return -1;
}

int	DoomMap::index(Side* i, bool force_search)
{
	if (!i)
		return -1;

	if (i->get_index() != -1 && !force_search)
		return i->get_index();

	if (i == def_side)
		return -1;

	for (int a = 0; a < sides.size(); a++)
	{
		if (sides[a] == i)
		{
			sides[a]->set_index(a);
			return a;
		}
	}

	return -1;
}

int	DoomMap::index(Sector* i, bool force_search)
{
	if (!i)
		return -1;

	if (i->get_index() != -1 && !force_search)
		return i->get_index();

	if (i == def_sector)
		return -1;

	for (int a = 0; a < sectors.size(); a++)
	{
		if (sectors[a] == i)
		{
			sectors[a]->set_index(a);
			return a;
		}
	}

	return -1;
}

int	DoomMap::index(Thing* i, bool force_search)
{
	if (!i)
		return -1;

	if (i->get_index() != -1 && !force_search)
		return i->get_index();

	if (i == def_thing)
		return -1;

	for (int a = 0; a < things.size(); a++)
	{
		if (things[a] == i)
		{
			things[a]->set_index(a);
			return a;
		}
	}

	return -1;
}

bool DoomMap::valid(Vertex* i, bool fullcheck)
{
	if (i == def_vertex || !i)
		return false;

	if (fullcheck)
	{
		if (index(i, true) == -1)
			return false;
	}

	return true;
}

bool DoomMap::valid(Line* i, bool fullcheck)
{
	if (i == def_line || !i)
		return false;

	if (fullcheck)
	{
		if (index(i, true) == -1)
			return false;
	}

	return true;
}

bool DoomMap::valid(Side* i, bool fullcheck)
{
	if (i == def_side || !i)
		return false;

	if (fullcheck)
	{
		if (index(i, true) == -1)
			return false;
	}

	return true;
}

bool DoomMap::valid(Sector* i, bool fullcheck)
{
	if (i == def_sector || !i)
		return false;

	if (fullcheck)
	{
		if (index(i, true) == -1)
			return false;
	}

	return true;
}

bool DoomMap::valid(Thing* i, bool fullcheck)
{
	if (i == def_thing || !i)
		return false;

	if (fullcheck)
	{
		if (index(i, true) == -1)
			return false;
	}

	return true;
}

bool DoomMap::open(Wad* wad, string mapname)
{
	if (wad->zip)
	{
		message_box(_T("Can't load a map straight from a zip!"));
		return false;
	}

	Lump* lump = NULL;
	long offset = wad->getLumpIndex(mapname);
	FILE* fp = NULL;
	long unit_size = 0;

	if (wad->parent)
	{
		string p = c_path(_T("sladetemp.wad"), DIR_TMP);
		wad->parent->dumpToFile(p);
		wad->open(p);
		fp = fopen(chr(p), "rb");
		offset = 0;
	}
	else
		fp = fopen(chr(wad->path), "rb");

	if (offset == -1)
	{
		log_message(s_fmt(_T("Map %s not found\n"), mapname.c_str()));
		return false;
	}

	// Check for BEHAVIOR lump
	long index = offset;
	bool done = false;

	while (!done)
	{
		index++;

		if (index >= wad->numLumps())
		{
			index--;
			done = true;
		}
		else if (wad->lumpAt(index)->getName() == _T("THINGS")	||
				wad->lumpAt(index)->getName() == _T("LINEDEFS") ||
				wad->lumpAt(index)->getName() == _T("SIDEDEFS") ||
				wad->lumpAt(index)->getName() == _T("VERTEXES") ||
				wad->lumpAt(index)->getName() == _T("SEGS")		||
				wad->lumpAt(index)->getName() == _T("SSECTORS") ||
				wad->lumpAt(index)->getName() == _T("NODES")	||
				wad->lumpAt(index)->getName() == _T("SECTORS")	||
				wad->lumpAt(index)->getName() == _T("REJECT")	||
				wad->lumpAt(index)->getName() == _T("SCRIPTS")	||
				wad->lumpAt(index)->getName() == _T("BLOCKMAP"))
		{
			done = false;
		}
		else if (wad->lumpAt(index)->getName() == _T("BEHAVIOR"))
		{
			if (hexen())
				done = true;
			else
			{
				message_box(_T("This looks like a hexen-format map, please select a different game configuration!"), _T("Error"));
				return false;
			}
		}
		else
			done = true;
	}

	if (wad->lumpAt(index)->getName() != _T("BEHAVIOR") && hexen())
	{
		message_box(_T("Map has no BEHAVIOR lump"), _T("Error"));
		return false;
	}

	close();

	if (scripts)
	{
		delete scripts;
		scripts = NULL;
	}

	if (behavior)
	{
		delete behavior;
		behavior = NULL;
	}

	name = mapname;

	// << ---- Read Vertices ---- >>
	splash(_T("Loading Vertices"));
	lump = wad->getLump(_T("VERTEXES"), offset);

	if (!lump)
	{
		log_message(_T("Map has no VERTEXES lump\n"));
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->getOffset(), SEEK_SET);

	// Read vertex data
	for (DWORD i = 0; i < lump->getSize() / 4; i++)
	{
		doomvertex_t v;
		fread(&v, 4, 1, fp);
		new Vertex(v, this);
	}

	// << ---- Read sectors ---- >>
	splash(_T("Loading Sectors"));
	lump = wad->getLump(_T("SECTORS"), offset);

	if (!lump)
	{
		log_message(_T("Map has no SECTORS lump\n"));
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->getOffset(), SEEK_SET);

	for (int a = 0; a < lump->getSize() / 26; a++)
	{
		doomsector_t s;
		fread(&s, 26, 1, fp);
		new Sector(s, this);
	}

	// << ---- Read sides ---- >>
	splash(_T("Loading Sides"));
	lump = wad->getLump(_T("SIDEDEFS"), offset);

	if (!lump)
	{
		log_message(_T("Map has no SIDEDEFS lump\n"));
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->getOffset(), SEEK_SET);

	// Read side data
	for (DWORD i = 0; i < lump->getSize() / 30; i++)
	{
		doomside_t s;
		fread(&s, 30, 1, fp);
		new Side(s, this);
	}

	// << ---- Read Lines ---- >>
	splash(_T("Loading Lines"));
	int max_vert = 0;
	lump = wad->getLump(_T("LINEDEFS"), offset);

	if (!lump)
	{
		log_message(_T("Map has no LINEDEFS lump\n"));
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->getOffset(), SEEK_SET);

	if (hexen())
	{
		for (int a = 0; a < lump->getSize() / 16; a++)
		{
			bool ok = true;
			hexenline_t l;
			fread(&l, 16, 1, fp);
			Line* line = new Line(l, this, ok);
			if (!ok) delete line;
		}
	}
	else
	{
		for (int a = 0; a < lump->getSize() / 14; a++)
		{
			bool ok = true;
			doomline_t l;
			fread(&l, 14, 1, fp);
			Line* line = new Line(l, this, ok);
			if (!ok) delete line;
		}
	}

	// << ---- Read Things ---- >>
	splash(_T("Loading Things"));
	lump = wad->getLump(_T("THINGS"), offset);

	if (!lump)
	{
		log_message(_T("Map has no THINGS lump\n"));
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->getOffset(), SEEK_SET);

	if (hexen())
	{
		for (int a = 0; a < lump->getSize() / 20; a++)
		{
			hexenthing_t t;
			fread(&t, 20, 1, fp);
			new Thing(t, this);
		}
	}
	else
	{
		for (int a = 0; a < lump->getSize() / 10; a++)
		{
			doomthing_t t;
			fread(&t, 10, 1, fp);
			new Thing(t, this);
		}
	}

	// << ---- Read Scripts/Behavior ---- >>
	if (hexen())
	{
		lump = wad->getLump(_T("SCRIPTS"), offset);

		if (lump)
		{
			scripts = new Lump(0, lump->getSize(), _T("SCRIPTS"));
			scripts->loadData(lump->getData(), lump->getSize());
		}
		else
			scripts = new Lump(0, 0, _T("SCRIPTS"));

		lump = wad->getLump(_T("BEHAVIOR"), offset);

		if (lump)
		{
			behavior = new Lump(0, lump->getSize(), _T("BEHAVIOR"));
			behavior->loadData(lump->getData(), lump->getSize());
		}
		else
			behavior = new Lump(0, 0, _T("BEHAVIOR"));
	}

	log_message(s_fmt(_T("%d Lines: %d bytes"), n_lines(), n_lines() * sizeof(Line)));
	log_message(s_fmt(_T("%d Sides: %d bytes"), n_sides(), n_sides() * sizeof(Side)));
	log_message(s_fmt(_T("%d Vertices: %d bytes"), n_verts(), n_verts() * sizeof(Vertex)));
	log_message(s_fmt(_T("%d Sectors: %d bytes"), n_sectors(), n_sectors() * sizeof(Sector)));
	log_message(s_fmt(_T("%d Things: %d bytes"), n_things(), n_things() * sizeof(Thing)));

	splash(_T("Removing Unused Vertices"));
	remove_unused_vertices();

	flags |= MAP_OPEN;
	changed = 0;

	camera_to_start();
	update_indices();

	splash_hide();

	return true;
}

void DoomMap::draw(rect_t vis_area, BYTE type)
{
	// Draw vertices
	if (type == 0)
	{
		draw_lines(vis_area, false);

		glEnable(GL_POINT_SMOOTH);

		for (int a = 0; a < vertices.size(); a++)
		{
			if (vertices[a]->x_pos() > vis_area.x1() && vertices[a]->x_pos() < vis_area.x2() &&
				vertices[a]->y_pos() > vis_area.y1() && vertices[a]->y_pos() < vis_area.y2())
				vertices[a]->draw();
		}

		if (state())
		{
			for (int a = 0; a < selected_items.size(); a++)
				vertex(selected_items[a])->draw(MISTYLE_SELECTED);
		}

		if (state(STATE_MOVING))
		{
			for (int a = 0; a < moving_items.size(); a++)
				vertex(moving_items[a])->draw(MISTYLE_MOVING);
		}

		if (hilight_item != -1 && state())
			vertex(hilight_item)->draw(MISTYLE_HILIGHTED);

		glDisable(GL_POINT_SMOOTH);
	}

	// Draw lines
	if (type == 1)
	{
		draw_lines(vis_area, true);

		if (state())
		{
			for (int a = 0; a < selected_items.size(); a++)
				line(selected_items[a])->draw(true, MISTYLE_SELECTED);

			for (int a = 0; a < tagged_items.size(); a++)
				line(tagged_items[a])->draw(false, MISTYLE_TAGGED);

			if (hilight_item != -1)
				line(hilight_item)->draw(true, MISTYLE_HILIGHTED);
		}

		if (state(STATE_MOVING))
		{
			if (hilight_item != -1 && selected_items.size() == 0)
				hilight_line()->draw(true, MISTYLE_MOVING);

			for (int a = 0; a < selected_items.size(); a++)
				line(selected_items[a])->draw(true, MISTYLE_MOVING);
		}
	}

	// Draw sectors
	if (type == 2)
	{
		draw_lines(vis_area, false);

		for (int a = 0; a < n_lines(); a++)
		{
			int s1 = line(a)->sector_index(true);
			int s2 = line(a)->sector_index(false);

			for (int b = 0; b < selected_items.size(); b++)
			{
				if (selected_items[b] == s1 || selected_items[b] == s2)
				{
					if (state(STATE_MOVING))
						line(a)->draw(false, MISTYLE_MOVING);
					else
						line(a)->draw(false, MISTYLE_SELECTED);

					break;
				}
			}

			if ((s1 == hilight_item || s2 == hilight_item) && hilight_item != -1)
			{
				if (state(STATE_MOVING))
					line(a)->draw(false, MISTYLE_MOVING);
				else if (state())
					line(a)->draw(false, MISTYLE_HILIGHTED);
			}
		}

		if (state())
		{
			for (int b = 0; b < tagged_items.size(); b++)
				line(tagged_items[b])->draw(true, MISTYLE_TAGGED);
		}
	}

	// Draw things
	if (type == 3)
	{
		draw_lines(vis_area, false);

		for (int a = 0; a < n_things(); a++)
			things[a]->draw(vis_area, MISTYLE_NORMAL);

		if (state())
		{
			for (int a = 0; a < selected_items.size(); a++)
				thing(selected_items[a])->draw(vis_area, MISTYLE_SELECTED);
		}

		if (state(STATE_MOVING))
		{
			for (int a = 0; a < moving_items.size(); a++)
				thing(moving_items[a])->draw(vis_area, MISTYLE_MOVING);
		}

		if (state() && hilight_item != -1)
			thing(hilight_item)->draw(vis_area, MISTYLE_HILIGHTED);
	}
}

void DoomMap::draw_lines(rect_t vis_area, bool side)
{
	for (int a = 0; a < lines.size(); a++)
	{
		bool visible = false;
		point2_t v1(lines[a]->x1(), lines[a]->y1());
		point2_t v2(lines[a]->x2(), lines[a]->y2());

		// Simple vertex check, if either vertex is within the screen
		// border, the line must be visible
		if ((v1.x > vis_area.x1() && v1.x < vis_area.x2() &&
			v1.y > vis_area.y1() && v1.y < vis_area.y2()) ||
			(v2.x > vis_area.x1() && v2.x < vis_area.x2() &&
			v2.y > vis_area.y1() && v2.y < vis_area.y2()))
		{
			lines[a]->draw(side);
			continue;
		}

		// More complex check
		rect_t rect = lines[a]->get_rect();
		if (!((rect.x1() < vis_area.x1() && rect.x2() < vis_area.x1()) ||
			(rect.x1() > vis_area.x2() && rect.x2() > vis_area.x2()) ||
			(rect.y1() < vis_area.y1() && rect.y2() < vis_area.y1()) ||
			(rect.y1() > vis_area.y2() && rect.y2() > vis_area.y2())))
		{
			lines[a]->draw(side);
			continue;
		}
	}
}

void DoomMap::select_item(int index)
{
	if (!(vector_exists(selected_items, index)))
		selected_items.push_back(index);
}

void DoomMap::select_deselect()
{
	if (hilight_item == -1)
		return;

	if (vector_exists(selected_items, hilight_item))
		selected_items.erase(find(selected_items.begin(), selected_items.end(), hilight_item));
	else
		vector_add_nodup(selected_items, hilight_item);
}

void DoomMap::clear_selection()
{
	selected_items.clear();
}

void DoomMap::update_tagged(int type)
{
	tagged_items.clear();

	if (hilight_item == -1)
		return;

	// Lines mode (check sectors)
	if (type == 1)
	{
		int tag = line(hilight_item)->get_sector_tag();

		if (tag == 0)
			return;

		for(DWORD l = 0; l < n_lines(); l++)
		{
			bool tagged = false;

			if (sector(line(l)->sector_index(true))->sector_tag() == tag)
				tagged = true;

			if (sector(line(l)->sector_index(false))->sector_tag() == tag)
				tagged = true;

			if (tagged)
				tagged_items.push_back(l);
		}
	}

	// Sectors mode (check lines)
	if (type == 2)
	{
		int tag = sector(hilight_item)->sector_tag();

		if (tag == 0)
			return;

		for(DWORD l = 0; l < n_lines(); l++)
		{
			int stag = line(l)->get_sector_tag();

			if (stag == tag)
				tagged_items.push_back(l);
		}
	}
}

int DoomMap::get_hilight_line(point2_t mouse, int m_d)
{
	double min_dist = m_d;
	int line = -1;

	for (int a = 0; a < lines.size(); a++)
	{
		rect_t r = lines[a]->get_rect();

		double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), mouse.x, mouse.y);

		if (dist < min_dist)
		{
			min_dist = dist;
			line = a;
		}
	}

	hilight_item = line;
	return line;
}

void DoomMap::select_lines_box(rect_t box)
{
	for (int a = 0; a < lines.size(); a++)
	{
		if (point_in_rect(box.left(), box.top(), box.right(), box.bottom(), lines[a]->x1(), lines[a]->y1())
			&& point_in_rect(box.left(), box.top(), box.right(), box.bottom(), lines[a]->x2(), lines[a]->y2()))
			vector_add_nodup(selected_items, a);
	}
}

int DoomMap::get_hilight_vertex(point2_t mouse)
{
	double min_dist = 32 / editor_window->map()->get_zoom();
	int vert = -1;

	for (int v = 0; v < vertices.size(); v++)
	{
		double dist = distance(vertex(v)->x_pos(), vertex(v)->y_pos(), mouse.x, mouse.y);

		if (dist < min_dist)
		{
			min_dist = dist;
			vert = v;
		}
	}

	hilight_item = vert;
	return vert;
}

void DoomMap::select_vertices_box(rect_t box)
{
	for (int a = 0; a < vertices.size(); a++)
	{
		if (point_in_rect(box.left(), box.top(), box.right(), box.bottom(), vertices[a]->x_pos(), vertices[a]->y_pos()))
			vector_add_nodup(selected_items, a);
	}
}

int	DoomMap::get_hilight_sector(point2_t mouse)
{
	int line = get_hilight_line(mouse, 9999999);

	if (line == -1)
	{
		hilight_item = -1;
		return -1;
	}

	int sector = -1;

	bool side = (determine_line_side(this->line(line)->get_rect(), mouse.x, mouse.y) > 0);
	sector = this->line(line)->sector_index(side);

	if (!valid(this->sector(sector)))
		hilight_item = -1;
	else
		hilight_item = sector;

	return hilight_item;
}

void DoomMap::select_sectors_box(rect_t box)
{
	for (int a = 0; a < lines.size(); a++)
	{
		if (point_in_rect(box.left(), box.top(), box.right(), box.bottom(), lines[a]->x1(), lines[a]->y1())
			&& point_in_rect(box.left(), box.top(), box.right(), box.bottom(), lines[a]->x2(), lines[a]->y2()))
		{
			if (lines[a]->sector_index(true) != -1)
				vector_add_nodup(selected_items, lines[a]->sector_index(true));

			if (lines[a]->sector_index(false) != -1)
				vector_add_nodup(selected_items, lines[a]->sector_index(false));
		}
	}
}

int	DoomMap::get_hilight_thing(point2_t mouse)
{
	double min_dist = 64;
	int thing = -1;

	for (WORD t = 0; t < n_things(); t++)
	{
		double dist = distance(things[t]->pos().x, things[t]->pos().y, mouse.x, mouse.y);

		if (dist < min_dist)
		{
			min_dist = dist;
			thing = t;
		}
	}

	hilight_item = thing;
	return thing;
}

void DoomMap::select_things_box(rect_t box)
{
	for (int a = 0; a < n_things(); a++)
	{
		if (point_in_rect(box.left(), box.top(), box.right(), box.bottom(), things[a]->pos().x, things[a]->pos().y))
			selected_items.push_back(a);
	}
}

int DoomMap::remove_unused_vertices()
{
	int deleted = 0;

	for (int a = 0; a < vertices.size(); a++)
	{
		if (vertices[a]->refs() == 0)
		{
			delete_vertex(a);
			a--;
			deleted++;
		}
	}

	return deleted;
}

rect_t DoomMap::bounds()
{
	int max_x = -999999;
	int min_x = 999999;
	int max_y = -999999;
	int min_y = 999999;

	for (int v = 0; v < n_verts(); v++)
	{
		if (vertices[v]->x_pos() < min_x)
			min_x = vertices[v]->x_pos();

		if (vertices[v]->x_pos() > max_x)
			max_x = vertices[v]->x_pos();

		if (vertices[v]->y_pos() < min_y)
			min_y = vertices[v]->y_pos();

		if (vertices[v]->y_pos() > max_y)
			max_y = vertices[v]->y_pos();
	}

	return rect_t(min_x, min_y, max_x, max_y);
}

point2_t DoomMap::middle()
{
	rect_t rect = bounds();
	return point2_t(rect.left() + (float(rect.width()) / 2.0f), rect.top() + (float(rect.height()) / 2.0f));
}

Vertex* DoomMap::hilight_vertex()
{
	if (hilight_item < 0)
		return def_vertex;
	else
		return vertex(hilight_item);
}

Line* DoomMap::hilight_line()
{
	if (hilight_item < 0)
		return def_line;
	else
		return line(hilight_item);
}

Sector* DoomMap::hilight_sector()
{
	if (hilight_item < 0)
		return def_sector;
	else
		return sector(hilight_item);
}

Thing* DoomMap::hilight_thing()
{
	if (hilight_item < 0)
		return def_thing;
	else
		return thing(hilight_item);
}

bool DoomMap::add_to_wad(Wad *wad)
{
	if (wad->locked)
		return false;

	splash(_T("Saving map..."));

	BYTE*	things_data = NULL;
	BYTE*	lines_data = NULL;
	BYTE*	sides_data = NULL;
	BYTE*	verts_data = NULL;
	BYTE*	sectors_data = NULL;

	int		thing_size = 10;
	int		line_size = 14;

	if (hexen())
	{
		thing_size = 20;
		line_size = 16;
	}

	// Cleanup/check map
	remove_zerolength_lines();
	remove_unused_sectors();
	//remove_unused_sides();
	update_vertex_refs();
	remove_unused_vertices();
	update_indices();

	//if (check_lines())
	//	console_window->Show();

	//if (check_sides())
	//	console_window->Show();

	// *** SETUP DATA ***

	// Setup things data
	things_data = (BYTE *)malloc(n_things() * thing_size);

	for (DWORD t = 0; t < n_things(); t++)
	{
		BYTE* p = things_data;
		p += (t * thing_size);

		if (hexen())
			memcpy(p, &(thing(t)->to_hexenformat()), thing_size);
		else
			memcpy(p, &(thing(t)->to_doomformat()), thing_size);

	}

	// Setup lines data
	lines_data = (BYTE *)malloc(n_lines() * line_size);

	// Only update progress for lines since they take the longest to setup by far
	int everything = n_lines();
	int prog = 0;
	for (DWORD l = 0; l < n_lines(); l++)
	{
		BYTE* p = lines_data;
		p += (l * line_size);

		if (hexen())
			memcpy(p, &(line(l)->to_hexenformat()), line_size);
		else
			memcpy(p, &(line(l)->to_doomformat()), line_size);

		prog++;
		if (prog % 20 == 0)
			splash_progress((double)prog / (double)everything);
	}

	// Setup sides data
	sides_data = (BYTE *)malloc(n_sides() * 30);

	for (DWORD s = 0; s < n_sides(); s++)
	{
		BYTE* p = sides_data + (s * 30);
		memcpy(p, &(side(s)->to_doomformat()), 30);
	}

	// Setup vertices data
	verts_data = (BYTE *)malloc(n_verts() * 4);

	for (DWORD v = 0; v < n_verts(); v++)
	{
		BYTE* p = verts_data + (v * 4);
		memcpy(p, &(vertex(v)->to_doomformat()), 4);
	}

	// Setup sectors data
	sectors_data = (BYTE *)malloc(n_sectors() * 26);

	for (DWORD s = 0; s < n_sectors(); s++)
	{
		BYTE* p = sectors_data + (s * 26);
		memcpy(p, &(sector(s)->to_doomformat()), 26);
	}

	// *** WRITE DATA TO WADFILE ***
	bool scripts = false;
	bool behavior = false;

	// If map already exists in wad, delete it
	long mapindex = wad->getLumpIndex(this->name, 0);

	if (mapindex != -1)
	{
		long index = mapindex + 1;
		bool done = false;

		while (!done)
		{
			if (index == wad->numLumps())
				done = true;
			else if (wad->lumpAt(index)->getName() == _T("THINGS") ||
				wad->lumpAt(index)->getName() == _T("LINEDEFS") ||
				wad->lumpAt(index)->getName() == _T("SIDEDEFS") ||
				wad->lumpAt(index)->getName() == _T("VERTEXES") ||
				wad->lumpAt(index)->getName() == _T("SEGS") ||
				wad->lumpAt(index)->getName() == _T("SSECTORS") ||
				wad->lumpAt(index)->getName() == _T("NODES") ||
				wad->lumpAt(index)->getName() == _T("SECTORS") ||
				wad->lumpAt(index)->getName() == _T("REJECT") ||
				wad->lumpAt(index)->getName() == _T("BLOCKMAP"))
			{
				//print(true, "Deleting map entry %s\n", wadfile->directory[index]->Name().c_str());
				wad->deleteLump(index);
				done = false;
			}
			else if (wad->lumpAt(index)->getName() == _T("BEHAVIOR"))
			{
				done = false;
				index++;
				behavior = true;
			}
			else if (wad->lumpAt(index)->getName() == _T("SCRIPTS"))
			{
				done = false;
				index++;
				scripts = true;
			}
			else
			{
				//print(true, "Found next non-map entry %s\n", wadfile->directory[index]->Name().c_str());
				done = true;
			}
		}
	}
	else
	{
		mapindex = 0;
		wad->addLump(name, mapindex);
	}

	// Add map lumps
	if (hexen())
	{
		if (!scripts)
			wad->addLump(_T("SCRIPTS"), mapindex + 1);

		if (!behavior)
			wad->addLump(_T("BEHAVIOR"), mapindex + 1);
	}

	wad->addLump(_T("SECTORS"), mapindex + 1);
	wad->addLump(_T("VERTEXES"), mapindex + 1);
	wad->addLump(_T("SIDEDEFS"), mapindex + 1);
	wad->addLump(_T("LINEDEFS"), mapindex + 1);
	wad->addLump(_T("THINGS"), mapindex + 1);

	// Write map data
	wad->replaceLump(wad->getLumpIndex(_T("THINGS"), mapindex), thing_size * n_things(), things_data);
	wad->replaceLump(wad->getLumpIndex(_T("LINEDEFS"), mapindex), line_size * n_lines(), lines_data);
	wad->replaceLump(wad->getLumpIndex(_T("SIDEDEFS"), mapindex), 30 * n_sides(), sides_data);
	wad->replaceLump(wad->getLumpIndex(_T("VERTEXES"), mapindex), 4 * n_verts(), verts_data);
	wad->replaceLump(wad->getLumpIndex(_T("SECTORS"), mapindex), 26 * n_sectors(), sectors_data);

	if (hexen())
	{
		wad->replaceLump(wad->getLumpIndex(_T("SCRIPTS"), mapindex), this->scripts->getSize(), this->scripts->getData());
		wad->replaceLump(wad->getLumpIndex(_T("BEHAVIOR"), mapindex), this->behavior->getSize(), this->behavior->getData());
	}

	splash_hide();

	return true;
}

void DoomMap::change_level(BYTE flags)
{
	if (!(changed & MC_SAVE_NEEDED))
	{
		wxString title = editor_window->GetTitle();
		title += _T("*");
		editor_window->SetTitle(title);
	}

	if (flags & MC_NODE_REBUILD)
		flags |= MC_SSECTS|MC_LINES|MC_THINGS;

	changed |= MC_SAVE_NEEDED;	// If anything changes a save is needed
	changed |= flags;
}

int DoomMap::get_free_tag()
{
	int tag = 1;
	bool tagmoved = false;

	while (1)
	{
		tagmoved = false;

		for (DWORD l = 0; l < n_lines(); l++)
		{
			if (line(l)->get_sector_tag() == tag)
			{
				tag++;
				tagmoved = true;
			}
		}

		for (DWORD s = 0; s < n_sectors(); s++)
		{
			if (sector(s)->sector_tag() == tag)
			{
				tag++;
				tagmoved = true;
			}
		}

		if (!tagmoved)
			break;
	}

	return tag;
}

void DoomMap::update_indices(BYTE types)
{
	if (types & MTYPE_VERTEX)
	{
		for (int a = 0; a < n_verts(); a++)
			vertices[a]->set_index(a);
	}

	if (types & MTYPE_LINE)
	{
		for (int a = 0; a < n_lines(); a++)
			lines[a]->set_index(a);
	}

	if (types & MTYPE_SIDE)
	{
		for (int a = 0; a < n_sides(); a++)
			sides[a]->set_index(a);
	}

	if (types & MTYPE_SECTOR)
	{
		for (int a = 0; a < n_sectors(); a++)
			sectors[a]->set_index(a);
	}

	if (types & MTYPE_THING)
	{
		for (int a = 0; a < n_things(); a++)
			things[a]->set_index(a);
	}
}

point2_t origin;
void DoomMap::move_items(point2_t mouse)
{
	point2_t pos(snap_to_grid(mouse.x), snap_to_grid(mouse.y));

	if (moving_items.size() == 0)
	{
		// Add move items
		origin.set(pos);

		// Vertices or things
		if (edit_mode == 0 || edit_mode == 3)
		{
			if (selected_items.size() == 0 && hilight_item != -1)
				moving_items.push_back(hilight_item);
			else if (selected_items.size() > 0)
			{
				for (int a = 0; a < selected_items.size(); a++)
					moving_items.push_back(selected_items[a]);
			}
		}

		// Lines
		if (edit_mode == 1)
		{
			if (selected_items.size() == 0 && hilight_item != -1)
			{
				moving_items.push_back(index(hilight_line()->vertex1()));
				moving_items.push_back(index(hilight_line()->vertex2()));
			}
			else if (selected_items.size() > 0)
			{
				for (int a = 0; a < selected_items.size(); a++)
				{
					int v1 = index(line(selected_items[a])->vertex1());
					int v2 = index(line(selected_items[a])->vertex2());
					vector_add_nodup(moving_items, v1);
					vector_add_nodup(moving_items, v2);
				}
			}
		}

		// Sectors
		if (edit_mode == 2)
		{
			for (int a = 0; a < n_lines(); a++)
			{
				bool add = false;

				if (selected_items.size() == 0 && hilight_item != -1)
				{
					if (line(a)->sector_index() == hilight_item || line(a)->sector_index(false) == hilight_item)
						add = true;
				}
				else if (selected_items.size() > 0)
				{
					if (vector_exists(selected_items, line(a)->sector_index()) ||
						vector_exists(selected_items, line(a)->sector_index(false)))
						add = true;
				}

				if (add)
				{
					int v1 = index(line(a)->vertex1());
					int v2 = index(line(a)->vertex2());
					vector_add_nodup(moving_items, v1);
					vector_add_nodup(moving_items, v2);
				}
			}
		}
	}

	if (moving_items.size() == 0)
		return;

	// Move vertices
	if (edit_mode == 0 || edit_mode == 1 || edit_mode == 2)
	{
		if (moving_items.size() == 1)
			vertex(moving_items[0])->set_pos(pos.x, pos.y);
		else
		{
			for (int a = 0; a < moving_items.size(); a++)
				vertex(moving_items[a])->move(pos.x - origin.x, pos.y - origin.y);
		}
	}

	// Move things
	if (edit_mode == 3)
	{
		if (moving_items.size() == 1)
			thing(moving_items[0])->set_pos(pos.x, pos.y);
		else
		{
			for (int a = 0; a < moving_items.size(); a++)
				thing(moving_items[a])->move(pos.x - origin.x, pos.y - origin.y);
		}
	}

	origin.set(pos);
}

void DoomMap::clear_move_items()
{
	if (edit_mode < 3 && (edit_auto_merge || edit_auto_split))
	{
		// Get moving vertices
		vector<Vertex*> m_verts;
		for (int a = 0; a < moving_items.size(); a++)
			m_verts.push_back(vertex(moving_items[a]));

		// Get relevant lines
		vector<Line*> m_lines;
		for (int l = 0; l < n_lines(); l++)
		{
			for (int a = 0; a < m_verts.size(); a++)
			{
				if (d_map.line(l)->has_vertex(m_verts[a]))
				{
					m_lines.push_back(d_map.line(l));
					break;
				}
			}
		}

		if (edit_auto_merge)
		{
			for (int a = 0; a < m_verts.size(); a++)
				merge_under_vertex(m_verts[a]);

			remove_zerolength_lines();
		}

		if (edit_auto_split)
		{
			// Split lines under vertices
			for (int a = 0; a < m_verts.size(); a++)
				check_split(m_verts[a]);
		}

		if (edit_auto_merge)
			remove_overlapping_lines(m_lines, true);

		update_indices(MTYPE_VERTEX);
	}

	moving_items.clear();
	selected_items.clear();
	hilight_item = -1;
}

void DoomMap::get_lines_to_vert(int vert, vector<int> &list, bool allowdup)
{
	Vertex *v = vertex(vert);

	if (!valid(v))
		return;

	int c = 0;
	for (int a = 0; a < n_lines(); a++)
	{
		if (!allowdup)
		{
			if (vector_exists(list, a))
				continue;
		}

		if (line(a)->has_vertex(v))
		{
			list.push_back(a);
			c++;
		}

		if (c == v->refs())
			break;
	}
}

void DoomMap::update_vertex_refs()
{
	for (int a = 0; a < n_verts(); a++)
		vertices[a]->set_refs(0);

	for (int a = 0; a < n_lines(); a++)
	{
		lines[a]->vertex1()->add_ref();
		lines[a]->vertex2()->add_ref();
	}
}

int	DoomMap::remove_unused_sectors()
{
	int c = 0;
	vector<Sector*> used_sectors;
	bool done = false;

	while (!done)
	{
		done = true;
		used_sectors.clear();

		for (int s = 0; s < n_sides(); s++)
		{
			if (!(vector_exists(used_sectors, side(s)->get_sector())))
				used_sectors.push_back(side(s)->get_sector());
		}

		for (int s = 0; s < n_sectors(); s++)
		{
			if (!(vector_exists(used_sectors, sector(s))))
			{
				delete_sector(s, false);
				done = false;
				c++;
				break;
			}
		}
	}

	return c;
}

int	DoomMap::remove_zerolength_lines()
{
	int ret = 0;
	for (int a = 0; a < n_lines(); a++)
	{
		if (line(a)->vertex1() == line(a)->vertex2())
		{
			delete_line(line(a));
			a--;
			ret++;
		}
	}

	return ret;
}

int DoomMap::check_vertex_spot(point2_t pos)
{
	for (int a = 0; a < n_verts(); a++)
	{
		if (vertices[a]->pos().x == pos.x && vertices[a]->pos().y == pos.y)
			return a;
	}

	return -1;
}

void DoomMap::remove_overlapping_lines(vector<Line*> &list, bool merge)
{
	for (int a = 0; a < list.size(); a++)
	{
		for (int l = 0; l < n_lines(); l++)
		{
			if (vector_exists(list, line(l)))
				continue;

			if (line(l)->has_vertex(list[a]->vertex1()) &&
				line(l)->has_vertex(list[a]->vertex2()))
			{
				if (merge)
				{
					delete_line(list[a]);
					line_correct_references(l);
				}
				else
				{
					list[a]->vertex1()->rem_ref();
					list[a]->vertex2()->rem_ref();
					delete list[a];
				}

				list[a] = list.back();
				list.pop_back();

				a--;
				break;
			}
		}
	}
}

void DoomMap::check_split(Vertex* vert)
{
	if (!valid(vert))
		return;

	for (int a = 0; a < n_lines(); a++)
	{
		if (line(a)->has_vertex(vert))
			continue;

		rect_t r = line(a)->get_rect();
		double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), vert->x_pos(), vert->y_pos());
		if (dist < edit_split_dist)
			line(a)->split(vert);
	}
}

void DoomMap::check_split(Vertex* vert, vector<Line*> &list)
{
	if (!valid(vert))
		return;

	for (int a = 0; a < list.size(); a++)
	{
		if (list[a]->has_vertex(vert))
			continue;

		rect_t r = list[a]->get_rect();
		double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), vert->x_pos(), vert->y_pos());
		if (dist < edit_split_dist)
		{
			Line* l = list[a]->split(vert);
			list.push_back(l);
		}
	}
}

void DoomMap::merge_vertices(Vertex* v1, Vertex* v2)
{
	for (int a = 0; a < n_lines(); a++)
	{
		if (line(a)->vertex1() == v2)
			line(a)->set_vertex1(v1);
		else if (line(a)->vertex2() == v2)
			line(a)->set_vertex2(v1);
	}

	delete_vertex(v2);
}

void DoomMap::merge_like_sectors()
{
}

void DoomMap::merge_under_vertex(Vertex* v)
{
	if (!valid(v))
		return;

	for (int a = 0; a < n_verts(); a++)
	{
		if (vertices[a] == v)
			continue;

		if (vertex(a)->pos() == v->pos())
		{
			merge_vertices(v, vertex(a));
			a--;
		}
	}
}

void DoomMap::merge_vertices_spot(point2_t pos)
{
	vector<Vertex*> merge;

	for (DWORD v = 0; v < n_verts(); v++)
	{
		if (vertices[v]->pos() == pos)
			merge.push_back(vertices[v]);
	}

	for (DWORD v = 1; v < merge.size(); v++)
		merge_vertices(merge[0], merge[v]);
}


void DoomMap::get_selection(vector<Vertex*> &list, bool hilight)
{
	if (selected_items.size() == 0 && valid(hilight_vertex()) && hilight)
	{
		list.push_back(hilight_vertex());
		return;
	}

	for (int a = 0; a < selected_items.size(); a++)
	{
		if (valid(vertex(selected_items[a])))
			list.push_back(vertex(selected_items[a]));
	}
}

void DoomMap::get_selection(vector<Line*> &list, bool hilight)
{
	if (selected_items.size() == 0 && valid(hilight_line()) && hilight)
	{
		list.push_back(hilight_line());
		return;
	}

	for (int a = 0; a < selected_items.size(); a++)
	{
		if (valid(line(selected_items[a])))
			list.push_back(line(selected_items[a]));
	}
}

void DoomMap::get_selection(vector<Sector*> &list, bool hilight)
{
	if (selected_items.size() == 0 && valid(hilight_sector()) && hilight)
	{
		list.push_back(hilight_sector());
		return;
	}

	for (int a = 0; a < selected_items.size(); a++)
	{
		if (valid(sector(selected_items[a])))
			list.push_back(sector(selected_items[a]));
	}
}

void DoomMap::get_selection(vector<Thing*> &list, bool hilight)
{
	if (selected_items.size() == 0 && valid(hilight_thing()) && hilight)
	{
		list.push_back(hilight_thing());
		return;
	}

	for (int a = 0; a < selected_items.size(); a++)
	{
		if (valid(thing(selected_items[a])))
			list.push_back(thing(selected_items[a]));
	}
}
