// <<--------------------------------------------->>
// << SLADE (SlayeR's 'LeetAss Doom Editor)       >>
// << By Simon Judd, 2004-05                      >>
// << ------------------------------------------- >>
// << <undoredo.cpp>                              >>
// << The undo/redo system                        >>
// <<--------------------------------------------->>

// Includes ------------------------------------- >>
#include "main.h"
#include "doom_map.h"
#include "dm_vertex.h"
#include "dm_line.h"
#include "dm_side.h"
#include "dm_sector.h"
#include "dm_thing.h"
#include "undoredo.h"

#include <wx/stopwatch.h>

// Variables ------------------------------------ >>
BackupManager bm;

CVAR(Int, max_undo_size, 10485760, CVAR_SAVE)

// External Variables --------------------------- >>
extern DoomMap d_map;


Backup::Backup()
{
	flags = 0;
	n_lines = 0;
	n_verts = 0;
	n_sides = 0;
	n_sectors = 0;
	n_things = 0;
	lines = NULL;
	sides = NULL;
	verts = NULL;
	sectors = NULL;
	things = NULL;
}

Backup::~Backup()
{
	delete[] verts;
	delete[] lines;
	delete[] sides;
	delete[] sectors;
	delete[] things;
}

void Backup::do_backup(WORD flags)
{
	long t_update = 0;
	long t_verts = 0;
	long t_sectors = 0;
	long t_sides = 0;
	long t_lines = 0;
	long t_things = 0;
	this->flags = flags;

	wxStopWatch sw;
	d_map.update_indices(MTYPE_SECTOR|MTYPE_SIDE|MTYPE_VERTEX);
	t_update = sw.Time();
	sw.Start();

	if (flags & BKUP_VERTS)
	{
		n_verts = d_map.n_verts();
		verts = new Vertex[n_verts];

		for (DWORD a = 0; a < d_map.n_verts(); a++)
			verts[a].set_pos(d_map.vertex(a)->x_pos(), d_map.vertex(a)->y_pos());
	}
	t_verts = sw.Time();
	sw.Start();

	if (flags & BKUP_SECTORS)
	{
		n_sectors = d_map.n_sectors();
		sectors = new Sector[n_sectors];

		for (DWORD a = 0; a < d_map.n_sectors(); a++)
			sectors[a].copy(d_map.sector(a), false);
	}
	t_sectors = sw.Time();
	sw.Start();

	if (flags & BKUP_SIDES)
	{
		n_sides = d_map.n_sides();
		sides = new bkup_side_t[n_sides];

		for (DWORD a = 0; a < d_map.n_sides(); a++)
		{
			sides[a].side.copy(d_map.side(a), false);
			sides[a].sector = d_map.side(a)->get_sector()->get_index();
		}
	}
	t_sides = sw.Time();
	sw.Start();

	if (flags & BKUP_LINES)
	{
		n_lines = d_map.n_lines();
		lines = new bkup_line_t[n_lines];

		for (DWORD a = 0; a < d_map.n_lines(); a++)
		{
			lines[a].line.copy(d_map.line(a));
			lines[a].s1 = d_map.line(a)->side1()->get_index();
			lines[a].s2 = d_map.line(a)->side2()->get_index();
			lines[a].v1 = d_map.line(a)->vertex1()->get_index();
			lines[a].v2 = d_map.line(a)->vertex2()->get_index();
		}
	}
	t_lines = sw.Time();
	sw.Start();

	if (flags & BKUP_THINGS)
	{
		n_things = d_map.n_things();
		things = new Thing[n_things];

		for (DWORD a = 0; a < d_map.n_things(); a++)
			things[a].copy(d_map.thing(a));
	}
	t_things = sw.Time();

	log_message(s_fmt("update indices %d", t_update));
	log_message(s_fmt("verts %d", t_verts));
	log_message(s_fmt("sectors %d", t_sectors));
	log_message(s_fmt("sides %d", t_sides));
	log_message(s_fmt("lines %d", t_lines));
	log_message(s_fmt("things %d", t_things));
}

DWORD Backup::get_size()
{
	DWORD ret = (DWORD)(n_verts * sizeof(Vertex) +
					n_lines * sizeof(bkup_line_t) +
					n_sides * sizeof(bkup_side_t) +
					n_sectors * sizeof(Sector) +
					n_things * sizeof(Thing));

	log_message(s_fmt("backup size %d bytes", ret));
	return ret;
}

