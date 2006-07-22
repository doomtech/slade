
#include "main.h"
#include "game_config.h"
#include "doom_map.h"
#include "wx_stuff.h"
#include "wad.h"
#include "colours.h"
#include "archive.h"

GameConfig game;

struct iwadinfo_t
{
	string game;
	string iwad_path;
};
vector<iwadinfo_t> game_iwads;

#define SECT_THINGS		0x01
#define SECT_SPECIALS	0x02
#define SECT_SECTORS	0x04
#define SECT_ARGS		0x08
#define SECT_TFLAGS		0x10
#define SECT_LFLAGS		0x20

extern DoomMap d_map;
extern WadList wads;
extern vector<string> spritenames;
extern bool mix_tex;

void load_game_iwads(Tokenizer *tz)
{
	tz->check_token("{");

	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "game")
		{
			iwadinfo_t ii;
			ii.game = tz->get_token();
			ii.iwad_path = tz->get_token();
			game_iwads.push_back(ii);
		}

		token = tz->get_token();
	}
}

void save_game_iwads(FILE* fp)
{
	fprintf(fp, "iwads\n{\n");

	for (int a = 0; a < game_iwads.size(); a++)
		fprintf(fp, "\tgame \"%s\" \"%s\"\n", game_iwads[a].game.c_str(), game_iwads[a].iwad_path.c_str());

	fprintf(fp, "}\n\n");
}

GameConfig::GameConfig()
{
	game_flags = 0;
	special_none.name = "None";
	special_none.type = 0;
}

GameConfig::~GameConfig()
{
}

bool GameConfig::load_config(string filename)
{
	// Open File
	Tokenizer tz;
	dump_from_pk3(filename);
	if (!tz.open_file("sladetemp", 0, 0))
		return false;

	string token = tz.get_token();

	if (token != "game")
	{
		message_box("Invalid game configuration file (must begin with \"game\")", "Error");
		return false;
	}

	// Read game name
	token = tz.get_token();

	// Load IWAD
	int i = -1;
	for (int a = 0; a < game_iwads.size(); a++)
	{
		if (game_iwads[a].game == token)
			i = a;
	}

	string iwad_path = "";
	if (i == -1)
	{
		wxMessageBox(str_to_wx(s_fmt("Please browse for the \"%s\" game IWAD...", token.c_str())), _T("IWAD Needed"));
		
		string filename = "";
		wxFileDialog browse(NULL, _T("Browse IWAD"), _T(""), _T(""), _T("Doom Wad Files (*.wad)|*.wad"), wxOPEN|wxFILE_MUST_EXIST);
		if (browse.ShowModal() == wxID_OK)
			filename = wx_to_str(browse.GetPath());

		if (filename != "")
			iwad_path = filename;
		else
			return false;
	}
	else
		iwad_path = game_iwads[i].iwad_path;

	wads.get_iwad()->close();
	action_specials.clear();
	thing_types.clear();
	valid_map_names.clear();
	thing_flags.clear();
	line_flags.clear();

	// Check opening brace
	tz.check_token("{");

	if (!wads.open_iwad(iwad_path))
	{
		wxMessageBox(str_to_wx(s_fmt("Couldn't open IWAD \"%s\"!", iwad_path.c_str())), _T("Error"));
		game_iwads.erase(game_iwads.begin() + i);
		return false;
	}
	else if (i == -1)
	{
		iwadinfo_t ii;
		ii.game = token;
		ii.iwad_path = iwad_path;
		game_iwads.push_back(ii);
	}

	// Read game section
	token = tz.get_token();
	while (token != "}")
	{
		if (token == "boom")
		{
			if(tz.get_bool())
				game_flags |= GAME_BOOM;
			else
				game_flags = (game_flags & ~GAME_BOOM);
		}

		if (token == "hexen")
			d_map.set_hexen(tz.get_bool());

		if (token == "zdoom")
		{
			if(tz.get_bool())
				game_flags |= GAME_ZDOOM;
			else
				game_flags = (game_flags & ~GAME_ZDOOM);

			mix_tex = zdoom();
		}

		if (token == "map_names")
		{
			if(tz.get_bool())
				game_flags |= GAME_MAPNAMES;
			else
				game_flags = (game_flags & ~GAME_MAPNAMES);
		}

		token = tz.get_token();
	}

	// Read map names
	if (tz.get_token() == "map_names")
	{
		tz.check_token("{");
		token = tz.get_token();

		while (token != "}")
		{
			valid_map_names.push_back(token);
			token = tz.get_token();
		}
	}

	// Read defaults
	if (tz.get_token() == "defaults")
	{
		tz.check_token("{");
		token = tz.get_token();

		while (token != "}")
		{
			if (token == "floor_tex")
				def_ftex = tz.get_token();

			if (token == "ceil_tex")
				def_ctex = tz.get_token();

			if (token == "upper_tex")
				def_uptex = tz.get_token();

			if (token == "lower_tex")
				def_lotex = tz.get_token();

			if (token == "middle_tex")
				def_midtex = tz.get_token();

			if (token == "door_tex")
				def_doortex = tz.get_token();

			if (token == "dtrack_tex")
				def_tracktex = tz.get_token();

			if (token == "floor_height")
				def_fheight = tz.get_integer();

			if (token == "ceil_height")
				def_cheight = tz.get_integer();

			if (token == "light")
				def_light = tz.get_integer();

			token = tz.get_token();
		}
	}

	read_types(&tz, 255);

	return true;
}

