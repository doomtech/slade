
#include "main.h"
#include "mathstuff.h"
#include "doom_map.h"
#include "dm_line.h"
#include "dm_side.h"
#include "dm_sector.h"
#include "dm_vertex.h"
#include "dm_thing.h"
#include "structs.h"
#include "game_config.h"
#include "undoredo.h"

extern DoomMap d_map;
extern int edit_mode;
extern GameConfig game;

// get_side_sector: Attempts to find what sector a line's side is in
// -------------------------------------------------------------- >>
int get_side_sector(int line, int side)
{
	rect_t linerect = d_map.line(line)->get_rect();
	point2_t mid = midpoint(linerect.tl, linerect.br);
	point2_t vec(linerect.x2() - linerect.x1(), linerect.y2() - linerect.y1());

	if (side == 2)
	{
		vec.x = -vec.x;
		vec.y = -vec.y;
	}

	int x = vec.y;
	int y = vec.x;
	x = -x;

	point2_t side_p(mid.x - x, mid.y - y);

	float min_dist = -1;
	int nearest_line = -1;
	point2_t nearest_midpoint;

	for (DWORD l = 0; l < d_map.n_lines(); l++)
	{
		if (l != line)
		{
			rect_t line_r = d_map.line(l)->get_rect();
			int x1 = line_r.x1();
			int x2 = line_r.x2();
			int y1 = line_r.y1();
			int y2 = line_r.y2();

			point2_t r1 = mid;
			point2_t r2 = side_p;

			float u_ray = (float(x2 - x1) * float(r1.y - y1) - float(y2 - y1) * float(r1.x - x1)) /
							(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));
				
			float u_line = (float(r2.x - r1.x) * float(r1.y - y1) - float(r2.y - r1.y) * float(r1.x - x1)) /
							(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));

			if ((u_ray >= 0) && (u_line >= 0) && (u_line <= 1))
			{
				if (u_ray < min_dist || min_dist == -1)
				{
					nearest_line = l;
					min_dist = u_ray;
					nearest_midpoint.set(midpoint(line_r.tl, line_r.br));
				}

				if (u_ray == min_dist)
				{
					double nearest_mid_dist = distance_to_line(linerect.x1(), linerect.y1(), linerect.x2(), linerect.y2(),
																nearest_midpoint.x, nearest_midpoint.y);

					point2_t this_midpoint(midpoint(line_r.tl, line_r.br));
					double this_mid_dist = distance_to_line(linerect.x1(), linerect.y1(), linerect.x2(), linerect.y2(),
															this_midpoint.x, this_midpoint.y);

					if (this_mid_dist < nearest_mid_dist)
					{
						nearest_line = l;
						min_dist = u_ray;
						nearest_midpoint.set(midpoint(line_r.tl, line_r.br));
					}
				}
			}
		}
	}

	if (nearest_line == -1)
		return -1;
	else
	{
		rect_t r = d_map.line(nearest_line)->get_rect();
		if (determine_line_side(r, mid.x, mid.y) >= 0)
			return d_map.line(nearest_line)->sector_index();
		else
			return d_map.line(nearest_line)->sector_index(false);
	}
}



// SECTORS

// sector_create_stairs: Creates stairs from a group of sectors
// --------------------------------------------------------- >>
void sector_create_stairs(int floor_step, int ceil_step)
{
	vector<int> selection = d_map.selection();

	if (selection.size() <= 1 || edit_mode != 2)
		return;

	make_backup(false, false, false, true, false);

	// Floor
	int height = d_map.sector(selection[0])->floor();
	for (DWORD a = 0; a < selection.size(); a++)
	{
		height += floor_step;
		d_map.sector(selection[a])->set_floor(height);
	}

	// Ceiling
	height = d_map.sector(selection[0])->ceiling();
	for (DWORD a = 0; a < selection.size(); a++)
	{
		height += ceil_step;
		d_map.sector(selection[a])->set_ceil(height);
	}

	d_map.change_level(MC_SSECTS|MC_LINES);
}

