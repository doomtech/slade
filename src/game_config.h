
#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

#define GAME_HEXEN		0x01	// Hexen format maps
#define GAME_BOOM		0x02	// Boom extras
#define GAME_ZDOOM		0x04	// Zdoom extras
#define GAME_MAPNAMES	0x08	// Only specific map names allowed

#define SECT_THINGS		0x01
#define SECT_SPECIALS	0x02
#define SECT_SECTORS	0x04
#define SECT_ARGS		0x08
#define SECT_TFLAGS		0x10
#define SECT_LFLAGS		0x20

#include "thing_type.h"
#include "action_special.h"
#include "args.h"
#include "wad.h"

struct sectortype_t
{
	int		type;
	string	name;

	sectortype_t(string name = _T("Unknown"), int type = -1)
	{
		this->name = name;
		this->type = type;
	}

	void parse(Tokenizer *tz)
	{
		type = tz->get_integer();
		name = tz->get_token();
	}
};

class GameConfig
{
private:
	BYTE					game_flags;
	vector<string>			valid_map_names;

	vector<ThingType>		thing_types;
	ThingType				thing_defaults;

	vector<ActionSpecial>	action_specials;
	ActionSpecial			special_unknown;
	ActionSpecial			special_none;

	vector<argtype_t*>		arg_types;

	vector<sectortype_t*>	sector_types;

	vector<key_value_t>		thing_flags;
	vector<key_value_t>		line_flags;

public:
	// Defaults
	string			def_ftex;
	string			def_ctex;
	string			def_uptex;
	string			def_lotex;
	string			def_midtex;
	string			def_doortex;
	string			def_tracktex;
	int				def_fheight;
	int				def_cheight;
	int				def_light;

	GameConfig();
	~GameConfig();

	bool hexen() { return !!(game_flags & GAME_HEXEN); }
	bool boom() { return !!(game_flags & GAME_BOOM); }
	bool zdoom() { return !!(game_flags & GAME_ZDOOM); }
	bool mapnames() { return !!(game_flags & GAME_MAPNAMES); }

	bool load_config(string filename);
	void read_types(Tokenizer *mr, BYTE sections);

	bool valid_map_name(string mapname);
	string ask_map_name();

	void		parse_thing_group(Tokenizer *tz);
	ThingType*	get_ttype(int type);

	void		clear_arg_types();
	argtype_t*	get_arg_type(string name);

	void			parse_action_group(Tokenizer *tz);
	ActionSpecial*	get_special(int type);

	string					get_sector_type(int type);
	vector<sectortype_t*>&	get_stypes() { return sector_types; }

	int			n_line_flags() { return (int)line_flags.size(); }
	key_value_t	line_flag(int index) { return line_flags[index]; }

	int			n_thing_flags() { return (int)thing_flags.size(); }
	key_value_t	thing_flag(int index) { return thing_flags[index]; }

	vector<ActionSpecial>& get_specials() { return action_specials; }
	vector<ThingType>& get_ttypes() { return thing_types; }

	void get_ttype_groups(vector<string>& list);
	void get_ttypes_group(string group, vector<ThingType*>& list);

	void read_decorate_things(Wad* wad, Lump* lump);
	void clear_decorate_things();
	void read_decorate_lumps();
};

void load_game_iwads(Tokenizer *tz);
void save_game_iwads(FILE* fp);

#endif
