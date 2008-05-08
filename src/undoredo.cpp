// <<--------------------------------------------->>
// << SLADE (SlayeR's 'LeetAss Doom Editor)       >>
// << By Simon Judd, 2004-05                      >>
// << ------------------------------------------- >>
// << <undoredo.cpp>                              >>
// << The undo/redo system                        >>
// <<--------------------------------------------->>

// Includes ------------------------------------- >>
#include "main.h"
#include "map.h"
#include "undoredo.h"

// Variables ------------------------------------ >>
backup_t	**undo_list = NULL;
int			n_backups = 0;

CVAR(Int, max_undo_levels, 10, CVAR_SAVE)

// External Variables --------------------------- >>


backup_t::backup_t(bool lines, bool sides, bool vertices, bool sectors, bool things)
{
	this->lines = lines;
	this->sides = sides;
	this->vertices = vertices;
	this->sectors = sectors;
	this->things = things;

	n_lines = n_sides = n_sectors = n_vertices = n_things = -1;

	if (lines)
	{
		n_lines = map.n_lines;
		c_lines = (linedef_t *)malloc(n_lines * sizeof(linedef_t));

		for (DWORD a = 0; a < n_lines; a++)
			memcpy(&c_lines[a], map.lines[a], sizeof(linedef_t));
	}

	if (sides)
	{
		n_sides = map.n_sides;
		c_sides = (sidedef_t *)malloc(n_sides * sizeof(sidedef_t));

		for (DWORD a = 0; a < n_sides; a++)
			memcpy(&c_sides[a], map.sides[a], sizeof(sidedef_t));
	}

	if (vertices)
	{
		n_vertices = map.n_verts;
		c_vertices = (vertex_t *)malloc(n_vertices * sizeof(vertex_t));

		for (DWORD a = 0; a < n_vertices; a++)
			memcpy(&c_vertices[a], map.verts[a], sizeof(vertex_t));
	}

	if (sectors)
	{
		n_sectors = map.n_sectors;
		c_sectors = (sector_t *)malloc(n_sectors * sizeof(sector_t));

		for (DWORD a = 0; a < n_sectors; a++)
			memcpy(&c_sectors[a], map.sectors[a], sizeof(sector_t));
	}

	if (things)
	{
		n_things = map.n_things;
		c_things = (thing_t *)malloc(n_things * sizeof(thing_t));

		for (DWORD a = 0; a < n_things; a++)
			memcpy(&c_things[a], map.things[a], sizeof(thing_t));
	}
}

// make_backup: Creates a backup copy of map data depending on parameters,
// and adds it to the undo list
// ------------------------------------------------------------------- >>
void make_backup(bool lines, bool sides, bool vertices, bool sectors, bool things)
{
	if (n_backups == max_undo_levels)
	{
		undo_list[0]->clear();
		free(undo_list[0]);

		for (int a = 1; a < n_backups; a++)
			undo_list[a - 1] = undo_list[a];

		undo_list[n_backups - 1] = new backup_t(lines, sides, vertices, sectors, things);
	}
	else
	{
		n_backups++;
		undo_list = (backup_t **)realloc(undo_list, n_backups * sizeof(backup_t *));
		undo_list[n_backups - 1] = new backup_t(lines, sides, vertices, sectors, things);
	}
}

// undo: Loads map data from the last backup in the undo list
// ------------------------------------------------------- >>
void undo()
{
	if (n_backups == 0)
		return;

	int u = n_backups - 1;

	if (undo_list[u]->lines)
	{
		map.n_lines = undo_list[u]->n_lines;
		map.lines = (linedef_t **)realloc(map.lines, map.n_lines * sizeof(linedef_t *));

		for (DWORD a = 0; a < map.n_lines; a++)
		{
			map.lines[a] = new linedef_t();
			memcpy(map.lines[a], &undo_list[u]->c_lines[a], sizeof(linedef_t));
		}
	}

	if (undo_list[u]->sides)
	{
		map.n_sides = undo_list[u]->n_sides;
		map.sides = (sidedef_t **)realloc(map.sides, map.n_sides * sizeof(sidedef_t *));

		for (DWORD a = 0; a < map.n_sides; a++)
		{
			map.sides[a] = new sidedef_t();
			memcpy(map.sides[a], &undo_list[u]->c_sides[a], sizeof(sidedef_t));
		}
	}

	if (undo_list[u]->vertices)
	{
		map.n_verts = undo_list[u]->n_vertices;
		map.verts = (vertex_t **)realloc(map.verts, map.n_verts * sizeof(vertex_t *));

		for (DWORD a = 0; a < map.n_verts; a++)
		{
			map.verts[a] = new vertex_t();
			memcpy(map.verts[a], &undo_list[u]->c_vertices[a], sizeof(vertex_t));
		}
	}

	if (undo_list[u]->sectors)
	{
		map.n_sectors = undo_list[u]->n_sectors;
		map.sectors = (sector_t **)realloc(map.sectors, map.n_sectors * sizeof(sector_t *));

		for (DWORD a = 0; a < map.n_sectors; a++)
		{
			map.sectors[a] = new sector_t();
			memcpy(map.sectors[a], &undo_list[u]->c_sectors[a], sizeof(sector_t));
		}
	}

	if (undo_list[u]->things)
	{
		map.n_things = undo_list[u]->n_things;
		map.things = (thing_t **)realloc(map.things, map.n_things * sizeof(thing_t *));

		for (DWORD a = 0; a < map.n_things; a++)
		{
			map.things[a] = new thing_t();
			memcpy(map.things[a], &undo_list[u]->c_things[a], sizeof(thing_t));
		}

		//update_map_things();
	}

	// No redo for now
	n_backups--;
	undo_list[n_backups]->clear();
	undo_list = (backup_t **)realloc(undo_list, n_backups * sizeof(backup_t *));
}
