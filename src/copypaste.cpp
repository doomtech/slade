
#include "main.h"
#include "copypaste.h"
#include "undoredo.h"
#include "draw.h"
#include "edit_misc.h"
#include "edit.h"
#include "doom_map.h"
#include "dm_vertex.h"
#include "dm_line.h"
#include "dm_side.h"
#include "dm_sector.h"
#include "dm_thing.h"
#include "input.h"

Clipboard clipboard;

extern int edit_mode;

extern rgba_t col_line_solid;
extern rgba_t col_moving;
extern float moving_size;
extern DoomMap d_map;

EXTERN_CVAR(Float, line_size)
EXTERN_CVAR(Bool, line_aa)

Clipboard::Clipboard()
{
}

Clipboard::~Clipboard()
{
}

void Clipboard::Clear()
{
	for (unsigned int a = 0; a < things.size(); a++)
		delete things[a];

	things.clear();

	for (unsigned int a = 0; a < lines.size(); a++)
		delete lines[a];

	lines.clear();

	for (unsigned int a = 0; a < sides.size(); a++)
		delete sides[a];

	sides.clear();

	for (unsigned int a = 0; a < sectors.size(); a++)
		delete sectors[a];

	sectors.clear();

	for (unsigned int a = 0; a < verts.size(); a++)
		delete verts[a];

	verts.clear();
}

