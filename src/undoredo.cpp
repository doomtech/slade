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

// Variables ------------------------------------ >>
BackupManager bm;

CVAR(Int, max_undo_size, 10485760, CVAR_SAVE)

// External Variables --------------------------- >>
extern DoomMap d_map;


Backup::Backup()
{
	flags = 0;
}

Backup::~Backup()
{
	for (DWORD a = 0; a < verts.size(); a++)
		delete verts[a];

	for (DWORD a = 0; a < lines.size(); a++)
		delete lines[a];

	for (DWORD a = 0; a < sides.size(); a++)
		delete sides[a];

	for (DWORD a = 0; a < sectors.size(); a++)
		delete sectors[a];

	for (DWORD a = 0; a < things.size(); a++)
		delete things[a];
}

void Backup::do_backup(WORD flags)
{
	this->flags = flags;
	d_map.update_indices(MTYPE_SECTOR|MTYPE_SIDE|MTYPE_VERTEX);

	if (flags & BKUP_VERTS)
	{
		for (DWORD a = 0; a < d_map.n_verts(); a++)
		{
			Vertex *v = new Vertex();
			v->set_pos(d_map.vertex(a)->x_pos(), d_map.vertex(a)->y_pos());
			verts.push_back(v);
		}
	}

	if (flags & BKUP_SECTORS)
	{
		for (DWORD a = 0; a < d_map.n_sectors(); a++)
		{
			Sector *s = new Sector();
			s->copy(d_map.sector(a));
			sectors.push_back(s);
		}
	}

	if (flags & BKUP_SIDES)
	{
		for (DWORD a = 0; a < d_map.n_sides(); a++)
		{
			bkup_side_t *s = new bkup_side_t;
			s->side.copy(d_map.side(a));
			s->sector = d_map.index(d_map.side(a)->get_sector());
			sides.push_back(s);
		}
	}

	if (flags & BKUP_LINES)
	{
		for (DWORD a = 0; a < d_map.n_lines(); a++)
		{
			bkup_line_t *l = new bkup_line_t;
			l->line.copy(d_map.line(a));
			l->s1 = d_map.index(d_map.line(a)->side1());
			l->s2 = d_map.index(d_map.line(a)->side2());
			l->v1 = d_map.index(d_map.line(a)->vertex1());
			l->v2 = d_map.index(d_map.line(a)->vertex2());
			lines.push_back(l);
		}
	}

	if (flags & BKUP_THINGS)
	{
		for (DWORD a = 0; a < d_map.n_things(); a++)
		{
			Thing *t = new Thing();
			t->copy(d_map.thing(a));
			things.push_back(t);
		}
	}
}

DWORD Backup::get_size()
{
	return (DWORD)(verts.size() * sizeof(Vertex) +
					lines.size() * sizeof(Line) +
					sides.size() * sizeof(Side) +
					sectors.size() * sizeof(Sector) +
					things.size() * sizeof(Thing));
}

void Backup::do_undo()
{
	if (flags & BKUP_VERTS)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_verts() == verts.size())
			{
				for (DWORD a = 0; a < d_map.n_verts(); a++)
					d_map.vertex(a)->set_pos(verts[a]->x_pos(), verts[a]->y_pos());
			}
		}
		else
		{
			d_map.clear_verts();
			for (DWORD a = 0; a < verts.size(); a++)
			{
				Vertex *i = new Vertex();
				memcpy(i, verts[a], sizeof(Vertex));
				d_map.add_vertex(i);
			}
		}
	}

	if (flags & BKUP_SECTORS)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_sectors() == sectors.size())
			{
				for (DWORD a = 0; a < d_map.n_sectors(); a++)
					d_map.sector(a)->copy(sectors[a]);
			}
		}
		else
		{
			d_map.clear_sectors();
			for (DWORD a = 0; a < sectors.size(); a++)
			{
				Sector *i = new Sector();
				memcpy(i, sectors[a], sizeof(Sector));
				d_map.add_sector(i);
			}
		}
	}

	if (flags & BKUP_SIDES)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_sides() == sides.size())
			{
				for (DWORD a = 0; a < d_map.n_sides(); a++)
					d_map.side(a)->copy(&sides[a]->side);
			}
		}
		else
		{
			d_map.clear_sides();
			for (DWORD a = 0; a < sides.size(); a++)
			{
				Side *i = new Side();
				memcpy(i, &sides[a]->side, sizeof(Side));
				d_map.add_side(i);
				i->set_sector(d_map.sector(sides[a]->sector));
			}
		}
	}

	if (flags & BKUP_LINES)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_lines() == lines.size())
			{
				for (DWORD a = 0; a < d_map.n_lines(); a++)
					d_map.line(a)->copy(&lines[a]->line);
			}
		}
		else
		{
			d_map.clear_lines();
			for (DWORD a = 0; a < lines.size(); a++)
			{
				Line *i = new Line();
				memcpy(i, &lines[a]->line, sizeof(Line));
				d_map.add_line(i);
				i->set_vertex1(d_map.vertex(lines[a]->v1));
				i->set_vertex2(d_map.vertex(lines[a]->v2));
				i->set_side1(d_map.side(lines[a]->s1));
				i->set_side2(d_map.side(lines[a]->s2));
			}
		}
	}

	if (flags & BKUP_THINGS)
	{
		if (flags & BKUP_MODIFY)
		{
			if (d_map.n_things() == things.size())
			{
				for (DWORD a = 0; a < d_map.n_things(); a++)
					d_map.thing(a)->copy(things[a]);
			}
		}
		else
		{
			d_map.clear_things();
			for (DWORD a = 0; a < things.size(); a++)
			{
				Thing *i = new Thing();
				memcpy(i, things[a], sizeof(Thing));
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
	backups.push_back(b);

	DWORD size = 0;
	for (DWORD a = 0; a < backups.size(); a++)
		size += backups[a]->get_size();

	if (size > max_undo_size)
		backups.erase(backups.begin());
}

void BackupManager::undo(bool m3d)
{
	if (backups.size() == 0)
		return;

	if (m3d && !(backups.back()->check_flag(BKUP_3DMODE)))
	{
		message_box("ghgggg");
		return;
	}

	backups.back()->do_undo();
	backups.pop_back();
}

void BackupManager::clear_3d()
{
	for (DWORD a = 0; a < backups.size(); a++)
	{
		if (backups[a]->check_flag(BKUP_3DMODE))
		{
			backups.erase(backups.begin() + a);
			a--;
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
