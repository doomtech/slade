
#ifndef __GAME_CONFIGURATION_H__
#define __GAME_CONFIGURATION_H__

#include "ActionSpecial.h"
#include "ThingType.h"
#include "UDMFProperty.h"

struct tt_t {
	ThingType* type;
	tt_t(ThingType* type = NULL) { this->type = type; }
	~tt_t() { if (type) delete type; }
};

WX_DECLARE_HASH_MAP(int, ActionSpecial, wxIntegerHash, wxIntegerEqual, ASpecialMap);
WX_DECLARE_HASH_MAP(int, tt_t, wxIntegerHash, wxIntegerEqual, ThingTypeMap);

class ParseTreeNode;
class ArchiveEntry;
class MapLine;
class MapThing;
class GameConfiguration {
private:
	string			name;
	int				map_format;
	string			udmf_namespace;
	bool			boom;
	string			game_filter;
	ASpecialMap		action_specials;
	ThingTypeMap	thing_types;
	ThingType		ttype_unknown;
	vector<string>	map_names;

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
	vector<UDMFProperty>	udmf_vertex_props;
	vector<UDMFProperty>	udmf_linedef_props;
	vector<UDMFProperty>	udmf_sidedef_props;
	vector<UDMFProperty>	udmf_sector_props;
	vector<UDMFProperty>	udmf_thing_props;

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

	string	getName() { return name; }
	int		getMapFormat() { return map_format; }
	bool	isBoom() { return boom; }

	void	init();

	// Config #include handling
	void	buildConfig(string filename, string& out);
	void	buildConfig(ArchiveEntry* entry, string& out);

	// Configuration reading
	void	readActionSpecials(ParseTreeNode* node, ActionSpecial* group_defaults = NULL);
	void	readThingTypes(ParseTreeNode* node, ThingType* group_defaults = NULL);
	void	readUDMFProperties(ParseTreeNode* node, vector<UDMFProperty>& plist);
	bool	readConfiguration(string& cfg, string source = "");
	bool 	open(string filename);
	bool	open(ArchiveEntry* entry);
	bool	openConfig(string name);

	// Action specials
	ActionSpecial&	actionSpecial(unsigned id) { return action_specials[id]; }
	string			actionSpecialName(int special);

	// Thing types
	ThingType*	thingType(unsigned type);

	// Flags
	int		nThingFlags() { return flags_thing.size(); }
	string	thingFlag(unsigned index);
	bool	thingFlagSet(unsigned index, MapThing* thing);
	string	thingFlagsString(int flags);
	int		nLineFlags() { return flags_line.size(); }
	string	lineFlag(unsigned index);
	bool	lineFlagSet(unsigned index, MapLine* line);
	string	lineFlagsString(MapLine* line);
	string	spacTriggerString(MapLine* line);

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
