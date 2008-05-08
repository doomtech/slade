// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004                  >>
// << ------------------------------------ >>
// << map.cpp - Map stuff                  >>
// << ------------------------------------ >>

// INCLUDES ------------------------------ >>
#include "byteswap.h"
#include "main.h"
#include "map.h"
#include "edit.h"
#include "checks.h"
#include "console.h"
#include "splash.h"
#include "console_window.h"
#include "editor_window.h"

// VARIABLES ----------------------------- >>
#ifdef map
#undef map
#endif
Slade::Map Slade::map;

string map_lumps[12] =
{
	"THINGS",
	"VERTEXES",
	"LINEDEFS",
	"SIDEDEFS",
	"SECTORS",
	"SEGS",
	"SSECTORS",
	"NODES",
	"BLOCKMAP",
	"REJECT",
	"SCRIPTS",
	"BEHAVIOR"
};

// EXTERNAL VARIABLES -------------------- >>
extern EditorWindow *editor_window;
extern ConsoleWindow *console_window;

// Slade::Map::create: Clears all map data and sets map name
// ----------------------------------------------- >>
void Slade::Map::create(string mapname)
{
	name = mapname;

	lines = (linedef_t **)NULL;
	sides = (sidedef_t **)NULL;
	verts = (vertex_t **)NULL;
	sectors = (sector_t **)NULL;
	things = (thing_t **)NULL;

	if (scripts)
		delete scripts;

	if (behavior)
		delete behavior;

	scripts = new Lump(0, 0, "SCRIPTS");
	behavior = new Lump(0, 0, "BEHAVIOR");

	n_lines = n_sides = n_verts = n_sectors = n_things = 0;
	opened = true;

	changed = 255;
	init_map();
}

// Slade::Map::close: Frees all map data
// --------------------------- >>
void Slade::Map::close()
{
	if (lines)
	{
		for (DWORD i = 0; i < n_lines; i++)
			free(lines[i]);

		free(lines);
	}

	if (sides)
	{
		for (DWORD i = 0; i < n_sides; i++)
			free(sides[i]);

		free(sides);
	}

	if (sectors)
	{
		for (DWORD i = 0; i < n_sectors; i++)
			free(sectors[i]);

		free(sectors);
	}

	if (verts)
	{
		for (DWORD i = 0; i < n_verts; i++)
			free(verts[i]);

		free(verts);
	}

	if (things)
	{
		for (DWORD i = 0; i < n_things; i++)
			free(things[i]);

		free(things);
	}

	lines = (linedef_t **)NULL;
	sides = (sidedef_t **)NULL;
	verts = (vertex_t **)NULL;
	sectors = (sector_t **)NULL;
	things = (thing_t **)NULL;

	n_lines = n_sides = n_verts = n_sectors = n_things = 0;
	opened = false;
	changed = 0;
}

