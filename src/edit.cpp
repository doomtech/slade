
#include "main.h"
#include "structs.h"
#include "doom_map.h"
#include "dm_vertex.h"
#include "dm_thing.h"
#include "dm_line.h"
#include "dm_sector.h"
#include "mathstuff.h"
#include "undoredo.h"

int	gridsize = 32;		// Grid size
int	edit_mode = 1;		// Edit mode: 0=vertices, 1=lines, 2=sectors, 3=things
int	hilight_item = -1;	// The currently hilighted item (-1 for none)

bool lock_hilight = false;	// Wether to 'lock' hilighting

BYTE edit_state = 0;	// Current editing state

CVAR(Bool, edit_snap_grid, true, CVAR_SAVE)
CVAR(Bool, edit_auto_split, true, CVAR_SAVE)
CVAR(Bool, edit_auto_merge, true, CVAR_SAVE)

extern DoomMap d_map;
extern Thing last_thing;

// increase_grid: Increases the grid size
// ----------------------------------- >>
void increase_grid()
{
	gridsize *= 2;

	if (gridsize > 1024)
		gridsize = 1024;

	update_statusbar();
}

// decrease_grid: Decreases the grid size
// ----------------------------------- >>
void decrease_grid()
{
	gridsize /= 2;

	if (gridsize < 1)
		gridsize = 1;

	update_statusbar();
}

// snap_to_grid: Finds the nearest grid line to a point
// ------------------------------------------------- >>
int snap_to_grid(double pos)
{
	if (!edit_snap_grid)
		return pos;

	int upper, lower;

	for (int i = pos; i >= (pos - gridsize); i--)
	{
		if ((i % gridsize) == 0)
		{
			lower = i;
			break;
		}
	}

	for (int i = pos; i < (pos + gridsize); i++)
	{
		if ((i % gridsize) == 0)
		{
			upper = i;
			break;
		}
	}

	double mid = lower + ((upper - lower) / 2.0);

	if (pos > mid)
		return upper;
	else
		return lower;

	return (int)pos;
}

// snap_to_grid_custom: Finds the nearest specified grid line to a point
// ------------------------------------------------------------------ >>
int snap_to_grid_custom(int pos, int grid)
{
	short upper, lower;

	for (int i = pos; i >= (pos - grid); i--)
	{
		if ((i % grid) == 0)
		{
			lower = i;
			break;
		}
	}

	for (int i = pos; i < (pos + grid); i++)
	{
		if ((i % grid) == 0)
		{
			upper = i;
			break;
		}
	}

	if ((upper - pos) < (pos - lower))
		return upper;
	else
		return lower;

	return pos;
}

// change_state: Attempts to change the current editing state
// ------------------------------------------------------- >>
bool change_state(BYTE new_state)
{
	if (new_state > 0 && edit_state != 0)
		return false;
	else
		edit_state = new_state;

	return true;
}

// state: Checks the current editing state
// ------------------------------------ >>
bool state(BYTE state_test)
{
	return (state_test == edit_state);
}

Vertex* create_vertex(point2_t mouse)
{
	if (edit_snap_grid)
	{
		mouse.x = snap_to_grid(mouse.x);
		mouse.y = snap_to_grid(mouse.y);
	}

	if (d_map.check_vertex_spot(mouse) != -1)
		return NULL;

	make_backup(BKUP_VERTS|BKUP_LINES|BKUP_SIDES);

	Vertex *v = new Vertex(mouse.x, mouse.y, &d_map);

	if (edit_auto_split)
		d_map.check_split(v);

	return v;
}

void create_line(bool close)
{
	vector<int> selection = d_map.selection();

	if (selection.size() <= 1)
		return;

	make_backup(BKUP_VERTS|BKUP_LINES|BKUP_SIDES);

	vector<int> n_lines;

	for (unsigned int a = 0; a < selection.size() - 1; a++)
	{
		Line* l = new Line(d_map.vertex(selection[a]), d_map.vertex(selection[a+1]), &d_map);
		n_lines.push_back(d_map.index(l));
	}

	if (close)
	{
		Line* l = new Line(d_map.vertex(selection.back()), d_map.vertex(selection[0]), &d_map);
		n_lines.push_back(d_map.index(l));
	}

	change_edit_mode(1);
	d_map.clear_selection();

	for (unsigned int a = 0; a < n_lines.size(); a++)
		d_map.select_item(n_lines[a]);
}