// sector_light_gradient: Creates a light gradient
// -------------------------------------------- >>
void sector_light_gradient(int step)
{
	vector<int> selection = d_map.selection();

	if (selection.size() <= 1 || edit_mode != 2)
		return;

	make_backup(false, false, false, true, false);

	if (step > 255)
	{
		int start = d_map.sector(selection[0])->light_level();
		int end = d_map.sector(selection.back())->light_level();
		step = float(end - start) / selection.size();
	}

	int l = d_map.sector(selection[0])->light_level();
	for (unsigned int a = 0; a < selection.size(); a++)
	{
		if (l > 255)
			l = 255;
		if (l < 0)
			l = 0;

		d_map.sector(selection[a])->set_light(l);

		l += step;
	}

	d_map.change_level(MC_SECTORS);
}

// sector_create_door: Creates a door from any selected sectors
// --------------------------------------------------------- >>
void sector_create_door(string texture)
{
	vector<int> selection = d_map.selection();

	if ((selection.size() == 0 && d_map.hilight() == -1) || edit_mode != 2)
		return;

	make_backup(true, true, false, true, false);

	vector<Sector*> items;

	if (selection.size() == 0)
		items.push_back(d_map.hilight_sector());
	else
	{
		for (unsigned int a = 0; a < selection.size(); a++)
			items.push_back(d_map.sector(selection[a]));
	}

	for (unsigned int a = 0; a < items.size(); a++)
	{
		Sector* s = items[a];

		// Set ceiling to floor
		s->set_ceil(s->floor());
		
		// Go through lines in the sector, set their texture and type accordingly
		for (unsigned int l = 0; l < d_map.n_lines(); l++)
		{
			Line* line = d_map.line(l);

			if (d_map.valid(line->side2()))
			{
				// Any 2-sided lines
				if (line->side2()->get_sector() == s)
				{
					line->side1()->set_texture(texture, TEX_UPPER);

					if (!game.zdoom())
						line->set_special(1);
					else
					{
						line->set_special(12);
						line->set_arg(1, 24);
						line->set_arg(2, 128);
						line->set_flag(0x0200);
						line->set_trigger(1);
					}
				}

				if (line->side1()->get_sector() == s)
				{
					line->side2()->set_texture(texture, TEX_UPPER);

					if (!game.zdoom())
						line->set_special(1);
					else
					{
						line->set_special(12);
						line->set_arg(1, 24);
						line->set_arg(2, 128);
						line->set_flag(0x0200);
						line->set_trigger(1);
					}
				}
			}
			else // 1-sided lines
			{
				if (line->side1()->get_sector() == s)
				{
					line->side1()->set_texture(game.def_tracktex);
					line->set_flag(LINE_LOWERUNPEGGED);
				}
			}
		}
	}

	redraw_map();
	d_map.change_level(MC_SSECTS|MC_LINES);
}

// sector_merge: Merges selected sectors to the first selected sector
// --------------------------------------------------------------- >>
void sector_merge(bool remove_lines)
{
	vector<int> selection = d_map.selection();

	if (selection.size() == 0 || edit_mode != 2)
		return;

	make_backup(true, true, false, true, false);

	int sector = selection[0];

	for (unsigned int l = 0; l < d_map.n_lines(); l++)
	{
		Line* line = d_map.line(l);
		bool s1_in = vector_exists(selection, line->sector_index(true));
		bool s2_in = vector_exists(selection, line->sector_index(false));

		if (s1_in)
			line->set_sector(1, d_map.sector(sector));

		if (s2_in)
			line->set_sector(2, d_map.sector(sector));

		if (remove_lines)
		{
			// If both sides of the line are selected, delete the line
			if (s1_in && s2_in)
			{
				d_map.delete_line(l);
				l--;
			}
		}
	}

	d_map.clear_selection();
	redraw_map();
	d_map.change_level(MC_NODE_REBUILD);
}