// Slade::Map::open: Opens a map from an open wadfile
// ---------------------------------------- >>
bool Slade::Map::open(Wad *wad, string mapname)
{
	Lump* lump = NULL;
	long offset = wad->get_lump_index(mapname);
	FILE* fp = fopen(wad->path.c_str(), "rb");
	long unit_size = 0;
	//long unit_count = 0;

	if (offset == -1)
	{
		printf("Map %s not found\n", mapname.c_str());
		return false;
	}

	// Check for BEHAVIOR lump
	//if (hexen)
	{
		long index = offset;
		bool done = false;

		while (!done)
		{
			index++;

			if (index == wad->num_lumps)
			{
				index--;
				done = true;
			}
			else if (!strncmp(wad->directory[index]->Name().c_str(), "THINGS", 6) ||
				!strncmp(wad->directory[index]->Name().c_str(), "LINEDEFS", 8) ||
				!strncmp(wad->directory[index]->Name().c_str(), "SIDEDEFS", 8) ||
				!strncmp(wad->directory[index]->Name().c_str(), "VERTEXES", 8) ||
				!strncmp(wad->directory[index]->Name().c_str(), "SEGS", 4) ||
				!strncmp(wad->directory[index]->Name().c_str(), "SSECTORS", 8) ||
				!strncmp(wad->directory[index]->Name().c_str(), "NODES", 5) ||
				!strncmp(wad->directory[index]->Name().c_str(), "SECTORS", 7) ||
				!strncmp(wad->directory[index]->Name().c_str(), "REJECT", 6) ||
				!strncmp(wad->directory[index]->Name().c_str(), "SCRIPTS", 7) ||
				!strncmp(wad->directory[index]->Name().c_str(), "BLOCKMAP", 8))
			{
				done = false;
			}
			else if (strncmp(wad->directory[index]->Name().c_str(), "BEHAVIOR", 8) == 0)
			{
				if (hexen)
					done = true;
				else
				{
					wxMessageBox(_T("This looks like a hexen-format map, please select a different game configuration!"), _T("Error"));
					return false;
				}
			}
			else
				done = true;
		}

		if (strncmp(wad->directory[index]->Name().c_str(), "BEHAVIOR", 8) != 0 && hexen)
		{
			wxMessageBox(_T("Map has no BEHAVIOR lump"), _T("Error"));
			return false;
		}
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
	splash("Loading Vertices");
	lump = wad->get_lump("VERTEXES", offset);

	if (!lump)
	{
		printf("Map has no VERTEXES lump\n");
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	// Setup vertices array
	unit_size = 4;
	n_verts = lump->Size() / unit_size;
	verts = (vertex_t **)calloc(n_verts, sizeof(vertex_t *));

	// Read vertex data
	for (DWORD i = 0; i < n_verts; i++)
	{
		verts[i] = new vertex_t;
		lefread(&verts[i]->x, 2, 1, fp);
		lefread(&verts[i]->y, 2, 1, fp);
	}

	// << ---- Read sides ---- >>
	splash("Loading Sides");
	lump = wad->get_lump("SIDEDEFS", offset);

	if (!lump)
	{
		printf("Map has no SIDEDEFS lump\n");
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	// Setup sides array
	unit_size = 30;
	n_sides = lump->Size() / unit_size;
	sides = (sidedef_t **)calloc(n_sides, sizeof(sidedef_t *));

	// Read side data
	for (DWORD i = 0; i < n_sides; i++)
	{
		sides[i] = new sidedef_t;
		char temp[9] = "";

		lefread(&sides[i]->x_offset, 2, 1, fp);
		lefread(&sides[i]->y_offset, 2, 1, fp);
		
		lefread(temp, 1, 8, fp);
		temp[8] = 0;
		sides[i]->tex_upper = str_upper(temp);

		lefread(temp, 1, 8, fp);
		temp[8] = 0;
		sides[i]->tex_lower = str_upper(temp);

		lefread(temp, 1, 8, fp);
		temp[8] = 0;
		sides[i]->tex_middle = str_upper(temp);

		lefread(&sides[i]->sector, 2, 1, fp);
	}

	// << ---- Read Lines ---- >>
	splash("Loading Lines");
	int max_vert = 0;
	lump = wad->get_lump("LINEDEFS", offset);

	if (!lump)
	{
		printf("Map has no LINEDEFS lump\n");
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	if (hexen)
	{
		// Setup & read hexen format lines

		// Setup lines array
		unit_size = 16;
		n_lines = lump->Size() / unit_size;
		lines = (linedef_t **)calloc(n_lines, sizeof(linedef_t *));

		// Read line data
		for (DWORD i = 0; i < n_lines; i++)
		{
			BYTE temp = 0;

			lines[i] = new linedef_t;
			lefread(&lines[i]->vertex1, 2, 1, fp);
			lefread(&lines[i]->vertex2, 2, 1, fp);
			lefread(&lines[i]->flags, 2, 1, fp);
			lefread(&temp, 1, 1, fp);
			lines[i]->type = temp;
			lefread(lines[i]->args, 1, 5, fp);

			short s1, s2;
			lefread(&s1, 2, 1, fp);
			lefread(&s2, 2, 1, fp);
			lines[i]->side1 = (int)s1;
			lines[i]->side2 = (int)s2;

			//wxLogMessage(_T("Line %d v1 %d v2 %d"), i, lines[i]->vertex1, lines[i]->vertex2);

			if (n_sides > 32767)
			{
				unsigned short us1 = static_cast<unsigned short>(s1);
				unsigned short us2 = static_cast<unsigned short>(s2);
				
				if (s1 != -1)
					lines[i]->side1 = us1;

				if (s2 != -1)
					lines[i]->side2 = us2;
			}

			if (lines[i]->vertex1 > max_vert)
				max_vert = lines[i]->vertex1;
			if (lines[i]->vertex2 > max_vert)
				max_vert = lines[i]->vertex2;
		}
	}
	else
	{
		// Setup lines array
		unit_size = 14;
		n_lines = lump->Size() / unit_size;
		lines = (linedef_t **)calloc(n_lines, sizeof(linedef_t *));

		// Read line data
		for (DWORD i = 0; i < n_lines; i++)
		{
			lines[i] = new linedef_t;
			lefread(&lines[i]->vertex1, 2, 1, fp);
			lefread(&lines[i]->vertex2, 2, 1, fp);
			lefread(&lines[i]->flags, 2, 1, fp);
			lefread(&lines[i]->type, 2, 1, fp);
			lefread(&lines[i]->sector_tag, 2, 1, fp);

			short s1, s2;
			lefread(&s1, 2, 1, fp);
			lefread(&s2, 2, 1, fp);
			lines[i]->side1 = (int)s1;
			lines[i]->side2 = (int)s2;

			if (n_sides > 32767)
			{
				unsigned short us1 = static_cast<unsigned short>(s1);
				unsigned short us2 = static_cast<unsigned short>(s2);
				
				if (s1 != -1)
					lines[i]->side1 = us1;

				if (s2 != -1)
					lines[i]->side2 = us2;
			}

			if (lines[i]->vertex1 > max_vert)
				max_vert = lines[i]->vertex1;
			if (lines[i]->vertex2 > max_vert)
				max_vert = lines[i]->vertex2;
		}
	}

	n_verts = max_vert + 1;

	// << ---- Read sectors ---- >>
	splash("Loading Sectors");
	lump = wad->get_lump("SECTORS", offset);

	if (!lump)
	{
		printf("Map has no SECTORS lump\n");
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	// Setup sides array
	unit_size = 26;
	n_sectors = lump->Size() / unit_size;
	sectors = (sector_t **)calloc(n_sectors, sizeof(sector_t *));

	// Read sector data
	for (DWORD i = 0; i < n_sectors; i++)
	{
		sectors[i] = new sector_t;
		char temp[9] = "";

		lefread(&sectors[i]->f_height, 2, 1, fp);
		lefread(&sectors[i]->c_height, 2, 1, fp);
		
		lefread(temp, 1, 8, fp);
		temp[8] = 0;
		sectors[i]->f_tex = str_upper(temp);

		lefread(temp, 1, 8, fp);
		temp[8] = 0;
		sectors[i]->c_tex = str_upper(temp);

		lefread(&sectors[i]->light, 2, 1, fp);
		lefread(&sectors[i]->special, 2, 1, fp);
		lefread(&sectors[i]->tag, 2, 1, fp);

		//sectors[i]->hilighted = false;
		//sectors[i]->selected = false;
		//sectors[i]->moving = false;
	}

	// << ---- Read Things ---- >>
	splash("Loading Things");
	lump = wad->get_lump("THINGS", offset);

	if (!lump)
	{
		printf("Map has no THINGS lump\n");
		splash_hide();
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	if (hexen)
	{
		// Setup & read hexen format things

		// Setup things array
		unit_size = 20;
		n_things = lump->Size() / unit_size;
		things = (thing_t **)calloc(n_things, sizeof(thing_t *));

		// Read thing data
		for (DWORD i = 0; i < n_things; i++)
		{
			things[i] = new thing_t;
			lefread(&things[i]->tid, 2, 1, fp);
			lefread(&things[i]->x, 2, 1, fp);
			lefread(&things[i]->y, 2, 1, fp);
			lefread(&things[i]->z, 2, 1, fp);
			lefread(&things[i]->angle, 2, 1, fp);
			lefread(&things[i]->type, 2, 1, fp);
			lefread(&things[i]->flags, 2, 1, fp);
			lefread(&things[i]->special, 1, 1, fp);
			lefread(things[i]->args, 1, 5, fp);
		}
	}
	else
	{
		// Setup things array
		unit_size = 10;
		n_things = lump->Size() / unit_size;
		things = (thing_t **)calloc(n_things, sizeof(thing_t *));

		// Read thing data
		for (DWORD i = 0; i < n_things; i++)
		{
			things[i] = new thing_t;
			lefread(&things[i]->x, 2, 1, fp);
			lefread(&things[i]->y, 2, 1, fp);
			lefread(&things[i]->angle, 2, 1, fp);
			lefread(&things[i]->type, 2, 1, fp);
			lefread(&things[i]->flags, 2, 1, fp);
		}
	}

	// << ---- Read Scripts/Behavior ---- >>
	if (hexen)
	{
		lump = wad->get_lump("SCRIPTS", offset);

		if (lump)
		{
			fseek(fp, lump->Offset(), SEEK_SET);
			scripts = new Lump(0, lump->Size(), "SCRIPTS");
			lefread(scripts->Data(), lump->Size(), 1, fp);
		}
		else
			scripts = new Lump(0, 0, "SCRIPTS");

		lump = wad->get_lump("BEHAVIOR", offset);

		if (lump)
		{
			fseek(fp, lump->Offset(), SEEK_SET);
			behavior = new Lump(0, lump->Size(), "BEHAVIOR");
			lefread(behavior->Data(), lump->Size(), 1, fp);
		}
		else
			behavior = new Lump(0, 0, "BEHAVIOR");
	}

	splash("Checking Lines");
	if (check_lines())
		console_window->Show();

	splash("Checking Sides");
	if (check_sides())
		console_window->Show();

	splash("Removing Unused Vertices");
	remove_free_verts();

	// Set thing colours/radii/angle
	splash("Init Thing Data");
	for (int a = 0; a < n_things; a++)
		things[a]->ttype = get_thing_type(things[a]->type);

	init_map();
	opened = true;

	splash_hide();
	return true;
}

short Slade::Map::l_getsector1(int l)
{
	if (lines[l]->side1 == -1 || n_sectors == 0)
		return -1;
	else
		return sides[lines[l]->side1]->sector;
}

short Slade::Map::l_getsector2(int l)
{
	if (lines[l]->side2 == -1 || n_sectors == 0)
		return -1;
	else
		return sides[lines[l]->side2]->sector;
}

rect_t Slade::Map::l_getrect(int l)
{
	if (l > n_lines || l < 0)
	{
		wxLogMessage(_T("Invalid line %d (/%d)"), l, n_lines);
		return rect_t(0, 0, 0, 0);
	}

	return rect_t(verts[lines[l]->vertex1]->x, verts[lines[l]->vertex1]->y, 
				verts[lines[l]->vertex2]->x, verts[lines[l]->vertex2]->y);
}

rect_t Slade::Map::l_getsrect(int l)
{
	if (l > n_lines || l < 0)
	{
		wxLogMessage(_T("Invalid line %d (/%d)"), l, n_lines);
		return rect_t(0, 0, 0, 0);
	}

	return rect_t(s_x(verts[lines[l]->vertex1]->x), s_y(-verts[lines[l]->vertex1]->y), 
				s_x(verts[lines[l]->vertex2]->x), s_y(-verts[lines[l]->vertex2]->y));
}

point2_t Slade::Map::v_getpoint(int v)
{
	if (v > n_verts || v < 0)
	{
		wxLogMessage(_T("Invalid vertex %d (/%d)"), v, n_verts);
		return point2_t(0, 0);
	}

	return point2_t(verts[v]->x, verts[v]->y);
}

point2_t Slade::Map::v_getspoint(int v)
{
	if (v > n_verts || v < 0)
	{
		wxLogMessage(_T("Invalid vertex %d (/%d)"), v, n_verts);
		return point2_t(0, 0);
	}

	return point2_t(s_x(verts[v]->x), s_y(-verts[v]->y));
}

/*
bool Slade::Map::s_ishilighted(int s)
{
	if (s < 0)
		return false;
	else
		return sectors[s]->hilighted;
}

bool Slade::Map::s_isselected(int s)
{
	if (s < 0)
		return false;
	else
		return sectors[s]->selected;
}

bool Slade::Map::s_ismoving(int s)
{
	if (s < 0 || s > n_sectors)
		return false;
	else
		return sectors[s]->moving;
}
*/

void Slade::Map::delete_vertex(int vertex)
{
	free(verts[vertex]);

	for (DWORD v = vertex; v < n_verts - 1; v++)
		verts[v] = verts[v + 1];

	n_verts--;
	verts = (vertex_t **)realloc(verts, n_verts * sizeof(vertex_t *));

	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == vertex || lines[l]->vertex2 == vertex)
		{
			delete_line(l);
			l--;
		}
		else
		{
			if (lines[l]->vertex1 > vertex)
				lines[l]->vertex1--;
		
			if (lines[l]->vertex2 > vertex)
				lines[l]->vertex2--;
		}
	}

	change_level(MC_NODE_REBUILD);
}

void Slade::Map::delete_line(int line)
{
	free(lines[line]);

	for (DWORD l = line; l < n_lines - 1; l++)
		lines[l] = lines[l + 1];

	n_lines--;
	lines = (linedef_t **)realloc(lines, n_lines * sizeof(linedef_t *));

	change_level(MC_NODE_REBUILD);
}

void Slade::Map::delete_thing(int thing)
{
	free(things[thing]);

	for (DWORD t = thing; t < n_things - 1; t++)
		things[t] = things[t + 1];

	n_things--;
	things = (thing_t **)realloc(things, n_things * sizeof(thing_t *));

	change_level(MC_THINGS);
}

void Slade::Map::delete_side(int side)
{
	free(sides[side]);

	for (DWORD s = side; s < n_sides - 1; s++)
		sides[s] = sides[s + 1];

	n_sides--;
	sides = (sidedef_t **)realloc(sides, n_sides * sizeof(sidedef_t *));

	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->side1 == side)
		{
			lines[l]->side1 = -1;

			if (lines[l]->side2 != -1 && lines[l]->side2 != side)
			{
				l_flip(l);
				l_flipsides(l);
				map.lines[l]->set_flag(LINE_IMPASSIBLE);
				map.lines[l]->clear_flag(LINE_TWOSIDED);
			}
		}

		if (lines[l]->side2 == side)
		{
			lines[l]->side2 = -1;
			map.lines[l]->set_flag(LINE_IMPASSIBLE);
			map.lines[l]->clear_flag(LINE_TWOSIDED);
		}

		if (lines[l]->side1 > side)
			lines[l]->side1--;

		if (lines[l]->side2 > side)
			lines[l]->side2--;
	}
}

void Slade::Map::delete_sector(int sector)
{
	free(sectors[sector]);

	for (int s = sector; s < n_sectors - 1; s++)
		sectors[s] = sectors[s + 1];

	n_sectors--;
	sectors = (sector_t **)realloc(sectors, n_sectors * sizeof(sector_t *));

	for (int s = 0; s < n_sides; s++)
	{
		if (sides[s]->sector == sector)
			sides[s]->sector = -1;

		if (sides[s]->sector > sector)
			sides[s]->sector--;
	}

	for (int s = 0; s < n_sides; s++)
	{
		if (sides[s]->sector == -1)
		{
			delete_side(s);
			s--;
		}
	}

	change_level(MC_SECTORS|MC_THINGS);
}

void Slade::Map::delete_vertex(vertex_t *vertex)
{
	for (int a = 0; a < n_verts; a++)
	{
		if (verts[a] == vertex)
		{
			delete_vertex(a);
			return;
		}
	}
}

void Slade::Map::delete_line(linedef_t *line)
{
	for (int a = 0; a < n_lines; a++)
	{
		if (lines[a] == line)
		{
			delete_line(a);
			return;
		}
	}
}

void Slade::Map::delete_sector(sector_t *sector)
{
	for (int a = 0; a < n_sectors; a++)
	{
		if (sectors[a] == sector)
		{
			delete_sector(a);
			return;
		}
	}
}

void Slade::Map::delete_side(sidedef_t *side)
{
	for (int a = 0; a < n_sides; a++)
	{
		if (sides[a] == side)
		{
			delete_side(a);
			return;
		}
	}
}

void Slade::Map::delete_thing(thing_t *thing)
{
	for (int a = 0; a < n_things; a++)
	{
		if (things[a] == thing)
		{
			delete_thing(a);
			return;
		}
	}
}

bool Slade::Map::v_isattached(int v)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v || lines[l]->vertex2 == v)
			return true;
	}
	
	return false;
}

