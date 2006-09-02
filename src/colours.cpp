// <<--------------------------------------------->>
// << SLADE (SlayeR's 'LeetAss Doom Editor)       >>
// << By Simon Judd, 2004                         >>
// << ------------------------------------------- >>
// << <colours.cpp>                               >>
// << Colour configuration stuff                  >>
// <<--------------------------------------------->>

// Includes ------------------------------------- >>
#include "main.h"
#include "colours.h"


// Variables ----------------------------- >>
float	hilight_size = 3.0f;
float	selection_size = 5.0f;
float	moving_size = 5.0f;
BYTE	hilight_style = 0;		// 0 = SLADE, 1 = Doom Builder etc

rgba_t col_hilight(0, 128, 255, 160, 1);
rgba_t col_selection(255, 0, 0, 147, 1);
rgba_t col_moving(50, 180, 50, 255, 1);
rgba_t col_tagged(255, 255, 0, 159, 1);
rgba_t col_grid(58, 124, 197, 55, 0);
rgba_t col_64grid(0, 255, 255, 17, 0);
rgba_t col_vertex(140, 140, 255, 255, 0);
rgba_t col_background(0, 0, 0, 255, 0);
rgba_t col_selbox(0, 140, 220, 100, 1);
rgba_t col_selbox_line(100, 180, 220, 200, 1);
rgba_t col_linedraw(127, 0, 255, 255, 0);
rgba_t col_crosshair(255, 255, 0, 50, 1);
rgba_t col_line_solid(200, 200, 200, 255, 0);
rgba_t col_line_2s(97, 119, 143, 255, 0);
rgba_t col_line_monster(0, 120, 0, 255, 0);
rgba_t col_line_nofirst(255, 0, 0, 255, 0);
rgba_t col_line_special(47, 71, 189, 255, 0);
rgba_t col_3d_crosshair(100, 180, 255, 180, 0);
rgba_t col_3d_hilight(0, 0, 0, 0, 1);
rgba_t col_3d_hilight_line(0, 200, 255, 220, 1);

/*
background      {	0	0	0	255	0	}
grid            {	58	124	197	55	0	}
64grid          {	0	255	255	17	0	}
line_solid      {	200	200	200	255	0	}
line_2s         {	97	119	143	255	0	}
line_monster    {	0	120	0	255	0	}
line_special    {	47	71	189	255	0	}
line_nofirst    {	255	0	0	255	0	}
hilight         {	0	128	255	160	1	}
selection       {	255	0	0	147	1	}
moving          {	255	0	0	255	1	}
tagged          {	255	255	0	159	1	}
vertex          {	140	140	255	255	0	}
linedraw        {	127	0	255	255	0	}
selbox          {	0	140	220	100	1	}
selbox_line     {	100	180	220	200	1	}
crosshair       {	255	255	0	50	1	}
3d_crosshair    {	100	180	255	180	0	}
3d_hilight      {	0	0	0	0	1	}
3d_hilight_line {	0	200	255	220	1	}
*/

coldef_t colours[] =
{
	{ _T("background"),			_T("Background"),				col_background },
	{ _T("grid"),				_T("Grid"),						col_grid },
	{ _T("64grid"),				_T("64 Grid"),					col_64grid },
	{ _T("line_solid"),			_T("Solid Line"),				col_line_solid },
	{ _T("line_2s"),			_T("2-Sided Line"),				col_line_2s },
	{ _T("line_monster"),		_T("Monster Blocking Line"),	col_line_monster },
	{ _T("line_special"),		_T("Special Line"),				col_line_special },
	{ _T("line_nofirst"),		_T("Invalid Line"),				col_line_nofirst },
	{ _T("hilight"),			_T("Hilighted Item"),			col_hilight },
	{ _T("selection"),			_T("Selected Item"),			col_selection },
	{ _T("moving"),				_T("Moving Item"),				col_moving },
	{ _T("tagged"),				_T("Tagged Item"),				col_tagged },
	{ _T("vertex"),				_T("Vertex"),					col_vertex },
	{ _T("linedraw"),			_T("Draw Lines"),				col_linedraw },
	{ _T("selbox"),				_T("Selection Box Inside"),		col_selbox },
	{ _T("selbox_line"),		_T("Selection Box Outline"),	col_selbox_line },
	{ _T("crosshair"),			_T("2D Mode Crosshair"),		col_crosshair },
	{ _T("3d_crosshair"),		_T("3D Mode Crosshair"),		col_3d_crosshair },
	{ _T("3d_hilight"),			_T("3D Mode Hilight Shade"),	col_3d_hilight },
	{ _T("3d_hilight_line"),	_T("3D Mode Hilight Outline"),	col_3d_hilight_line },
};
int n_colours = sizeof(colours) / sizeof(coldef_t);


