
#ifndef __GAME_CONFIGURATION_H__
#define __GAME_CONFIGURATION_H__

class ActionSpecial {
private:
	string	name;
	string	group;
	bool	tagged;
	
public:
	ActionSpecial(string name = "Unknown", string group = "") { this->name = name; this->group = group; tagged = false; }
	~ActionSpecial() {}
	
	string	getName() { return name; }
	string	getGroup() { return group; }
	bool	needsTag() { return tagged; }
	
	void	setName(string name) { this->name = name; }
	void	setGroup(string group) { this->group = group; }
	void	setTagged(bool tagged) { this->tagged = tagged; }
};

WX_DECLARE_HASH_MAP(int, ActionSpecial, wxIntegerHash, wxIntegerEqual, ASpecialMap);

class ParseTreeNode;
class ArchiveEntry;
class GameConfiguration {
private:
	string		name;
	ASpecialMap	action_specials;
	
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
	bool	readConfiguration(string& cfg);
	bool 	open(string filename);
	bool	open(ArchiveEntry* entry);
	
	// Testing
	void	dumpActionSpecials();
};

// Define for less cumbersome GameConfiguration::getInstance()
#define theGameConfiguration GameConfiguration::getInstance()

#endif//__GAME_CONFIGURATION_H__