bool Slade::Map::v_isattached_sector(int v)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v || lines[l]->vertex2 == v)
		{
			if (map.l_getsector1(l) != -1 || map.l_getsector1(l) != -1)
				return true;
		}
	}
	
	return false;
}

bool Slade::Map::l_needsuptex(int l, int side)
{
	int sector1 = l_getsector1(l);
	int sector2 = l_getsector2(l);

	// False if not two-sided
	if (sector1 == -1 || sector2 == -1)
		return false;

	if (side == 1)
	{
		if (sectors[l_getsector1(l)]->c_height > sectors[l_getsector2(l)]->c_height)
			return true;
		else
			return false;
	}

	if (side == 2)
	{
		if (sectors[l_getsector2(l)]->c_height > sectors[l_getsector1(l)]->c_height)
			return true;
		else
			return false;
	}

	return false;
}

bool Slade::Map::l_needsmidtex(int l, int side)
{
	if (side == 1)
	{
		if (lines[l]->side2 == -1)
			return true;
		else
		{
			if (sides[lines[l]->side1]->tex_middle == "-")
				return false;
			else
				return true;
		}
	}

	if (side == 2 && lines[l]->side1 != -1)
	{
		if (sides[lines[l]->side2]->tex_middle == "-")
			return false;
		else
			return true;
	}

	return false;
}

