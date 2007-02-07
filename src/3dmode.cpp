
#include "main.h"
#include "structs.h"
#include "doom_map.h"
#include "dm_sector.h"
#include "dm_thing.h"
#include "dm_line.h"
#include "dm_side.h"
#include "dm_vertex.h"
#include "camera.h"
#include "render.h"
#include "tex_browser.h"
#include "3dmode.h"
#include "bsp.h"
#include "edit_misc.h"
#include "undoredo.h"
#include "splash.h"
#include "mathstuff.h"
#include "game_config.h"

float grav = 0.5f;

BYTE	*vis_sectors = NULL;

// Copy/paste stuff
string	c_wall = _T("");
string	c_flat = _T("");
Side*	copy_side;
int		c_ttype = -1;

extern DoomMap d_map;
extern Camera camera;
extern double elapsed;

extern Line* hl_line;
extern BYTE hl_part;
extern int hl_sector;
extern Thing* hl_thing;

extern vector<point2_t> map_verts;
extern vector<gl_vertex_t> gl_verts;
extern vector<gl_seg_t>	gl_segs;
extern vector<gl_ssect_t> gl_ssects;
extern vector<gl_node_t> gl_nodes;
extern int* ssect_sectors;
extern bool mix_tex;
extern plane_t *f_planes, *c_planes;
extern GameConfig game;


void setup_slopes_3d(int sector)
{
	// Check lines
	for (DWORD a = 0; a < d_map.n_lines(); a++)
	{
		Line *l = d_map.line(a);

		if (sector != -1 &&
			l->sector_index() != sector &&
			l->sector_index(false) != sector)
			continue;

		// Plane_Align
		if (game.zdoom() && l->get_special() == 181)
		{
			// Get start height and end height depending on the type of slope
			float f_start_height = 0;
			float f_end_height = 0;
			float c_start_height = 0;
			float c_end_height = 0;
			bool valid = false;
			int sector = -1;

			// Slope front side floor
			if (l->arg(0) == 1 && d_map.valid(l->side1()) && d_map.valid(l->side2()))
			{
				f_start_height = l->side2()->get_sector()->floor();
				f_end_height = l->side1()->get_sector()->floor();
				sector = l->sector_index();
			}

			// Slope back side floor
			if (l->arg(0) == 2 && d_map.valid(l->side1()) && d_map.valid(l->side2()))
			{
				f_start_height = l->side1()->get_sector()->floor();
				f_end_height = l->side2()->get_sector()->floor();
				sector = l->sector_index(false);
			}

			// Slope front side ceiling
			if (l->arg(1) == 1 && d_map.valid(l->side1()) && d_map.valid(l->side2()))
			{
				c_start_height = l->side2()->get_sector()->ceiling();
				c_end_height = l->side1()->get_sector()->ceiling();
				sector = l->sector_index();
			}

			// Slope back side ceiling
			if (l->arg(1) == 2 && d_map.valid(l->side1()) && d_map.valid(l->side2()))
			{
				c_start_height = l->side1()->get_sector()->ceiling();
				c_end_height = l->side2()->get_sector()->ceiling();
				sector = l->sector_index(false);
			}

			if (sector != -1)
			{
				bool c_sloped = false;
				bool f_sloped = false;
				point3_t c_1, c_2, c_3;
				point3_t f_1, f_2, f_3;

				// Set start point
				rect_t s_line = l->get_rect();
				if (l->arg(0) > 0)
				{
					f_1.set(s_line.x1(), s_line.y1(), f_start_height);
					f_2.set(s_line.x2(), s_line.y2(), f_start_height);
					f_sloped = true;
				}

				if (l->arg(1) > 0)
				{
					c_1.set(s_line.x1(), s_line.y1(), c_start_height);
					c_2.set(s_line.x2(), s_line.y2(), c_start_height);
					c_sloped = true;
				}

				// Get end point
				point3_t mid(float(s_line.x1()) + ((s_line.x2() - s_line.x1()) / 2.0f),
					float(s_line.y1()) + ((s_line.y2() - s_line.y1()) / 2.0f), 0.0f);
				point3_t dir(float(s_line.y2() - s_line.y1()), -float(s_line.x2() - s_line.x1()), 0.1f);
				point3_t side(mid.x + dir.x, mid.y + dir.y, mid.z + dir.z);
				float max_dist = 0.0f;
				float min_dist = 0.0f;

				// Run through all lines in the sector and get their vertices
				vector<Vertex*> verts;
				for (DWORD l2 = 0; l2 < d_map.n_lines(); l2++)
				{
					Line *line = d_map.line(l2);
					if ((line->sector_index() == sector ||
						line->sector_index(false) == sector) &&
						l2 != a)
					{
						verts.push_back(line->vertex1());
						verts.push_back(line->vertex2());
					}
				}

				for (DWORD b = 0; b < verts.size(); b++)
				{
					float x1 = (float)verts[b]->x_pos();
					float x2 = x1 + float(s_line.x2() - s_line.x1());
					float y1 = (float)verts[b]->y_pos();
					float y2 = y1 + float(s_line.y2() - s_line.y1());

					if ((y2 - y1) * (side.x - mid.x) - (x2 - x1) * (side.y - mid.y) != 0.0f)
					{
						float dist = ((x2 - x1) * (mid.y - y1) - (y2 - y1) * (mid.x - (x1))) /
							((y2 - y1) * (side.x - mid.x) - (x2 - x1) * (side.y - mid.y));

						if (dist > max_dist)
							max_dist = dist;

						if (dist < min_dist)
							min_dist = dist;
					}
				}

				// Get 2 intersection points: max for side1 and min for side2
				float max_x = float(mid.x) + (float(dir.x) * max_dist);
				float max_y = float(mid.y) + (float(dir.y) * max_dist);
				float min_x = float(mid.x) + (float(dir.x) * min_dist);
				float min_y = float(mid.y) + (float(dir.y) * min_dist);

				if (l->arg(0) == 1)
					f_3.set(max_x, max_y, f_end_height);

				if (l->arg(0) == 2)
					f_3.set(min_x, min_y, f_end_height);

				if (l->arg(1) == 1)
					c_3.set(max_x, max_y, c_end_height);

				if (l->arg(1) == 2)
					c_3.set(min_x, min_y, c_end_height);

				if (f_sloped)
					f_planes[sector].from_triangle(f_3, f_1, f_2);

				if (c_sloped)
					c_planes[sector].from_triangle(c_3, c_1, c_2);
			}
		}
	}
}

