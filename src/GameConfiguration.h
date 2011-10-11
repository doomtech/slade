
#ifndef __GAME_CONFIGURATION_H__
#define __GAME_CONFIGURATION_H__

#include "ActionSpecial.h"
#include "ThingType.h"
struct tt_t {
	ThingType* type;
	tt_t(ThingType* type = NULL) { this->type = type; }
	~tt_t() { if (type) delete type; }
};

WX_DECLARE_HASH_MAP(int, ActionSpecial, wxIntegerHash, wxIntegerEqual, ASpecialMap);
WX_DECLARE_HASH_MAP(int, tt_t, wxIntegerHash, wxIntegerEqual, ThingTypeMap);

class ParseTreeNode;
class ArchiveEntry;
class GameConfiguration {
private:
	string			name;
	int				map_format;
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

	void	init();

	// Config #include handling
	void	buildConfig(string filename, string& out);
	void	buildConfig(ArchiveEntry* entry, string& out);
	
	// Configuration reading
	void	readActionSpecials(ParseTreeNode* node);
	void	readThingTypes(ParseTreeNode* node);
	bool	readConfiguration(string& cfg, string source = "");
	bool 	open(string filename);
	bool	open(ArchiveEntry* entry);
	bool	openConfig(string name);

	// Action specials
	ActionSpecial&	actionSpecial(unsigned id) { return action_specials[id]; }
	string			actionSpecialName(int special);

	// Thing types
	ThingType*		thingType(unsigned type);

	// Flags
	string		thingFlagsString(int flags);
	string		lineFlagsString(int flags);
	
	// Testing
	void	dumpActionSpecials();
	void	dumpThingTypes();
	void	dumpValidMapNames();
};

// Define for less cumbersome GameConfiguration::getInstance()
#define theGameConfiguration GameConfiguration::getInstance()

#endif//__GAME_CONFIGURATION_H__
