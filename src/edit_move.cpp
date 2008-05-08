
#include "main.h"
#include "map.h"
#include "edit.h"
#include "undoredo.h"
#include "edit_move.h"
#include "checks.h"

// EXTERNAL VARIABLES -------------------- >>

extern int edit_mode, hilight_item;
extern movelist_t move_list;
extern vector<int> selected_items;
extern point2_t mouse;

// add_move_items: Adds selected or hilighted item(s) to the move list
// ---------------------------------------------------------------- >>
void add_move_items()
{
	fpoint2_t offset;
	bool	selection = !!selected_items.size();

	// Vertices
	if (edit_mode == 0)
	{
		// Move selected vertices
		for (int a = 0; a < selected_items.size(); a++)
		{
			int v = selected_items[a];

			offset.x = map.verts[v]->x - m_x(mouse.x);
			offset.y = -map.verts[v]->y - m_y(mouse.y);

			move_list.add(offset, v);
			move_list.cursor_snap = true;

			move_list.cursor_offset.set((m_x(mouse.x) - snap_to_grid(m_x(mouse.x))),
				(m_y(mouse.y) - snap_to_grid(m_y(mouse.y))));
		}

		if (selection)
			make_backup(true, true, true, false, false);
		
		// If no selection, but something is highlighted, move that
		if (!selection && hilight_item != -1)
		{
			make_backup(true, true, true, false, false);
			DWORD v = hilight_item;
			
			offset.x = map.verts[v]->x - m_x(mouse.x);
			offset.y = -map.verts[v]->y - m_y(mouse.y);
			
			move_list.add(offset, v);
			move_list.cursor_snap = false;
		}
	}
	
	// Lines
	if (edit_mode == 1)
	{
		for (int a = 0; a < selected_items.size(); a++)
		{
			int l = selected_items[a];
			rect_t r = map.l_getrect(l);

			// Add vertex 1
			offset.x = r.x1() - m_x(mouse.x);
			offset.y = -r.y1() - m_y(mouse.y);
			move_list.add(offset, map.lines[l]->vertex1);

			// Add vertex 2
			offset.x = r.x2() - m_x(mouse.x);
			offset.y = -r.y2() - m_y(mouse.y);
			move_list.add(offset, map.lines[l]->vertex2);

			move_list.cursor_snap = true;

			move_list.cursor_offset.set((m_x(mouse.x) - snap_to_grid(m_x(mouse.x))),
				(m_y(mouse.y) - snap_to_grid(m_y(mouse.y))));
		}


		if (selection)
			make_backup(true, true, true, false, false);

		// If no selection, but something is highlighted, move that
		if (!selection && hilight_item != -1)
		{
			make_backup(true, true, true, false, false);
			DWORD l = hilight_item;
			rect_t r = map.l_getrect(l);
			
			// Add vertex 1
			offset.x = r.x1() - m_x(mouse.x);
			offset.y = -r.y1() - m_y(mouse.y);
			move_list.add(offset, map.lines[l]->vertex1);
			
			// Add vertex 2
			offset.x = r.x2() - m_x(mouse.x);
			offset.y = -r.y2() - m_y(mouse.y);
			move_list.add(offset, map.lines[l]->vertex2);

			move_list.cursor_snap = true;
			
			move_list.cursor_offset.set((m_x(mouse.x) - snap_to_grid(m_x(mouse.x))),
										(m_y(mouse.y) - snap_to_grid(m_y(mouse.y))));
		}
	}

	// Sectors
	if (edit_mode == 2)
	{
		for (DWORD l = 0; l < map.n_lines; l++)
		{
			short sector1 = map.l_getsector1(l);
			short sector2 = map.l_getsector2(l);

			if (vector_exists(selected_items, sector1))
			{
				rect_t r = map.l_getrect(l);

				// Add vertex 1
				offset.x = r.x1() - m_x(mouse.x);
				offset.y = -r.y1() - m_y(mouse.y);
				move_list.add(offset, map.lines[l]->vertex1);
				
				// Add vertex 2
				offset.x = r.x2() - m_x(mouse.x);
				offset.y = -r.y2() - m_y(mouse.y);
				move_list.add(offset, map.lines[l]->vertex2);

				move_list.cursor_snap = true;
				
				move_list.cursor_offset.set((m_x(mouse.x) - snap_to_grid(m_x(mouse.x))),
											(m_y(mouse.y) - snap_to_grid(m_y(mouse.y))));
			}

			if (vector_exists(selected_items, sector2))
			{
				rect_t r = map.l_getrect(l);

				// Add vertex 1
				offset.x = r.x1() - m_x(mouse.x);
				offset.y = -r.y1() - m_y(mouse.y);
				move_list.add(offset, map.lines[l]->vertex1);
				
				// Add vertex 2
				offset.x = r.x2() - m_x(mouse.x);
				offset.y = -r.y2() - m_y(mouse.y);
				move_list.add(offset, map.lines[l]->vertex2);

				move_list.cursor_snap = true;
				
				move_list.cursor_offset.set((m_x(mouse.x) - snap_to_grid(m_x(mouse.x))),
											(m_y(mouse.y) - snap_to_grid(m_y(mouse.y))));
			}
		}

		if (!selection)
		{
			for (DWORD l = 0; l < map.n_lines; l++)
			{
				short sector1 = map.l_getsector1(l);
				short sector2 = map.l_getsector2(l);

				if (hilight_item == sector1 && hilight_item != -1)
				{
					rect_t r = map.l_getrect(l);

					// Add vertex 1
					offset.x = r.x1() - m_x(mouse.x);
					offset.y = -r.y1() - m_y(mouse.y);
					move_list.add(offset, map.lines[l]->vertex1);
					
					// Add vertex 2
					offset.x = r.x2() - m_x(mouse.x);
					offset.y = -r.y2() - m_y(mouse.y);
					move_list.add(offset, map.lines[l]->vertex2);

					move_list.cursor_snap = true;
					
					move_list.cursor_offset.set((m_x(mouse.x) - snap_to_grid(m_x(mouse.x))),
												(m_y(mouse.y) - snap_to_grid(m_y(mouse.y))));
					
					selection = true;
				}

				if (hilight_item == sector2 && hilight_item != -1)
				{
					rect_t r = map.l_getrect(l);

					// Add vertex 1
					offset.x = r.x1() - m_x(mouse.x);
					offset.y = -r.y1() - m_y(mouse.y);
					move_list.add(offset, map.lines[l]->vertex1);
					
					// Add vertex 2
					offset.x = r.x2() - m_x(mouse.x);
					offset.y = -r.y2() - m_y(mouse.y);
					move_list.add(offset, map.lines[l]->vertex2);

					move_list.cursor_snap = true;
					
					move_list.cursor_offset.set((m_x(mouse.x) - snap_to_grid(m_x(mouse.x))),
												(m_y(mouse.y) - snap_to_grid(m_y(mouse.y))));
					
					selection = true;
				}
			}
		}

		if (selection)
			make_backup(true, true, true, false, false);
	}
	
	// Things
	if (edit_mode == 3)
	{
		// Move selected things
		for (int a = 0; a < selected_items.size(); a++)
		{
			int t = selected_items[a];

			offset.x = map.things[t]->x - m_x(mouse.x);
			offset.y = -map.things[t]->y - m_y(mouse.y);

			move_list.add(offset, t);
			move_list.cursor_snap = true;

			move_list.cursor_offset.set((m_x(mouse.x) - snap_to_grid(m_x(mouse.x))),
				(m_y(mouse.y) - snap_to_grid(m_y(mouse.y))));
		}

		if (selection)
			make_backup(false, false, false, false, true);
		
		// If no selection, but something is highlighted, move that
		if (!selection && hilight_item != -1)
		{
			make_backup(false, false, false, false, true);
			DWORD t = hilight_item;
			
			offset.x = map.things[t]->x - m_x(mouse.x);
			offset.y = -map.things[t]->y - m_y(mouse.y);
			
			move_list.add(offset, t);
			move_list.cursor_snap = false;
		}
	}
}