// read_types: Reads either line, sector, thing or arg types from a file
// ------------------------------------------------------------------ >>
void GameConfig::read_types(Tokenizer *mr, BYTE sections)
{
	string token = mr->get_token();
	while (token != "!END")
	{
		// Thing types
		if (sections & SECT_THINGS && token == "thing_types")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "group")
					parse_thing_group(mr);

				if (token == "import")
				{
					Tokenizer mr2;
					dump_from_pk3("games/" + mr->get_token());
					mr2.open_file("sladetemp");
					read_types(&mr2, SECT_THINGS);
				}

				token = mr->get_token();
			}
		}

		// Action specials
		if (sections & SECT_SPECIALS && token == "action_specials")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "group")
					parse_action_group(mr);

				if (token == "import")
				{
					Tokenizer mr2;
					dump_from_pk3("games/" + mr->get_token());
					mr2.open_file("sladetemp");
					read_types(&mr2, SECT_SPECIALS);
				}

				token = mr->get_token();
			}
		}

		// Sector types
		if (sections & SECT_SECTORS && token == "sector_types")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "type")
				{
					sectortype_t *stype = new sectortype_t();
					stype->parse(mr);
					sector_types.push_back(stype);
				}

				if (token == "import")
				{
					Tokenizer mr2;
					dump_from_pk3("games/" + mr->get_token());
					mr2.open_file("sladetemp");
					read_types(&mr2, SECT_SECTORS);
				}

				token = mr->get_token();
			}
		}

		// Arg types
		if (sections & SECT_ARGS && token == "arg_types")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "type")
				{
					argtype_t *type = new argtype_t();
					type->parse(mr);
					arg_types.push_back(type);
				}

				if (token == "import")
				{
					Tokenizer mr2;
					dump_from_pk3("games/" + mr->get_token());
					mr2.open_file("sladetemp");
					read_types(&mr2, SECT_ARGS);
				}

				token = mr->get_token();
			}
		}

		if (sections & SECT_TFLAGS && token == "thing_flags")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "flag")
				{
					int val = mr->get_integer();
					string name = mr->get_token();

					thing_flags.push_back(key_value_t(name, val));
				}

				if (token == "import")
				{
					Tokenizer mr2;
					dump_from_pk3("games/" + mr->get_token());
					mr2.open_file("sladetemp");
					read_types(&mr2, SECT_TFLAGS);
				}

				token = mr->get_token();
			}
		}

		if (sections & SECT_LFLAGS && token == "line_flags")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "flag")
				{
					int val = mr->get_integer();
					string name = mr->get_token();

					line_flags.push_back(key_value_t(name, val));
				}

				if (token == "import")
				{
					Tokenizer mr2;
					dump_from_pk3("games/" + mr->get_token());
					mr2.open_file("sladetemp");
					read_types(&mr2, SECT_LFLAGS);
				}

				token = mr->get_token();
			}
		}

		token = mr->get_token();
	}
}

bool GameConfig::valid_map_name(string mapname)
{
	if (mapname.size() > 8)
		return false;

	if (valid_map_names.size() == 0)
		return true;
	else if (vector_exists(valid_map_names, mapname))
		return true;

	return false;
}

string GameConfig::ask_map_name()
{
	if (valid_map_names.size() == 0)
	{
		wxString mname;
		mname = wxGetTextFromUser(_T("Enter Map Name:"), _T("Rename Map"));
		mname.UpperCase();
		mname.Truncate(8);
		return wx_to_str(mname);
	}
	else
	{
		wxArrayString mapnames;
		for (int a = 0; a < valid_map_names.size(); a++)
			mapnames.Add(str_to_wx(valid_map_names[a]));

		return wx_to_str(wxGetSingleChoice(_T("Select Map Name"), _T("Rename Map"), mapnames)).c_str();
	}

	return "";
}

void GameConfig::parse_thing_group(Tokenizer *tz)
{
	thing_defaults.group = tz->get_token();

	tz->check_token("{");

	// Parse the group
	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "default_colour")
		{
			rgba_t col;
			parse_rgba(tz, &col);
			col.a = 255;
			thing_defaults.colour.set(col);
		}
		if (token == "default_size")
			thing_defaults.radius = tz->get_integer();
		if (token == "angle")
			thing_defaults.set_flag(TTYPE_ANGLE, tz->get_bool());
		if (token == "hanging")
			thing_defaults.set_flag(TTYPE_HANGING, tz->get_bool());

		if (token == "thing")
		{
			ThingType newtype(&thing_defaults);
			newtype.parse(tz);
			thing_types.push_back(newtype);
		}

		token = tz->get_token();
	}

	thing_defaults.setup();
}