void init_3d_mode()
{
	if (ssect_sectors) delete[] ssect_sectors;
	ssect_sectors = new int[gl_ssects.size()];

	if (vis_sectors) delete[] vis_sectors;
	vis_sectors = new BYTE[d_map.n_sectors()];

	splash(_T("Init Things..."), true);
	for (int a = 0; a < (int)d_map.n_things(); a++)
	{
		d_map.thing(a)->get_sector();
		if (a % 20 == 0)
			splash_progress((double)a / (double)d_map.n_things());
	}
	splash_hide();

	for (unsigned int a = 0; a < gl_ssects.size(); a++)
	{
		ssect_sectors[a] = -1;

		int start = gl_ssects[a].first_seg;
		int end = start + gl_ssects[a].count;

		for (int s = start; s < end; s++)
		{
			if (gl_segs[s].linedef != SEG_MINISEG)
			{
				ssect_sectors[a] = d_map.line(gl_segs[s].linedef)->sector_index(gl_segs[s].side == 0);
				break;
			}
		}
	}

	if (f_planes)
		delete[] f_planes;
	if (c_planes)
		delete[] c_planes;

	f_planes = new plane_t[d_map.n_sectors()];
	c_planes = new plane_t[d_map.n_sectors()];

	if (game.zdoom())
		splash(_T("Init Slopes..."), true);

	for (DWORD a = 0; a < d_map.n_sectors(); a++)
	{
		f_planes[a].set(0.0f, 0.0f, 1.0f, d_map.sector(a)->floor());
		c_planes[a].set(0.0f, 0.0f, 1.0f, d_map.sector(a)->ceiling());

		if (game.zdoom())
		{
			setup_slopes_3d(a);
			if (a % 20 == 0)
				splash_progress((double)a / (double)d_map.n_sectors());
		}
	}
}


