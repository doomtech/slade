
#include "main.h"
#include "doom_map.h"
#include "dm_line.h"
#include "dm_side.h"
#include "dm_sector.h"
#include "dm_vertex.h"
#include "dm_thing.h"
#include "camera.h"
#include "textures.h"
#include "draw.h"
#include "bsp.h"
#include "mathstuff.h"
#include "render.h"
#include "game_config.h"

#include <GL/gl.h>
#include <GL/glu.h>

struct fademsg_t
{
	string	message;
	int		fade;
};

vector<fademsg_t> messages_3d;
GLuint last_tex = 999999;

vector<Line*>	trans_lines;

int vis_line = 0;
int vis_ssect = 0;
int vis_thing = 0;
float render_hilight_width = 3.0f;
float min_dist = -1;
int *ssect_sectors = NULL;
rgba_t sky_avg_col;

plane_t *f_planes = NULL;
plane_t *c_planes = NULL;

Line*			hl_line = NULL;
BYTE			hl_part = 0;
int				hl_sector = -1;
Thing*			hl_thing = NULL;

CVAR(Bool, render_fog, true, CVAR_SAVE)
CVAR(Bool, render_fullbright, false, CVAR_SAVE)
CVAR(Bool, render_wireframe, false, 0)
CVAR(Bool, render_hilight, true, CVAR_SAVE)
CVAR(Bool, render_thing_hilight, true, CVAR_SAVE)
CVAR(Bool, render_testinfo, false, 0)
CVAR(Bool, render_sky, true, CVAR_SAVE)
CVAR(Int, render_things, 2, CVAR_SAVE)
CVAR(Int, render_text_scale, 1, CVAR_SAVE)

extern DoomMap d_map;
extern Camera camera;
extern rgba_t col_3d_crosshair, col_3d_hilight_line;
extern bool* lines_visible;
extern vector<gl_ssect_t> gl_ssects;
extern vector<gl_seg_t> gl_segs;
extern vector<gl_vertex_t> gl_verts;
extern vector<point2_t> map_verts;
extern bool* ssects_visible;
extern BYTE *vis_sectors;
extern GameConfig game;

void bind_tex(GLuint id)
{
	if (id != last_tex)
	{
		glBindTexture(GL_TEXTURE_2D, id);
		last_tex = id;
	}
}

void set_light(rgba_t col, int light)
{
	if (light > 255)
		light = 255;
	if (light < 0)
		light = 0;

	if ((render_fullbright || light == 255) && col.r == col.g && col.r == col.b)
	{
		glDisable(GL_FOG);
		glColor4f(1.0f, 1.0f, 1.0f, col.a);
		return;
	}

	if (render_fog)
	{
		glEnable(GL_FOG);
		glFogf(GL_FOG_END, ((light/200.0f) * (light/200.0f) * 40.0f));
	}
	else
		glDisable(GL_FOG);

	// If we have a non-coloured light, darken it a bit to
	// closer resemble the software renderer light level
	if (col.r == col.g && col.r == col.b)
		glColor4f(col.fr()*(col.fr()*1.3f), col.fg()*(col.fg()*1.3f), col.fb()*(col.fb()*1.3f), col.fa());
	else
		glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
}

void render_line_trans(Line* line, bool s, bool hl = false)
{
	if (line->sector_index(true) == -1)
		return;

	if (!line->needs_middle(s))
		return;

	Side* side = NULL;

	if (s)
		side = line->side1();
	else
		side = line->side2();

	BYTE a = 255;
	if (game.zdoom() && line->get_special() == 208 && line->arg(0) == 0)
	{
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);
		a = line->arg(1);
	}
	else
	{
		glEnable(GL_ALPHA_TEST);
		glDepthMask(GL_TRUE);
	}

	int l = side->get_sector()->light_level();
	set_light(rgba_t(l, l, l, a), l);

	rect_t rect = line->get_rect();
	int length = lround(rect.length());
	int ceil1 = line->side1()->get_sector()->ceiling();
	int floor1 = line->side1()->get_sector()->floor();
	int ceil2 = line->side2()->get_sector()->ceiling();
	int floor2 = line->side2()->get_sector()->floor();

	float dist = line_intersect(camera.position, camera.view, (float)rect.x1(), (float)rect.y1(), (float)rect.x2(), (float)rect.y2());
	bool front = (determine_line_side(rect, camera.position.x, camera.position.y) > 0);

	// Get texture
	Texture* tex = side->get_tex(TEX_MIDDLE);
	bind_tex(tex->get_gl_id());

	// Setup texture coordinates
	rect_t t_c;
	int t = min(ceil1, ceil2);
	int b = max(floor1, floor2);
	int rt = 0;
	int rb = 0;

	if (line->check_flag(0x0010))
	{
		rb = b + side->get_yoff();
		rt = rb + tex->height;
	}
	else
	{
		rt = t + side->get_yoff();
		rb = rt - tex->height;
	}

	int h = min(rt, t) - max(rb, b);

	t_c.tl.set(side->get_xoff(), 0);
	t_c.br.set(side->get_xoff() + length, h);

	if (rt > t)
	{
		t_c.tl.y = rt - t;
		t_c.br.y = t_c.tl.y + h;
		rt = t;
	}

	if (rb < b)
		rb = b;

	// Draw
	if (s)
	{
		if (hl)
		{
			// Draw hilight
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			set_gl_colour(col_3d_hilight_line);
			glDisable(GL_FOG);
			glBegin(GL_QUADS);
			glVertex3d(rect.x1(), rect.y1(), rt);
			glVertex3d(rect.x2(), rect.y2(), rt);
			glVertex3d(rect.x2(), rect.y2(), rb);
			glVertex3d(rect.x1(), rect.y1(), rb);
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_FOG);
		}
		else
		{
			glBegin(GL_TRIANGLE_FAN);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), rt);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), rt);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), rb);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), rb);

			glEnd();

			// Check for hilight
			if ((dist < min_dist || min_dist == -1) && dist != -1 && front)
			{
				point3_t hit_point(camera.position.x + ((camera.view - camera.position).x * dist),
					camera.position.y + ((camera.view - camera.position).y * dist),
					camera.position.z + ((camera.view - camera.position).z * dist));

				if (hit_point.z > rb && hit_point.z < rt)
				{
					hl_line = line;
					hl_part = PART_MID1;
					min_dist = dist;
				}
			}
		}
	}
	else
	{
		if (hl)
		{
			// Draw hilight
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			set_gl_colour(col_3d_hilight_line);
			glDisable(GL_FOG);
			glBegin(GL_QUADS);
			glVertex3d(rect.x1(), rect.y1(), rb);
			glVertex3d(rect.x2(), rect.y2(), rb);
			glVertex3d(rect.x2(), rect.y2(), rt);
			glVertex3d(rect.x1(), rect.y1(), rt);
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_FOG);
		}
		else
		{
			glBegin(GL_TRIANGLE_FAN);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), rb);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), rb);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), rt);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), rt);

			glEnd();

			// Check for hilight
			if ((dist < min_dist || min_dist == -1) && dist != -1 && !front)
			{
				point3_t hit_point(camera.position.x + ((camera.view - camera.position).x * dist),
					camera.position.y + ((camera.view - camera.position).y * dist),
					camera.position.z + ((camera.view - camera.position).z * dist));

				if (hit_point.z > rb && hit_point.z < rt)
				{
					hl_line = line;
					hl_part = PART_MID2;
					min_dist = dist;
				}
			}
		}
	}
}

