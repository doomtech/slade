// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-05               >>
// << ------------------------------------ >>
// << linedraw.cpp - Line drawing stuff    >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "map.h"
#include "undoredo.h"
#include "checks.h"
#include "struct_3d.h"
#include "mathstuff.h"
#include "edit_move.h"
#include "editor_window.h"
#include "edit.h"

// Variables ----------------------------- >>
//bool line_draw = false;		// Line drawing currently active?
pointlist_t ldraw_points;	// Points drawn (used to create verts & lines
numlist_t ldraw_lines;		// Lines drawn
numlist_t ldraw_verts;		// Vertices drawn

// External Variables -------------------- >>

extern point2_t mouse;
extern rect_t sel_box;
extern int hilight_item;
extern vector<int> selected_items;
extern EditorWindow *editor_window;

// lines_clockwise: Checks if a group of lines are all facing eachother
// ----------------------------------------------------------------- >>
bool lines_clockwise(numlist_t *lines)
{
	for (DWORD a = 0; a < lines->n_numbers; a++)
	{
		int l = lines->numbers[a];

		rect_t linerect = map.l_getrect(l);
		point2_t mid = midpoint(linerect.tl, linerect.br);
		point2_t vec(linerect.x2() - linerect.x1(), linerect.y2() - linerect.y1());
		int x = vec.y;
		int y = vec.x;
		x = -x;
		point2_t side(mid.x + x, mid.y + y);
		
		bool intersect = false;

		for (DWORD b = 0; b < lines->n_numbers; b++)
		{
			if (b != a)
			{
				rect_t line = map.l_getrect(lines->numbers[b]);

				int x1 = line.x1();
				int x2 = line.x2();
				int y1 = line.y1();
				int y2 = line.y2();

				point2_t r1 = mid;
				point2_t r2 = side;

				float u_ray = (float(x2 - x1) * float(r1.y - y1) - float(y2 - y1) * float(r1.x - x1)) /
							(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));
				
				float u_line = (float(r2.x - r1.x) * float(r1.y - y1) - float(r2.y - r1.y) * float(r1.x - x1)) /
							(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));
				
				if((u_ray >= 0)/* && (u_ray <= 1024)*/ && (u_line >= 0) && (u_line <= 1))
					intersect = true;
			}
		}

		if (!intersect)
			return true;
	}

	return false;
}

// check_cplit: Checks if line drawing ended in a 'real' sector split
// --------------------------------------------------------------- >>
bool check_split(int split_sector)
{
	// Cycle through lines starting with the end vertex, and if we get back to the end
	// vertex before the start vertex, it means we don't have a proper sector split
	numlist_t checked_lines, a_lines, prev_verts;
	int vertex = map.v_getvertatpoint(ldraw_points.get_last());
	int end_vert = map.v_getvertatpoint(ldraw_points.points[0]);
	int start = vertex;
	prev_verts.add(start, false);
	bool done = false;

	while (!done)
	{
		// Get any lines attached to the current vertex
		a_lines.clear();
		map.v_getattachedlines(vertex, &a_lines);

		for (DWORD l = 0; l < a_lines.n_numbers; l++)
		{
			if (!checked_lines.exists(a_lines.numbers[l]) &&
				!ldraw_lines.exists(a_lines.numbers[l]))
			{
				// Ignore lines with both sides referencing our sector
				if (map.l_getsector1(a_lines.numbers[l]) == split_sector
					&& map.l_getsector2(a_lines.numbers[l]) == split_sector)
				{
					checked_lines.add(a_lines.numbers[l], true);
					continue;
				}

				if (map.lines[a_lines.numbers[l]]->vertex1 == vertex &&
					map.l_getsector1(a_lines.numbers[l]) == split_sector)
				{
					vertex = map.lines[a_lines.numbers[l]]->vertex2;
					checked_lines.add(a_lines.numbers[l], true);
					prev_verts.add(vertex, false);

					if (vertex == end_vert)
						return true;

					// If we're back at the start vertex, go to the previous vertex
					if (vertex == start)
					{
						vertex = prev_verts.get_last();
						prev_verts.remove_last();

						if (prev_verts.n_numbers == 0)
							return false;
					}

					break;
				}

				if (map.lines[a_lines.numbers[l]]->vertex2 == vertex &&
					map.l_getsector2(a_lines.numbers[l]) == split_sector)
				{
					vertex = map.lines[a_lines.numbers[l]]->vertex1;
					checked_lines.add(a_lines.numbers[l], true);
					prev_verts.add(vertex, false);

					if (vertex == end_vert)
						return true;

					if (vertex == start)
					{
						vertex = prev_verts.get_last();
						prev_verts.remove_last();

						if (prev_verts.n_numbers == 0)
							return false;
					}

					break;
				}
			}

			// If we're at the end of the attached lines without
			// coming across any not already checked or part of the sector,
			// either go back a vertex (if we're not at the start), or exit
			// the loop (and return false)
			if (l == a_lines.n_numbers - 1)
			{
				if (vertex == start)
					done = true;
				else
				{
					vertex = prev_verts.get_last();
					prev_verts.remove_last();

					if (prev_verts.n_numbers == 0)
						return false;
				}
			}
		}
	}

	return false;
}