// apply_gravity: Applies gravity to the camera
// ----------------------------------------- >>
void apply_gravity()
{
	if (elapsed == 0)
		elapsed = 1;

	float mult = elapsed * 0.1f;
	int sector = d_map.get_hilight_sector(point2_t(camera.position.x, camera.position.y));

	if (sector != -1 && sector < d_map.n_sectors())
	{
		float f_h = plane_height(f_planes[sector], camera.position.x, camera.position.y) + 40.0f;
		float c_h = plane_height(c_planes[sector], camera.position.x, camera.position.y);
		//float f_h = (float)d_map.sector(sector)->floor() + 40.0f;
		//float c_h = (float)d_map.sector(sector)->ceiling();

		if (f_h >= c_h)
		{
			f_h = c_h - 4;
			float diff = f_h - camera.position.z;
			camera.position.z = f_h;
			camera.view.z += diff;
			grav = 0.5f;
			return;
		}

		if (camera.position.z > f_h + grav)
		{
			grav = grav * (1.2 * mult);
			camera.position.z -= grav;
			camera.view.z -= grav;
		}
		else if (camera.position.z < f_h - grav)
		{
			grav = grav * (1.2 * mult);
			camera.position.z += grav;
			camera.view.z += grav;
		}

		if (camera.position.z > f_h - grav && camera.position.z < f_h + grav)
		{
			float diff = f_h - camera.position.z;
			camera.position.z = f_h;
			camera.view.z += diff;
			grav = 0.5f;
			return;
		}
	}
}

void camera_to_start()
{
	point3_t pos;
	point3_t view;

	for (DWORD a = 0; a < d_map.n_things(); a++)
	{
		Thing* t = d_map.thing(a);
		if (t->get_type() == 1)
		{
			pos.x = t->pos().x;
			pos.y = t->pos().y;

			int sector = d_map.get_hilight_sector(point2_t(t->pos().x, t->pos().y));
			
			if (sector != -1)
				pos.z = (d_map.sector(sector)->floor() + 40);

			if (t->get_angle() == 0)			// east
				view.set(1.0f, 0.0f, 0.0f);
			else if (t->get_angle() == 45)		// northeast
				view.set(0.7f, 0.7f, 0.0f);
			else if (t->get_angle() == 90)		// north
				view.set(0.0f, 1.0f, 0.0f);
			else if (t->get_angle() == 135)		// northwest
				view.set(-0.7f, 0.7f, 0.0f);
			else if (t->get_angle() == 180)		// west
				view.set(-1.0f, 0.0f, 0.0f);
			else if (t->get_angle() == 225)		// southwest
				view.set(-0.7f, -0.7f, 0.0f);
			else if (t->get_angle() == 270)		// south
				view.set(0.0f, -1.0f, 0.0f);
			else if (t->get_angle() == 315)		// southeast
				view.set(0.7f, -0.7f, 0.0f);
			else
				view.set(1.0f, 0.0f, 0.0f);

			camera.position.set(pos);
			camera.view.set(pos + view);
			break;
		}
	}
}

void change_offsets_3d(int x, int y)
{
	if (hl_line)
	{
		Side* side;

		if (hl_part == PART_MID1 || hl_part == PART_LO1 || hl_part == PART_UP1)
			side = hl_line->side1();
		else
			side = hl_line->side2();

		side->set_xoff(side->get_xoff() + x);
		side->set_yoff(side->get_yoff() + y);
		add_3d_message(s_fmt(_T("Offsets: (%d, %d)"), side->get_xoff(), side->get_yoff()));
		d_map.change_level(MC_SAVE_NEEDED);
	}
}

void change_sector_height_3d(int amount, bool floor, bool force)
{
	int sector = -1;
	if (hl_line)
	{
		if (hl_part == PART_MID1 || hl_part == PART_UP1 || hl_part == PART_LO1)
			sector = hl_line->sector_index(true);
		else
			sector = hl_line->sector_index(false);
	}
	else if (hl_sector != -1)
	{
		sector = hl_sector;

		if (!force)
			floor = (hl_part == PART_FLOOR);
	}

	if (sector == -1)
		return;

	if (floor)
	{
		d_map.sector(sector)->set_floor(d_map.sector(sector)->floor() + amount);
		add_3d_message(s_fmt(_T("Floor Height: %d"), d_map.sector(sector)->floor()));
		f_planes[sector].set(0.0f, 0.0f, 1.0f, d_map.sector(sector)->floor());
	}
	else
	{
		d_map.sector(sector)->set_ceil(d_map.sector(sector)->ceiling() + amount);
		add_3d_message(s_fmt(_T("Ceiling Height: %d"), d_map.sector(sector)->ceiling()));
		c_planes[sector].set(0.0f, 0.0f, 1.0f, d_map.sector(sector)->ceiling());
	}

	d_map.change_level(MC_SAVE_NEEDED);
}

