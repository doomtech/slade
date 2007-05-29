
#include "main.h"
#include "game_config.h"
#include "doom_map.h"
#include "editor_window.h"
#include "wad.h"
#include "colours.h"

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
extern Wad reswad;
extern EditorWindow *editor_window;

void load_game_iwads(Tokenizer *tz)
{
	tz->check_token(_T("{"));

	string token = tz->get_token();
	while (token != _T("}"))
	{
		if (token == _T("game"))
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

	for (unsigned int a = 0; a < game_iwads.size(); a++)
		fprintf(fp, "\tgame \"%s\" \"%s\"\n", chr(game_iwads[a].game), chr(game_iwads[a].iwad_path));

	fprintf(fp, "}\n\n");
}

GameConfig::GameConfig()
{
	game_flags = 0;
	special_none.name = _T("None");
	special_none.type = 0;

	def_ftex = _T("");
	def_ctex = _T("");
	def_uptex = _T("");
	def_lotex = _T("");
	def_midtex = _T("");
	def_doortex = _T("");
	def_tracktex = _T("");
}

GameConfig::~GameConfig()
{
}

bool GameConfig::load_config(string filename)
{
	// Open File
	Tokenizer tz;
	//MemLump ml;

	//get_from_pk3(ml, filename);
	Lump* lump = reswad.getLump(filename, 0, true, true);
	tz.open_mem((char*)lump->getData(), lump->getSize());

	string token = tz.get_token();

	if (token != _T("game"))
	{
		message_box(_T("Invalid game configuration file (must begin with \"game\")"), _T("Error"));
		return false;
	}

	// Read game name
	token = tz.get_token();

	// Load IWAD
	int i = -1;
	for (unsigned int a = 0; a < game_iwads.size(); a++)
	{
		if (game_iwads[a].game == token)
			i = a;
	}

	string iwad_path = _T("");
	if (i == -1)
	{
		wxMessageBox(s_fmt(_T("Please browse for the \"%s\" game IWAD..."), token.c_str()), _T("IWAD Needed"));

		string filename = _T("");
		wxFileDialog browse(NULL, _T("Browse IWAD"), _T(""), _T(""), _T("Doom Wad Files (*.wad)|*.wad"), wxOPEN|wxFILE_MUST_EXIST);
		if (browse.ShowModal() == wxID_OK)
			filename = browse.GetPath();

		if (filename.size())
			iwad_path = filename;
		else
			return false;
	}
	else
		iwad_path = game_iwads[i].iwad_path;

	wads.getWad()->close();
	action_specials.clear();
	thing_types.clear();
	valid_map_names.clear();
	thing_flags.clear();
	line_flags.clear();

	// Check opening brace
	tz.check_token(_T("{"));

	if (!wads.open(iwad_path, WL_IWAD))
	{
		wxMessageBox(s_fmt(_T("Couldn't open IWAD \"%s\"!"), iwad_path.c_str()), _T("Error"));
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
	while (token != _T("}"))
	{
		if (token == _T("boom"))
		{
			if(tz.get_bool())
				game_flags |= GAME_BOOM;
			else
				game_flags = (game_flags & ~GAME_BOOM);
		}

		if (token == _T("hexen"))
			d_map.set_hexen(tz.get_bool());

		if (token == _T("zdoom"))
		{
			if(tz.get_bool())
				game_flags |= GAME_ZDOOM;
			else
				game_flags = (game_flags & ~GAME_ZDOOM);

			mix_tex = zdoom();
		}

		if (token == _T("map_names"))
		{
			if(tz.get_bool())
				game_flags |= GAME_MAPNAMES;
			else
				game_flags = (game_flags & ~GAME_MAPNAMES);
		}

		token = tz.get_token();
	}

	// Read map names
	token = tz.get_token();
	if (token == _T("map_names"))
	{
		tz.check_token(_T("{"));
		token = tz.get_token();

		while (token != _T("}"))
		{
			valid_map_names.push_back(token);
			token = tz.get_token();
		}

		token = tz.get_token();
	}

	// Read defaults
	if (token == _T("defaults"))
	{
		tz.check_token(_T("{"));
		token = tz.get_token();

		while (token != _T("}"))
		{
			if (token == _T("floor_tex"))
				def_ftex = tz.get_token();

			if (token == _T("ceil_tex"))
				def_ctex = tz.get_token();

			if (token == _T("upper_tex"))
				def_uptex = tz.get_token();

			if (token == _T("lower_tex"))
				def_lotex = tz.get_token();

			if (token == _T("middle_tex"))
				def_midtex = tz.get_token();

			if (token == _T("door_tex"))
				def_doortex = tz.get_token();

			if (token == _T("dtrack_tex"))
				def_tracktex = tz.get_token();

			if (token == _T("floor_height"))
				def_fheight = tz.get_integer();

			if (token == _T("ceil_height"))
				def_cheight = tz.get_integer();

			if (token == _T("light"))
				def_light = tz.get_integer();

			token = tz.get_token();
		}
	}

	read_types(&tz, 255);

	// Load default textures/info to theme sidebar
	editor_window->setup_theme_sidebar();

	return true;
}

// read_types: Reads either line, sector, thing or arg types from a file
// ------------------------------------------------------------------ >>
void GameConfig::read_types(Tokenizer *mr, BYTE sections)
{
	string token = mr->get_token();
	while (token != _T("!END"))
	{
		// Thing types
		if (sections & SECT_THINGS && token == _T("thing_types"))
		{
			mr->check_token(_T("{"));

			token = mr->get_token();
			while (token != _T("}"))
			{
				if (token == _T("group"))
					parse_thing_group(mr);

				if (token == _T("import"))
				{
					Tokenizer mr2;
					Lump* lump = reswad.getLump(_T("games/") + mr->get_token(), 0, true, true);
					mr2.open_mem((char*)lump->getData(), lump->getSize());
					read_types(&mr2, SECT_THINGS);
				}

				token = mr->get_token();
			}
		}

		// Action specials
		if (sections & SECT_SPECIALS && token == _T("action_specials"))
		{
			mr->check_token(_T("{"));

			token = mr->get_token();
			while (token != _T("}"))
			{
				if (token == _T("group"))
					parse_action_group(mr);

				if (token == _T("import"))
				{
					Tokenizer mr2;
					Lump* lump = reswad.getLump(_T("games/") + mr->get_token(), 0, true, true);
					mr2.open_mem((char*)lump->getData(), lump->getSize());
					read_types(&mr2, SECT_SPECIALS);
				}

				token = mr->get_token();
			}
		}

		// Sector types
		if (sections & SECT_SECTORS && token == _T("sector_types"))
		{
			mr->check_token(_T("{"));

			token = mr->get_token();
			while (token != _T("}"))
			{
				if (token == _T("type"))
				{
					sectortype_t *stype = new sectortype_t();
					stype->parse(mr);
					sector_types.push_back(stype);
				}

				if (token == _T("import"))
				{
					Tokenizer mr2;
					Lump* lump = reswad.getLump(_T("games/") + mr->get_token(), 0, true, true);
					mr2.open_mem((char*)lump->getData(), lump->getSize());
					read_types(&mr2, SECT_SECTORS);
				}

				token = mr->get_token();
			}
		}

		// Arg types
		if (sections & SECT_ARGS && token == _T("arg_types"))
		{
			mr->check_token(_T("{"));

			token = mr->get_token();
			while (token != _T("}"))
			{
				if (token == _T("type"))
				{
					argtype_t *type = new argtype_t();
					type->parse(mr);
					arg_types.push_back(type);
				}

				if (token == _T("import"))
				{
					Tokenizer mr2;
					Lump* lump = reswad.getLump(_T("games/") + mr->get_token(), 0, true, true);
					mr2.open_mem((char*)lump->getData(), lump->getSize());
					read_types(&mr2, SECT_ARGS);
				}

				token = mr->get_token();
			}
		}

		if (sections & SECT_TFLAGS && token == _T("thing_flags"))
		{
			mr->check_token(_T("{"));

			token = mr->get_token();
			while (token != _T("}"))
			{
				if (token == _T("flag"))
				{
					int val = mr->get_integer();
					string name = mr->get_token();

					thing_flags.push_back(key_value_t(name, val));
				}

				if (token == _T("import"))
				{
					Tokenizer mr2;
					Lump* lump = reswad.getLump(_T("games/") + mr->get_token(), 0, true, true);
					mr2.open_mem((char*)lump->getData(), lump->getSize());
					read_types(&mr2, SECT_TFLAGS);
				}

				token = mr->get_token();
			}
		}

		if (sections & SECT_LFLAGS && token == _T("line_flags"))
		{
			mr->check_token(_T("{"));

			token = mr->get_token();
			while (token != _T("}"))
			{
				if (token == _T("flag"))
				{
					int val = mr->get_integer();
					string name = mr->get_token();

					line_flags.push_back(key_value_t(name, val));
				}

				if (token == _T("import"))
				{
					Tokenizer mr2;
					Lump* lump = reswad.getLump(_T("games/") + mr->get_token(), 0, true, true);
					mr2.open_mem((char*)lump->getData(), lump->getSize());
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
		return mname;
	}
	else
	{
		wxArrayString mapnames;
		for (unsigned int a = 0; a < valid_map_names.size(); a++)
			mapnames.Add(valid_map_names[a]);

		return wxGetSingleChoice(_T("Select Map Name"), _T("Rename Map"), mapnames);
	}

	return _T("");
}

void GameConfig::parse_thing_group(Tokenizer *tz)
{
	thing_defaults.group = tz->get_token();

	tz->check_token(_T("{"));

	// Parse the group
	string token = tz->get_token();
	while (token != _T("}"))
	{
		if (token == _T("default_colour"))
		{
			rgba_t col;
			parse_rgba(tz, &col);
			col.a = 255;
			thing_defaults.colour.set(col);
		}
		if (token == _T("default_size"))
			thing_defaults.radius = tz->get_integer();
		if (token == _T("angle"))
			thing_defaults.set_flag(TTYPE_ANGLE, tz->get_bool());
		if (token == _T("hanging"))
			thing_defaults.set_flag(TTYPE_HANGING, tz->get_bool());

		if (token == _T("thing"))
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
	for (unsigned int a = 0; a < thing_types.size(); a++)
	{
		if (thing_types[a].type == type)
			return &thing_types[a];
	}

	return &thing_defaults;
}

void GameConfig::clear_arg_types()
{
	for (unsigned int a = 0; a < arg_types.size(); a++)
		delete arg_types[a];

	arg_types.clear();
}

argtype_t* GameConfig::get_arg_type(string name)
{
	for (unsigned int a = 0; a < arg_types.size(); a++)
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

	tz->check_token(_T("{"));

	// Parse the group
	string token = tz->get_token();
	while (token != _T("}"))
	{
		if (token == _T("tagged"))
			tagged = tz->get_bool();
		if (token == _T("arg_tag"))
			arg_tag = tz->get_integer();
		if (token == _T("tagtype"))
			tagtype = tz->get_integer();
		if (token == _T("type"))
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

	for (unsigned int a = 0; a < action_specials.size(); a++)
	{
		if (action_specials[a].type == type)
			return &action_specials[a];
	}

	return &special_unknown;
}

string GameConfig::get_sector_type(int type)
{
	for (unsigned int a = 0; a < sector_types.size(); a++)
	{
		if (sector_types[a]->type == type)
			return sector_types[a]->name;
	}

	return _T("Unknown");
}

void GameConfig::read_decorate_things(Wad* wad, Lump* lump)
{
	Tokenizer *tz = new Tokenizer();
	//tz->open_file(_T("sladetemp"), 0, 0);
	tz->open_mem((char*)lump->getData(), lump->getSize());

	string token = tz->get_token();

	while (token != _T("!END"))
	{
		if (token == _T("#include"))
			read_decorate_things(wad, wad->getLump(tz->get_token(), 0, true, true));

		if (!token.CmpNoCase(_T("Actor")))
		{
			ThingType newtype;
			newtype.type = -100;

			// Read name
			newtype.name = tz->get_token();

			// Skip parent class if present
			if (tz->peek_token() == _T(":"))
			{
				tz->get_token();
				tz->get_token();
			}

			// Skip 'replaces' stuff
			if (!tz->peek_token().CmpNoCase(_T("replaces")))
			{
				tz->get_token();
				tz->get_token();
			}
			else
			{
				// Read doomednum if present
				if (tz->peek_token() != _T("{"))
					newtype.type = tz->get_integer();
			}

			// Read through actor definition
			token = tz->get_token();
			if (token == _T("{"))
			{
				int level = 1;
				bool states = false;

				while (level > 0)
				{
					token = tz->get_token();

					if (token == _T("{"))
					{
						level++;
						continue;
					}

					if (token == _T("}"))
					{
						level--;
						states = false;
						continue;
					}

					if (newtype.type != -100)
					{
						if (!token.CmpNoCase(_T("//$Category")))
							newtype.group = tz->get_token();

						if (!token.CmpNoCase(_T("states")))
						{
							states = true;
							continue;
						}

						if (!token.CmpNoCase(_T("radius")))
							newtype.radius = tz->get_integer();

						if (!token.CmpNoCase(_T("xscale")))
							newtype.x_scale = tz->get_float();

						if (!token.CmpNoCase(_T("yscale")))
							newtype.y_scale = tz->get_float();

						if (!token.CmpNoCase(_T("scale")))
						{
							float scale = tz->get_float();
							newtype.x_scale = scale;
							newtype.y_scale = scale;
						}

						if (!token.CmpNoCase(_T("alpha")))
							newtype.alpha = tz->get_float();

						// Spawn state overrides others
						if (!token.CmpNoCase(_T("spawn:")))
							states = true;

						// Try to get sprite from states
						if (states)
						{
							if (token.Right(1) == _T(":"))
								continue;

							string sprite = token;
							if (tz->peek_token() == _T("}"))
								continue;

							string frames = tz->get_token();
							if (tz->peek_token() == _T("}"))
								continue;

							string len = tz->get_token();

							if (len.IsNumber() && len != _T("0"))
							{
								string spritename = sprite + frames.Left(1) + _T("0");
								Lump *lump = wads.getLump(spritename);
								if (!lump)
								{
									spritename = sprite + frames.Left(1) + _T("1");
									lump = wads.getLump(spritename);
								}

								if (lump)
								{
									if (!(vector_exists(spritenames, lump->getName())))
										spritenames.push_back(lump->getName());

									newtype.spritename = lump->getName();
									states = false;
								}
							}
							else
								tz->next_line();
						}
					}
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
	for (unsigned int a = 0; a < thing_types.size(); a++)
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

	for (int a = 0; a < wads.nWads(); a++)
	{
		if (wads.getWad(a)->getLump(_T("DECORATE"), 0))
			read_decorate_things(wads.getWad(a), wads.getWad(a)->getLump(_T("DECORATE"), 0));
	}
}

void GameConfig::get_ttype_groups(vector<string>& list)
{
	for (unsigned int a = 0; a < thing_types.size(); a++)
	{
		string group = thing_types[a].group;
		if (!(vector_exists(list, group)))
			list.push_back(group);
	}
}

void GameConfig::get_ttypes_group(string group, vector<ThingType*>& list)
{
	for (unsigned int a = 0; a < thing_types.size(); a++)
	{
		if (thing_types[a].group == group)
			list.push_back(&thing_types[a]);
	}
}