void check_lines_twosided()
{
	
}

// end_linedraw: Finishes a line drawing session, builds lines/vertices/sectors
// ------------------------------------------------------------------------- >>
void end_linedraw()
{
	make_backup(true, true, true, true, false);

	if (state(STATE_LINEDRAW) && ldraw_points.n_points > 2) // Line drawing finished without error/cancellation
	{
		vector<linedef_t*>	new_lines;

		bool split = false;

		// If the last 2 points are the same, we have a sector split
		if (ldraw_points.get_last().equals(ldraw_points.points[ldraw_points.n_points - 2]))
			split = true;

		// Add any vertices that have been 'drawn over'
		bool verts_added = false;
		for (int a = 0; a < ldraw_points.n_points - 1; a++)
		{
			vector<int> verts;
			rect_t line(ldraw_points.points[a], ldraw_points.points[a+1]);

			for (int v = 0; v < map.n_verts; v++)
			{
				if (map.verts[v]->x == ldraw_points.points[a].x &&
					map.verts[v]->y == ldraw_points.points[a].y)
					continue;

				if (map.verts[v]->x == ldraw_points.points[a+1].x &&
					map.verts[v]->y == ldraw_points.points[a+1].y)
					continue;

				if (distance_to_line(line.x1(), line.y1(), line.x2(), line.y2(), map.verts[v]->x, map.verts[v]->y) < 1)
				{
					verts.push_back(v);
					verts_added = true;
				}
			}

			if (verts.size() == 0)
				continue;

			// Sort vertices by distance from the first point
			if (verts.size() > 1)
			{
				bool done = false;

				while (!done)
				{
					done = true;
					for (int b = 0; b < verts.size() - 1; b++)
					{
						int d1 = distance(ldraw_points.points[a].x, ldraw_points.points[a].y,
											map.verts[verts[b]]->x, map.verts[verts[b]]->y);

						int d2 = distance(ldraw_points.points[a].x, ldraw_points.points[a].y,
											map.verts[verts[b+1]]->x, map.verts[verts[b+1]]->y);

						if (d1 > d2)
						{
							int temp = verts[b];
							verts[b] = verts[b+1];
							verts[b+1] = temp;
							done = false;
						}
					}
				}
			}

			// For easy inserting convert ldraw_points to a vector
			vector<point2_t> points;
			for (int b = 0; b < ldraw_points.n_points; b++)
				points.push_back(ldraw_points.points[b]);

			// Add points at vertices
			for (int b = 0; b < verts.size(); b++)
				points.insert(points.begin() + a + b + 1, map.v_getpoint(verts[b]));

			// Rebuild ldraw_points
			ldraw_points.clear();
			for (int b = 0; b < points.size(); b++)
				ldraw_points.add(points[b], false);
		}


		// *** SPLIT SECTOR ***
		// I'm sure theres a much nicer way to do this, but the method I use actually works,
		// So it'll do for now :P
		if (split)
		{
			numlist_t a_lines; // List of lines attached to vertex being examined

			// Determine sector to split, if any.
			// Do so by finding the nearest sector to the midpoint of the last line drawn
			point2_t mid = midpoint(ldraw_points.points[0], ldraw_points.points[1]);
			int split_sector = determine_sector(mid.x, mid.y);

			// Add a vertex at each point, also add vertex numbers to the created vertices list
			// Also, split any lines if need be, and ignore duplicate points
			for (int p = 0; p < ldraw_points.n_points; p++)
			{
				if (!ldraw_points.points[p].equals(ldraw_points.points[p - 1]))
					ldraw_verts.add(map.add_vertex(ldraw_points.points[p].x, ldraw_points.points[p].y), true);

				int split_line = check_vertex_split(map.n_verts - 1);

				if (split_line != -1)
					map.l_split(split_line, map.n_verts - 1);
			}

			// Add lines from the vertices, also adding line numbers to the created lines list
			for (int v = 1; v < ldraw_verts.n_numbers; v++)
				ldraw_lines.add(map.add_line(ldraw_verts.numbers[v - 1], ldraw_verts.numbers[v]), true);

			// Merge vertices (in the entire map, for the moment)
			merge_verts();
			remove_free_verts();

			// Rebuild vertices list
			ldraw_verts.clear();
			for (DWORD l = 0; l < new_lines.size(); l++)
				ldraw_verts.add(map.lines[ldraw_lines.numbers[l]]->vertex1, true);

			ldraw_verts.add(map.lines[ldraw_lines.get_last()]->vertex2, true);

			// If the split line(s) has/have a loose end
			if (!map.v_isattached_sector(map.v_getvertatpoint(ldraw_points.points[0]))
				|| !map.v_isattached_sector(map.v_getvertatpoint(ldraw_points.get_last())))
			{
				// Run through newly created lines, set both their sides to the 'split' sector
				for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
				{
					map.l_setsector(ldraw_lines.numbers[l], 1, split_sector);
					map.l_setsector(ldraw_lines.numbers[l], 2, split_sector);
					map.lines[ldraw_lines.numbers[l]]->set_flag(LINE_TWOSIDED);
				}
			}
			else if (!check_split(split_sector))
			{
				//printf("check_split failed\n");

				// Run through newly created lines, set both their sides to the 'split' sector
				for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
				{
					map.l_setsector(ldraw_lines.numbers[l], 1, split_sector);
					map.l_setsector(ldraw_lines.numbers[l], 2, split_sector);
					map.lines[ldraw_lines.numbers[l]]->set_flag(LINE_TWOSIDED);
				}
			}
			else
			{
				// Create a new sector
				int new_sector = map.add_sector();

				// Copy split sector attributes to new sector
				if (split_sector != -1)
					memcpy(map.sectors[new_sector], map.sectors[split_sector], sizeof(sector_t));

				// Run through newly created lines, set their first side to the new
				// sector, and their second side to the split sector
				numlist_t changed_lines;

				for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
				{
					map.l_setsector(ldraw_lines.numbers[l], 1, new_sector);
					map.l_setsector(ldraw_lines.numbers[l], 2, split_sector);
					map.lines[ldraw_lines.numbers[l]]->set_flag(LINE_TWOSIDED);
					changed_lines.add(ldraw_lines.numbers[l], true);
				}

				// Starting at the last new vertex, cycle through lines that are part of
				// the sector to be split, and set whatever side's sector reference
				// to the new sector. End once we arrive at the first created vertex. Phew.
				bool done = false;
				bool not_split = false;
				int vertex = map.v_getvertatpoint(ldraw_points.get_last());
				int end_vert = map.v_getvertatpoint(ldraw_points.points[0]);
				int start_vert = vertex;

				while (!done)
				{
					// Get any lines attached to the current vertex
					map.v_getattachedlines(vertex, &a_lines);

					for (DWORD l = 0; l < a_lines.n_numbers; l++)
					{
						if (!ldraw_lines.exists(a_lines.numbers[l]) &&
							!changed_lines.exists(a_lines.numbers[l]))
						{
							// If the line has both sides pointing to the sector to be split, skip it
							// We'll deal with these lines later
							if (map.l_getsector1(a_lines.numbers[l]) == split_sector
								&& map.l_getsector2(a_lines.numbers[l]) == split_sector)
								l++;

							if (map.lines[a_lines.numbers[l]]->vertex1 == vertex &&
								map.l_getsector1(a_lines.numbers[l]) == split_sector)
							{
								map.l_setsector(a_lines.numbers[l], 1, new_sector);
								vertex = map.lines[a_lines.numbers[l]]->vertex2;
								changed_lines.add(a_lines.numbers[l], true);

								if (vertex == end_vert)
									done = true;

								break;
							}

							if (map.lines[a_lines.numbers[l]]->vertex2 == vertex &&
								map.l_getsector2(a_lines.numbers[l]) == split_sector)
							{
								map.l_setsector(a_lines.numbers[l], 2, new_sector);
								vertex = map.lines[a_lines.numbers[l]]->vertex1;
								changed_lines.add(a_lines.numbers[l], true);

								if (vertex == end_vert)
									done = true;

								break;
							}
						}

						// To prevent an infinite loop
						if (l == a_lines.n_numbers - 1)
							done = true;
					}
				}

				// Reverse cycle (just to add some changed_lines for later)
				done = false;
				vertex = map.v_getvertatpoint(ldraw_points.get_last());
				start_vert = vertex;

				while (!done)
				{
					// Get any lines attached to the current vertex
					map.v_getattachedlines(vertex, &a_lines);

					for (DWORD l = 0; l < a_lines.n_numbers; l++)
					{
						if (!ldraw_lines.exists(a_lines.numbers[l]) &&
							!changed_lines.exists(a_lines.numbers[l]))
						{
							// If the line has both sides pointing to the sector to be split, skip it
							// We'll deal with these lines later
							if (map.l_getsector1(a_lines.numbers[l]) == split_sector
								&& map.l_getsector2(a_lines.numbers[l]) == split_sector)
								l++;

							if (map.lines[a_lines.numbers[l]]->vertex2 == vertex &&
								map.l_getsector1(a_lines.numbers[l]) == split_sector
								&& !changed_lines.exists(a_lines.numbers[l]))
							{
								vertex = map.lines[a_lines.numbers[l]]->vertex1;
								changed_lines.add(a_lines.numbers[l], true);

								if (vertex == end_vert)
									done = true;

								break;
							}

							if (map.lines[a_lines.numbers[l]]->vertex1 == vertex &&
								map.l_getsector2(a_lines.numbers[l]) == split_sector
								&& !changed_lines.exists(a_lines.numbers[l]))
							{
								vertex = map.lines[a_lines.numbers[l]]->vertex2;
								changed_lines.add(a_lines.numbers[l], true);

								if (vertex == end_vert)
									done = true;

								break;
							}
						}

						// To prevent an infinite loop
						if (l == a_lines.n_numbers - 1)
							done = true;
					}
				}

				if (not_split)
				{
					for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
					{
						map.l_setsector(ldraw_lines.numbers[l], 1, split_sector);
						map.l_setsector(ldraw_lines.numbers[l], 2, split_sector);
					}
				}
				else
				{
					// Now we go through all the lines that are part of the sector to be split
					// and determine which side of the split they are on
					for (DWORD l = 0; l < map.n_lines; l++)
					{
						if ((map.l_getsector1(l) == split_sector || map.l_getsector2(l) == split_sector)
							&& !changed_lines.exists(l) && !ldraw_lines.exists(l))
						{
							int closest_line = -1;
							int min_dist = -1;

							rect_t line_rect = map.l_getrect(l);
							float mid_x = (float)midpoint(line_rect.tl, line_rect.br).x;
							float mid_y = (float)midpoint(line_rect.tl, line_rect.br).y;

							if (map.l_getsector1(l) == split_sector)
							{
								if (line_rect.x1() > line_rect.x2())
									mid_y += 0.5f;
								if (line_rect.x2() > line_rect.x1())
									mid_y -= 0.5f;
								if (line_rect.y1() > line_rect.y2())
									mid_x -= 0.5f;
								if (line_rect.y2() > line_rect.y1())
									mid_x += 0.5f;
							}

							if (map.l_getsector2(l) == split_sector)
							{
								if (line_rect.x1() > line_rect.x2())
									mid_y -= 0.5f;
								if (line_rect.x2() > line_rect.x1())
									mid_y += 0.5f;
								if (line_rect.y1() > line_rect.y2())
									mid_x += 0.5f;
								if (line_rect.y2() > line_rect.y1())
									mid_x -= 0.5f;
							}

							point2_t mid(midpoint(map.l_getrect(l).tl, map.l_getrect(l).br));

							for (DWORD a = 0; a < changed_lines.n_numbers; a++)
							{
								rect_t line2(map.l_getrect(changed_lines.numbers[a]).tl, map.l_getrect(changed_lines.numbers[a]).br);
								int dist = distance_to_line(line2.x1(), line2.y1(), line2.x2(), line2.y2(), mid.x, mid.y);

								if (min_dist == -1 || dist < min_dist)
								{
									min_dist = dist;
									closest_line = changed_lines.numbers[a];
								}
							}

							// Now figure out what sector the line should be in
							int sector = -1;

							if (map.l_getsector1(l) == split_sector)
							{
								if (determine_line_side(closest_line, mid_x, mid_y))
								{
									if (map.l_getsector1(closest_line) == new_sector)
										map.l_setsector(l, 1, new_sector);
								}
								else
								{
									if (map.l_getsector2(closest_line) == new_sector)
										map.l_setsector(l, 1, new_sector);
								}
							}

							if (map.l_getsector2(l) == split_sector)
							{
								if (determine_line_side(closest_line, mid_x, mid_y))
								{
									if (map.l_getsector1(closest_line) == new_sector)
										map.l_setsector(l, 2, new_sector);
								}
								else
								{
									if (map.l_getsector2(closest_line) == new_sector)
										map.l_setsector(l, 2, new_sector);
								}
							}
						}
					}
				}

				// Check for any new non-twosided lines that have the 2s flag
				for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
				{
					int a = ldraw_lines.numbers[l];
					if (map.lines[a]->flags & LINE_TWOSIDED)
					{
						if (map.lines[a]->side2 == -1)
							map.lines[a]->clear_flag(LINE_TWOSIDED);
					}
				}
			}
		}
		else // *** CLOSED SECTOR ADDED ***
		{
			bool new_sector_used = false;
			bool new_sector_copied = false;
			int new_sector = map.add_sector();
			numlist_t new_lines;
			//numlist_t changed_sides;
			numlist_t check_sectors;
			numlist_t changed_lines;

			// Go through all new points
			for (DWORD p = 0; p < ldraw_points.n_points - 1; p++)
			{
				bool done = false;
				point2_t p1 = ldraw_points.points[p];
				point2_t p2 = ldraw_points.points[p + 1];
				point2_t p3;
				int v1 = -1;
				int v2 = -1;

				if (p == 0)
					p3 = ldraw_points.points[p + 2];
				else
					p3 = ldraw_points.points[p - 1];

				// If we have a point on a line that isn't a current vertex,
				// add the vertex and split the line
				if (map.v_checkspot(p1.x, p1.y))
				{
					v1 = map.add_vertex(p1.x, p1.y);

					if (check_vertex_split(p1) != -1)
						map.l_split(check_vertex_split(p1), v1);
				}
				else
					v1 = map.v_getvertatpoint(p1);

				if (map.v_checkspot(p2.x, p2.y))
				{
					v2 = map.add_vertex(p2.x, p2.y);

					if (check_vertex_split(p2) != -1)
						map.l_split(check_vertex_split(p2), v2);
				}
				else
					v2 = map.v_getvertatpoint(p2);

				// If the two points we're looking at are both current vertices
				if (v1 != -1 && v2 != -1)
				{
					for (DWORD l = 0; l < map.n_lines; l++)
					{
						// If the points are the front side of the line
						if (map.lines[l]->vertex1 == v1 && map.lines[l]->vertex2 == v2)
						{
							// Set first side to the new sector
							if (map.l_getsector1(l) != -1 && !new_sector_copied)
							{
								memcpy(map.sectors[new_sector], map.sectors[map.sides[map.lines[l]->side1]->sector], sizeof(sector_t));
								new_sector_copied = true;
							}
							
							check_sectors.add(map.l_getsector1(l), true);
							map.l_setsector(l, 1, new_sector);

							changed_lines.add(l, true);
							done = true;
							new_sector_used = true;
							break;
						}

						// If the points are the back side of the line
						if (map.lines[l]->vertex2 == v1 && map.lines[l]->vertex1 == v2)
						{
							// Set second side to the new sector
							if (map.l_getsector1(l) != -1 && !new_sector_copied)
							{
								memcpy(map.sectors[new_sector], map.sectors[map.sides[map.lines[l]->side1]->sector], sizeof(sector_t));
								new_sector_copied = true;
							}

							check_sectors.add(map.l_getsector1(l), true);
							map.l_setsector(l, 2, new_sector);

							changed_lines.add(l, true);
							done = true;
							new_sector_used = true;
							break;
						}
					}
				}

				if (!done)
				{
					// If not, create the needed vertices and a new line
					if (v1 == -1)
						v1 = map.add_vertex(p1.x, p1.y);

					if (v2 == -1)
						v2 = map.add_vertex(p2.x, p2.y);

					int new_line = map.add_line(v1, v2);
					new_lines.add(new_line, true);
				}
			}

			// If the lines are clockwise, set their first side to the new sector
			// otherwise set their second sides to the new sector. Also set the opposite
			// side to whatever sector it's in
			numlist_t all_new_lines;
			all_new_lines.add_numlist(&new_lines, true);
			all_new_lines.add_numlist(&changed_lines, true);

			if (lines_clockwise(&all_new_lines))
			{
				for (DWORD a = 0; a < new_lines.n_numbers; a++)
				{
					int l = new_lines.numbers[a];
					int sector2 = get_side_sector(l, 2);
					check_sectors.add(sector2, true);

					if (!new_sector_copied && sector2 != -1)
					{
						memcpy(map.sectors[new_sector], map.sectors[sector2], sizeof(sector_t));
						new_sector_copied = true;
					}

					map.l_setsector(l, 1, new_sector);
					map.l_setsector(l, 2, sector2);
					map.l_setdeftextures(l);
					new_sector_used = true;
				}
			}
			else
			{
				// If we have no changed lines, it means we have a column
				if (changed_lines.n_numbers == 0)
				{
					//printf("column\n");

					for (DWORD a = 0; a < new_lines.n_numbers; a++)
					{
						int l = new_lines.numbers[a];
						int sector1 = get_side_sector(l, 1);
						check_sectors.add(sector1, true);

						map.l_setsector(l, 1, sector1);
						map.l_setdeftextures(l);
						new_sector_used = false;
					}
				}
				else
				{
					for (DWORD a = 0; a < new_lines.n_numbers; a++)
					{
						int l = new_lines.numbers[a];
						int sector2 = get_side_sector(l, 2);
						check_sectors.add(sector2, true);

						if (!new_sector_copied && sector2 != -1)
						{
							memcpy(map.sectors[new_sector], map.sectors[sector2], sizeof(sector_t));
							new_sector_copied = true;
						}

						map.l_setsector(l, 1, new_sector);
						map.l_setsector(l, 2, sector2);
						map.l_setdeftextures(l);
						new_sector_used = true;
					}
				}
			}

			// Now we process any lines that might need sector references changed
			// in a similar fashion to above (determine what side is in what sector)
			if (check_sectors.exists(-1))
				check_sectors.remove(-1);

			for (DWORD s = 0; s < check_sectors.n_numbers; s++)
			{
				int sector = check_sectors.numbers[s];

				for (DWORD l = 0; l < map.n_lines; l++)
				{
					if (!all_new_lines.exists(l))
					{
						if (map.l_getsector1(l) == sector)
						{
							int d_sector = get_side_sector(l, 1);

							if (d_sector != sector && d_sector != -1)
								map.l_setsector(l, 1, d_sector);
						}

						if (map.l_getsector2(l) == sector)
						{
							int d_sector = get_side_sector(l, 2);

							if (d_sector != sector && d_sector != -1)
								map.l_setsector(l, 2, d_sector);
						}
					}
				}
			}

			// Check for any new lines that need to be split
			if (verts_added)
			{
				for (int a = 0; a < new_lines.n_numbers; a++)
				{
					rect_t rect = map.l_getrect(new_lines.numbers[a]);

					for (int v = 0; v < map.n_verts; v++)
					{
						int l = check_vertex_split(v);
						if (new_lines.exists(l) && l != -1)
						{
							map.l_split(l, v);
							selected_items.clear();
							hilight_item = v;
							add_move_items();
							clear_move_items();
							hilight_item = -1;
						}
					}
				}
			}
		}

		// Finish up
		remove_zerolength_lines();
		remove_free_verts();
		ldraw_verts.clear();
		ldraw_lines.clear();
		//force_map_redraw(true, false);
		editor_window->redraw_map(true, false);
		map.change_level(MC_NODE_REBUILD);
	}

	//line_draw = false;
	change_state();
	ldraw_points.clear();
}

