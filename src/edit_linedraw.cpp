
#include "main.h"
#include "structs.h"
#include "edit.h"
#include "edit_misc.h"
#include "doom_map.h"
#include "dm_vertex.h"
#include "dm_line.h"
#include "input.h"
#include "draw.h"
#include "mathstuff.h"
#include "undoredo.h"

vector<point2_t>	ldraw_points;

bool				ldraw_bound_start = false;
rect_t				ldraw_bound(-1, -1, -1, -1);

CVAR(Bool, edit_auto_createsector, true, CVAR_SAVE)

extern DoomMap d_map;
extern rgba_t col_linedraw;
extern Sector* last_copy_sector;

EXTERN_CVAR(Bool, line_aa)
EXTERN_CVAR(Float, line_size)
EXTERN_CVAR(Bool, edit_snap_grid)
EXTERN_CVAR(Bool, edit_auto_merge)
EXTERN_CVAR(Bool, edit_auto_split)

// lines_clockwise: Checks if a group of lines are all facing eachother
// ----------------------------------------------------------------- >>
bool lines_clockwise(vector<Line*> &list)
{
	for (unsigned int a = 0; a < list.size(); a++)
	{
		rect_t linerect = list[a]->get_rect();
		point2_t mid = linerect.middle();
		point2_t vec(linerect.x2() - linerect.x1(), linerect.y2() - linerect.y1());
		int x = vec.y;
		int y = vec.x;
		x = -x;
		point2_t side(mid.x + x, mid.y + y);

		bool intersect = false;
		for (unsigned int b = 0; b < list.size(); b++)
		{
			if (b != a)
			{
				rect_t line = list[b]->get_rect();

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
			return true;
	}

	return false;
}

void ldraw_end()
{
	make_backup(BKUP_VERTS|BKUP_LINES|BKUP_SIDES|BKUP_SECTORS);

	// Add vertices
	vector<Vertex*> new_verts;

	// Check for intersections
	// (still seem to be some problems with this due to the inaccuracy of the doom map format, oh well)
	bool gsnap = edit_snap_grid;
	edit_snap_grid = false;
	for (unsigned int a = 0; a < ldraw_points.size() - 1; a++)
	{
		int lines = d_map.n_lines();
		for (int b = 0; b < lines; b++)
		{
			rect_t r1 = d_map.line(b)->get_rect();
			rect_t r2(ldraw_points[a], ldraw_points[a+1]);

			if (!rects_intersect(r1, r2))
				continue;

			fpoint2_t ip;
			if (lines_intersect(r1, r2, &ip))
			{
				//log_message(s_fmt("intersect at %1.2f,%1.2f", ip.x, ip.y));
				if (d_map.check_vertex_spot(point2_t(lround(ip.x), lround(ip.y))) == -1)
				{
					Vertex *v = new Vertex(lround(ip.x), lround(ip.y), &d_map);
					d_map.line(b)->split(v);
					lines = d_map.n_lines();
				}
			}
		}
	}
	edit_snap_grid = gsnap;

	for (unsigned int a = 0; a < ldraw_points.size() - 1; a++)
	{
		int v = d_map.check_vertex_spot(ldraw_points[a]);

		if (v == -1 || !edit_auto_merge)
			new_verts.push_back(new Vertex(ldraw_points[a].x, ldraw_points[a].y, &d_map));
		else
			new_verts.push_back(d_map.vertex(v));
	}

	// Check for line splits
	if (edit_auto_split)
	{
		for (unsigned int a = 0; a < new_verts.size(); a++)
			d_map.check_split(new_verts[a]);
	}

	// Add lines
	vector<Line*> new_lines;

	for (unsigned int a = 0; a < new_verts.size() - 1; a++)
		new_lines.push_back(new Line(new_verts[a], new_verts[a+1], NULL));

	// If it's not a split add a line from last to first,
	// and also check wether they are clockwise or not
	bool clockwise = true;
	if (ldraw_points[0].x == ldraw_points.back().x && ldraw_points[0].y == ldraw_points.back().y)
	{
		new_lines.push_back(new Line(new_verts.back(), new_verts[0], NULL));
		clockwise = lines_clockwise(new_lines);
	}

	// Check for new line splits
	if (edit_auto_split)
	{
		for (unsigned int a = 0; a < d_map.n_verts(); a++)
			d_map.check_split(d_map.vertex(a), new_lines);
	}

	// Remove overlapping lines
	d_map.remove_overlapping_lines(new_lines);

	// Flip lines if anticlockwise
	if (!clockwise)
	{
		for (unsigned int a = 0; a < new_lines.size(); a++)
			new_lines[a]->flip(true, false);
	}

	// Check wether to run create sector on the back sides of the new lines
	bool *cs_back = new bool[new_lines.size()];
	memset(cs_back, 0, new_lines.size());
	for (unsigned int a = 0; a < new_lines.size(); a++)
	{
		point2_t pos = new_lines[a]->get_side_point(false);
		if (d_map.get_hilight_sector(pos) != -1)
			cs_back[a] = true;
	}

	// Add new lines to map
	for (unsigned int a = 0; a < new_lines.size(); a++)
	{
		d_map.add_line(new_lines[a]);
		new_lines[a]->set_side1(d_map.side(-1, false));
		new_lines[a]->set_side2(d_map.side(-1, false));
	}

	//log_message(s_fmt("Created %d lines", new_lines.size()));

	if (edit_auto_createsector && new_lines.size() > 0)
	{
		bool *fs = new bool[d_map.n_lines()];
		bool *bs = new bool[d_map.n_lines()];
		memset(fs, 0, d_map.n_lines());
		memset(bs, 0, d_map.n_lines());
		vector<Sector*> new_sectors;
		last_copy_sector = NULL;

		for (unsigned int a = 0; a < new_lines.size(); a++)
		{
			if (!fs[d_map.index(new_lines[a])])
				sector_create(new_lines[a]->get_side_point(true), new_sectors, fs, bs);
			
			if (!bs[d_map.index(new_lines[a])] && cs_back[a])
				sector_create(new_lines[a]->get_side_point(false), new_sectors, fs, bs);
		}

		if (last_copy_sector)
		{
			for (unsigned int a = 0; a < new_sectors.size(); a++)
				new_sectors[a]->copy(last_copy_sector);
		}
	}

	ldraw_points.clear();
	change_state();
	d_map.clear_selection();
	d_map.remove_unused_sectors();
	d_map.update_vertex_refs();
	ldraw_bound_start = false;
}

void ldraw_addpoint(bool nearest_vert)
{
	if (state(STATE_LINEDRAW))
	{
		int x = snap_to_grid(down_pos(true).x);
		int y = snap_to_grid(down_pos(true).y);

		if (nearest_vert)
		{
			int v = d_map.get_hilight_vertex(down_pos(true));

			if (v != -1)
			{
				x = d_map.vertex(v)->x_pos();
				y = d_map.vertex(v)->y_pos();
			}
		}

		for (unsigned int a = 0; a < ldraw_points.size(); a++)
		{
			if (ldraw_points[a].x == x && ldraw_points[a].y == y)
			{
				ldraw_points.push_back(point2_t(x, y));
				ldraw_end();
				return;
			}
		}

		ldraw_points.push_back(point2_t(x, y));
	}

	if (state(STATE_SHAPEDRAW))
	{
		if (!ldraw_bound_start)
		{
			ldraw_bound_start = true;
			ldraw_bound.tl.set(snap_to_grid(down_pos(true).x), snap_to_grid(down_pos(true).y));
		}
		else
			ldraw_end();
	}
}

void ldraw_drawrect(point2_t mouse, bool square)
{
	if (!ldraw_bound_start)
		return;

	ldraw_bound.br.set(mouse);

	if (square)
	{
		int size_x = snap_to_grid(max(ldraw_bound.awidth(), ldraw_bound.aheight()));
		int size_y = snap_to_grid(max(ldraw_bound.awidth(), ldraw_bound.aheight()));

		if (ldraw_bound.width() < 0)
			size_x = -size_x;

		if (ldraw_bound.height() < 0)
			size_y = -size_y;

		ldraw_bound.br.set(ldraw_bound.tl.x + size_x, ldraw_bound.tl.y + size_y);
	}

	ldraw_points.clear();
	ldraw_points.push_back(point2_t(snap_to_grid(ldraw_bound.x1()),
									snap_to_grid(ldraw_bound.y1())));
	ldraw_points.push_back(point2_t(snap_to_grid(ldraw_bound.x2()),
									snap_to_grid(ldraw_bound.y1())));
	ldraw_points.push_back(point2_t(snap_to_grid(ldraw_bound.x2()),
									snap_to_grid(ldraw_bound.y2())));
	ldraw_points.push_back(point2_t(snap_to_grid(ldraw_bound.x1()),
									snap_to_grid(ldraw_bound.y2())));
	ldraw_points.push_back(point2_t(snap_to_grid(ldraw_bound.x1()),
									snap_to_grid(ldraw_bound.y1())));
}

void ldraw_removepoint()
{
	if (state(STATE_SHAPEDRAW))
	{
		ldraw_points.clear();
		ldraw_bound_start = false;
		change_state();
		return;
	}

	if (ldraw_points.size() == 0)
	{
		change_state();
		return;
	}

	ldraw_points.pop_back();
}

void ldraw_draw_lines(point2_t mouse)
{
	point2_t mp(snap_to_grid(mouse.x), snap_to_grid(mouse.y));

	glEnable(GL_POINT_SMOOTH);
	glLineStipple(4, 0xBBBB);
	glEnable(GL_LINE_STIPPLE);

	rgba_t c = col_linedraw.amp(64, 64, 64, 0);

	if (ldraw_points.size() > 1)
	{
		for (unsigned int a = 0; a < ldraw_points.size() - 1; a++)
		{
			rect_t r(ldraw_points[a], ldraw_points[a+1]);
			draw_line(r, col_linedraw, line_aa, false, line_size);
			draw_point(ldraw_points[a].x, ldraw_points[a].y, 8, col_linedraw);
			draw_text(r.middle().x, r.middle().y, c, 1, true, "%d", lround(r.length()));
		}
	}

	if (ldraw_points.size() > 0 && !state(STATE_SHAPEDRAW))
	{
		rect_t r(ldraw_points.back(), mp);
		draw_line(r, col_linedraw, line_aa, false, line_size);
		draw_point(ldraw_points.back().x, ldraw_points.back().y, 8, col_linedraw);
		draw_text(r.middle().x, r.middle().y, c, 1, true, "%d", lround(r.length()));
	}

	draw_point(mp.x, mp.y, 8, col_linedraw);

	glDisable(GL_LINE_STIPPLE);
	glDisable(GL_POINT_SMOOTH);
}