void Clipboard::Copy()
{
	if (d_map.selection().size() == 0 && d_map.hilight() == -1)
		return;

	// Clear the clipboard
	Clear();

	// Thing copy
	if (edit_mode == 3)
	{
		copy_type = COPY_THINGS;

		vector<Thing*> list;
		d_map.get_selection(list, true);

		// Get middle point
		int min_x = list[0]->pos().x;
		int min_y = list[0]->pos().y;
		int max_x = min_x;
		int max_y = min_y;

		for (unsigned int a = 0; a < list.size(); a++)
		{
			int x = list[a]->pos().x;
			int y = list[a]->pos().y;

			if (x < min_x) min_x = x;
			if (x > max_x) max_x = x;
			if (y < min_y) min_y = y;
			if (y > max_y) max_y = y;
		}

		int mid_x = min_x + ((max_x - min_x) / 2);
		int mid_y = min_y + ((max_y - min_y) / 2);

		// Add things
		for (unsigned int a = 0; a < list.size(); a++)
		{
			Thing *t = new Thing();
			t->copy(list[a]);
			t->set_pos(t->pos().x - mid_x, t->pos().y - mid_y);
			things.push_back(t);
		}
	}

	// Architecture copy
	if (edit_mode > 0 && edit_mode < 3)
	{
		//vector<int> copy_lines;
		vector<Line*> copy_lines;

		if (edit_mode == 1)
			d_map.get_selection(copy_lines, true);

		if (edit_mode == 2)
		{
			vector<Sector*> selection;
			d_map.get_selection(selection, true);

			if (selection.size() > 0)
			{
				for (unsigned int a = 0; a < d_map.n_lines(); a++)
				{
					if (vector_exists(selection, d_map.line(a)->side1()->get_sector()))
					{
						copy_lines.push_back(d_map.line(a));
						continue;
					}

					if (vector_exists(selection, d_map.line(a)->side2()->get_sector()))
					{
						copy_lines.push_back(d_map.line(a));
						continue;
					}
				}
			}
		}

		if (copy_lines.size() == 0)
			return;

		copy_type = COPY_ARCHITECTURE;

		// Get sides and sectors to copy
		vector<Sector*> copy_sectors;
		vector<Side*> copy_sides;
		for (DWORD a = 0; a < copy_lines.size(); a++)
		{
			if (copy_lines[a]->sector_index(true) != -1)
			{
				vector_add_nodup(copy_sectors, copy_lines[a]->side1()->get_sector());
				copy_sides.push_back(copy_lines[a]->side1());
			}

			if (copy_lines[a]->sector_index(false) != -1)
			{
				vector_add_nodup(copy_sectors, copy_lines[a]->side2()->get_sector());
				copy_sides.push_back(copy_lines[a]->side2());
			}
		}

		// Copy the sectors
		for (DWORD a = 0; a < copy_sectors.size(); a++)
		{
			Sector* sector = new Sector();
			sector->copy(copy_sectors[a]);
			sectors.push_back(sector);
		}

		// Copy the sides
		for (DWORD a = 0; a < copy_sides.size(); a++)
		{
			Side* side = new Side();
			side->copy(copy_sides[a]);

			// Get relative sector
			for (DWORD b = 0; b < copy_sectors.size(); b++)
			{
				if (copy_sides[a]->get_sector() == copy_sectors[b])
				{
					side->set_sector(sectors[b]);
					break;
				}
			}

			sides.push_back(side);
		}

		// Get vertices to copy, and the average midpoint of the vertices
		vector<Vertex*> copy_verts;
		int min_x = 9999999;
		int min_y = 9999999;
		int max_x = -9999999;
		int max_y = -9999999;

		for (DWORD a = 0; a < copy_lines.size(); a++)
		{
			Vertex *v1 = copy_lines[a]->vertex1();
			Vertex *v2 = copy_lines[a]->vertex2();
			vector_add_nodup(copy_verts, v1);
			vector_add_nodup(copy_verts, v2);

			if (v1->x_pos() < min_x) min_x = v1->x_pos();
			if (v1->x_pos() > max_x) max_x = v1->x_pos();
			if (v1->y_pos() < min_y) min_y = v1->y_pos();
			if (v1->y_pos() > max_y) max_y = v1->y_pos();

			if (v2->x_pos() < min_x) min_x = v2->x_pos();
			if (v2->x_pos() > max_x) max_x = v2->x_pos();
			if (v2->y_pos() < min_y) min_y = v2->y_pos();
			if (v2->y_pos() > max_y) max_y = v2->y_pos();
		}

		int mid_x = snap_to_grid(min_x + ((max_x - min_x) / 2));
		int mid_y = snap_to_grid(min_y + ((max_y - min_y) / 2));

		// Copy the vertices
		for (DWORD a = 0; a < copy_verts.size(); a++)
		{
			Vertex* vertex = new Vertex();
			vertex->set_pos(copy_verts[a]->x_pos() - mid_x, copy_verts[a]->y_pos() - mid_y);
			verts.push_back(vertex);
		}

		// Finally, copy the lines
		for (DWORD a = 0; a < copy_lines.size(); a++)
		{
			Line* line = new Line();
			line->copy(copy_lines[a]);

			// Get relative sides
			bool s1 = false;
			bool s2 = false;
			for (DWORD b = 0; b < copy_sides.size(); b++)
			{
				if (copy_lines[a]->side1() == copy_sides[b] && !s1)
				{
					line->set_side1(sides[b]);
					s1 = true;
				}

				if (copy_lines[a]->side2() == copy_sides[b] && !s2)
				{
					line->set_side2(sides[b]);
					s2 = true;
				}

				if (s1 && s2)
					break;
			}

			// Get relative verts
			bool v1 = false;
			bool v2 = false;
			for (DWORD b = 0; b < copy_verts.size(); b++)
			{
				if (copy_lines[a]->vertex1() == copy_verts[b] && !v1)
				{
					line->set_vertex1(verts[b]);
					v1 = true;
				}

				if (copy_lines[a]->vertex2() == copy_verts[b] && !v2)
				{
					line->set_vertex2(verts[b]);
					v2 = true;
				}

				if (v1 && v2)
					break;
			}

			lines.push_back(line);
		}
	}
}

