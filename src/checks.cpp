// <<--------------------------------------------->>
// << SLADE (SlayeR's 'LeetAss Doom Editor)       >>
// << By Simon Judd, 2004                         >>
// << ------------------------------------------- >>
// << <checks.cpp>                                >>
// << Map checking functions                      >>
// <<--------------------------------------------->>

// Includes ------------------------------------- >>
#include "main.h"
#include "map.h"
#include "textures.h"
//#include "line_edit.h"
//#include "sector_edit.h"
#include "console.h"

// Variables ------------------------------------ >>

// External Variables --------------------------- >>

extern int hilight_item;
extern vector<int> selected_items;

// clean_tags: Finds any unpaired tags and frees them
// ----------------------------------------------- >>
DWORD clean_tags()
{
	vector<int> ltags;
	vector<int> stags;
	DWORD c = 0;
	
	// Add all existing sector tags
	for (DWORD s = 0; s < map.n_sectors; s++)
	{
		if (map.sectors[s]->tag > 0 && !(vector_exists(stags, map.sectors[s]->tag)))
			stags.push_back(map.sectors[s]->tag);
	}

	// Add all existing line tags, except those without a matching
	// sector tag (in that case, clear the tag)
	for (DWORD l = 0; l < map.n_lines; l++)
	{
		if (map.lines[l]->sector_tag > 0)
		{
			if (!(vector_exists(stags, map.lines[l]->sector_tag)))
			{
				map.lines[l]->sector_tag = 0;
				c++;
			}
			else if (!(vector_exists(ltags, map.lines[l]->sector_tag)))
				ltags.push_back(map.lines[l]->sector_tag);
		}
	}
	
	// Clear all unmatched sector tags
	for (DWORD s = 0; s < map.n_sectors; s++)
	{
		if (map.sectors[s]->tag > 0)
		{
			if (!(vector_exists(ltags, map.sectors[s]->tag)))
			{
				map.sectors[s]->tag = 0;
				c++;
			}
		}
	}
	
	return c;
}

// remove_free_verts: Deletes any vertices not attached to a line
// ----------------------------------------------------------- >>
DWORD remove_free_verts()
{
	DWORD c = 0;

	vector<vertex_t*> used_verts;
	for (int a = 0; a < map.n_lines; a++)
	{
		vector_add_nodup(used_verts, map.verts[map.lines[a]->vertex1]);
		vector_add_nodup(used_verts, map.verts[map.lines[a]->vertex2]);
	}

	for (int a = 0; a < map.n_verts; a++)
	{
		if (vector_exists(used_verts, map.verts[a]))
			continue;

		map.delete_vertex(a);
		a--;
		c++;
	}

	return c;
}

// remove_zerolength_lines: Removes any lines with both ends at the same position
// --------------------------------------------------------------------------- >>
DWORD remove_zerolength_lines()
{
	DWORD c = 0;

	for (DWORD l = 0; l < map.n_lines; l++)
	{
		rect_t r = map.l_getrect(l);

		if (r.x1() == r.x2()
			&& r.y1() == r.y2())
		{
			map.delete_line(l);
			l--;
			c++;
		}
	}

	map.change_level(MC_LINES);
	//remove_free_verts();

	return c;
}

// remove_unused_sectors: Removes any unused sectors in the map
// --------------------------------------------------------- >>
DWORD remove_unused_sectors()
{
	DWORD c = 0;
	vector<int> used_sectors;
	bool done = false;

	while (!done)
	{
		done = true;
		used_sectors.clear();

		for (DWORD s = 0; s < map.n_sides; s++)
		{
			if (!(vector_exists(used_sectors, map.sides[s]->sector)))
				used_sectors.push_back(map.sides[s]->sector);
		}

		for (DWORD s = 0; s < map.n_sectors; s++)
		{
			if (!(vector_exists(used_sectors, s)))
			{
				map.delete_sector(s);
				done = false;
				c++;
				break;
			}
		}
	}

	return c;
}

DWORD remove_unused_sides()
{
	DWORD c = 0;
	bool done = false;
	vector<int> used_sides;

	while (!done)
	{
		done = true;
		used_sides.clear();

		for (int a = 0; a < map.n_lines; a++)
		{
			used_sides.push_back(map.lines[a]->side1);
			used_sides.push_back(map.lines[a]->side2);
		}

		for (int a = 0; a < map.n_sides; a++)
		{
			if (!(vector_exists(used_sides, a)))
			{
				map.delete_side(a);
				done = false;
				c++;
				break;
			}
		}
	}

	return c;
}

// get_free_tag: Gets the lowest free tag,
// might be slow on large maps with lots of tags, but at least it doesn't
// simply find the highest tag used and return 1 higher...
// ------------------------------------------------------------------- >>
WORD get_free_tag()
{
	WORD tag = 1;
	bool tagmoved = false;
	
	while (1)
	{
		tagmoved = false;
		
		for (DWORD l = 0; l < map.n_lines; l++)
		{
			if (map.lines[l]->sector_tag == tag)
			{
				tag++;
				tagmoved = true;
			}
		}
		
		for (DWORD s = 0; s < map.n_sectors; s++)
		{
			if (map.sectors[s]->tag == tag)
			{
				tag++;
				tagmoved = true;
			}
		}
		
		if (!tagmoved)
			break;
	}
	
	return tag;
}