bool Slade::Map::l_needslotex(int l, int side)
{
	// False if not two-sided
	if (l_getsector1(l) == -1 || l_getsector2(l) == -1)
		return false;

	if (side == 1)
	{
		if (sectors[l_getsector2(l)]->f_height > sectors[l_getsector1(l)]->f_height)
			return true;
		else
			return false;
	}

	if (side == 2)
	{
		if (sectors[l_getsector1(l)]->f_height > sectors[l_getsector2(l)]->f_height)
			return true;
		else
			return false;
	}

	return false;
}

short Slade::Map::l_getxoff(int l, int side)
{
	if (side == 1 && lines[l]->side1 != -1)
		return sides[lines[l]->side1]->x_offset;

	if (side == 2 && lines[l]->side2 != -1)
		return sides[lines[l]->side2]->x_offset;

	return 0;
}

short Slade::Map::l_getyoff(int l, int side)
{
	if (side == 1 && lines[l]->side1 != -1)
		return sides[lines[l]->side1]->y_offset;

	if (side == 2 && lines[l]->side2 != -1)
		return sides[lines[l]->side2]->y_offset;

	return 0;
}

void Slade::Map::l_setmidtex(int l, int side, string tex)
{
	int s;

	if (side == 1)
		s = lines[l]->side1;

	if (side == 2)
		s = lines[l]->side2;

	if (s != -1)
		sides[s]->tex_middle = tex;

	change_level(MC_LINES);
}