void render_line(Line* line, BYTE part, bool hl = false)
{
	if (line->sector_index(true) == -1)
		return;

	vis_line++;

	rect_t rect = line->get_rect();
	int length = lround(rect.length());
	int ceil1 = line->side1()->get_sector()->ceiling();
	int floor1 = line->side1()->get_sector()->floor();
	int ceil2 = line->side2()->get_sector()->ceiling();
	int floor2 = line->side2()->get_sector()->floor();

	float dist = line_intersect(camera.position, camera.view, (float)rect.x1(), (float)rect.y1(), (float)rect.x2(), (float)rect.y2());
	bool front = false;
	if (dist < min_dist || min_dist == -1)
		front = (determine_line_side(rect, camera.position.x, camera.position.y) > 0);

	Texture *tex;

	int l = line->side1()->get_sector()->light_level();
	set_light(rgba_t(l, l, l, 255), l);

	// Front middle
	if (part & PART_MID1 && line->sector_index(false) == -1)
	{
		if (hl)
		{
			// Draw hilight
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			set_gl_colour(col_3d_hilight_line);
			glDisable(GL_FOG);
			glBegin(GL_QUADS);
			glVertex3d(rect.x1(), rect.y1(), ceil1);
			glVertex3d(rect.x2(), rect.y2(), ceil1);
			glVertex3d(rect.x2(), rect.y2(), floor1);
			glVertex3d(rect.x1(), rect.y1(), floor1);
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_FOG);
		}
		else
		{
			// Get texture
			tex = line->side1()->get_tex(TEX_MIDDLE);
			bind_tex(tex->get_gl_id());

			// Setup texture coordinates
			rect_t t_c;
			if (line->check_flag(0x0010)) // Lower unpegged
			{
				t_c.tl.set(line->side1()->get_xoff(), line->side1()->get_yoff() - (ceil1-floor1));
				t_c.br.set(line->side1()->get_xoff() + length, line->side1()->get_yoff());
			}
			else
			{
				t_c.tl.set(line->side1()->get_xoff(), line->side1()->get_yoff());
				t_c.br.set(line->side1()->get_xoff() + length, line->side1()->get_yoff() + (ceil1-floor1));
			}

			// Draw
			glBegin(GL_TRIANGLE_FAN);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), ceil1);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), ceil1);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), floor1);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), floor1);

			glEnd();

			// Check for hilight
			if ((dist < min_dist || min_dist == -1) && dist != -1 && front)
			{
				point3_t hit_point(camera.position.x + ((camera.view - camera.position).x * dist),
					camera.position.y + ((camera.view - camera.position).y * dist),
					camera.position.z + ((camera.view - camera.position).z * dist));

				if (hit_point.z > floor1 && hit_point.z < ceil1)
				{
					hl_line = line;
					hl_part = PART_MID1;
					min_dist = dist;
				}
			}
		}
	}

	if (line->sector_index(false) == -1)
		return;

	// Front or back trans
	if (line->needs_middle() || line->needs_middle(false))
	{
		if (hl)
		{
			if (part == PART_MID1 && line->needs_middle())
				render_line_trans(line, true, true);

			if (part == PART_MID2 && line->needs_middle(false))
				render_line_trans(line, false, true);
		}
		else
			trans_lines.push_back(line);
	}

	bool up_sky = (line->side1()->get_sector()->tex_ceil() == _T("F_SKY1") &&
					line->side2()->get_sector()->tex_ceil() == _T("F_SKY1"));
	bool lo_sky = (line->side1()->get_sector()->tex_floor() == _T("F_SKY1") &&
					line->side2()->get_sector()->tex_floor() == _T("F_SKY1"));

	if (!render_sky)
	{
		up_sky = false;
		lo_sky = false;
	}

	// Front upper
	if (part & PART_UP1 && line->needs_upper())
	{
		if (hl)
		{
			// Draw hilight
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			set_gl_colour(col_3d_hilight_line);
			glDisable(GL_FOG);
			glBegin(GL_QUADS);
			glVertex3d(rect.x1(), rect.y1(), ceil1);
			glVertex3d(rect.x2(), rect.y2(), ceil1);
			glVertex3d(rect.x2(), rect.y2(), ceil2);
			glVertex3d(rect.x1(), rect.y1(), ceil2);
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_FOG);
		}
		else
		{
			// Get texture
			tex = line->side1()->get_tex(TEX_UPPER);
			bind_tex(tex->get_gl_id());

			// Setup texture coordinates
			rect_t t_c;
			if (line->check_flag(0x0008))
			{
				t_c.tl.set(line->side1()->get_xoff(), line->side1()->get_yoff());
				t_c.br.set(line->side1()->get_xoff() + length, line->side1()->get_yoff() + (ceil1-ceil2));
			}
			else
			{
				t_c.tl.set(line->side1()->get_xoff(), (tex->height + line->side1()->get_yoff()) - (ceil1-ceil2));
				t_c.br.set(line->side1()->get_xoff() + length, tex->height + line->side1()->get_yoff());
			}

			if (up_sky)
			{
				glDisable(GL_TEXTURE_2D);
				glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
			}

			// Draw
			glBegin(GL_TRIANGLE_FAN);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), ceil1);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), ceil1);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), ceil2);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), ceil2);

			glEnd();

			if (up_sky)
			{
				glEnable(GL_TEXTURE_2D);
				set_light(rgba_t(l, l, l, 255), l);
			}

			// Check for hilight
			if ((dist < min_dist || min_dist == -1) && dist != -1 && front)
			{
				point3_t hit_point(camera.position.x + ((camera.view - camera.position).x * dist),
					camera.position.y + ((camera.view - camera.position).y * dist),
					camera.position.z + ((camera.view - camera.position).z * dist));

				if (hit_point.z > ceil2 && hit_point.z < ceil1)
				{
					hl_line = line;
					hl_part = PART_UP1;
					min_dist = dist;
				}
			}
		}
	}

	// Front lower
	if (part & PART_LO1 && line->needs_lower() && !lo_sky)
	{
		if (hl)
		{
			// Draw hilight
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			set_gl_colour(col_3d_hilight_line);
			glDisable(GL_FOG);
			glBegin(GL_QUADS);
			glVertex3d(rect.x1(), rect.y1(), floor2);
			glVertex3d(rect.x2(), rect.y2(), floor2);
			glVertex3d(rect.x2(), rect.y2(), floor1);
			glVertex3d(rect.x1(), rect.y1(), floor1);
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_FOG);
		}
		else
		{
			// Get texture
			tex = line->side1()->get_tex(TEX_LOWER);
			bind_tex(tex->get_gl_id());

			// Setup texture coordinates
			rect_t t_c;
			if (line->check_flag(0x0010))
			{
				t_c.tl.set(line->side1()->get_xoff(), (ceil1-floor2) + line->side1()->get_yoff());
				t_c.br.set(line->side1()->get_xoff() + length, (ceil1-floor2) + line->side1()->get_yoff() + (floor2-floor1));
			}
			else
			{
				t_c.tl.set(line->side1()->get_xoff(), line->side1()->get_yoff());
				t_c.br.set(line->side1()->get_xoff() + length, line->side1()->get_yoff() + (floor2-floor1));
			}

			if (lo_sky)
			{
				glDisable(GL_TEXTURE_2D);
				glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
			}

			// Draw
			glBegin(GL_TRIANGLE_FAN);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), floor2);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), floor2);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), floor1);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), floor1);

			glEnd();

			if (lo_sky)
			{
				glEnable(GL_TEXTURE_2D);
				set_light(rgba_t(l, l, l, 255), l);
			}

			// Check for hilight
			if ((dist < min_dist || min_dist == -1) && dist != -1 && front)
			{
				point3_t hit_point(camera.position.x + ((camera.view - camera.position).x * dist),
					camera.position.y + ((camera.view - camera.position).y * dist),
					camera.position.z + ((camera.view - camera.position).z * dist));

				if (hit_point.z > floor1 && hit_point.z < floor2)
				{
					hl_line = line;
					hl_part = PART_LO1;
					min_dist = dist;
				}
			}
		}
	}

	l = line->side2()->get_sector()->light_level();
	set_light(rgba_t(l, l, l, 255), l);

	// Back upper
	if (part & PART_UP2 && line->needs_upper(false) && !up_sky)
	{
		if (hl)
		{
			// Draw hilight
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			set_gl_colour(col_3d_hilight_line);
			glDisable(GL_FOG);
			glBegin(GL_QUADS);
			glVertex3d(rect.x1(), rect.y1(), ceil1);
			glVertex3d(rect.x2(), rect.y2(), ceil1);
			glVertex3d(rect.x2(), rect.y2(), ceil2);
			glVertex3d(rect.x1(), rect.y1(), ceil2);
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_FOG);
		}
		else
		{
			// Get texture
			tex = line->side2()->get_tex(TEX_UPPER);
			bind_tex(tex->get_gl_id());

			// Setup texture coordinates
			rect_t t_c;
			if (line->check_flag(0x0008))
			{
				t_c.tl.set(line->side2()->get_xoff(), line->side2()->get_yoff());
				t_c.br.set(line->side2()->get_xoff() + length, line->side2()->get_yoff() + (ceil2-ceil1));
			}
			else
			{
				t_c.tl.set(line->side2()->get_xoff(), (tex->height + line->side2()->get_yoff()) - (ceil2-ceil1));
				t_c.br.set(line->side2()->get_xoff() + length, tex->height + line->side2()->get_yoff());
			}

			if (up_sky)
			{
				glDisable(GL_TEXTURE_2D);
				glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
			}

			// Draw
			glBegin(GL_TRIANGLE_FAN);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), ceil1);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), ceil1);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), ceil2);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), ceil2);

			glEnd();

			if (up_sky)
			{
				glEnable(GL_TEXTURE_2D);
				set_light(rgba_t(l, l, l, 255), l);
			}

			// Check for hilight
			if ((dist < min_dist || min_dist == -1) && dist != -1 && !front)
			{
				point3_t hit_point(camera.position.x + ((camera.view - camera.position).x * dist),
								camera.position.y + ((camera.view - camera.position).y * dist),
								camera.position.z + ((camera.view - camera.position).z * dist));

				if (hit_point.z > ceil1 && hit_point.z < ceil2)
				{
					hl_line = line;
					hl_part = PART_UP2;
					min_dist = dist;
				}
			}
		}
	}

	// Back lower
	if (part & PART_LO2 && line->needs_lower(false) && !lo_sky)
	{
		if (hl)
		{
			// Draw hilight
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			set_gl_colour(col_3d_hilight_line);
			glDisable(GL_FOG);
			glBegin(GL_QUADS);
			glVertex3d(rect.x1(), rect.y1(), floor2);
			glVertex3d(rect.x2(), rect.y2(), floor2);
			glVertex3d(rect.x2(), rect.y2(), floor1);
			glVertex3d(rect.x1(), rect.y1(), floor1);
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_FOG);
		}
		else
		{
			// Get texture
			tex = line->side2()->get_tex(TEX_LOWER);
			bind_tex(tex->get_gl_id());

			// Setup texture coordinates
			rect_t t_c;
			if (line->check_flag(0x0010))
			{
				t_c.tl.set(line->side2()->get_xoff(), (ceil2-floor1) + line->side2()->get_yoff());
				t_c.br.set(line->side2()->get_xoff() + length, (ceil2-floor1) + line->side2()->get_yoff() + (floor1-floor2));
			}
			else
			{
				t_c.tl.set(line->side2()->get_xoff(), line->side2()->get_yoff());
				t_c.br.set(line->side2()->get_xoff() + length, line->side2()->get_yoff() + (floor1-floor2));
			}

			if (lo_sky)
			{
				glDisable(GL_TEXTURE_2D);
				glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
			}

			// Draw
			glBegin(GL_TRIANGLE_FAN);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), floor2);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), floor2);

			glTexCoord2f((float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x2(), rect.y2(), floor1);

			glTexCoord2f((float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height));
			glVertex3d(rect.x1(), rect.y1(), floor1);

			glEnd();

			if (lo_sky)
			{
				glEnable(GL_TEXTURE_2D);
				set_light(rgba_t(l, l, l, 255), l);
			}

			// Check for hilight
			if ((dist < min_dist || min_dist == -1) && dist != -1 && !front)
			{
				point3_t hit_point(camera.position.x + ((camera.view - camera.position).x * dist),
								camera.position.y + ((camera.view - camera.position).y * dist),
								camera.position.z + ((camera.view - camera.position).z * dist));

				if (hit_point.z > floor2 && hit_point.z < floor1)
				{
					hl_line = line;
					hl_part = PART_LO2;
					min_dist = dist;
				}
			}
		}
	}
}