// line_drawpoint: Adds the current mouse position to the line draw points list
// ------------------------------------------------------------------------- >>
void line_drawpoint(bool nearest_vert)
{
	if (sel_box.x1() == -1)
	{
		int x = snap_to_grid(m_x(mouse.x));
		int y = snap_to_grid(-m_y(mouse.y));

		if (nearest_vert)
		{
			int v = get_nearest_vertex(mouse.x, mouse.y);

			if (v != -1)
			{
				x = map.verts[v]->x;
				y = map.verts[v]->y;
			}
		}

		if (!ldraw_points.add(point2_t(x, y), false))
			end_linedraw();
	}
	else
	{
		end_linedraw();
		sel_box.set(-1, -1, -1, -1);
	}
}

// line_drawbox: Sets the line draw points to the selection box
// --------------------------------------------------------- >>
void line_drawbox(bool square)
{
	rect_t box;

	box.set(m_x(sel_box.x1()), -m_y(sel_box.y1()), m_x(sel_box.x2()), -m_y(sel_box.y2()));

	if (square)
	{
		int size_x = snap_to_grid(max(box.awidth(), box.aheight()));
		int size_y = -snap_to_grid(max(box.awidth(), box.aheight()));

		if (sel_box.width() < 0)
			size_x = -size_x;

		if (sel_box.height() < 0)
			size_y = -size_y;

		box.br.set(box.tl.x + size_x, box.tl.y + size_y);
	}

	ldraw_points.clear();
	ldraw_points.add(point2_t(snap_to_grid(box.x1()),
							  snap_to_grid(box.y1())), false);
	ldraw_points.add(point2_t(snap_to_grid(box.x2()),
							  snap_to_grid(box.y1())), false);
	ldraw_points.add(point2_t(snap_to_grid(box.x2()),
							  snap_to_grid(box.y2())), false);
	ldraw_points.add(point2_t(snap_to_grid(box.x1()),
							  snap_to_grid(box.y2())), false);
	ldraw_points.add(point2_t(snap_to_grid(box.x1()),
							  snap_to_grid(box.y1())), false);
}

