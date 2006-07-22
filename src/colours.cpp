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
	{ "background",			"Background",				col_background },
	{ "grid",				"Grid",						col_grid },
	{ "64grid",				"64 Grid",					col_64grid },
	{ "line_solid",			"Solid Line",				col_line_solid },
	{ "line_2s",			"2-Sided Line",				col_line_2s },
	{ "line_monster",		"Monster Blocking Line",	col_line_monster },
	{ "line_special",		"Special Line",				col_line_special },
	{ "line_nofirst",		"Invalid Line",				col_line_nofirst },
	{ "hilight",			"Hilighted Item",			col_hilight },
	{ "selection",			"Selected Item",			col_selection },
	{ "moving",				"Moving Item",				col_moving },
	{ "tagged",				"Tagged Item",				col_tagged },
	{ "vertex",				"Vertex",					col_vertex },
	{ "linedraw",			"Draw Lines",				col_linedraw },
	{ "selbox",				"Selection Box Inside",		col_selbox },
	{ "selbox_line",		"Selection Box Outline",	col_selbox_line },
	{ "crosshair",			"2D Mode Crosshair",		col_crosshair },
	{ "3d_crosshair",		"3D Mode Crosshair",		col_3d_crosshair },
	{ "3d_hilight",			"3D Mode Hilight Shade",	col_3d_hilight },
	{ "3d_hilight_line",	"3D Mode Hilight Outline",	col_3d_hilight_line },
};
int n_colours = sizeof(colours) / sizeof(coldef_t);


// parse_rgba: Parses a colour like { R G B (A) (Bl) }
// ------------------------------------------------ >>
void parse_rgba(Tokenizer *tz, rgba_t* col)
{
	// Read opening brace
	tz->check_token("{");

	// Read rgba
	col->r = tz->get_integer();
	col->g = tz->get_integer();
	col->b = tz->get_integer();

	// Read alpha if specified
	if (tz->peek_token() != "}")
		col->a = tz->get_integer();

	// Read blend if specified
	if (tz->peek_token() != "}")
		col->blend = tz->get_integer();

	// Read closing brace
	tz->check_token("}");
}

// load_colour_config: Reads a colour configuration file
// -------------------------------------------------- >>
void load_colour_config(string filename)
{
	Tokenizer tz;
	if (!tz.open_file(filename, 0, 0))
		return;

	string token = tz.get_token();

	if (token != "colour_config")
		return;

	tz.check_token("{");

	hilight_size = 3.0f;
	selection_size = 5.0f;
	moving_size = 5.0f;
	hilight_style = 0;

	token = tz.get_token();
	while (token != "}")
	{
		string name = token;
		
		if (name == "hilight_size")
			hilight_size = tz.get_float();
		else if (name == "selection_size")
			selection_size = tz.get_float();
		else if (name == "moving_size")
			moving_size = tz.get_float();
		else if (name == "hilight_style")
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
	tz->check_token("{");

	hilight_size = 3.0f;
	selection_size = 5.0f;
	moving_size = 5.0f;
	hilight_style = 0;

	string token = tz->get_token();
	while (token != "}")
	{
		string name = token;

		if (name == "hilight_size")
			hilight_size = tz->get_float();
		else if (name == "selection_size")
			selection_size = tz->get_float();
		else if (name == "moving_size")
			moving_size = tz->get_float();
		else if (name == "hilight_style")
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
	if (filename == "")
		return;

	int max_size = 0;
	for (DWORD c = 0; c < n_colours; c++)
	{
		if (colours[c].name.size() > max_size)
			max_size = colours[c].name.size();
	}

	FILE* fp = fopen(filename.c_str(), "wt");

	fprintf(fp, "\ncolour_config\n");
	fprintf(fp, "{\n");

	for (int a = 0; a < n_colours; a++)
	{
		fprintf(fp, "\t%s ", colours[a].name.c_str());

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
		fprintf(fp, "\t%s ", colours[a].name.c_str());

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