void render_lines()
{
	trans_lines.clear();
	hl_line = NULL;

	for (unsigned int a = 0; a < d_map.n_lines(); a++)
	{
		if (lines_visible[a])
			render_line(d_map.line(a), 255);
	}
}

void render_trans_lines()
{
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.8f);
	glDepthMask(GL_TRUE);
	for (unsigned int a = 0; a < trans_lines.size(); a++)
	{
		render_line_trans(trans_lines[a], true);
		render_line_trans(trans_lines[a], false);
	}
	glEnable(GL_ALPHA_TEST);
	glDepthMask(GL_TRUE);
}

void render_ssect_hilight(unsigned int ssect)
{
	int start = gl_ssects[ssect].first_seg;
	int end = start + gl_ssects[ssect].count;

	int height = 0;

	plane_t plane;
	if (hl_part == PART_FLOOR)
		plane = f_planes[hl_sector];

	if (hl_part == PART_CEIL)
		plane = c_planes[hl_sector];

	for (int s = start; s < end; s++)
	{
		if (gl_segs[s].linedef == SEG_MINISEG)
			continue;

		int v1 = gl_segs[s].start_vertex;
		int v2 = gl_segs[s].end_vertex;
		float x1 = 0;
		float y1 = 0;
		float x2 = 0;
		float y2 = 0;

		if (v1 & VERT_IS_GL)
		{
			x1 = gl_verts[v1 & ~VERT_IS_GL].x;
			y1 = gl_verts[v1 & ~VERT_IS_GL].y;
		}
		else
		{
			x1 = (float)map_verts[v1].x;
			y1 = (float)map_verts[v1].y;
		}

		if (v2 & VERT_IS_GL)
		{
			x2 = gl_verts[v2 & ~VERT_IS_GL].x;
			y2 = gl_verts[v2 & ~VERT_IS_GL].y;
		}
		else
		{
			x2 = (float)map_verts[v2].x;
			y2 = (float)map_verts[v2].y;
		}

		glBegin(GL_LINES);
		glVertex3f(x1, y1, plane_height(plane, x1, y1));
		glVertex3f(x2, y2, plane_height(plane, x2, y2));
		glEnd();
	}
}