// sector_changeheight: Changes floor/ceiling height of hilighted or selected sector(s)
// --------------------------------------------------------------------------------- >>
void sector_changeheight(bool floor, int amount)
{
	vector<int> selection = d_map.selection();

	if (selection.size() == 0)
	{
		Sector* s = d_map.hilight_sector();

		if (s)
		{
			if (floor)
				s->set_floor(s->floor() + amount);
			else
				s->set_ceil(s->ceiling() + amount);
		}
	}
	else
	{
		for (unsigned int a = 0; a < selection.size(); a++)
		{
			Sector* s = d_map.sector(selection[a]);

			if (s)
			{
				if (floor)
					s->set_floor(s->floor() + amount);
				else
					s->set_ceil(s->ceiling() + amount);
			}
		}
	}

	update_infobar();
	d_map.change_level(MC_LINES|MC_SSECTS);
}

void sector_changelight(int amount)
{
	vector<int> selection = d_map.selection();

	if (edit_mode != 2 || (selection.size() == 0 && d_map.hilight() == -1))
		return;

	make_backup(false, false, false, true, false);

	vector<Sector*> sectors;

	// Get sectors to change
	if (selection.size() == 0)
		sectors.push_back(d_map.hilight_sector());
	else
	{
		for (unsigned int a = 0; a < selection.size(); a++)
			vector_add_nodup(sectors, d_map.sector(selection[a]));
	}

	// Change light levels
	for (unsigned int a = 0; a < sectors.size(); a++)
	{
		int light = sectors[a]->light_level();

		if (light == 255)
			light = 256;

		light += amount;

		if (light < 0)
			light = 0;
		if (light > 255)
			light = 255;

		sectors[a]->set_light(light);
	}

	if (d_map.hilight() != -1)
		update_infobar();
}

void sector_paint_tex(int sector, string tex, bool floor, vector<int> &sectors)
{
	if (sector < 0 || sector > (int)d_map.n_sectors())
		return;

	sectors.push_back(sector);

	bool cont;
	do {
		cont = false;
		for (unsigned int a = 0; a < d_map.n_lines(); a++)
		{
			int s1 = d_map.line(a)->sector_index(true);
			int s2 = d_map.line(a)->sector_index(false);

			if (vector_exists(sectors, s1) && s2 != -1 && !(vector_exists(sectors, s2)))
			{
				if ((floor && d_map.sector(s1)->floor() == d_map.sector(s2)->floor()) ||
				    (!floor && d_map.sector(s1)->ceiling() == d_map.sector(s2)->ceiling()))
				{
					vector_add_nodup(sectors, s2);
					cont = true;
					break;
				}
			}

			if (vector_exists(sectors, s2) && s1 != -1 && !(vector_exists(sectors, s1)))
			{
				if ((floor && d_map.sector(s1)->floor() == d_map.sector(s2)->floor()) ||
				    (!floor && d_map.sector(s1)->ceiling() == d_map.sector(s2)->ceiling()))
				{
					vector_add_nodup(sectors, s1);
					cont = true;
					break;
				}
			}
		}
	} while(cont);

	for (unsigned int a = 0; a < sectors.size(); a++)
	{
		int s = sectors[a];

		if (floor)
			d_map.sector(s)->set_ftex(tex);
		else
			d_map.sector(s)->set_ctex(tex);
	}
}



// LINES

// line_extrude: 'extrudes' selected lines by a specified amount
// ---------------------------------------------------------- >>
void line_extrude(int amount)
{
	vector<Line*> lines;
	d_map.get_selection(lines, true);

	if (lines.size() == 0)
		return;

	make_backup(true, true, true, false, false);

	for (unsigned int a = 0; a < lines.size(); a++)
	{
		rect_t rect = lines[a]->get_rect();

		// Get a perpendicular vector to the line
		point2_t line(rect.x2() - rect.x1(), rect.y2() - rect.y1());
		point3_t vec(-line.y, line.x, 0.0f);
		vec = vec.normalize();

		// Add the new vertices
		Vertex* v1 = new Vertex(rect.x1() - (vec.x * amount), rect.y1() - (vec.y * amount), &d_map);
		Vertex* v2 = new Vertex(rect.x2() - (vec.x * amount), rect.y2() - (vec.y * amount), &d_map);

		// Split the line through the new vertices
		lines[a]->split(v2);
		lines[a]->split(v1);
	}

	d_map.clear_selection();
	d_map.change_level(MC_NODE_REBUILD);
	redraw_map();
}