void Backup::do_undo()
{
	if (flags & BKUP_VERTS)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_verts() == n_verts)
			{
				for (DWORD a = 0; a < d_map.n_verts(); a++)
					d_map.vertex(a)->set_pos(verts[a].x_pos(), verts[a].y_pos());
			}
		}
		else
		{
			d_map.clear_verts();
			for (DWORD a = 0; a < n_verts; a++)
			{
				Vertex *i = new Vertex();
				i->set_pos(verts[a].x_pos(), verts[a].y_pos());
				d_map.add_vertex(i);
			}
		}
	}

	if (flags & BKUP_SECTORS)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_sectors() == n_sectors)
			{
				for (DWORD a = 0; a < d_map.n_sectors(); a++)
					d_map.sector(a)->copy(&sectors[a]);
			}
		}
		else
		{
			d_map.clear_sectors();
			for (DWORD a = 0; a < n_sectors; a++)
			{
				Sector *i = new Sector();
				i->copy(&sectors[a], false);
				d_map.add_sector(i);
			}
		}
	}

	if (flags & BKUP_SIDES)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_sides() == n_sides)
			{
				for (DWORD a = 0; a < d_map.n_sides(); a++)
					d_map.side(a)->copy(&sides[a].side);
			}
		}
		else
		{
			d_map.clear_sides();
			for (DWORD a = 0; a < n_sides; a++)
			{
				Side *i = new Side();
				i->copy(&sides[a].side, false);
				d_map.add_side(i);
				i->set_sector(d_map.sector(sides[a].sector));
			}
		}
	}

	if (flags & BKUP_LINES)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_lines() == n_lines)
			{
				for (DWORD a = 0; a < d_map.n_lines(); a++)
					d_map.line(a)->copy(&lines[a].line);
			}
		}
		else
		{
			d_map.clear_lines();
			for (DWORD a = 0; a < n_lines; a++)
			{
				Line *i = new Line();
				i->copy(&lines[a].line, false);
				d_map.add_line(i);
				i->set_vertex1(d_map.vertex(lines[a].v1));
				i->set_vertex2(d_map.vertex(lines[a].v2));
				i->set_side1(d_map.side(lines[a].s1, lines[a].s1 != -1));
				i->set_side2(d_map.side(lines[a].s2, lines[a].s2 != -1));
			}
		}
	}

	if (flags & BKUP_THINGS)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_things() == n_things)
			{
				for (DWORD a = 0; a < d_map.n_things(); a++)
					d_map.thing(a)->copy(&things[a]);
			}
		}
		else
		{
			d_map.clear_things();
			for (DWORD a = 0; a < n_things; a++)
			{
				Thing *i = new Thing();
				i->copy(&things[a]);
				d_map.add_thing(i);
			}
		}
	}
}

BackupManager::BackupManager()
{
}

BackupManager::~BackupManager()
{
}

void BackupManager::backup(BYTE flags)
{
	Backup *b = new Backup();
	b->do_backup(flags);
	backups = (Backup**)realloc(backups, sizeof(Backup*) * ++n_backups);
	backups[n_backups - 1] = b;

	DWORD size = 0;
	for (DWORD a = 0; a < n_backups; a++)
	{
		if (backups[a])
			size += backups[a]->get_size();
	}

	if (size > max_undo_size)
	{
		//message_box("max undo size reached");
		delete backups[0];
		for (int a = 0; a < n_backups-1; a++)
			backups[a] = backups[a+1];
		backups = (Backup**)realloc(backups, sizeof(Backup*) * --n_backups);
	}
}

void BackupManager::undo(bool m3d)
{
	if (n_backups == 0)
		return;

	if (m3d && !(backups[n_backups-1]->check_flag(BKUP_3DMODE)))
		return;

	backups[n_backups-1]->do_undo();
	delete backups[n_backups-1];
	backups = (Backup**)realloc(backups, sizeof(Backup*) * --n_backups);

	d_map.clear_selection();
	d_map.clear_hilight();
}

void BackupManager::clear_3d()
{
	for (DWORD a = 0; a < n_backups; a++)
	{
		if (backups[a]->check_flag(BKUP_3DMODE))
		{
			delete backups[a];
			for (int b = a; b < n_backups-1; b++)
			backups[b] = backups[b+1];
			backups = (Backup**)realloc(backups, sizeof(Backup*) * --n_backups);
		}
	}
}



void undo(bool m3d)
{
	bm.undo(m3d);
}

void make_backup(bool lines, bool sides, bool vertices, bool sectors, bool things, bool m3d, bool mod)
{
	WORD flags = 0;
	if (lines) flags |= BKUP_LINES;
	if (sides) flags |= BKUP_SIDES;
	if (vertices) flags |= BKUP_VERTS;
	if (sectors) flags |= BKUP_SECTORS;
	if (things) flags |= BKUP_THINGS;
	if (m3d) flags |= BKUP_3DMODE;
	if (mod) flags |= BKUP_MODIFY;

	make_backup(flags);
}

void make_backup(WORD flags)
{
	bm.backup(flags);
}