void Slade::Map::l_setuptex(int l, int side, string tex)
{
	int s;

	if (side == 1)
		s = lines[l]->side1;

	if (side == 2)
		s = lines[l]->side2;

	if (s != -1)
		sides[s]->tex_upper = tex;

	change_level(MC_LINES);
}

void Slade::Map::l_setlotex(int l, int side, string tex)
{
	int s;

	if (side == 1)
		s = lines[l]->side1;

	if (side == 2)
		s = lines[l]->side2;

	if (s != -1)
		sides[s]->tex_lower = tex;

	change_level(MC_LINES);
}

int Slade::Map::l_split(int l, int vertex)
{
	int vertex2 = lines[l]->vertex2;

	int side1 = -1;
	int side2 = -1;
	int new_line = -1;

	// Add new side for side1
	if (lines[l]->side1 != -1)
	{
		side1 = add_side();
		memcpy(sides[side1], sides[lines[l]->side1], sizeof(sidedef_t));
	}

	// Add new side for side2
	if (lines[l]->side2 != -1)
	{
		side2 = add_side();
		memcpy(sides[side2], sides[lines[l]->side2], sizeof(sidedef_t));
	}

	// Create new line
	lines[l]->vertex2 = vertex;
	new_line = add_line(vertex, vertex2);

	// Setup new line
	memcpy(lines[new_line], lines[l], sizeof(linedef_t));
	lines[new_line]->vertex1 = vertex;
	lines[new_line]->vertex2 = vertex2;
	lines[new_line]->side1 = side1;
	lines[new_line]->side2 = side2;

	change_level(MC_NODE_REBUILD);

	return new_line;
}

bool Slade::Map::v_checkspot(int x, int y)
{
	for (DWORD v = 0; v < n_verts; v++)
	{
		if (verts[v]->x == x && verts[v]->y == y)
			return false;
	}

	return true;
}

void Slade::Map::v_merge(int v1, int v2)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v1)
			lines[l]->vertex1 = v2;
		else if (lines[l]->vertex2 == v1)
			lines[l]->vertex2 = v2;
	}

	delete_vertex(v1);
	change_level(MC_NODE_REBUILD);
}

void Slade::Map::v_mergespot(int x, int y)
{
	//numlist_t merge;
	vector<int> merge;

	for (DWORD v = 0; v < n_verts; v++)
	{
		if (verts[v]->x == x && verts[v]->y == y)
			merge.push_back(v);
	}

	for (DWORD v = 1; v < merge.size(); v++)
		v_merge(merge[v], merge[0]);
}