void render_hilight_ssects()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	set_gl_colour(col_3d_hilight_line);
	glDisable(GL_FOG);

	for (DWORD a = 0; a < gl_ssects.size(); a++)
	{
		if (ssect_sectors[a] == hl_sector)
			render_ssect_hilight(a);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_FOG);
}

void render_ssect(unsigned int ssect)
{
	if (!ssect_sectors)
		return;

	if (ssect_sectors[ssect] == -1)
		return;

	Sector* sec = d_map.sector(ssect_sectors[ssect]);
	vis_sectors[ssect_sectors[ssect]] = 1;

	vis_ssect++;

	int l = sec->light_level();
	set_light(rgba_t(l, l, l, 255), sec->light_level());

	plane_t c_plane, f_plane;
	f_plane = f_planes[ssect_sectors[ssect]];
	c_plane = c_planes[ssect_sectors[ssect]];

	float x = 0;
	float y = 0;
	float last_x = 0;
	float last_y = 0;
	int v = 0;

	// Draw floor
	bool draw = (sec->get_tex()->name != _T("F_SKY1") || !render_sky);

	if (draw)
	{
		bind_tex(sec->get_tex()->get_gl_id());
		glBegin(GL_TRIANGLE_FAN);
	}

	int start = gl_ssects[ssect].first_seg;
	int end = start + gl_ssects[ssect].count;
	v = gl_segs[start].start_vertex;

	point3_t dir = camera.view - camera.position;
	float dist = dist_ray_plane(camera.position, dir, f_plane);
	point3_t hit = camera.position + (dir * dist);

	if (v & VERT_IS_GL)
	{
		x = gl_verts[v & ~VERT_IS_GL].x;
		y = gl_verts[v & ~VERT_IS_GL].y;
	}
	else
	{
		x = (float)map_verts[v].x;
		y = (float)map_verts[v].y;
	}

	if (draw)
	{
		glTexCoord2f(x * (1.0f / sec->get_tex()->width), -y * (1.0f / sec->get_tex()->height));
		glVertex3f(x, y, plane_height(f_plane, x, y));
	}

	last_x = x;
	last_y = y;

	bool in = (dist > 0 && (dist < min_dist || min_dist == -1));

	if (camera.position.z < sec->floor())
		in = false;

	for (int s = start; s < end; s++)
	{
		int v = gl_segs[s].end_vertex;

		if (v & VERT_IS_GL)
		{
			x = gl_verts[v & ~VERT_IS_GL].x;
			y = gl_verts[v & ~VERT_IS_GL].y;
		}
		else
		{
			x = (float)map_verts[v].x;
			y = (float)map_verts[v].y;
		}

		if (draw)
		{
			glTexCoord2f(x * (1.0f / sec->get_tex()->width), -y * (1.0f / sec->get_tex()->height));
			glVertex3f(x, y, plane_height(f_plane, x, y));
		}

		if (in)
			in = (determine_line_side(last_x, last_y, x, y, hit.x, hit.y) <= 0);

		last_x = x;
		last_y = y;
	}

	if (draw)
		glEnd();

	if (in)
	{
		min_dist = dist;
		hl_line = NULL;
		hl_sector = ssect_sectors[ssect];
		hl_part = PART_FLOOR;
	}

	// Draw ceiling
	dist = dist_ray_plane(camera.position, dir, c_plane);
	hit = camera.position + (dir * dist);
	draw = (sec->get_tex(false)->name != _T("F_SKY1") || !render_sky);

	if (draw)
	{
		bind_tex(sec->get_tex(false)->get_gl_id());
		glCullFace(GL_BACK);
		glBegin(GL_TRIANGLE_FAN);
	}

	v = gl_segs[start].start_vertex;

	if (v & VERT_IS_GL)
	{
		x = gl_verts[v & ~VERT_IS_GL].x;
		y = gl_verts[v & ~VERT_IS_GL].y;
	}
	else
	{
		x = (float)map_verts[v].x;
		y = (float)map_verts[v].y;
	}

	if (draw)
	{
		glTexCoord2f(x * (1.0f / sec->get_tex(false)->width), -y * (1.0f / sec->get_tex(false)->height));
		glVertex3f(x, y, plane_height(c_plane, x, y));
	}

	last_x = x;
	last_y = y;

	in = (dist > 0 && (dist < min_dist || min_dist == -1));

	if (camera.position.z > sec->ceiling())
		in = false;

	for (int s = start; s < end; s++)
	{
		int v = gl_segs[s].end_vertex;

		if (v & VERT_IS_GL)
		{
			x = gl_verts[v & ~VERT_IS_GL].x;
			y = gl_verts[v & ~VERT_IS_GL].y;
		}
		else
		{
			x = (float)map_verts[v].x;
			y = (float)map_verts[v].y;
		}

		if (draw)
		{
			glTexCoord2f(x * (1.0f / sec->get_tex(false)->width), -y * (1.0f / sec->get_tex(false)->height));
			glVertex3f(x, y, plane_height(c_plane, x, y));
		}

		if (in)
			in = (determine_line_side(last_x, last_y, x, y, hit.x, hit.y) <= 0);

		last_x = x;
		last_y = y;
	}

	if (in)
	{
		min_dist = dist;
		hl_line = NULL;
		hl_sector = ssect_sectors[ssect];
		hl_part = PART_CEIL;
	}

	if (draw)
	{
		glEnd();
		glCullFace(GL_FRONT);
	}
}