// line_drawcircle: Draws a circle or ellipse within the selection box
// ---------------------------------------------------------------- >>
void line_drawcircle(int sides, bool circle)
{
	ldraw_points.clear();

	rect_t box(snap_to_grid(m_x(sel_box.x1())), snap_to_grid(-m_y(sel_box.y1())),
				snap_to_grid(m_x(sel_box.x2())), snap_to_grid(-m_y(sel_box.y2())));

	point2_t mid(box.x1() + (box.width() / 2), box.y1() + (box.height() / 2));

	int width = box.width() / 2;
	int height = box.height() / 2;

	if (circle)
	{
		width = box.length() / 2;
		height = box.length() / 2;
	}

	double rot = 0;
	point2_t start;
	for (int a = 0; a < sides; a++)
	{
		point2_t point(mid.x + sin(rot) * width, mid.y - cos(rot) * height);
		ldraw_points.add(point, false);
		rot += (3.1415926535897932384626433832795 * 2) / (double)sides;

		if (a == 0)
			start = point;
	}

	ldraw_points.add(start, false);
}

// line_undrawpoint: Removes the last entry in the line draw points list
// ------------------------------------------------------------------ >>
void line_undrawpoint()
{
	if (sel_box.x1() != -1)
		ldraw_points.clear();
	else
		ldraw_points.remove(ldraw_points.get_last());

	if (ldraw_points.n_points == 0)
	{
		ldraw_points.clear();
		//line_draw = false;
		change_state();
		sel_box.set(-1, -1, -1, -1);
	}
}
