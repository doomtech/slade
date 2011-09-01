
/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "GameConfiguration.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "Misc.h"
#include "Console.h"
#include "Archive.h"
#include <wx/textfile.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
GameConfiguration* GameConfiguration::instance = NULL;


/*******************************************************************
 * GAMECONFIGURATION CLASS FUNCTIONS
 *******************************************************************/

GameConfiguration::GameConfiguration() {
}

GameConfiguration::~GameConfiguration() {
}

/* GameConfiguration::buildConfig
 * Reads the text file at [filename], processing any #include
 * statements in the file recursively. The resulting 'expanded' text
 * is written to [out]
 *******************************************************************/
void GameConfiguration::buildConfig(string filename, string& out) {
	// Open file
	wxTextFile file;
	if (!file.Open(filename))
		return;
	
	// Get file path
	wxFileName fn(filename);
	string path = fn.GetPath(true);
	
	// Go through line-by-line
	string line = file.GetNextLine();
	while (!file.Eof()) {
		// Check for #include
		if (line.StartsWith("#include")) {
			// Get filename to include
			Tokenizer tz;
			tz.openString(line);
			tz.getToken();	// Skip #include
			string file = tz.getToken();
			
			// Process the file
			buildConfig(path + file, out);
		}
		else
			out.Append(line + "\n");
		
		line = file.GetNextLine();
	}
}

/* GameConfiguration::buildConfig
 * Reads the text entry [entry], processing any #include statements
 * in the entry text recursively. The resulting 'expanded' text
 * is written to [out]
 *******************************************************************/
void GameConfiguration::buildConfig(ArchiveEntry* entry, string& out) {
	// Check entry was given
	if (!entry)
		return;

	// Write entry to temp file
	string filename = appPath(entry->getName(), DIR_TEMP);
	entry->exportFile(filename);

	// Open file
	wxTextFile file;
	if (!file.Open(filename))
		return;

	// Go through line-by-line
	string line = file.GetNextLine();
	while (!file.Eof()) {
		// Check for #include
		if (line.StartsWith("#include")) {
			// Get name of entry to include
			Tokenizer tz;
			tz.openString(line);
			tz.getToken();	// Skip #include
			string name = entry->getPath() + tz.getToken();
			
			// Get the entry
			ArchiveEntry* entry_inc = entry->getParent()->entryAtPath(name);
			if (entry_inc)
				buildConfig(entry_inc, out);
			else
				wxLogMessage("Error: Attempting to #include nonexistant entry \"%s\"", CHR(name));
		}
		else
			out.Append(line + "\n");
		
		line = file.GetNextLine();
	}

	// Delete temp file
	wxRemoveFile(filename);
}

void GameConfiguration::readActionSpecials(ParseTreeNode* node) {
	// Determine current 'group'
	ParseTreeNode* group = node;
	string groupname = "";
	while (true) {
		if (group->getName() == "action_specials" || !group)
			break;
		else {
			// Add current node name to group path
			groupname.Prepend(group->getName() + "/");
			group = (ParseTreeNode*)group->getParent();
		}
	}
	if (groupname.EndsWith("/"))
		groupname.RemoveLast();	// Remove last '/'
	
	// Check if this group's action specials require a tag
	bool tagged = false;
	if (node->getChild("tagged"))
		tagged = ((ParseTreeNode*)node->getChild("tagged"))->getBoolValue();
	
	// Go through all child nodes
	for (unsigned a = 0; a < node->nChildren(); a++) {
		ParseTreeNode* child = (ParseTreeNode*)node->getChild(a);
		
		// Check for 'group'
		if (S_CMPNOCASE(child->getType(), "group"))
			readActionSpecials(child);
			
		// Action special
		else if (S_CMPNOCASE(child->getType(), "special")) {
			// Get special id as integer
			long special;
			child->getName().ToLong(&special);
			
			// Check for simple definition
			if (child->isLeaf()) {
				action_specials[special].setName(child->getStringValue());
				action_specials[special].setGroup(groupname);
				action_specials[special].setTagged(tagged);
			}
		}
	}
}

bool GameConfiguration::readConfiguration(string& cfg) {
	// Testing
	MemChunk mc;
	mc.write(CHR(cfg), cfg.Length());
	mc.exportFile("gctest.txt");
	
	// Parse the full configuration
	Parser parser;
	parser.parseText(cfg);
	
	// Process parsed data
	ParseTreeNode* base = parser.parseTreeRoot();
	
	// 'Game' section (this is required for it to be a valid game configuration)
	ParseTreeNode* node_game = (ParseTreeNode*)base->getChild("game");
	if (!node_game) {
		wxLogMessage("Error: Invalid game configuration, 'game' section required but not found");
		return false;
	}
	for (unsigned a = 0; a < node_game->nChildren(); a++) {
		ParseTreeNode* node = (ParseTreeNode*)node_game->getChild(a);
		
		// Game name
		if (S_CMPNOCASE(node->getName(), "name"))
			this->name = node->getStringValue();
			
		// TODO: Valid map names
	}
	
	// Action specials
	ParseTreeNode* node_specials = (ParseTreeNode*)base->getChild("action_specials");
	if (node_specials)
		readActionSpecials(node_specials);
	
	return true;
}

bool GameConfiguration::open(string filename) {
	// Build configuration string from file (process #includes, etc)
	string cfg;
	buildConfig(filename, cfg);

	return readConfiguration(cfg);
}

bool GameConfiguration::open(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Build configuration string from entry (process #includes, etc)
	string cfg;
	buildConfig(entry, cfg);

	return readConfiguration(cfg);
}

void GameConfiguration::dumpActionSpecials() {
	ASpecialMap::iterator i = action_specials.begin();
	
	while (i != action_specials.end()) {
		string tagged = "not tagged";
		if (i->second.needsTag()) tagged = "tagged";
		wxLogMessage("Action special %d = \"%s\" in group \"%s\" (%s)", i->first, CHR(i->second.getName()), CHR(i->second.getGroup()), CHR(tagged));
		i++;
	}
}


#include "ArchiveManager.h"
CONSOLE_COMMAND(testgc, 0) {
	Archive* slade_pk3 = theArchiveManager->programResourceArchive();
	theGameConfiguration->open(slade_pk3->entryAtPath("config/games/doom1.cfg"));
	theGameConfiguration->dumpActionSpecials();
}