void render_ssects()
{
	hl_sector = -1;

	for (unsigned int a = 0; a < gl_ssects.size(); a++)
	{
		if (ssects_visible[a])
			render_ssect(a);
	}

	/*
	glDisable(GL_TEXTURE_2D);

	for (int a = 0; a < gl_segs.size(); a++)
	{
		//if (!(gl_segs[a].start_vertex & VERT_IS_GL &&
		//	gl_segs[a].end_vertex & VERT_IS_GL))
		//	continue;

		float x1, y1, x2, y2;

		int v = gl_segs[a].start_vertex;

		if (v & VERT_IS_GL)
		{
			x1 = gl_verts[v & ~VERT_IS_GL].x;
			y1 = gl_verts[v & ~VERT_IS_GL].y;
		}
		else
		{
			//x1 = (float)d_map.vertex(v+1)->x_pos();
			//y1 = (float)d_map.vertex(v+1)->y_pos();
			x1 = (float)map_verts[v].x;
			y1 = (float)map_verts[v].y;
		}

		v = gl_segs[a].end_vertex;

		if (v & VERT_IS_GL)
		{
			x2 = gl_verts[v & ~VERT_IS_GL].x;
			y2 = gl_verts[v & ~VERT_IS_GL].y;
		}
		else
		{
			//x2 = (float)d_map.vertex(v+1)->x_pos();
			//y2 = (float)d_map.vertex(v+1)->y_pos();
			x2 = (float)map_verts[v].x;
			y2 = (float)map_verts[v].y;
		}

		glBegin(GL_LINES);
		glVertex3f(x1, y1, 0.0f);
		glVertex3f(x2, y2, 0.0f);
	}

	glEnable(GL_TEXTURE_2D);
	*/
}