// line_align_x: Aligns selected lines on the x axis
// ---------------------------------------------- >>
void line_align_x()
{
	vector<Line*> lines;
	d_map.get_selection(lines);

	if (lines.size() <= 1)
		return;

	make_backup(false, true, false, false, false);

	int offset = lines[0]->get_rect().length();
	for (unsigned int a = 1; a < lines.size(); a++)
	{
		Side *side = lines[a]->side1();
		if (d_map.valid(side)) side->set_xoff(offset);

		side = lines[a]->side2();
		if (d_map.valid(side)) side->set_xoff(offset);

		offset += lines[a]->get_rect().length();
	}
}

// line_correct_references: Attempts to correct a line's sector references
// -------------------------------------------------------------------- >>
void line_correct_references(int line)
{
	if (line != -1 && d_map.valid(d_map.line(line)))
	{
		make_backup(true, true, false, false, false);

		d_map.line(line)->set_sector(1, d_map.sector(get_side_sector(line, 1)));
		d_map.line(line)->set_sector(2, d_map.sector(get_side_sector(line, 2)));

		return;
	}

	vector<Line*> lines;
	d_map.get_selection(lines, true);

	if (lines.size() == 0)
		return;

	make_backup(true, true, false, false, false);

	for (unsigned int a = 0; a < lines.size(); a++)
	{
		lines[a]->set_sector(1, d_map.sector(get_side_sector(d_map.index(lines[a]), 1)));
		lines[a]->set_sector(2, d_map.sector(get_side_sector(d_map.index(lines[a]), 2)));
	}
}

bool line_hastex(Line* line, bool front, string tex)
{
	Side* s = line->side(front);

	if (!s)
		return false;

	if (line->needs_upper(front) && s->get_texname(TEX_UPPER) == tex)
		return true;
	if (line->needs_middle(front) && s->get_texname(TEX_MIDDLE) == tex)
		return true;
	if (line->needs_lower(front) && s->get_texname(TEX_LOWER) == tex)
		return true;

	return false;
}

void line_paint_tex(int line, bool front, string otex, string ntex, vector<int> &processed_lines)
{
	Line* l = d_map.line(line);
	Side* s = l->side(front);

	if (d_map.valid(s))
	{
		if (l->needs_lower(front) && s->get_texname(TEX_LOWER) == otex)
			s->set_texture(ntex, TEX_LOWER);
		if (l->needs_middle(front) && s->get_texname(TEX_MIDDLE) == otex)
			s->set_texture(ntex, TEX_MIDDLE);
		if (l->needs_upper(front) && s->get_texname(TEX_UPPER) == otex)
			s->set_texture(ntex, TEX_UPPER);
	}

	processed_lines.push_back(line);

	vector<int> a_lines;
	d_map.get_lines_to_vert(d_map.index(l->vertex1()), a_lines);
	d_map.get_lines_to_vert(d_map.index(l->vertex2()), a_lines);

	for (unsigned int a = 0; a < a_lines.size(); a++)
	{
		if (find(processed_lines.begin(), processed_lines.end(), a_lines[a]) == processed_lines.end())
		{
			if (line_hastex(d_map.line(a_lines[a]), true, otex))
				line_paint_tex(a_lines[a], true, otex, ntex, processed_lines);

			if (line_hastex(d_map.line(a_lines[a]), false, otex))
				line_paint_tex(a_lines[a], false, otex, ntex, processed_lines);
		}
	}
}