void change_texture_3d(bool paint)
{
	if (hl_line)
	{
		int part = -1;
		bool side = true;

		if (hl_part == PART_MID2 || hl_part == PART_LO2 || hl_part == PART_UP2)
			side = false;

		if (hl_part == PART_MID1 || hl_part == PART_MID2)
			part = 1;

		if (hl_part == PART_UP1 || hl_part == PART_UP2)
			part = 0;

		if (hl_part == PART_LO1 || hl_part == PART_LO2)
			part = 2;

		string otex = _T("");

		//log_message("1");
		if (side)
			otex = hl_line->side1()->get_texname(part);
		else
			otex = hl_line->side2()->get_texname(part);

		//log_message("2");
		setup_tex_browser(1);
		TextureBrowser tb(otex, true);

		//log_message("3");
		if (tb.ShowModal() == wxID_OK && tb.get_texture().size())
		{
			make_backup(BKUP_SIDES|BKUP_3DMODE|BKUP_MODIFY);

			if (paint)
			{
				vector<int> lines;
				line_paint_tex(d_map.index(hl_line), side, otex, tb.get_texture(), lines);
			}
			else
			{
				if (side)
					hl_line->side1()->set_texture(tb.get_texture(), part);
				else
					hl_line->side2()->set_texture(tb.get_texture(), part);
			}
		}

		reset_3d_mouse();
	}
	else if (hl_sector != -1)
	{
		string otex = _T("");

		if (hl_part == PART_FLOOR)
			otex = d_map.sector(hl_sector)->tex_floor();

		if (hl_part == PART_CEIL)
			otex = d_map.sector(hl_sector)->tex_ceil();

		setup_tex_browser(2);
		TextureBrowser tb(otex, true);

		if (tb.ShowModal() == wxID_OK && tb.get_texture().size())
		{
			make_backup(BKUP_SECTORS|BKUP_3DMODE|BKUP_MODIFY);

			if (paint)
			{
				vector<int> temp;
				sector_paint_tex(hl_sector, tb.get_texture(), hl_part == PART_FLOOR, temp);
			}
			else
			{
				if (hl_part == PART_FLOOR)
					d_map.sector(hl_sector)->set_ftex(tb.get_texture());
				else if (hl_part == PART_CEIL)
					d_map.sector(hl_sector)->set_ctex(tb.get_texture());
			}
		}

		reset_3d_mouse();
	}
	else if (hl_thing)
	{
		string othing = s_fmt(_T("%d"), hl_thing->get_type());
		
		setup_tex_browser(3);
		TextureBrowser tb(othing, true);

		if (tb.ShowModal() == wxID_OK && tb.get_texture().size())
		{
			make_backup(BKUP_THINGS|BKUP_3DMODE|BKUP_MODIFY);
			hl_thing->set_type(atoi(chr(tb.get_texture())));
		}

		reset_3d_mouse();
	}
}

void copy_texture_3d()
{
	if (hl_line)
	{
		bool side = true;
		BYTE part = 1;

		if (hl_part == PART_MID2 || hl_part == PART_LO2 || hl_part == PART_UP2)
			side = false;

		if (hl_part == PART_UP1 || hl_part == PART_UP2)
			part = 0;

		if (hl_part == PART_LO1 || hl_part == PART_LO2)
			part = 2;

		if (side)
			c_wall = hl_line->side1()->get_texname(part);
		else
			c_wall = hl_line->side2()->get_texname(part);

		add_3d_message(s_fmt(_T("Copied texture \"%s\""), c_wall.c_str()));
	}
	else if (hl_sector != -1)
	{
		string ctex = _T("");

		if (hl_part == PART_FLOOR)
			ctex = d_map.sector(hl_sector)->tex_floor();
		if (hl_part == PART_CEIL)
			ctex = d_map.sector(hl_sector)->tex_ceil();

		if (mix_tex)
			c_wall = ctex;
		else
			c_flat = ctex;

		add_3d_message(s_fmt(_T("Copied texture \"%s\""), ctex.c_str()));
	}
	else if (hl_thing)
	{
		c_ttype = hl_thing->get_type();
		add_3d_message(s_fmt(_T("Copied thing type \"%s\""), hl_thing->get_ttype()->name.c_str()));
	}
}

