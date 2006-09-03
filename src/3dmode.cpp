
#include "main.h"
#include "structs.h"
#include "doom_map.h"
#include "dm_sector.h"
#include "dm_thing.h"
#include "dm_line.h"
#include "dm_side.h"
#include "camera.h"
#include "render.h"
#include "tex_browser.h"
#include "3dmode.h"
#include "bsp.h"
#include "edit_misc.h"

float grav = 0.5f;

// Copy/paste stuff
string	c_wall = "";
string	c_flat = "";
Side*	copy_side;

extern DoomMap d_map;
extern Camera camera;
extern double elapsed;

extern Line* hl_line;
extern BYTE hl_part;
extern int hl_sector;

extern vector<point2_t> map_verts;
extern vector<gl_vertex_t> gl_verts;
extern vector<gl_seg_t>	gl_segs;
extern vector<gl_ssect_t> gl_ssects;
extern vector<gl_node_t> gl_nodes;
extern int* ssect_sectors;
extern bool mix_tex;

void init_3d_mode()
{
	if (ssect_sectors) delete ssect_sectors;
	ssect_sectors = new int[gl_ssects.size()];

	for (int a = 0; a < gl_ssects.size(); a++)
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
}

// apply_gravity: Applies gravity to the camera
// ----------------------------------------- >>
void apply_gravity()
{
	if (elapsed == 0)
		elapsed = 1;

	float mult = elapsed * 0.1f;
	int sector = d_map.get_hilight_sector(point2_t(camera.position.x, camera.position.y));

	if (sector != -1)
	{
		//float f_h = plane_height(sector_info[sector].f_plane, camera.position.x, camera.position.y) + (40 * SCALE_3D);
		//float c_h = plane_height(sector_info[sector].c_plane, camera.position.x, camera.position.y);
		float f_h = (float)d_map.sector(sector)->floor() + 40.0f;
		float c_h = (float)d_map.sector(sector)->ceiling();

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

	for (int a = 0; a < d_map.n_things(); a++)
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
	}
	else
	{
		d_map.sector(sector)->set_ceil(d_map.sector(sector)->ceiling() + amount);
		add_3d_message(s_fmt(_T("Ceiling Height: %d"), d_map.sector(sector)->ceiling()));
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

		if (side)
			otex = hl_line->side1()->get_texname(part);
		else
			otex = hl_line->side2()->get_texname(part);

		setup_tex_browser(1);
		TextureBrowser tb(otex, true);

		if (tb.ShowModal() == wxID_OK && tb.get_texture() != "")
		{
			if (side)
				hl_line->side1()->set_texture(tb.get_texture(), part);
			else
				hl_line->side2()->set_texture(tb.get_texture(), part);
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

		if (tb.ShowModal() == wxID_OK && tb.get_texture() != "")
		{
			if (hl_part == PART_FLOOR)
				d_map.sector(hl_sector)->set_ftex(tb.get_texture());

			if (hl_part == PART_CEIL)
				d_map.sector(hl_sector)->set_ctex(tb.get_texture());
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

		add_3d_message(s_fmt(_T("Copied texture \"%s\""), chr(c_wall)));
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

		add_3d_message(s_fmt(_T("Copied texture \"%s\""), chr(ctex)));
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

		if (c_wall != _T(""))
		{
			if (!paint)
			{
				hl_line->side(side)->set_texture(c_wall, part);
				add_3d_message(s_fmt(_T("Pasted texture \"%s\""), chr(c_wall)));
			}
			else
			{
				vector<int> lines;
				line_paint_tex(d_map.index(hl_line), side, hl_line->side(side)->get_texname(part), c_wall, lines);

				add_3d_message(s_fmt(_T("Painted texture \"%s\""), chr(c_wall)));
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

		if (ntex != _T(""))
		{
			if (hl_part == PART_FLOOR)
				d_map.sector(hl_sector)->set_ftex(ntex);
			if (hl_part == PART_CEIL)
				d_map.sector(hl_sector)->set_ctex(ntex);

			add_3d_message(s_fmt(_T("Pasted texture \"%s\""), chr(ntex)));
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

	/*
	else if (hl_thing)
	{
		for (int a = 0; a < things_3d.size(); a++)
		{
			if (things_3d[a] == hl_thing)
			{
				map.things[a]->z = 0;
				add_3d_message(parse_string("Z height reset", map.things[a]->z));
				map.change_level(MC_SAVE_NEEDED);
				return;
			}
		}
	}
	*/
}

void auto_align_x_3d()
{
	if (!hl_line)
		return;

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

	add_3d_message("Auto X-Alignment done");
	d_map.change_level(MC_SAVE_NEEDED);
}
