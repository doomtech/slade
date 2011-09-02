
#ifndef __GAME_CONFIGURATION_H__
#define __GAME_CONFIGURATION_H__

#include "ActionSpecial.h"
WX_DECLARE_HASH_MAP(int, ActionSpecial, wxIntegerHash, wxIntegerEqual, ASpecialMap);

#include "ThingType.h"
WX_DECLARE_HASH_MAP(int, ThingType, wxIntegerHash, wxIntegerEqual, ThingTypeMap);

class ParseTreeNode;
class ArchiveEntry;
class GameConfiguration {
private:
	string			name;
	ASpecialMap		action_specials;
	ThingTypeMap	thing_types;
	vector<string>	map_names;
	
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

	// Config #include handling
	void	buildConfig(string filename, string& out);
	void	buildConfig(ArchiveEntry* entry, string& out);
	
	// Configuration reading
	void	readActionSpecials(ParseTreeNode* node);
	void	readThingTypes(ParseTreeNode* node);
	bool	readConfiguration(string& cfg, string source = "");
	bool 	open(string filename);
	bool	open(ArchiveEntry* entry);

	// Action specials
	ActionSpecial&	actionSpecial(unsigned id) { return action_specials[id]; }
	string			actionSpecialName(int special);

	// Thing types
	ThingType&		thingType(unsigned type) { return thing_types[type]; }
	
	// Testing
	void	dumpActionSpecials();
	void	dumpThingTypes();
	void	dumpValidMapNames();
};

// Define for less cumbersome GameConfiguration::getInstance()
#define theGameConfiguration GameConfiguration::getInstance()

#endif//__GAME_CONFIGURATION_H__