// clear_move_items: Clears the move list and unflags items
// ----------------------------------------------------- >>
void clear_move_items()
{
	bool merge = false;

	// Vertices
	if (edit_mode == 0)
	{
		make_backup(true, true, false, false, false);

		for (DWORD v = 0; v < map.n_verts; v++)
		{
			if (MOVING(v))
			{
				if (!map.v_checkspot(map.verts[v]->x, map.verts[v]->y))
				{
					map.v_mergespot(map.verts[v]->x, map.verts[v]->y);
					merge = true;
				}

				if (v >= map.n_verts)
					continue;

				int split_line = check_vertex_split(v);
				
				if (split_line != -1)
					map.l_split(split_line, v);
			}
		}
	}

	// Check for overlapping/split lines
	if (edit_mode != 3 && map.n_lines > 0)
	{
		vector<int> moving_lines;

		// Get a list of the moving lines
		for (DWORD l = 0; l < map.n_lines; l++)
		{
			if (MOVING(map.lines[l]->vertex1) || MOVING(map.lines[l]->vertex2))
				vector_add_nodup(moving_lines, l);
		}

		// Split any lines that need splitting
		for (DWORD a = 0; a < moving_lines.size(); a++)
		{
			int line = moving_lines[a];

			// Vertex 1
			int split_line = check_vertex_split(map.lines[line]->vertex1);

			if (split_line != -1 && split_line != line)
				map.l_split(split_line, map.lines[line]->vertex1);

			// Vertex 2
			split_line = check_vertex_split(map.lines[line]->vertex2);

			if (split_line != -1 && split_line != line)
				map.l_split(split_line, map.lines[line]->vertex2);
		}

		// Get the list of the moving lines again
		moving_lines.clear();
		for (DWORD l = 0; l < map.n_lines-1; l++)
		{
			if (MOVING(map.lines[l]->vertex1) || MOVING(map.lines[l]->vertex2))
				vector_add_nodup(moving_lines, l);
		}

		// Check if we have any overlapping lines, if so merge them
		if (check_overlapping_lines(moving_lines))
		{
			make_backup(true, true, false, false, false);
			
			while (merge_overlapping_lines(moving_lines))
			{
				// Get the list of the moving lines again
				moving_lines.clear();
				for (DWORD l = 0; l < map.n_lines-1; l++)
				{
					if (MOVING(map.lines[l]->vertex1) || MOVING(map.lines[l]->vertex2))
						vector_add_nodup(moving_lines, l);
				}
			}
		}
	}
	
	if (edit_mode != 3)
	{
		// Remove any free vertices or zerolength lines
		merge_verts();
		remove_zerolength_lines();

		if (edit_mode != 0)
			remove_free_verts();
	}

	move_list.clear();
}

// move_items: Moves any items in the move list
// ----------------------------------------- >>
void move_items()
{
	// Vertices, lines or sectors
	if (edit_mode < 3)
	{
		for (DWORD v = 0; v < map.n_verts; v++)
		{
			if (MOVING(v))
			{
				map.verts[v]->x = move_list.get_x(v, m_x(mouse.x));
				map.verts[v]->y = -move_list.get_y(v, m_y(mouse.y));
				map.change_level(MC_NODE_REBUILD);
			}
		}
	}
	
	// Things
	if (edit_mode == 3)
	{
		for (DWORD t = 0; t < map.n_things; t++)
		{
			if (MOVING(t))
			{
				map.things[t]->x = move_list.get_x(t, m_x(mouse.x));
				map.things[t]->y = -move_list.get_y(t, m_y(mouse.y));

				map.change_level(MC_THINGS);
			}
		}
	}
}