void render_skydome()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	Texture* stex = get_texture(_T("SKY1"), 1);
	bind_tex(stex->get_gl_id());

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y - 64.0f, camera.position.z + 64.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y - 64.0f, camera.position.z + 64.0f);
	glTexCoord2f(1.0f, 2.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y - 64.0f, camera.position.z - 64.0f);
	glTexCoord2f(0.0f, 2.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y - 64.0f, camera.position.z - 64.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y + 64.0f, camera.position.z + 64.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y + 64.0f, camera.position.z + 64.0f);
	glTexCoord2f(1.0f, 2.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y + 64.0f, camera.position.z - 64.0f);
	glTexCoord2f(0.0f, 2.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y + 64.0f, camera.position.z - 64.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y - 64.0f, camera.position.z + 64.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y + 64.0f, camera.position.z + 64.0f);
	glTexCoord2f(1.0f, 2.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y + 64.0f, camera.position.z - 64.0f);
	glTexCoord2f(0.0f, 2.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y - 64.0f, camera.position.z - 64.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y + 64.0f, camera.position.z + 64.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y - 64.0f, camera.position.z + 64.0f);
	glTexCoord2f(1.0f, 2.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y - 64.0f, camera.position.z - 64.0f);
	glTexCoord2f(0.0f, 2.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y + 64.0f, camera.position.z - 64.0f);
	glEnd();

	set_gl_colour(stex->avg_col);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	glVertex3f(camera.position.x + 64.0f, camera.position.y + 64.0f, camera.position.z + 64.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y - 64.0f, camera.position.z + 64.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y - 64.0f, camera.position.z + 64.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y + 64.0f, camera.position.z + 64.0f);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(camera.position.x + 64.0f, camera.position.y + 64.0f, camera.position.z - 64.0f);
	glVertex3f(camera.position.x + 64.0f, camera.position.y - 64.0f, camera.position.z - 64.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y - 64.0f, camera.position.z - 64.0f);
	glVertex3f(camera.position.x - 64.0f, camera.position.y + 64.0f, camera.position.z - 64.0f);
	glEnd();

	glDepthMask(GL_TRUE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

void render_3d_thing_box(Thing* t)
{
	// Get thing sector
	int sec = t->get_sector();
	if (sec < 0 || sec > d_map.n_sectors())
		return;

	// Return if sector isn't visible
	if (!vis_sectors[sec])
		return;

	// Get thing sprite if it isn't initialised
	if (!t->get_ttype()->sprite)
		t->get_ttype()->sprite = get_texture(t->get_ttype()->spritename, 3, true);

	if (!t->get_ttype()->sprite)
		return;

	float r = ((t->get_ttype()->sprite->width / 2) * t->get_ttype()->x_scale);
	float h = (t->get_ttype()->sprite->height * t->get_ttype()->y_scale);

	float shrink = 0.001f;
	float f = 0.0f;

	if (t->get_ttype()->flags & TTYPE_HANGING)
		f = d_map.sector(sec)->ceiling() - h;
		//f = plane_height(sector_info[sector].c_plane, x, y) - h;
	else
		f = d_map.sector(sec)->floor();
		//f = plane_height(sector_info[sector].f_plane, x, y);

	if (d_map.hexen() && t->get_type() != 9500 && t->get_type() != 9501)
		f += t->z_pos();

	if (t->get_ttype()->radius == -1)
		f -= 4;

	float x1 = t->pos().x - t->get_ttype()->radius;
	float y1 = t->pos().y - t->get_ttype()->radius;
	float x2 = t->pos().x + t->get_ttype()->radius;
	float y2 = t->pos().y + t->get_ttype()->radius;

	// Draw floor box
	if (render_things >= 2)
	{
		rgba_t col = t->get_ttype()->colour;
		glColor4f(col.fr(), col.fg(), col.fb(), 0.8f);

		// Outline
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(4.0f);
		glBegin(GL_QUADS);
		glVertex3f(x1, y1, f + shrink);
		glVertex3f(x2, y1, f + shrink);
		glVertex3f(x2, y2, f + shrink);
		glVertex3f(x1, y2, f + shrink);
		glEnd();

		// Direction
		if (t->get_ttype()->flags & TTYPE_ANGLE)
		{
			glBegin(GL_LINES);
			glLineWidth(10.0f);
			glVertex3f(t->pos().x, t->pos().y, f + shrink);

			glLineWidth(4.0f);
			int angle = t->get_angle();

			switch (angle)
			{
			case 0:
				glVertex3f(x2, t->pos().y, f + shrink); break;
			case 45:
				glVertex3f(x2, y2, f + shrink); break;
			case 90:
				glVertex3f(t->pos().x, y2, f + shrink); break;
			case 135:
				glVertex3f(x1, y2, f + shrink); break;
			case 180:
				glVertex3f(x1, t->pos().y, f + shrink); break;
			case 225:
				glVertex3f(x1, y1, f + shrink); break;
			case 270:
				glVertex3f(t->pos().x, y1, f + shrink); break;
			case 315:
				glVertex3f(x2, y1, f + shrink); break;
			default:
				glVertex3f(t->pos().x, t->pos().y, f + shrink); break;
			}

			glEnd();
		}

		// Fill
		glColor4f(col.fr(), col.fg(), col.fb(), 0.3f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_QUADS);
		glVertex3f(x1, y1, f + shrink);
		glVertex3f(x2, y1, f + shrink);
		glVertex3f(x2, y2, f + shrink);
		glVertex3f(x1, y2, f + shrink);
		glEnd();
	}

	// Draw full box
	if (render_things >= 3)
	{
		float top = f + h;
		rgba_t col = t->get_ttype()->colour;

		// OUTLINES
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glColor4f(col.fr(), col.fg(), col.fb(), 0.8f);

		// Top
		glBegin(GL_QUADS);
		glVertex3f(x1, y1, top - shrink);
		glVertex3f(x2, y1, top - shrink);
		glVertex3f(x2, y2, top - shrink);
		glVertex3f(x1, y2, top - shrink);
		glEnd();

		// Left
		glBegin(GL_QUADS);
		glVertex3f(x1, y1, top - shrink);
		glVertex3f(x1, y2, top - shrink);
		glVertex3f(x1, y2, f + shrink);
		glVertex3f(x1, y1, f + shrink);
		glEnd();

		// Right
		glBegin(GL_QUADS);
		glVertex3f(x2, y2, top - shrink);
		glVertex3f(x2, y1, top - shrink);
		glVertex3f(x2, y1, f + shrink);
		glVertex3f(x2, y2, f + shrink);
		glEnd();

		// Front
		glBegin(GL_QUADS);
		glVertex3f(x1, y2, top - shrink);
		glVertex3f(x2, y2, top - shrink);
		glVertex3f(x2, y2, f + shrink);
		glVertex3f(x1, y2, f + shrink);
		glEnd();

		// Back
		glBegin(GL_QUADS);
		glVertex3f(x2, y1, top - shrink);
		glVertex3f(x1, y1, top - shrink);
		glVertex3f(x1, y1, f + shrink);
		glVertex3f(x2, y1, f + shrink);
		glEnd();


		// FILLS
		glColor4f(col.fr(), col.fg(), col.fb(), 0.3f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Top
		glBegin(GL_QUADS);
		glVertex3f(x1, y1, top - shrink);
		glVertex3f(x2, y1, top - shrink);
		glVertex3f(x2, y2, top - shrink);
		glVertex3f(x1, y2, top - shrink);
		glEnd();

		// Left
		glBegin(GL_QUADS);
		glVertex3f(x1, y1, top - shrink);
		glVertex3f(x1, y2, top - shrink);
		glVertex3f(x1, y2, f + shrink);
		glVertex3f(x1, y1, f + shrink);
		glEnd();

		// Right
		glBegin(GL_QUADS);
		glVertex3f(x2, y2, top - shrink);
		glVertex3f(x2, y1, top - shrink);
		glVertex3f(x2, y1, f + shrink);
		glVertex3f(x2, y2, f + shrink);
		glEnd();

		// Front
		glBegin(GL_QUADS);
		glVertex3f(x1, y2, top - shrink);
		glVertex3f(x2, y2, top - shrink);
		glVertex3f(x2, y2, f + shrink);
		glVertex3f(x1, y2, f + shrink);
		glEnd();

		// Back
		glBegin(GL_QUADS);
		glVertex3f(x2, y1, top - shrink);
		glVertex3f(x1, y1, top - shrink);
		glVertex3f(x1, y1, f + shrink);
		glVertex3f(x2, y1, f + shrink);
		glEnd();
	}
}

void render_3d_thing(Thing* t, bool hilight)
{
	// Get thing sector
	int sec = t->get_sector();
	if (sec < 0 || sec > d_map.n_sectors())
	{
		//log_message(s_fmt("thing in invalid sector %d", sec));
		return;
	}

	// Return if sector isn't visible
	if (!vis_sectors[sec])
		return;

	// Get thing sprite if it isn't initialised
	if (!t->get_ttype()->sprite)
		t->get_ttype()->sprite = get_texture(t->get_ttype()->spritename, 3, true);

	if (!t->get_ttype()->sprite)
		return;

	vis_thing++;
	float r = ((t->get_ttype()->sprite->width / 2) * t->get_ttype()->x_scale);
	float h = (t->get_ttype()->sprite->height * t->get_ttype()->y_scale);
	float f;

	if (t->get_ttype()->flags & TTYPE_HANGING)
		f = d_map.sector(sec)->ceiling() - h;
		//f = plane_height(sector_info[sector].c_plane, x, y) - h;
	else
		f = d_map.sector(sec)->floor();
		//f = plane_height(sector_info[sector].f_plane, x, y);

	if (d_map.hexen() && t->get_type() != 9500 && t->get_type() != 9501)
		f += t->z_pos();

	if (t->get_ttype()->radius == -1)
	{
		r = 4;
		h = 8;
		f -= 4;
	}

	float x1 = t->pos().x - camera.strafe.x * r;
	float y1 = t->pos().y - camera.strafe.y * r;
	float x2 = t->pos().x + camera.strafe.x * r;
	float y2 = t->pos().y + camera.strafe.y * r;

	int l = d_map.sector(sec)->light_level();
	set_light(rgba_t(l, l, l, 255 * t->get_ttype()->alpha), l);

	if (t->get_ttype()->alpha < 1.0f)
	{
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);
	}
	else
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.8f);
		glDepthMask(GL_TRUE);
	}

	if (hilight)
	{
		// Outline
		glDisable(GL_FOG);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(render_hilight_width);
		set_gl_colour(col_3d_hilight_line);

		glBegin(GL_QUADS);
		glVertex3f(x1, y1, f + h);
		glVertex3f(x2, y2, f + h);
		glVertex3f(x2, y2, f);
		glVertex3f(x1, y1, f);
		glEnd();

		/*
		// Fill
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		set_gl_colour(col_3d_hilight);

		glBegin(GL_QUADS);
		glVertex3f(x1, y1, f + h);
		glVertex3f(x2, y2, f + h);
		glVertex3f(x2, y2, f);
		glVertex3f(x1, y1, f);
		glEnd();
		*/

		glEnable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);

		return;
	}

	glCullFace(GL_FRONT);

	// Bind texture
	Texture* tex = t->get_ttype()->sprite;
	glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());

	// Draw sprite
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x1, y1, f + h);
	glTexCoord2f(tex->t_x, 0.0f);
	glVertex3f(x2, y2, f + h);
	glTexCoord2f(tex->t_x, tex->t_y);
	glVertex3f(x2, y2, f);
	glTexCoord2f(0.0f, tex->t_y);
	glVertex3f(x1, y1, f);
	glEnd();

	// Check for hilight
	float dist = line_intersect(camera.position, camera.view, x1, y1, x2, y2);
	if (dist != -1 && dist < min_dist)
	{
		point3_t direction = camera.view - camera.position;

		point3_t hit_point(camera.position.x + (direction.x * dist),
			camera.position.y + (direction.y * dist),
			camera.position.z + (direction.z * dist));

		if (hit_point.z >= f && hit_point.z <= f + h)
		{
			min_dist = dist;
			hl_thing = t;
			hl_line = NULL;
			hl_sector = -1;
		}
	}
}