void paste_texture_3d(bool paint)
{
	if (hl_line)
	{
		bool side = true;
		BYTE part = 1;

		if (hl_part == PART_MID2 || hl_part == PART_LO2 || hl_part == PART_UP2)
			side = false;

		if (hl_part == PART_UP1 || hl_part == PART_UP2)
			part = 0;

		if (hl_part == PART_LO1 || hl_part == PART_LO2)
			part = 2;

		if (c_wall.size())
		{
			make_backup(BKUP_SIDES|BKUP_3DMODE|BKUP_MODIFY);

			if (!paint)
			{
				hl_line->side(side)->set_texture(c_wall, part);
				add_3d_message(s_fmt(_T("Pasted texture \"%s\""), c_wall.c_str()));
			}
			else
			{
				vector<int> lines;
				line_paint_tex(d_map.index(hl_line), side, hl_line->side(side)->get_texname(part), c_wall, lines);

				add_3d_message(s_fmt(_T("Painted texture \"%s\""), c_wall.c_str()));
			}
		}
	}
	else if (hl_sector != -1)
	{
		string ntex;

		if (mix_tex)
			ntex = c_wall;
		else
			ntex = c_flat;

		if (ntex.size())
		{
			make_backup(BKUP_SECTORS|BKUP_3DMODE|BKUP_MODIFY);

			if (paint)
			{
				vector<int> temp;
				sector_paint_tex(hl_sector, ntex, hl_part == PART_FLOOR, temp);
				add_3d_message(s_fmt(_T("Painted texture \"%s\""), ntex.c_str()));
			}
			else
			{
				if (hl_part == PART_FLOOR)
					d_map.sector(hl_sector)->set_ftex(ntex);
				else if (hl_part == PART_CEIL)
					d_map.sector(hl_sector)->set_ctex(ntex);

				add_3d_message(s_fmt(_T("Pasted texture \"%s\""), ntex.c_str()));
			}
		}
	}
	else if (hl_thing)
	{
		if (c_ttype != -1)
		{
			hl_thing->set_type(c_ttype);
			add_3d_message(s_fmt(_T("Pasted thing type \"%s\""), hl_thing->get_ttype()->name.c_str()));
		}
	}
}

void copy_side_3d(bool xoff, bool yoff, bool textures)
{
	if (hl_line)
	{
		if (!copy_side)
			copy_side = new Side(NULL);

		Side *side = NULL;
		
		if (hl_part == PART_MID1 || hl_part == PART_UP1 || hl_part == PART_LO1)
			side = hl_line->side1();
		else
			side = hl_line->side2();

		if (xoff)
		{
			copy_side->set_xoff(side->get_xoff());
			add_3d_message(_T("Copied X offset"));
		}

		if (yoff)
		{
			copy_side->set_yoff(side->get_yoff());
			add_3d_message(_T("Copied Y offset"));
		}

		if (textures)
		{
			copy_side->set_texture(side->get_texname(0), 0);
			copy_side->set_texture(side->get_texname(1), 1);
			copy_side->set_texture(side->get_texname(2), 2);
			add_3d_message(_T("Copied textures"));
		}
	}
}

void paste_side_3d(bool xoff, bool yoff, bool textures)
{
	if (hl_line && copy_side)
	{
		Side *side = NULL;
		make_backup(BKUP_SIDES|BKUP_3DMODE|BKUP_MODIFY);

		if (hl_part == PART_MID1 || hl_part == PART_UP1 || hl_part == PART_LO1)
			side = hl_line->side1();
		else
			side = hl_line->side2();

		if (xoff)
		{
			side->set_xoff(copy_side->get_xoff());
			add_3d_message(_T("Copied X offset"));
		}

		if (yoff)
		{
			side->set_yoff(copy_side->get_yoff());
			add_3d_message(_T("Copied Y offset"));
		}

		if (textures)
		{
			side->set_texture(copy_side->get_texname(0), 0);
			side->set_texture(copy_side->get_texname(1), 1);
			side->set_texture(copy_side->get_texname(2), 2);
			add_3d_message(_T("Copied textures"));
		}

		d_map.change_level(MC_SAVE_NEEDED);
	}
}