// parse_rgba: Parses a colour like { R G B (A) (Bl) }
// ------------------------------------------------ >>
void parse_rgba(Tokenizer *tz, rgba_t* col)
{
	// Read opening brace
	tz->check_token(_T("{"));

	// Read rgba
	col->r = tz->get_integer();
	col->g = tz->get_integer();
	col->b = tz->get_integer();

	// Read alpha if specified
	if (tz->peek_token() != _T("}"))
		col->a = tz->get_integer();

	// Read blend if specified
	if (tz->peek_token() != _T("}"))
		col->blend = tz->get_integer();

	// Read closing brace
	tz->check_token(_T("}"));
}

// load_colour_config: Reads a colour configuration file
// -------------------------------------------------- >>
void load_colour_config(string filename)
{
	Tokenizer tz;
	if (!tz.open_file(filename, 0, 0))
		return;

	string token = tz.get_token();

	if (token != _T("colour_config"))
		return;

	tz.check_token(_T("{"));

	hilight_size = 3.0f;
	selection_size = 5.0f;
	moving_size = 5.0f;
	hilight_style = 0;

	token = tz.get_token();
	while (token != _T("}"))
	{
		string name = token;
		
		if (name == _T("hilight_size"))
			hilight_size = tz.get_float();
		else if (name == _T("selection_size"))
			selection_size = tz.get_float();
		else if (name == _T("moving_size"))
			moving_size = tz.get_float();
		else if (name == _T("hilight_style"))
			hilight_style = tz.get_integer();
		else
		{
			rgba_t colour;
			parse_rgba(&tz, &colour);

			for (int a = 0; a < n_colours; a++)
			{
				if (colours[a].name == name)
					colours[a].col = colour;
			}
		}

		token = tz.get_token();
	}
}

// load_colours: Loads colours from slade.cfg
// --------------------------------------- >>
void load_colours(Tokenizer *tz)
{
	tz->check_token(_T("{"));

	hilight_size = 3.0f;
	selection_size = 5.0f;
	moving_size = 5.0f;
	hilight_style = 0;

	string token = tz->get_token();
	while (token != _T("}"))
	{
		string name = token;

		if (name == _T("hilight_size"))
			hilight_size = tz->get_float();
		else if (name == _T("selection_size"))
			selection_size = tz->get_float();
		else if (name == _T("moving_size"))
			moving_size = tz->get_float();
		else if (name == _T("hilight_style"))
			hilight_style = tz->get_integer();
		else
		{
			rgba_t colour;
			parse_rgba(tz, &colour);

			for (int a = 0; a < n_colours; a++)
			{
				if (colours[a].name == name)
					colours[a].col = colour;
			}
		}

		token = tz->get_token();
	}
}

// save_colour_config: Saves the current colour configuration to a file
// ----------------------------------------------------------------- >>
void save_colour_config(string filename)
{
	if (filename == _T(""))
		return;

	int max_size = 0;
	for (DWORD c = 0; c < n_colours; c++)
	{
		if (colours[c].name.size() > max_size)
			max_size = colours[c].name.size();
	}

	FILE* fp = fopen(chr(filename), "wt");

	fprintf(fp, "\ncolour_config\n");
	fprintf(fp, "{\n");

	for (int a = 0; a < n_colours; a++)
	{
		fprintf(fp, "\t%s ", chr(colours[a].name));

		int spaces = max_size - colours[a].name.size();
		for (int b = 0; b < spaces; b++) fprintf(fp, " ");

		fprintf(fp, "{\t%d\t%d\t%d\t%d\t%d\t}\n", colours[a].col.r, colours[a].col.g,
				colours[a].col.b, colours[a].col.a, colours[a].col.blend);
	}

	fprintf(fp, "\n\thilight_size\t%1.2f\n", hilight_size);
	fprintf(fp, "\tselection_size\t%1.2f\n", selection_size);
	fprintf(fp, "\tmoving_size\t%1.2f\n", moving_size);
	fprintf(fp, "\thilight_style\t%d\n", hilight_style);

	fprintf(fp, "}\n\n");

	fclose(fp);
}

// save_colours: Saves colours to slade.cfg
// ------------------------------------- >>
void save_colours(FILE* fp)
{
	int max_size = 0;
	for (DWORD c = 0; c < n_colours; c++)
	{
		if (colours[c].name.size() > max_size)
			max_size = colours[c].name.size();
	}

	fprintf(fp, "colours\n");
	fprintf(fp, "{\n");

	for (int a = 0; a < n_colours; a++)
	{
		fprintf(fp, "\t%s ", chr(colours[a].name));

		int spaces = max_size - colours[a].name.size();
		for (int b = 0; b < spaces; b++) fprintf(fp, " ");

		fprintf(fp, "{\t%d\t%d\t%d\t%d\t%d\t}\n", colours[a].col.r, colours[a].col.g,
				colours[a].col.b, colours[a].col.a, colours[a].col.blend);
	}

	fprintf(fp, "\n\thilight_size\t%1.2f\n", hilight_size);
	fprintf(fp, "\tselection_size\t%1.2f\n", selection_size);
	fprintf(fp, "\tmoving_size\t\t%1.2f\n", moving_size);
	fprintf(fp, "\thilight_style\t%d\n", hilight_style);

	fprintf(fp, "}\n\n");
}
