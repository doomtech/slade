#include "main.h"
#include "map.h"

string def_midtex = "STONE2";
string def_uptex = "STONE2";
string def_lotex = "STONE2";

sidedef_t::sidedef_t()
{
	x_offset = 0;
	y_offset = 0;
	tex_upper = "-";
	tex_lower = "-";
	tex_middle = "-";
	sector = -1;
}

void sidedef_t::def_tex(BYTE tex)
{
	if (tex == TEX_MIDDLE)
		tex_middle = def_midtex;

	if (tex == TEX_UPPER)
		tex_upper = def_uptex;

	if (tex == TEX_LOWER)
		tex_lower = def_lotex;
}