void toggle_texture_peg_3d(bool upper)
{
	if (hl_line)
	{
		string msg;
		if (upper)
		{
			hl_line->toggle_flag(0x0008);
			msg = _T("Upper texture unpegged: ");
			if (hl_line->check_flag(0x0008))
				msg += _T("On");
			else
				msg += _T("Off");
		}
		else
		{
			hl_line->toggle_flag(0x0010);
			msg = _T("Lower texture unpegged: ");
			if (hl_line->check_flag(0x0010))
				msg += _T("On");
			else
				msg += _T("Off");
		}

		add_3d_message(msg);
		d_map.change_level(MC_SAVE_NEEDED);
	}
}

void change_light_3d(int amount)
{
	int sector;

	if (hl_line)
	{
		if (hl_part == PART_MID1 || hl_part == PART_UP1 || hl_part == PART_LO1)
			sector = hl_line->sector_index(true);
		else
			sector = hl_line->sector_index(false);
	}
	else if (hl_sector != -1)
		sector = hl_sector;
	else
		return;

	int light = d_map.sector(sector)->light_level();
	
	if (light == 255 && amount < -1)
		light = 256;

	light += amount;

	if (light > 255)
		light = 255;
	if (light < 0)
		light = 0;
	
	d_map.sector(sector)->set_light(light);

	add_3d_message(s_fmt(_T("Light level: %d"), light));
	d_map.change_level(MC_SAVE_NEEDED);
}

void reset_offsets_3d(bool x, bool y)
{
	if (hl_line)
	{
		Side* side = NULL;
		make_backup(BKUP_SIDES|BKUP_3DMODE|BKUP_MODIFY);

		if (hl_part == PART_MID2 || hl_part == PART_LO2 || hl_part == PART_UP2)
			side = hl_line->side2();
		else
			side = hl_line->side1();

		if (x)
		{
			side->set_xoff(0);
			add_3d_message(_T("X Offset reset"));
		}

		if (y)
		{
			side->set_yoff(0);
			add_3d_message(_T("Y Offset reset"));
		}

		d_map.change_level(MC_SAVE_NEEDED);
	}

	else if (hl_thing)
	{
		hl_thing->set_z(0);
		add_3d_message(_T("Z height reset"));
		d_map.change_level(MC_SAVE_NEEDED);
		return;
	}
}

void auto_align_x_3d()
{
	if (!hl_line)
		return;

	make_backup(BKUP_SIDES|BKUP_3DMODE|BKUP_MODIFY);

	bool side = true;
	BYTE part = 1;

	if (hl_part == PART_MID2 || hl_part == PART_LO2 || hl_part == PART_UP2)
		side = false;

	if (hl_part == PART_UP1 || hl_part == PART_UP2)
		part = 0;

	if (hl_part == PART_LO1 || hl_part == PART_LO2)
		part = 2;

	Texture* tex = hl_line->side(side)->get_tex(part);

	vector<int> lines;
	line_auto_align_x(d_map.index(hl_line), -51403, side, tex->name, tex->width, lines);

	add_3d_message(_T("Auto X-Alignment done"));
	d_map.change_level(MC_SAVE_NEEDED);
}


void change_thing_angle_3d(int amount)
{
	if (hl_thing)
	{
		hl_thing->set_angle(hl_thing->get_angle() + amount);
		add_3d_message(s_fmt(_T("Angle: %s"), hl_thing->angle_string().c_str()));
		d_map.change_level(MC_SAVE_NEEDED);
	}
}

void change_thing_z_3d(int amount)
{
	if (hl_thing)
	{
		// Disallow changing z height of slope things for now
		if (hl_thing->get_type() == 9500 || hl_thing->get_type() == 9501)
			return;

		hl_thing->set_z(hl_thing->z_pos() + amount);
		add_3d_message(s_fmt(_T("Z Height: %d"), hl_thing->z_pos()));
		d_map.change_level(MC_SAVE_NEEDED);
	}
}