void render_3d_things()
{
	if (render_things == 0)
		return;

	for (int a = 0; a < (int)d_map.n_things(); a++)
		render_3d_thing(d_map.thing(a), false);

	if (render_things >= 2)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		glDisable(GL_TEXTURE_2D);

		for (int a = 0; a < (int)d_map.n_things(); a++)
			render_3d_thing_box(d_map.thing(a));
	}

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void render_3d_view(int width, int height)
{
	// Set 3d screen
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (GLfloat)320/(GLfloat)200, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Clear screen
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);

	glScalef(0.01f, 0.01f, 0.01f);

	camera.update();
	camera.look();

	update_visibility();
	memset(vis_sectors, 0, d_map.n_sectors());
	hl_thing = NULL;

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	min_dist = -1;

	if (render_sky)
		render_skydome();

	last_tex = 999999;
	render_lines();
	render_ssects();
	render_trans_lines();
	render_3d_things();

	// Render hilight
	if (render_hilight)
	{
		glDisable(GL_FOG);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glLineWidth(render_hilight_width);
		glEnable(GL_LINE_SMOOTH);
		glDisable(GL_ALPHA_TEST);

		if (hl_line)
			render_line(hl_line, hl_part, true);
		else if (hl_sector != -1)
			render_hilight_ssects();
		else if (hl_thing)
			render_3d_thing(hl_thing, true);

		glLineWidth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_FOG);
		glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
	}

	// 2d Stuff
	last_tex = 999999;
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glCullFace(GL_NONE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Crosshair
	draw_texture((width / 2) - 16, (height / 2) - 16, 32, 32, _T("_xhair"), 4, col_3d_crosshair);

	// Messages
	for (unsigned int a = 0; a < messages_3d.size(); a++)
	{
		if (messages_3d[a].fade <= 0)
		{
			messages_3d.erase(messages_3d.begin() + a);
			a--;
			continue;
		}

		BYTE al = 255;
		if (messages_3d[a].fade < 255)
			al = messages_3d[a].fade;

		draw_text_scale(0, a*(10*render_text_scale), rgba_t(255, 255, 255, al), 0, render_text_scale, chr(messages_3d[a].message));

		messages_3d[a].fade -= 7;
	}

	draw_text(0, height - 10, COL_WHITE, 0, false, "Lines: %d", vis_line);
	draw_text(0, height - 20, COL_WHITE, 0, false, "Ssects: %d", vis_ssect);
	draw_text(0, height - 30, COL_WHITE, 0, false, "Things: %d", vis_thing);
	vis_line = vis_ssect = vis_thing = 0;
}

void add_3d_message(string message)
{
	if (messages_3d.size() > 6)
		messages_3d.erase(messages_3d.begin());

	fademsg_t msg;
	msg.message = message;
	msg.fade = 1024;
	messages_3d.push_back(msg);
}
