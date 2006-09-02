
#include "main.h"
#include "copypaste.h"
//#include "undoredo.h"
#include "draw.h"
#include "edit_misc.h"
#include "doom_map.h"
#include "dm_vertex.h"
#include "dm_line.h"
#include "dm_side.h"
#include "dm_sector.h"
#include "dm_thing.h"

Clipboard clipboard;

extern int edit_mode;

extern rgba_t col_line_solid;
extern rgba_t col_moving;
extern float moving_size;

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
	for (int a = 0; a < things.size(); a++)
		delete things[a];

	things.clear();

	for (int a = 0; a < lines.size(); a++)
		delete lines[a];

	lines.clear();

	for (int a = 0; a < sides.size(); a++)
		delete sides[a];

	sides.clear();

	for (int a = 0; a < sectors.size(); a++)
		delete sectors[a];

	sectors.clear();

	for (int a = 0; a < verts.size(); a++)
		delete verts[a];

	verts.clear();
}

void Clipboard::Copy()
{
	/*
	if (selected_items.size() == 0 && hilight_item == -1)
		return;

	// Clear the clipboard
	Clear();

	// Thing copy
	if (edit_mode == 3)
	{
		copy_type = COPY_THINGS;

		if (selected_items.size() > 0)
		{
			// Get middle point
			int min_x = map.things[selected_items[0]]->x;
			int min_y = map.things[selected_items[0]]->y;
			int max_x = map.things[selected_items[0]]->x;
			int max_y = map.things[selected_items[0]]->y;

			for (int a = 0; a < selected_items.size(); a++)
			{
				int x = map.things[selected_items[a]]->x;
				int y = map.things[selected_items[a]]->y;

				if (x < min_x) min_x = x;
				if (x > max_x) max_x = x;
				if (y < min_y) min_y = y;
				if (y > max_y) max_y = y;
			}

			int mid_x = min_x + ((max_x - min_x) / 2);
			int mid_y = min_y + ((max_y - min_y) / 2);

			// Add things
			for (int a = 0; a < selected_items.size(); a++)
			{
				thing_t *t = new thing_t();
				memcpy(t, map.things[selected_items[a]], sizeof(thing_t));

				t->x -= mid_x;
				t->y -= mid_y;

				things.push_back(t);
			}
		}
		else
		{
			thing_t *t = new thing_t();
			memcpy(t, map.things[hilight_item], sizeof(thing_t));
			t->x = 0;
			t->y = 0;
			things.push_back(t);
		}
	}

	// Architecture copy
	if (edit_mode > 0 && edit_mode < 3)
	{
		vector<int> copy_lines;

		if (edit_mode == 1)
		{
			if (selected_items.size() > 0)
			{
				for (int a = 0; a < selected_items.size(); a++)
					copy_lines.push_back(selected_items[a]);
			}
			else
				copy_lines.push_back(hilight_item);
		}

		if (edit_mode == 2)
		{
			if (selected_items.size() > 0)
			{
				for (int l = 0; l < map.n_lines; l++)
				{
					if (vector_exists(selected_items, map.l_getsector1(l)))
					{
						copy_lines.push_back(l);
						continue;
					}

					if (vector_exists(selected_items, map.l_getsector2(l)))
					{
						copy_lines.push_back(l);
						continue;
					}
				}
			}
			else
			{
				for (int l = 0; l < map.n_lines; l++)
				{
					if (map.l_getsector1(l) == hilight_item)
					{
						copy_lines.push_back(l);
						continue;
					}

					if (map.l_getsector2(l) == hilight_item)
					{
						copy_lines.push_back(l);
						continue;
					}
				}
			}
		}

		if (copy_lines.size() == 0)
			return;

		copy_type = COPY_ARCHITECTURE;

		// Get sectors to copy
		vector<int> copy_sectors;
		for (int a = 0; a < copy_lines.size(); a++)
		{
			if (map.l_getsector1(copy_lines[a]) != -1)
				vector_add_nodup(copy_sectors, map.l_getsector1(copy_lines[a]));

			if (map.l_getsector2(copy_lines[a]) != -1)
				vector_add_nodup(copy_sectors, map.l_getsector2(copy_lines[a]));
		}

		// Copy the sectors
		for (int a = 0; a < copy_sectors.size(); a++)
		{
			sector_t* sector = new sector_t();
			memcpy(sector, map.sectors[copy_sectors[a]], sizeof(sector_t));
			sectors.push_back(sector);
		}

		// Get sides to copy
		vector<int> copy_sides;
		for (int a = 0; a < copy_lines.size(); a++)
		{
			if (map.lines[copy_lines[a]]->side1 != -1)
				copy_sides.push_back(map.lines[copy_lines[a]]->side1);

			if (map.lines[copy_lines[a]]->side2 != -1)
				copy_sides.push_back(map.lines[copy_lines[a]]->side2);
		}

		// Copy the sides
		for (int a = 0; a < copy_sides.size(); a++)
		{
			sidedef_t* side = new sidedef_t();
			memcpy(side, map.sides[copy_sides[a]], sizeof(sidedef_t));

			for (int a = 0; a < copy_sectors.size(); a++)
			{
				if (side->sector == copy_sectors[a])
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
	}
	*/
}

void Clipboard::Paste()
{
	/*
	if (copy_type == COPY_THINGS && things.size() > 0)
	{
		make_backup(false, false, false, false, true);
		map.change_level(MC_THINGS);

		for (int a = 0; a < things.size(); a++)
		{
			int t = map.add_thing(0, 0, *things[a]);
			memcpy(map.things[t], things[a], sizeof(thing_t));

			map.things[t]->x = snap_to_grid(m_x(mouse.x) + things[a]->x);
			map.things[t]->y = snap_to_grid(-m_y(mouse.y) + things[a]->y);
		}
	}

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
	/*
	if (copy_type == COPY_THINGS && things.size() > 0)
	{
		thing_t* t = new thing_t();

		for (int a = 0; a < things.size(); a++)
		{
			memcpy(t, things[a], sizeof(thing_t));
			t->x = snap_to_grid(m_x(mouse.x) + things[a]->x);
			t->y = snap_to_grid(-m_y(mouse.y) + things[a]->y);
			draw_thing(t, true);
		}

		delete t;
	}

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