void create_sector()
{
	vector<int> selection = d_map.selection();

	if (selection.size() == 0)
		return;

	make_backup(BKUP_LINES|BKUP_SIDES|BKUP_SECTORS);

	Sector *ns = new Sector(&d_map);

	for (unsigned int a = 0; a < selection.size(); a++)
	{
		if (d_map.valid(d_map.line(selection[a])->side1()))
		{
			ns->copy(d_map.sector(d_map.line(selection[a])->sector_index(true)));
			break;
		}

		if (d_map.valid(d_map.line(selection[a])->side2()))
		{
			ns->copy(d_map.sector(d_map.line(selection[a])->sector_index(false)));
			break;
		}
	}

	for (unsigned int a = 0; a < selection.size(); a++)
	{
		Side *s1 = d_map.line(selection[a])->side1();
		Side *s2 = d_map.line(selection[a])->side2();

		if (!d_map.valid(s1))
			d_map.line(selection[a])->set_sector(1, ns);
		else if (!d_map.valid(s2))
			d_map.line(selection[a])->set_sector(2, ns);
	}
}


void create_thing(point2_t mouse)
{
	make_backup(BKUP_THINGS);

	if (edit_snap_grid)
	{
		mouse.x = snap_to_grid(mouse.x);
		mouse.y = snap_to_grid(mouse.y);
	}

	Thing *t = new Thing(&d_map);
	t->copy(&last_thing);
	t->set_pos(mouse.x, mouse.y);
}


void check_textures()
{
	d_map.clear_selection();
	hilight_item = -1;
	int line = -1;

	// Check line textures
	for (int l = 0; l < (int)d_map.n_lines(); l++)
	{
		//sidedef_t *side = map.l_getside(l, 1);
		Side* side = d_map.line(l)->side1();

		if (d_map.valid(side))
		{
			string tex = side->get_texname(TEX_UPPER);
			if (get_texture(tex, 1)->name == _T("_notex") && tex != _T("-"))
			{
				string line = s_fmt(_T("Line %d has invalid front upper texture \"%s\"\n'Yes' to edit, 'No' to stop checking"),
											l, tex.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}

			tex = side->get_texname(TEX_MIDDLE);
			if (get_texture(tex, 1)->name == _T("_notex") && tex != _T("-"))
			{
				string line = s_fmt(_T("Line %d has invalid front middle texture \"%s\"\n'Yes' to edit, 'No' to stop checking"),
											l, side->get_texname(TEX_MIDDLE).c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}

			tex = side->get_texname(TEX_LOWER);
			if (get_texture(tex, 1)->name == _T("_notex") && tex != _T("-"))
			{
				string line = s_fmt(_T("Line %d has invalid front lower texture \"%s\"\n'Yes' to edit, 'No' to stop checking"),
											l, tex.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}
		}

		side = d_map.line(l)->side2();

		if (d_map.valid(side))
		{
			string tex = side->get_texname(TEX_UPPER);
			if (get_texture(tex, 1)->name == _T("_notex") && tex != _T("-"))
			{
				string line = s_fmt(_T("Line %d has invalid back upper texture \"%s\"\n'Yes' to edit, 'No' to stop checking"),
											l, tex.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}

			tex = side->get_texname(TEX_MIDDLE);
			if (get_texture(tex, 1)->name == _T("_notex") && tex != _T("-"))
			{
				string line = s_fmt(_T("Line %d has invalid back middle texture \"%s\"\n'Yes' to edit, 'No' to stop checking"),
											l, tex.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}

			tex = side->get_texname(TEX_LOWER);
			if (get_texture(tex, 1)->name == _T("_notex") && tex != _T("-"))
			{
				string line = s_fmt(_T("Line %d has invalid back lower texture \"%s\"\n'Yes' to edit, 'No' to stop checking"),
											l, tex.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}
		}
	}

	// Check flat textures
	for (int s = 0; s < (int)d_map.n_sectors(); s++)
	{
		if (get_texture(d_map.sector(s)->tex_floor(), 2)->name == _T("_notex"))
		{
			string line = s_fmt(_T("Sector %d has invalid floor texture \"%s\"\n'Yes' to edit, 'No' to stop checking"),
										s, d_map.sector(s)->tex_floor().c_str());
			//if (!yesno_box(line))
			//	return;

			hilight_item = s;
			//open_sector_edit();
		}

		if (get_texture(d_map.sector(s)->tex_ceil(), 2)->name == _T("_notex"))
		{
			string line = s_fmt(_T("Sector %d has invalid ceiling texture \"%s\"\n'Yes' to edit, 'No' to stop checking"),
										s, d_map.sector(s)->tex_ceil().c_str());
			//if (!yesno_box(line))
			//	return;

			hilight_item = s;
			//open_sector_edit();
		}
	}
}