void line_auto_align_x(int line, int offset, bool front, string tex, int texwidth, vector<int> &processed_lines)
{
	Side* s = d_map.line(line)->side(front);

	if (offset == -51403)
		offset = s->get_xoff();
	else
		s->set_xoff(offset);

	offset += (int)d_map.line(line)->get_rect().length();
	processed_lines.push_back(line);

	if (offset >= texwidth)
		offset -= texwidth;

	vector<int> a_lines;

	if (front)
		d_map.get_lines_to_vert(d_map.index(d_map.line(line)->vertex2()), a_lines);
	else
		d_map.get_lines_to_vert(d_map.index(d_map.line(line)->vertex1()), a_lines);

	for (unsigned int a = 0; a < a_lines.size(); a++)
	{
		if (find(processed_lines.begin(), processed_lines.end(), a_lines[a]) == processed_lines.end())
		{
			Side* sd = d_map.line(a_lines[a])->side(true);
			if (d_map.valid(sd))
			{
				if (sd->get_texname(TEX_LOWER) == tex ||
					sd->get_texname(TEX_MIDDLE) == tex ||
					sd->get_texname(TEX_UPPER) == tex)
					line_auto_align_x(a_lines[a], offset, true, tex, texwidth, processed_lines);
			}

			sd = d_map.line(a_lines[a])->side(false);
			if (d_map.valid(sd))
			{
				if (sd->get_texname(TEX_LOWER) == tex ||
					sd->get_texname(TEX_MIDDLE) == tex ||
					sd->get_texname(TEX_UPPER) == tex)
					line_auto_align_x(a_lines[a], offset, false, tex, texwidth, processed_lines);
			}
		}
	}
}

void line_split(int splits)
{
	vector<Line*> lines;
	d_map.get_selection(lines, true);

	if (lines.size() == 0)
		return;

	make_backup(true, true, true, false, false);

	for (unsigned int a = 0; a < lines.size(); a++)
	{
		rect_t rect = lines[a]->get_rect();

		float x_split = float(rect.x2() - rect.x1()) / (float)splits;
		float y_split = float(rect.y2() - rect.y1()) / (float)splits;

		Line* line = lines[a];
		for (int a = 1; a < splits; a++)
		{
			int x = rect.x1() + int(a * x_split);
			int y = rect.y1() + int(a * y_split);

			Vertex *v = new Vertex(x, y, &d_map);
			line = line->split(v);
		}
	}

	redraw_map();
}

void line_split_at(point2_t pos)
{
	int l = d_map.get_hilight_line(pos);
	Line* line = d_map.line(l);
	if (!d_map.valid(line))
		return;

	make_backup(true, true, true, false, false);

	rect_t rect = line->get_rect();

	// If not zero length
	double u = 0;
	if(rect.length() != 0)
	{
		// Get the intersection offset
		u = double((pos.x - rect.x1()) * (rect.x2() - rect.x1()) +
			(pos.y - rect.y1()) * (rect.y2() - rect.y1())) /
			(rect.length() * rect.length());

		// Limit intersection offset to the line
		double lbound = 1 / rect.length();
		double ubound = 1 - lbound;
		if(u < lbound) u = lbound;
		if(u > ubound) u = ubound;
	}
	else
		u = 0;

	// Calculate intersection point
	double ix = double(rect.x1()) + u * double(rect.x2() - rect.x1());
	double iy = double(rect.y1()) + u * double(rect.y2() - rect.y1());

	// Create a new vertex at intersection point and split the line there
	Vertex *v = new Vertex(lround(ix), lround(iy), &d_map);
	line->split(v);
}

void line_flip(bool verts, bool sides)
{
	vector<Line*> lines;
	d_map.get_selection(lines, true);

	if (lines.size() == 0)
		return;

	make_backup(true, false, false, false, false);

	for (unsigned int a = 0; a < lines.size(); a++)
		lines[a]->flip(verts, sides);
}