int Slade::Map::add_thing(short x, short y, thing_t properties)
{
	n_things++;
	things = (thing_t **)realloc(things, n_things * sizeof(thing_t *));
	things[n_things - 1] = new thing_t();

	things[n_things - 1]->x = x;
	things[n_things - 1]->y = y;
	things[n_things - 1]->angle = properties.angle;
	things[n_things - 1]->type = properties.type;
	things[n_things - 1]->flags = properties.flags;
	//things[n_things - 1]->selected = false;
	//things[n_things - 1]->hilighted = false;
	//things[n_things - 1]->moving = false;

	//update_map_things();

	things[n_things - 1]->ttype = get_thing_type(things[n_things - 1]->type);

	change_level(MC_THINGS);

	return n_things - 1;
}

int Slade::Map::add_vertex(short x, short y)
{
	n_verts++;
	verts = (vertex_t **)realloc(verts, n_verts * sizeof(vertex_t *));
	verts[n_verts - 1] = new vertex_t(x, y);

	change_level(MC_LINES);

	return n_verts - 1;
}

int Slade::Map::add_line(int v1, int v2)
{
	n_lines++;
	lines = (linedef_t **)realloc(lines, n_lines * sizeof(linedef_t *));
	lines[n_lines - 1] = new linedef_t(v1, v2);
	lines[n_lines - 1]->set_flag(LINE_IMPASSIBLE);

	change_level(MC_NODE_REBUILD);

	return n_lines - 1;
}

int Slade::Map::add_sector()
{
	n_sectors++;
	sectors = (sector_t **)realloc(sectors, n_sectors * sizeof(sector_t *));
	sectors[n_sectors - 1] = new sector_t();

	change_level(MC_SECTORS);

	return n_sectors - 1;
}

int Slade::Map::add_side()
{
	n_sides++;
	sides = (sidedef_t **)realloc(sides, n_sides * sizeof(sidedef_t *));
	sides[n_sides - 1] = new sidedef_t();
	sides[n_sides - 1]->sector = n_sectors - 1;

	return n_sides - 1;
}

void Slade::Map::l_setdeftextures(int l)
{
	if (l_getsector1(l) != -1)
	{
		sides[lines[l]->side1]->tex_upper = "-";
		sides[lines[l]->side1]->tex_lower = "-";
		sides[lines[l]->side1]->tex_middle = "-";

		if (l_needsuptex(l, 1))
			sides[lines[l]->side1]->def_tex(TEX_UPPER);

		if (l_needsmidtex(l, 1))
			sides[lines[l]->side1]->def_tex(TEX_MIDDLE);

		if (l_needslotex(l, 1))
			sides[lines[l]->side1]->def_tex(TEX_LOWER);
	}

	if (l_getsector2(l) != -1)
	{
		sides[lines[l]->side2]->tex_upper = "-";
		sides[lines[l]->side2]->tex_lower = "-";
		sides[lines[l]->side2]->tex_middle = "-";

		if (l_needslotex(l, 2))
			sides[lines[l]->side2]->def_tex(TEX_LOWER);

		if (l_needsuptex(l, 2))
			sides[lines[l]->side2]->def_tex(TEX_UPPER);

		if (l_needsmidtex(l, 2))
			sides[lines[l]->side2]->def_tex(TEX_MIDDLE);
	}
}

