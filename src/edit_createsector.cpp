

/*
1 - Find closest line to the origin
2 - From the line, follow attached lines around (minimum angles) until the first line is met again
3 - If the lines are clockwise and the origin is on the front side of the first line, or if the lines are
    anticlockwise and the origin is on the back side, go to 5
4 - Cast a ray from the middle of the first line to the origin, and get the line that it first hits, then repeat from 2
5 - Get a list of all lines totally within the outer bounds of the 'outside' lines
6 - If there are 0 lines in the list, finish
7 - For each line in the list, find the closest outside line to it's midpoint, and if it's on the outside line's front side,
    add it to the sector
*/

#include "main.h"
#include "doom_map.h"
#include "dm_line.h"
#include "dm_side.h"
#include "dm_sector.h"
#include "dm_vertex.h"
//#include "struct_3d.h"
#include "mathstuff.h"
//#include "undoredo.h"
//#include "linedraw.h"

extern DoomMap d_map;

#define ORTN_FRONT	0
#define ORTN_BACK	1
struct csline_t
{
	int		line;
	BYTE	orientation;

	csline_t() { line = -1; orientation = ORTN_FRONT; }
	csline_t(int l, BYTE o) { line = l; orientation = o; }
};

// cycle_clockwise: Checks if a group of lines are all facing eachother
// ----------------------------------------------------------------- >>
bool cycle_clockwise(vector<csline_t> &list)
{
	for (int a = 0; a < list.size(); a++)
	{
		rect_t linerect = d_map.line(list[a].line)->get_rect();
		point2_t mid = linerect.middle();
		point2_t vec(linerect.x2() - linerect.x1(), linerect.y2() - linerect.y1());
		int x = vec.y;
		int y = vec.x;
		x = -x;
		point2_t side;

		if (list[a].orientation == ORTN_FRONT)
			side.set(mid.x + x, mid.y + y);
		else
			side.set(mid.x - x, mid.y - y);

		bool intersect = false;

		for (int b = 0; b < list.size(); b++)
		{
			if (b != a)
			{
				rect_t line = d_map.line(list[b].line)->get_rect();

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

				if((u_ray >= 0) && (u_line >= 0) && (u_line <= 1))
					intersect = true;
			}
		}

		if (!intersect)
		{
			//message_box("Clockwise");
			return true;
		}
	}

	//message_box("Anticlockwise");
	return false;
}

void get_line_cycle(csline_t start_line, vector<csline_t> &list)
{
	csline_t cur_line(start_line.line, start_line.orientation);

	bool done = false;
	while (!done)
	{
		list.push_back(cur_line);

		int vertex = -1;
		if (cur_line.orientation == ORTN_FRONT)
			vertex = d_map.index(d_map.line(cur_line.line)->vertex2());
		else
			vertex = d_map.index(d_map.line(cur_line.line)->vertex1());

		vector<int> a_lines;
		//numlist_t a_lines;
		//map.v_getattachedlines(vertex, &a_lines);
		d_map.get_lines_to_vert(vertex, a_lines);

		float min_angle = 9999;
		csline_t line(-1, 0);

		if (a_lines.size() == 1)
		{
			line.line = cur_line.line;

			if (cur_line.orientation == ORTN_FRONT)
				line.orientation = ORTN_BACK;
			else
				line.orientation = ORTN_FRONT;

		}
		else
		{
			for (int a = 0; a < a_lines.size(); a++)
			{
				float angle = 0.0f;
				csline_t t_line(a_lines[a], ORTN_FRONT);

				if (t_line.line == cur_line.line)
					angle = 360.0f;
				else
				{
					if (vertex == d_map.index(d_map.line(t_line.line)->vertex2()))
						t_line.orientation = ORTN_BACK;

					Vertex* v1 = NULL;
					Vertex* v2 = NULL;
					Vertex* v3 = NULL;

					if (cur_line.orientation == ORTN_FRONT)
					{
						v1 = d_map.line(cur_line.line)->vertex1();
						v2 = d_map.line(cur_line.line)->vertex2();
					}
					else
					{
						v1 = d_map.line(cur_line.line)->vertex2();
						v2 = d_map.line(cur_line.line)->vertex1();
					}

					if (t_line.orientation == ORTN_FRONT)
						v3 = d_map.line(t_line.line)->vertex2();
					else
						v3 = d_map.line(t_line.line)->vertex1();

					angle = get_2d_angle(point3_t(v1->x_pos(), v1->y_pos(), 0.0f),
										point3_t(v2->x_pos(), v2->y_pos(), 0.0f),
										point3_t(v3->x_pos(), v3->y_pos(), 0.0f));
				}

				//log_message(s_fmt("Angle between line %d and %d is %f", cur_line.line, t_line.line, angle));

				if (angle < min_angle)
				{
					min_angle = angle;
					line.line = t_line.line;
					line.orientation = t_line.orientation;
				}
			}
		}

		if (line.line == -1)
			return;

		for (int a = 0; a < list.size(); a++)
		{
			if (list[a].line == line.line &&
				list[a].orientation == line.orientation)
			{
				done = true;
				//log_message("done\n");
				break;
			}
		}

		cur_line.line = line.line;
		cur_line.orientation = line.orientation;
	}
}