ThingType* GameConfig::get_ttype(int type)
{
	for (int a = 0; a < thing_types.size(); a++)
	{
		if (thing_types[a].type == type)
			return &thing_types[a];
	}

	return &thing_defaults;
}

void GameConfig::clear_arg_types()
{
	for (int a = 0; a < arg_types.size(); a++)
		delete arg_types[a];

	arg_types.clear();
}

argtype_t* GameConfig::get_arg_type(string name)
{
	for (int a = 0; a < arg_types.size(); a++)
	{
		if (arg_types[a]->name == name)
			return arg_types[a];
	}

	return NULL;
}

void GameConfig::parse_action_group(Tokenizer *tz)
{
	// Defaults
	bool tagged = false;
	char arg_tag = 0;
	char tagtype = 0;

	// Get group name
	string group = tz->get_token();

	tz->check_token("{");

	// Parse the group
	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "tagged")
			tagged = tz->get_bool();
		if (token == "arg_tag")
			arg_tag = tz->get_integer();
		if (token == "tagtype")
			tagtype = tz->get_integer();
		if (token == "type")
		{
			ActionSpecial newspecial(group);
			newspecial.tagtype = tagtype;

			if (tagged)
				newspecial.arg_tag = arg_tag;
			else
				newspecial.arg_tag = -1;

			newspecial.parse(tz);
			action_specials.push_back(newspecial);
		}

		token = tz->get_token();
	}
}

ActionSpecial* GameConfig::get_special(int type)
{
	if (type == 0)
		return &special_none;

	for (int a = 0; a < action_specials.size(); a++)
	{
		if (action_specials[a].type == type)
			return &action_specials[a];
	}

	return &special_unknown;
}

string GameConfig::get_sector_type(int type)
{
	for (int a = 0; a < sector_types.size(); a++)
	{
		if (sector_types[a]->type == type)
			return sector_types[a]->name;
	}

	return "Unknown";
}

void GameConfig::read_decorate_things(Wad* wad, Lump* lump)
{
	lump->DumpToFile("sladetemp");

	Tokenizer *tz = new Tokenizer();
	tz->open_file("sladetemp", 0, 0);

	string token = tz->get_token();

	while (token != "!END")
	{
		//wxLogMessage("%s", token.c_str());
		if (token == "#include")
		{
			//wxLogMessage("Include %s", tz->peek_token().c_str());
			read_decorate_things(wad, wad->get_lump(tz->get_token(), 0));
		}

		if (token == "actor" || token == "ACTOR" || token == "Actor")
		{
			ThingType newtype;

			// Read name
			newtype.name = tz->get_token();

			// Skip parent class if present
			if (tz->peek_token() == ":")
			{
				tz->get_token();
				tz->get_token();
			}

			// Read doomednum if present
			newtype.type = -100;
			if (tz->peek_token() != "{")
				newtype.type = tz->get_integer();

			// Read through actor definition
			token = tz->get_token();
			if (token == "{")
			{
				int level = 1;

				while (level > 0)
				{
					token = tz->get_token();

					if (token == "{")
						level++;

					if (token == "}")
						level--;

					if (token == "//$Category")
						newtype.group = tz->get_token();

					if (token == "//$EditorSprite")
					{
						string spritename = tz->get_token();

						if (!(vector_exists(spritenames, spritename)))
							spritenames.push_back(spritename);

						newtype.spritename = spritename;
					}

					if (token == "radius")
						newtype.radius = tz->get_integer();

					if (token == "xscale")
						newtype.x_scale = tz->get_float();

					if (token == "yscale")
						newtype.y_scale = tz->get_float();

					if (token == "scale")
					{
						float scale = tz->get_float();
						newtype.x_scale = scale;
						newtype.y_scale = scale;
					}

					if (token == "alpha")
						newtype.alpha = tz->get_float();
				}
			}

			// If the actor has a doomednum
			if (newtype.type != -100)
			{
				newtype.flags |= TTYPE_DECORATE;
				thing_types.push_back(newtype);
			}
		}

		token = tz->get_token();
	}
}

void GameConfig::clear_decorate_things()
{
	for (int a = 0; a < thing_types.size(); a++)
	{
		if (thing_types[a].flags & TTYPE_DECORATE)
		{
			vector<ThingType>::iterator i = thing_types.begin();
			i += a;
			thing_types.erase(i);
			a--;
		}
	}
}

void GameConfig::read_decorate_lumps()
{
	if (!zdoom())
		return;

	clear_decorate_things();

	for (int a = 0; a < wads.n_wads; a++)
	{
		if (wads.get_wad(a)->get_lump("DECORATE", 0))
			read_decorate_things(wads.get_wad(a), wads.get_wad(a)->get_lump("DECORATE", 0));
	}
}