// Slade::Map::add_to_wad: Adds raw map data to a wad file
// --------------------------------------------- >>
void Slade::Map::add_to_wad(Wad *wadfile)
{
	if (wadfile->locked)
		return;

	//print(true, "Saving %s to %s..,\n", name.c_str(), wadfile->path.c_str());

	BYTE*	things_data = NULL;
	BYTE*	lines_data = NULL;
	BYTE*	sides_data = NULL;
	BYTE*	verts_data = NULL;
	BYTE*	sectors_data = NULL;

	int		thing_size = 10;
	int		line_size = 14;

	if (hexen)
	{
		thing_size = 20;
		line_size = 16;
	}

	// Cleanup/check map
	remove_free_verts();
	remove_zerolength_lines();
	remove_unused_sectors();
	remove_unused_sides();

	if (check_lines())
		console_window->Show();

	if (check_sides())
		console_window->Show();

	// *** SETUP DATA ***

	// Setup things data
	things_data = (BYTE *)malloc(n_things * thing_size);

	for (DWORD t = 0; t < n_things; t++)
	{
		BYTE* p = things_data;
		p += (t * thing_size);

		if (hexen)
		{
			lememcpy(p, &things[t]->tid, 2); p += 2;
			lememcpy(p, &things[t]->x, 2); p += 2;
			lememcpy(p, &things[t]->y, 2); p += 2;
			lememcpy(p, &things[t]->z, 2); p += 2;
			lememcpy(p, &things[t]->angle, 2); p += 2;
			lememcpy(p, &things[t]->type, 2); p += 2;
			lememcpy(p, &things[t]->flags, 2); p += 2;
			memcpy(p, &things[t]->special, 1); p += 1;
			memcpy(p, things[t]->args, 5); p += 5;
		}
		else
		{
			lememcpy(p, &things[t]->x, 2); p += 2;
			lememcpy(p, &things[t]->y, 2); p += 2;
			lememcpy(p, &things[t]->angle, 2); p += 2;
			lememcpy(p, &things[t]->type, 2); p += 2;
			lememcpy(p, &things[t]->flags, 2); p += 2;
		}
	}

	// Setup lines data
	lines_data = (BYTE *)malloc(n_lines * line_size);

	for (DWORD l = 0; l < n_lines; l++)
	{
		BYTE* p = lines_data;
		p += (l * line_size);

		if (hexen)
		{
			BYTE temp = lines[l]->type;

			lememcpy(p, &lines[l]->vertex1, 2); p += 2;
			lememcpy(p, &lines[l]->vertex2, 2); p += 2;
			lememcpy(p, &lines[l]->flags, 2); p += 2;
			memcpy(p, &temp, 1); p += 1;
			memcpy(p, lines[l]->args, 5); p += 5;
			lememcpy(p, &lines[l]->side1, 2); p += 2;
			lememcpy(p, &lines[l]->side2, 2); p += 2;
		}
		else
		{
			lememcpy(p, &lines[l]->vertex1, 2); p += 2;
			lememcpy(p, &lines[l]->vertex2, 2); p += 2;
			lememcpy(p, &lines[l]->flags, 2); p += 2;
			lememcpy(p, &lines[l]->type, 2); p += 2;
			lememcpy(p, &lines[l]->sector_tag, 2); p += 2;
			lememcpy(p, &lines[l]->side1, 2); p += 2;
			lememcpy(p, &lines[l]->side2, 2); p += 2;
		}
	}

	// Setup sides data
	sides_data = (BYTE *)malloc(n_sides * 30);

	for (DWORD s = 0; s < n_sides; s++)
	{
		// Pad texture names with 0's
		char up[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };
		char lo[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };
		char mid[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };

		// Read texture names into temp char arrays
		for (int c = 0; c < sides[s]->tex_upper.length(); c++)
			up[c] = sides[s]->tex_upper[c];
		for (int c = 0; c < sides[s]->tex_lower.length(); c++)
			lo[c] = sides[s]->tex_lower[c];
		for (int c = 0; c < sides[s]->tex_middle.length(); c++)
			mid[c] = sides[s]->tex_middle[c];

		BYTE* p = sides_data + (s * 30);

		lememcpy(p, &sides[s]->x_offset, 2); p += 2;
		lememcpy(p, &sides[s]->y_offset, 2); p += 2;
		memcpy(p, up, 8); p += 8;
		memcpy(p, lo, 8); p += 8;
		memcpy(p, mid, 8); p += 8;
		lememcpy(p, &sides[s]->sector, 2); p += 2;
	}

	// Setup vertices data
	verts_data = (BYTE *)malloc(n_verts * 4);

	for (DWORD v = 0; v < n_verts; v++)
	{
		BYTE* p = verts_data + (v * 4);

		lememcpy(p, &verts[v]->x, 2); p += 2;
		lememcpy(p, &verts[v]->y, 2); p += 2;
	}

	// Setup sectors data
	sectors_data = (BYTE *)malloc(n_sectors * 26);

	for (DWORD s = 0; s < n_sectors; s++)
	{
		// Pad texture names with 0's
		char floor[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };
		char ceil[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };

		// Read texture names into temp char arrays
		for (int c = 0; c < sectors[s]->f_tex.length(); c++)
			floor[c] = sectors[s]->f_tex[c];
		for (int c = 0; c < sectors[s]->c_tex.length(); c++)
			ceil[c] = sectors[s]->c_tex[c];

		BYTE* p = sectors_data + (s * 26);

		lememcpy(p, &sectors[s]->f_height, 2); p += 2;
		lememcpy(p, &sectors[s]->c_height, 2); p += 2;
		memcpy(p, floor, 8); p += 8;
		memcpy(p, ceil, 8); p += 8;
		lememcpy(p, &sectors[s]->light, 2); p += 2;
		lememcpy(p, &sectors[s]->special, 2); p += 2;
		lememcpy(p, &sectors[s]->tag, 2); p += 2;
	}

	// *** WRITE DATA TO WADFILE ***
	bool scripts = false;
	bool behavior = false;

	// If map already exists in wad, delete it
	long mapindex = wadfile->get_lump_index(this->name, 0);

	if (mapindex != -1)
	{
		long index = mapindex + 1;
		bool done = false;

		while (!done)
		{
			if (index == wadfile->num_lumps)
				done = true;
			else if (strncmp(wadfile->directory[index]->Name().c_str(), "THINGS", 6) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "LINEDEFS", 8) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "SIDEDEFS", 8) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "VERTEXES", 8) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "SEGS", 4) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "SSECTORS", 8) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "NODES", 5) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "SECTORS", 7) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "REJECT", 6) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "BLOCKMAP", 8) == 0)
			{
				//print(true, "Deleting map entry %s\n", wadfile->directory[index]->Name().c_str());
				wadfile->delete_lump(wadfile->directory[index]->Name(), mapindex);
				done = false;
			}
			else if (strncmp(wadfile->directory[index]->Name().c_str(), "BEHAVIOR", 8) == 0)
			{
				done = false;
				index++;
				behavior = true;
			}
			else if (strncmp(wadfile->directory[index]->Name().c_str(), "SCRIPTS", 7) == 0)
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
		wadfile->add_lump(name, mapindex);
	}

	// Add map lumps
	if (hexen)
	{
		if (!scripts)
			wadfile->add_lump("SCRIPTS", mapindex + 1);

		if (!behavior)
			wadfile->add_lump("BEHAVIOR", mapindex + 1);
	}

	wadfile->add_lump("SECTORS", mapindex + 1);
	wadfile->add_lump("VERTEXES", mapindex + 1);
	wadfile->add_lump("SIDEDEFS", mapindex + 1);
	wadfile->add_lump("LINEDEFS", mapindex + 1);
	wadfile->add_lump("THINGS", mapindex + 1);

	// Write map data
	wadfile->replace_lump("THINGS", thing_size * n_things, things_data, mapindex);
	wadfile->replace_lump("LINEDEFS", line_size * n_lines, lines_data, mapindex);
	wadfile->replace_lump("SIDEDEFS", 30 * n_sides, sides_data, mapindex);
	wadfile->replace_lump("VERTEXES", 4 * n_verts, verts_data, mapindex);
	wadfile->replace_lump("SECTORS", 26 * n_sectors, sectors_data, mapindex);

	if (hexen)
	{
		wadfile->replace_lump("SCRIPTS", this->scripts->Size(), this->scripts->Data(), mapindex);
		wadfile->replace_lump("BEHAVIOR", this->behavior->Size(), this->behavior->Data(), mapindex);
	}
}

