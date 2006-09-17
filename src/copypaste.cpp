
#include "main.h"
#include "copypaste.h"
//#include "undoredo.h"
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

		/*
		copy_type = COPY_ARCHITECTURE;

		// Get sides and sectors to copy
		vector<Sector*> copy_sectors;
		vector<Side*> copy_sides;
		for (int a = 0; a < copy_lines.size(); a++)
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
		for (int a = 0; a < copy_sectors.size(); a++)
		{
			Sector* sector = new Sector();
			sector->copy(copy_sectors[a]);
			sectors.push_back(sector);
		}

		// Copy the sides
		for (int a = 0; a < copy_sides.size(); a++)
		{
			Side* side = new Side();
			side->copy(copy_sides[a]);

			for (int a = 0; a < copy_sectors.size(); a++)
			{
				if (side->get_sector() == copy_sectors[a])
				{
					side->sector = a;
					break;
				}
			}

			sides.push_back(side);
		}

		// Get vertices to copy, and the average midpoint of the vertices
		vector<int> copy_verts;
		int min_x = 9999999;
		int min_y = 9999999;
		int max_x = -9999999;
		int max_y = -9999999;

		for (int a = 0; a < copy_lines.size(); a++)
		{
			vector_add_nodup(copy_verts, map.lines[copy_lines[a]]->vertex1);
			vector_add_nodup(copy_verts, map.lines[copy_lines[a]]->vertex2);

			int x1 = map.verts[map.lines[copy_lines[a]]->vertex1]->x;
			int y1 = map.verts[map.lines[copy_lines[a]]->vertex1]->y;
			int x2 = map.verts[map.lines[copy_lines[a]]->vertex2]->x;
			int y2 = map.verts[map.lines[copy_lines[a]]->vertex2]->y;

			if (x1 < min_x) min_x = x1;
			if (x1 > max_x) max_x = x1;
			if (y1 < min_y) min_y = y1;
			if (y1 > max_y) max_y = y1;

			if (x2 < min_x) min_x = x2;
			if (x2 > max_x) max_x = x2;
			if (y2 < min_y) min_y = y2;
			if (y2 > max_y) max_y = y2;
		}

		int mid_x = snap_to_grid(min_x + ((max_x - min_x) / 2));
		int mid_y = snap_to_grid(min_y + ((max_y - min_y) / 2));

		// Copy the vertices
		for (int a = 0; a < copy_verts.size(); a++)
		{
			vertex_t* vertex = new vertex_t();
			memcpy(vertex, map.verts[copy_verts[a]], sizeof(vertex_t));
			vertex->x -= mid_x;
			vertex->y -= mid_y;
			verts.push_back(vertex);
		}

		// Finally, copy the lines
		for (int a = 0; a < copy_lines.size(); a++)
		{
			linedef_t* line = new linedef_t();
			memcpy(line, map.lines[copy_lines[a]], sizeof(linedef_t));

			// Get relative sides
			bool s1 = false;
			bool s2 = false;
			for (int a = 0; a < copy_sides.size(); a++)
			{
				if (line->side1 == copy_sides[a] && !s1)
				{
					s1 = true;
					line->side1 = a;
				}

				if (line->side2 == copy_sides[a] && !s2)
				{
					s2 = true;
					line->side2 = a;
				}
			}

			// Get relative verts
			bool v1 = false;
			bool v2 = false;
			for (int a = 0; a < copy_verts.size(); a++)
			{
				if (line->vertex1 == copy_verts[a] && !v1)
				{
					v1 = true;
					line->vertex1 = a;
				}

				if (line->vertex2 == copy_verts[a] && !v2)
				{
					v2 = true;
					line->vertex2 = a;
				}
			}

			lines.push_back(line);
		}
		*/
	}
}