void Clipboard::Paste()
{
	if (copy_type == COPY_THINGS && things.size() > 0)
	{
		make_backup(false, false, false, false, true);
		d_map.change_level(MC_THINGS);

		for (unsigned int a = 0; a < things.size(); a++)
		{
			Thing* t = new Thing();
			t->copy(things[a]);
			t->set_pos(snap_to_grid(mouse_pos(true).x) + t->pos().x,
						snap_to_grid(mouse_pos(true).y) + t->pos().y);

			d_map.add_thing(t);
		}
	}

	if (copy_type == COPY_ARCHITECTURE && lines.size() > 0)
	{
		make_backup(true, true, true, true, false);

		// Add vertices
		vector<Vertex*> new_verts;
		for (DWORD a = 0; a < verts.size(); a++)
		{
			Vertex* v = new Vertex();
			v->set_pos(snap_to_grid(mouse_pos(true).x) + verts[a]->x_pos(),
							snap_to_grid(mouse_pos(true).y) + verts[a]->y_pos());
			d_map.add_vertex(v);
			new_verts.push_back(v);
		}

		// Add sectors
		vector<Sector*> new_sectors;
		for (DWORD a = 0; a < sectors.size(); a++)
		{
			Sector *s = new Sector();
			s->copy(sectors[a]);
			d_map.add_sector(s);
			new_sectors.push_back(s);
		}

		// Add sides
		vector<Side*> new_sides;
		for (DWORD a = 0; a < sides.size(); a++)
		{
			Side *s = new Side();
			s->copy(sides[a]);
			d_map.add_side(s);
			new_sides.push_back(s);

			// Set relative sector
			for (DWORD b = 0; b < sectors.size(); b++)
			{
				if (sides[a]->get_sector() == sectors[b])
				{
					s->set_sector(new_sectors[b]);
					break;
				}
			}
		}

		// Add lines
		vector<Line*> new_lines;
		for (DWORD a = 0; a < lines.size(); a++)
		{
			Line *l = new Line();
			l->copy(lines[a]);
			d_map.add_line(l);
			new_lines.push_back(l);

			// Set relative sides
			bool s1 = false;
			bool s2 = false;
			for (DWORD b = 0; b < sides.size(); b++)
			{
				if (lines[a]->side1() == sides[b] && !s1)
				{
					l->set_side1(new_sides[b]);
					s1 = true;
				}

				if (lines[a]->side2() == sides[b] && !s2)
				{
					l->set_side2(new_sides[b]);
					s2 = true;
				}

				if (s1 && s2)
					break;
			}

			// Set relative vertices
			bool v1 = false;
			bool v2 = false;
			for (DWORD b = 0; b < verts.size(); b++)
			{
				if (lines[a]->vertex1() == verts[b] && !v1)
				{
					l->set_vertex1(new_verts[b]);
					v1 = true;
				}

				if (lines[a]->vertex2() == verts[b] && !v2)
				{
					l->set_vertex2(new_verts[b]);
					v2 = true;
				}

				if (v1 && v2)
					break;
			}
		}

		d_map.update_indices(MTYPE_LINE|MTYPE_SIDE|MTYPE_VERTEX|MTYPE_SECTOR);

		// Merge vertices
		for (DWORD a = 0; a < new_verts.size(); a++)
			d_map.merge_under_vertex(new_verts[a]);

		// Split lines
		for (DWORD a = 0; a < new_verts.size(); a++)
			d_map.check_split(new_verts[a]);

		// Merge lines
		d_map.remove_overlapping_lines(new_lines, true);

		d_map.update_indices(MTYPE_LINE|MTYPE_SIDE|MTYPE_VERTEX|MTYPE_SECTOR);

		// Correct refs (maybe i'll change this to only correct refs on the lines/sides that need it)
		for (DWORD a = 0; a < new_lines.size(); a++)
			line_correct_references(d_map.index(new_lines[a]));
	}
}

void Clipboard::DrawPaste()
{
	if (copy_type == COPY_THINGS && things.size() > 0)
	{
		Thing* t = new Thing();

		for (DWORD a = 0; a < things.size(); a++)
		{
			t->copy(things[a]);
			t->set_pos(snap_to_grid(mouse_pos(true).x) + t->pos().x,
						snap_to_grid(mouse_pos(true).y) + t->pos().y);
			t->draw(rect_t(0, 0, 65535, 65535, RECT_CENTER));
			t->draw(rect_t(0, 0, 65535, 65535, RECT_CENTER), MISTYLE_MOVING);
		}

		delete t;
	}

	if (copy_type == COPY_ARCHITECTURE && lines.size() > 0)
	{
		Line* drawline = new Line();
		Vertex* v1 = new Vertex();
		Vertex* v2 = new Vertex();
		drawline->set_vertex1(v1);
		drawline->set_vertex2(v2);

		for (DWORD a = 0; a < lines.size(); a++)
		{
			drawline->copy(lines[a]);
			drawline->set_side1(lines[a]->side1());
			drawline->set_side2(lines[a]->side2());
			v1->set_pos(snap_to_grid(mouse_pos(true).x) + lines[a]->vertex1()->x_pos(),
						snap_to_grid(mouse_pos(true).y) + lines[a]->vertex1()->y_pos());
			v2->set_pos(snap_to_grid(mouse_pos(true).x) + lines[a]->vertex2()->x_pos(),
						snap_to_grid(mouse_pos(true).y) + lines[a]->vertex2()->y_pos());

			drawline->draw(edit_mode == 1);
			drawline->draw(edit_mode == 1, MISTYLE_MOVING);
		}

		delete drawline;
	}
}