void Slade::Map::l_flip(int l)
{
	WORD temp = lines[l]->vertex1;
	lines[l]->vertex1 = lines[l]->vertex2;
	lines[l]->vertex2 = temp;

	change_level(MC_NODE_REBUILD);
}

void Slade::Map::l_flipsides(int l)
{
	short temp = lines[l]->side1;
	lines[l]->side1 = lines[l]->side2;
	lines[l]->side2 = temp;

	change_level(MC_NODE_REBUILD|MC_SECTORS);
}

void Slade::Map::s_changeheight(int s, bool floor, int amount)
{
	if (floor)
		sectors[s]->f_height += amount;
	else
		sectors[s]->c_height += amount;

	change_level(MC_SECTORS);
}

int	Slade::Map::v_getvertatpoint(point2_t point)
{
	if (n_verts == 0)
		return -1;

	for (int v = n_verts - 1; v >= 0; v--)
	{
		if (verts[v]->x == point.x && verts[v]->y == point.y)
			return v;
	}

	return -1;
}

vector<int> Slade::Map::v_getattachedlines(int v)
{
	vector<int> ret;

	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v || lines[l]->vertex2 == v)
		{
			if (vector_exists(ret, l))
				ret.push_back(l);
		}
	}

	return ret;
}

void Slade::Map::l_setsector(int l, int side, int sector)
{
	if (side == 1)
	{
		if (sector == -1)
		{
			lines[l]->side1 = -1;
			return;
		}

		if (lines[l]->side1 == -1)
			lines[l]->side1 = add_side();

		sides[lines[l]->side1]->sector = sector;
	}
	else
	{
		if (sector == -1)
		{
			lines[l]->side2 = -1;
			lines[l]->clear_flag(LINE_TWOSIDED);
			lines[l]->set_flag(LINE_IMPASSIBLE);
			return;
		}

		if (lines[l]->side2 == -1)
		{
			lines[l]->side2 = add_side();
			lines[l]->set_flag(LINE_TWOSIDED);
			lines[l]->clear_flag(LINE_IMPASSIBLE);
			l_setmidtex(l, 1, "-");
		}

		sides[lines[l]->side2]->sector = sector;
	}

	change_level(MC_NODE_REBUILD);
}

int	Slade::Map::l_getindex(linedef_t* line)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l] == line)
			return l;
	}

	return -1;
}

sidedef_t* Slade::Map::l_getside(int l, int side)
{
	if (side == 1)
	{
		if (map.lines[l]->side1 == -1)
			return NULL;
		else
			return map.sides[map.lines[l]->side1];
	}
	else
	{
		if (map.lines[l]->side2 == -1)
			return NULL;
		else
			return map.sides[map.lines[l]->side2];
	}
}

point2_t Slade::Map::l_getmidpoint(int l)
{
	point2_t tl(map.verts[map.lines[l]->vertex1]->x,
				map.verts[map.lines[l]->vertex1]->y);
	point2_t br(map.verts[map.lines[l]->vertex2]->x,
				map.verts[map.lines[l]->vertex2]->y);

	return midpoint(tl, br);
}

void Slade::Map::l_getfromid(int line_id, vector<int> *vec)
{
	for (int l = 0; l < n_lines; l++)
	{
		if (lines[l]->type == 121)
		{
			if (line_id == lines[l]->args[0])
			{
				if (find(vec->begin(), vec->end(), l) == vec->end())
					vec->push_back(l);
			}
		}
	}
}

void Slade::Map::v_getattachedlines(int v, numlist_t* list)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v || lines[l]->vertex2 == v)
			list->add(l, false);
	}
}

void Slade::Map::change_level(BYTE flags)
{
	if (!(changed & MC_SAVE_NEEDED))
	{
		wxString title = editor_window->GetTitle();//gtk_window_get_title(GTK_WINDOW(editor_window));
		title += _T("*");
		editor_window->SetTitle(title);
	}

	if (flags & MC_NODE_REBUILD)
		flags |= MC_SSECTS|MC_LINES|MC_THINGS;

	changed |= MC_SAVE_NEEDED;	// If anything changes a save is needed
	changed |= flags;
}