void Clipboard::Paste()
{
	if (copy_type == COPY_THINGS && things.size() > 0)
	{
		//make_backup(false, false, false, false, true);
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

	/*
	if (copy_type == COPY_ARCHITECTURE && lines.size() > 0)
	{
		int vert_offset = map.n_verts;
		int side_offset = map.n_sides;
		int sect_offset = map.n_sectors;
		int line_offset = map.n_lines;

		make_backup(true, true, true, true, false);

		// Add vertices
		for (int a = 0; a < verts.size(); a++)
		{
			int x = snap_to_grid(m_x(mouse.x)) + verts[a]->x;
			int y = snap_to_grid(-m_y(mouse.y)) + verts[a]->y;
			map.add_vertex(x, y);
		}

		// Add sectors
		for (int a = 0; a < sectors.size(); a++)
		{
			int s = map.add_sector();
			memcpy(map.sectors[s], sectors[a], sizeof(sector_t));
		}

		// Add sides
		for (int a = 0; a < sides.size(); a++)
		{
			int s = map.add_side();
			memcpy(map.sides[s], sides[a], sizeof(sidedef_t));
			
			if (sides[a]->sector != -1)
				map.sides[s]->sector = sides[a]->sector + sect_offset;
		}

		// Add lines
		for (int a = 0; a < lines.size(); a++)
		{
			int l = map.add_line(0, 0);
			memcpy(map.lines[l], lines[a], sizeof(linedef_t));

			map.lines[l]->vertex1 = vert_offset + lines[a]->vertex1;
			map.lines[l]->vertex2 = vert_offset + lines[a]->vertex2;

			if (lines[a]->side1 != -1)
				map.lines[l]->side1 = lines[a]->side1 + side_offset;

			if (lines[a]->side2 != -1)
				map.lines[l]->side2 = lines[a]->side2 + side_offset;
		}

		merge_verts();

		for (int a = vert_offset; a < map.n_verts; a++)
		{
			int split = check_vertex_split(a);
			if (split != -1)
				map.l_split(split, a);
		}

		vector<int> lines;
		for (int a = line_offset; a < map.n_lines-1; a++)
			lines.push_back(a);

		if (check_overlapping_lines(lines))
		{
			while(merge_overlapping_lines(lines))
			{
				lines.clear();
				for (int a = line_offset; a < map.n_lines-1; a++)
					lines.push_back(a);
			}
		}

		for (int a = line_offset; a < map.n_lines; a++)
			line_correct_references(a);

		remove_unused_sectors();
	}
	*/
}

void Clipboard::DrawPaste()
{
	if (copy_type == COPY_THINGS && things.size() > 0)
	{
		Thing* t = new Thing();

		for (unsigned int a = 0; a < things.size(); a++)
		{
			t->copy(things[a]);
			t->set_pos(snap_to_grid(mouse_pos(true).x) + t->pos().x,
						snap_to_grid(mouse_pos(true).y) + t->pos().y);
			t->draw(rect_t(0, 0, 65535, 65535, RECT_CENTER));
			t->draw(rect_t(0, 0, 65535, 65535, RECT_CENTER), MISTYLE_MOVING);
		}

		delete t;
	}

	/*
	if (copy_type == COPY_ARCHITECTURE && lines.size() > 0)
	{
		for (int a = 0; a < lines.size(); a++)
		{
			int x = snap_to_grid(m_x(mouse.x));
			int y = -snap_to_grid(-m_y(mouse.y));

			int x1 = s_x(x + verts[lines[a]->vertex1]->x);
			int y1 = s_y(y - verts[lines[a]->vertex1]->y);
			int x2 = s_x(x + verts[lines[a]->vertex2]->x);
			int y2 = s_y(y - verts[lines[a]->vertex2]->y);

			glLineWidth(line_size);
			draw_line(rect_t(x1, y1, x2, y2), col_line_solid, line_aa, true);
			glLineWidth(moving_size);
			draw_line(rect_t(x1, y1, x2, y2), col_moving, line_aa, true);
			glLineWidth(line_size);
		}
	}
	*/
}