// get_free_tid: Gets the lowest free thing id
// ---------------------------------------- >>
WORD get_free_tid()
{
	WORD tid = 1;
	bool moved = false;
	
	while (1)
	{
		moved = false;
		
		for (DWORD t = 0; t < map.n_things; t++)
		{
			if (map.things[t]->tid == tid)
			{
				tid++;
				moved = true;
			}
		}
		
		if (!moved)
			break;
	}
	
	return tid;
}

void check_textures()
{
	selected_items.clear();
	hilight_item = -1;
	int line = -1;

	// Check line textures
	for (int l = 0; l < map.n_lines; l++)
	{
		sidedef_t *side = map.l_getside(l, 1);

		if (side)
		{
			if (get_texture(side->tex_upper, 1)->name == "_notex"
				&& side->tex_upper != "-")
			{
				string line = parse_string("Line %d has invalid front upper texture \"%s\"\n'Yes' to edit, 'No' to stop checking",
											l, side->tex_upper.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}

			if (get_texture(side->tex_middle, 1)->name == "_notex"
				&& side->tex_middle != "-")
			{
				string line = parse_string("Line %d has invalid front middle texture \"%s\"\n'Yes' to edit, 'No' to stop checking",
											l, side->tex_middle.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}

			if (get_texture(side->tex_lower, 1)->name == "_notex"
				&& side->tex_lower != "-")
			{
				string line = parse_string("Line %d has invalid front lower texture \"%s\"\n'Yes' to edit, 'No' to stop checking",
											l, side->tex_lower.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}
		}

		side = map.l_getside(l, 2);

		if (side)
		{
			if (get_texture(side->tex_upper, 1)->name == "_notex"
				&& side->tex_upper != "-")
			{
				string line = parse_string("Line %d has invalid back upper texture \"%s\"\n'Yes' to edit, 'No' to stop checking",
											l, side->tex_upper.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}

			if (get_texture(side->tex_middle, 1)->name == "_notex"
				&& side->tex_middle != "-")
			{
				string line = parse_string("Line %d has invalid back middle texture \"%s\"\n'Yes' to edit, 'No' to stop checking",
											l, side->tex_middle.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}

			if (get_texture(side->tex_lower, 1)->name == "_notex"
				&& side->tex_lower != "-")
			{
				string line = parse_string("Line %d has invalid back lower texture \"%s\"\n'Yes' to edit, 'No' to stop checking",
											l, side->tex_lower.c_str());
				//if (!yesno_box(line))
				//	return;

				hilight_item = l;
				//open_line_edit();
			}
		}
	}

	// Check flat textures
	for (int s = 0; s < map.n_sectors; s++)
	{
		if (get_texture(map.sectors[s]->f_tex, 2)->name == "_notex")
		{
			string line = parse_string("Sector %d has invalid floor texture \"%s\"\n'Yes' to edit, 'No' to stop checking",
										s, map.sectors[s]->f_tex.c_str());
			//if (!yesno_box(line))
			//	return;

			hilight_item = s;
			//open_sector_edit();
		}

		if (get_texture(map.sectors[s]->c_tex, 2)->name == "_notex")
		{
			string line = parse_string("Sector %d has invalid ceiling texture \"%s\"\n'Yes' to edit, 'No' to stop checking",
										s, map.sectors[s]->c_tex.c_str());
			//if (!yesno_box(line))
			//	return;

			hilight_item = s;
			//open_sector_edit();
		}
	}
}

bool check_lines()
{
	bool errors = false;

	for (int a = 0; a < map.n_lines; a++)
	{
		bool error = false;

		if (map.lines[a]->vertex1 > map.n_verts)
		{
			console_print(parse_string("Line #%d has invalid vertex %d! Deleting.", a, map.lines[a]->vertex1));
			error = true;
		}

		if (map.lines[a]->vertex2 > map.n_verts && !error)
		{
			console_print(parse_string("Line #%d has invalid vertex %d! Deleting.", a, map.lines[a]->vertex2));
			error = true;
		}

		if (map.lines[a]->side1 > (double)map.n_sides)
		{
			console_print(parse_string("Line %d references invalid sidedef %d! Setting it to -1.", a, map.lines[a]->side1));
			map.lines[a]->side1 = -1;
		}

		if (map.lines[a]->side2 > (double)map.n_sides)
		{
			console_print(parse_string("Line %d references invalid sidedef %d! Setting it to -1.", a, map.lines[a]->side2));
			map.lines[a]->side2 = -1;
		}


		if (error)
		{
			map.delete_line(a);
			a--;
			errors = true;
		}
	}

	return errors;
}

bool check_sides()
{
	bool errors = false;

	for (int a = 0; a < map.n_sides; a++)
	{
		bool error = false;

		if (map.sides[a]->sector > map.n_sectors || map.sides[a]->sector < 0)
		{
			console_print(parse_string("Sidedef #%d has invalid sector reference %d! Deleting.", a, map.sides[a]->sector));
			error = true;
		}

		if (error)
		{
			map.delete_side(a);
			a--;
			errors = true;
		}
	}

	return errors;
}