rect_t get_cycle_border(vector<csline_t> &cycle)
{
	int max_x = -999999;
	int min_x = 999999;
	int max_y = -999999;
	int min_y = 999999;

	for (int a = 0; a < cycle.size(); a++)
	{
		rect_t r = d_map.line(cycle[a].line)->get_rect();

		if (r.x1() < min_x)
			min_x = r.x1();

		if (r.x2() < min_x)
			min_x = r.x2();

		if (r.x1() > max_x)
			max_x = r.x1();

		if (r.x2() > max_x)
			max_x = r.x2();

		if (r.y1() < min_y)
			min_y = r.y1();

		if (r.y2() < min_y)
			min_y = r.y2();

		if (r.y1() > max_y)
			max_y = r.y1();

		if (r.y2() > max_y)
			max_y = r.y2();
	}

	return rect_t(min_x, min_y, max_x, max_y);
}

bool sector_create(point2_t point, bool* front_sides, bool* back_sides)
{
	double min_dist = -1;
	int line = -1;

	for (int l = 0; l < d_map.n_lines(); l++)
	{
		rect_t r = d_map.line(l)->get_rect();

		double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), point.x, point.y);

		if (min_dist == -1 || dist < min_dist)
		{
			min_dist = dist;
			line = l;
		}
	}

	if (line == -1)
		return false;

	//make_backup(true, true, false, true, false);

	vector<csline_t> lines;
	vector<csline_t> all_lines;
	vector<csline_t> border_lines;
	rect_t border;

	bool done = false;
	bool first = true;
	while (!done)
	{
		done = false;

		lines.clear();
		if (determine_line_side(d_map.line(line)->get_rect(), point.x, point.y) >= 0)
			get_line_cycle(csline_t(line, ORTN_FRONT), lines);
		else
			get_line_cycle(csline_t(line, ORTN_BACK), lines);

		for (int a = 0; a < lines.size(); a++)
			all_lines.push_back(lines[a]);

		border.set(get_cycle_border(all_lines));

		if (cycle_clockwise(lines))
			done = true;
		else
		{
			// Check for void
			if (first)
			{
				for (int a = 0; a < lines.size(); a++)
				{
					int nl = -1;
					min_dist = -1;
					for (int l = 0; l < d_map.n_lines(); l++)
					{
						rect_t r = d_map.line(l)->get_rect();
						point2_t p = r.middle();

						if (point_in_rect(border.x1(), border.y1(), border.x2(), border.y2(), r.x1(), r.y1()) ||
							point_in_rect(border.x1(), border.y1(), border.x2(), border.y2(), r.x2(), r.y2()))
							continue;

						bool front = (determine_line_side(d_map.line(lines[a].line)->get_rect(), p.x, p.y) > 0);
						if ((front && lines[a].orientation == ORTN_BACK) ||
							(!front && lines[a].orientation == ORTN_FRONT))
							continue;

						double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), p.x, p.y);

						if (min_dist == -1 || dist < min_dist)
						{
							min_dist = dist;
							nl = l;
						}
					}

					if (nl == -1)
					{
						if (front_sides)
						{
							for (int b = 0; b < lines.size(); b++)
							{
								if (lines[a].orientation == ORTN_FRONT)
									front_sides[lines[b].line] = true;
							}
						}

						if (back_sides)
						{
							for (int b = 0; b < lines.size(); b++)
							{
								if (lines[a].orientation == ORTN_BACK)
									back_sides[lines[b].line] = true;
							}
						}

						return false;
					}
				}
			}

			min_dist = -1;
			line = -1;

			for (int l = 0; l < d_map.n_lines(); l++)
			{
				rect_t r = d_map.line(l)->get_rect();

				if (point_in_rect(border.x1(), border.y1(), border.x2(), border.y2(), r.x1(), r.y1()) ||
					point_in_rect(border.x1(), border.y1(), border.x2(), border.y2(), r.x2(), r.y2()))
					continue;

				double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), point.x, point.y);

				if (min_dist == -1 || dist < min_dist)
				{
					min_dist = dist;
					line = l;
				}
			}

			if (line == -1)
				return false;
		}

		first = false;
	}

	// Get all lines within the border that aren't part of the sector yet
	vector<int> in_lines;
	for (int a = 0; a < d_map.n_lines(); a++)
	{
		bool cont = false;
		for (int b = 0; b < all_lines.size(); b++)
		{
			if (all_lines[b].line == a)
			{
				cont = true;
				break;
			}
		}

		if (cont) continue;

		rect_t r = d_map.line(a)->get_rect();

		if (point_in_rect(border.x1(), border.y1(), border.x2(), border.y2(), r.x1(), r.y1()) &&
			point_in_rect(border.x1(), border.y1(), border.x2(), border.y2(), r.x2(), r.y2()))
		{
			point2_t midpoint = r.middle();

			min_dist = -1;
			for (WORD l = 0; l < all_lines.size(); l++)
			{
				rect_t r2 = d_map.line(all_lines[l].line)->get_rect();
				double dist = distance_to_line(r2.x1(), r2.y1(), r2.x2(), r2.y2(), midpoint.x, midpoint.y);

				if (min_dist == -1 || dist < min_dist)
				{
					min_dist = dist;
					line = l;
				}
			}

			bool front = (determine_line_side(d_map.line(all_lines[line].line)->get_rect(), midpoint.x, midpoint.y) >= 0);

			if ((front && all_lines[line].orientation == ORTN_FRONT) ||
				(!front && all_lines[line].orientation == ORTN_BACK))
			{
				in_lines.push_back(a);
			}
		}
	}

	if (in_lines.size() > 0)
	{
		for (int a = 0; a < all_lines.size(); a++)
		{
			// Get closest inside line
			point.set(d_map.line(all_lines[a].line)->get_rect().middle());

			min_dist = -1;
			line = -1;
			for (WORD l = 0; l < in_lines.size(); l++)
			{
				rect_t r = d_map.line(in_lines[l])->get_rect();
				double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), point.x, point.y);

				if (min_dist == -1 || dist < min_dist)
				{
					min_dist = dist;
					line = in_lines[l];
				}
			}

			if (line == -1)
				break;

			lines.clear();
			if (determine_line_side(d_map.line(line)->get_rect(), point.x, point.y) >= 0)
				get_line_cycle(csline_t(line, ORTN_FRONT), lines);
			else
				get_line_cycle(csline_t(line, ORTN_BACK), lines);

			// Add lines to all_lines list and remove them from in_lines
			for (int b = 0; b < lines.size(); b++)
			{
				for (int l = 0; l < in_lines.size(); l++)
				{
					if (lines[b].line == in_lines[l])
					{
						in_lines.erase(find(in_lines.begin(), in_lines.end(), in_lines[l]));
						break;
					}
				}

				all_lines.push_back(lines[b]);
			}

			// Re-check in-lines
			for (int b = 0; b < in_lines.size(); b++)
			{
				rect_t r = d_map.line(in_lines[b])->get_rect();
				point2_t midpoint = r.middle();

				line = -1;
				min_dist = -1;
				for (WORD l = 0; l < all_lines.size(); l++)
				{
					rect_t r2 = d_map.line(all_lines[l].line)->get_rect();
					double dist = distance_to_line(r2.x1(), r2.y1(), r2.x2(), r2.y2(), midpoint.x, midpoint.y);

					if (min_dist == -1 || dist < min_dist)
					{
						min_dist = dist;
						line = l;
					}
				}

				if (line == -1)
					continue;

				bool front = (determine_line_side(d_map.line(all_lines[line].line)->get_rect(), midpoint.x, midpoint.y) >= 0);

				if ((!front && all_lines[line].orientation == ORTN_FRONT) ||
					(front && all_lines[line].orientation == ORTN_BACK))
				{
					in_lines.erase(find(in_lines.begin(), in_lines.end(), in_lines[b]));
					b = -1;
				}
			}

			// If there are no inside lines left, exit loop
			if (in_lines.size() == 0)
				break;
		}
	}

	Sector* sector = new Sector(&d_map);
	Sector* copy_sector = NULL;

	for (int a = 0; a < all_lines.size(); a++)
	{
		Line* line = d_map.line(all_lines[a].line);
		Side* side1 = line->side1();
		Side* side2 = line->side2();

		if (all_lines[a].orientation == ORTN_FRONT)
		{
			if (d_map.valid(side1) && !copy_sector)
				copy_sector = side1->get_sector();

			if (d_map.valid(side2) && !copy_sector)
				copy_sector = side2->get_sector();

			if (front_sides)
				front_sides[all_lines[a].line] = true;

			line->set_sector(1, sector);
		}
		else
		{
			if (d_map.valid(side2) && !copy_sector)
				copy_sector = side2->get_sector();

			if (d_map.valid(side1) && !copy_sector)
				copy_sector = side1->get_sector();

			if (back_sides)
				back_sides[all_lines[a].line] = true;

			line->set_sector(2, sector);
		}
	}

	if (copy_sector)
		sector->copy(copy_sector);

	d_map.change_level(MC_NODE_REBUILD|MC_SECTORS|MC_LINES);
	//d_map.remove_unused_sectors();
	redraw_map();

	return true;
}
