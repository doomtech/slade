
#ifndef __GAME_CONFIGURATION_H__
#define __GAME_CONFIGURATION_H__

#include "ActionSpecial.h"
#include "ThingType.h"
#include "UDMFProperty.h"

#ifndef __ARCHIVE_H__
// Define map types
enum MapTypes {
	MAP_UNKNOWN,
	MAP_DOOM,
	MAP_HEXEN,
	MAP_DOOM64,
	MAP_UDMF,
};
#endif//__ARCHIVE_H__

struct tt_t {
	ThingType*	type;
	int			number;
	int			index;
	tt_t(ThingType* type = NULL) { this->type = type; index = 0; }

	bool operator< (const tt_t& right) const { return (index < right.index); }
	bool operator> (const tt_t& right) const { return (index > right.index); }
};

struct as_t {
	ActionSpecial*	special;
	int				number;
	int				index;
	as_t(ActionSpecial* special = NULL) { this->special = special; index = 0; }

	bool operator< (const as_t& right) const { return (index < right.index); }
	bool operator> (const as_t& right) const { return (index > right.index); }
};

struct udmfp_t {
	UDMFProperty*	property;
	int				index;
	udmfp_t(UDMFProperty* property = NULL) { this->property = property; index = 0; }

	bool operator< (const udmfp_t& right) const { return (index < right.index); }
	bool operator> (const udmfp_t& right) const { return (index > right.index); }
};

WX_DECLARE_HASH_MAP(int, as_t, wxIntegerHash, wxIntegerEqual, ASpecialMap);
WX_DECLARE_HASH_MAP(int, tt_t, wxIntegerHash, wxIntegerEqual, ThingTypeMap);
WX_DECLARE_STRING_HASH_MAP(udmfp_t, UDMFPropMap);

class ParseTreeNode;
class ArchiveEntry;
class MapLine;
class MapThing;
class MapObject;
class GameConfiguration {
private:
	string			name;
	int				map_format;
	string			udmf_namespace;
	bool			boom;
	string			game_filter;
	ASpecialMap		action_specials;
	ActionSpecial	as_unknown;
	ThingTypeMap	thing_types;
	ThingType		ttype_unknown;
	vector<string>	map_names;
	bool			any_map_name;
	bool			mix_tex_flats;
	bool			tx_textures;

	struct gconf_t {
		string	title;
		string	filename;
		bool operator>(const gconf_t& right) const { return title > right.title; }
		bool operator<(const gconf_t& right) const { return title < right.title; }
	};
	vector<gconf_t>	game_configs;
	size_t lastDefaultConfig;

	// Flags
	struct flag_t {
		int		flag;
		string	name;
		flag_t() { flag = 0; name = ""; }
		flag_t(int flag, string name) { this->flag = flag; this->name = name; }
	};
	vector<flag_t>	flags_thing;
	vector<flag_t>	flags_line;
	vector<flag_t>	triggers_line;

	// Sector types
	struct sectype_t {
		int		type;
		string	name;
		sectype_t() { type = -1; name = "Unknown"; }
		sectype_t(int type, string name) { this->type = type; this->name = name; }
	};
	vector<sectype_t>	sector_types;

	// UDMF properties
	UDMFPropMap	udmf_vertex_props;
	UDMFPropMap	udmf_linedef_props;
	UDMFPropMap	udmf_sidedef_props;
	UDMFPropMap	udmf_sector_props;
	UDMFPropMap	udmf_thing_props;

	// Singleton instance
	static GameConfiguration*	instance;

public:
	GameConfiguration();
	~GameConfiguration();

	// Singleton implementation
	static GameConfiguration* getInstance() {
		if (!instance)
			instance = new GameConfiguration();

		return instance;
	}

	void	setDefaults();
	string	getName() { return name; }
	int		getMapFormat() { return map_format; }
	bool	isBoom() { return boom; }
	bool	anyMapName() { return any_map_name; }
	bool	mixTexFlats() { return mix_tex_flats; }
	bool	txTextures() { return tx_textures; }
	string	udmfNamespace() { return udmf_namespace; }

	string			readConfigName(MemChunk& mc);
	void			init();
	unsigned		nConfigs() { return game_configs.size(); }
	string			configTitle(unsigned index);
	string			configName(unsigned index);
	unsigned		nMapNames() { return map_names.size(); }
	string			mapName(unsigned index) { return map_names[index]; }

	// Config #include handling
	void	buildConfig(string filename, string& out);
	void	buildConfig(ArchiveEntry* entry, string& out);

	// Configuration reading
	void	readActionSpecials(ParseTreeNode* node, ActionSpecial* group_defaults = NULL);
	void	readThingTypes(ParseTreeNode* node, ThingType* group_defaults = NULL);
	void	readUDMFProperties(ParseTreeNode* node, UDMFPropMap& plist);
	bool	readConfiguration(string& cfg, string source = "");
	bool 	open(string filename);
	bool	open(ArchiveEntry* entry);
	bool	openConfig(string name);
	bool	openEmbeddedConfig(ArchiveEntry* entry);
	bool	removeEmbeddedConfig(string name);

	// Action specials
	ActionSpecial*	actionSpecial(unsigned id);
	string			actionSpecialName(int special);
	vector<as_t>	allActionSpecials();

	// Thing types
	ThingType*		thingType(unsigned type);
	vector<tt_t>	allThingTypes();

	// Thing flags
	int		nThingFlags() { return flags_thing.size(); }
	string	thingFlag(unsigned flag_index);
	bool	thingFlagSet(unsigned flag_index, MapThing* thing);
	string	thingFlagsString(int flags);
	void	setThingFlag(unsigned flag_index, MapThing* thing, bool set = true);

	// Line flags
	int		nLineFlags() { return flags_line.size(); }
	string	lineFlag(unsigned flag_index);
	bool	lineFlagSet(unsigned flag_index, MapLine* line);
	string	lineFlagsString(MapLine* line);
	void	setLineFlag(unsigned flag_index, MapLine* line, bool set = true);
	void	setLineBasicFlag(string flag, MapLine* line, bool set = true);

	// Line action (SPAC) triggers
	string			spacTriggerString(MapLine* line);
	wxArrayString	allSpacTriggers();
	void			setLineSpacTrigger(unsigned trigger_index, MapLine* line);

	// UDMF properties
	UDMFProperty*	getUDMFProperty(string name, int type);
	vector<udmfp_t>	allUDMFProperties(int type);

	// Sector types
	string	sectorTypeName(int type);

	// Testing
	void	dumpActionSpecials();
	void	dumpThingTypes();
	void	dumpValidMapNames();
	void	dumpUDMFProperties();
};

// Define for less cumbersome GameConfiguration::getInstance()
#define theGameConfiguration GameConfiguration::getInstance()

#endif//__GAME_CONFIGURATION_H__
