
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
#include "ArchiveManager.h"
#include "SLADEMap.h"
#include <wx/textfile.h>
#include <wx/filename.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
GameConfiguration* GameConfiguration::instance = NULL;
CVAR(String, game_configuration, "", CVAR_SAVE)


/*******************************************************************
 * GAMECONFIGURATION CLASS FUNCTIONS
 *******************************************************************/

GameConfiguration::GameConfiguration() {
	game_filter = "";
	udmf_namespace = "";
	ttype_unknown.icon = "unknown";
	ttype_unknown.shrink = true;
}

GameConfiguration::~GameConfiguration() {
}

void GameConfiguration::init() {
	// Load last configuration if any
	if (!string(game_configuration).IsEmpty())
		openConfig(game_configuration);
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

void GameConfiguration::readActionSpecials(ParseTreeNode* node, ActionSpecial* group_defaults) {
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

	// --- Set up group default properties ---
	ActionSpecial* as_defaults = NULL;
	if (group_defaults) as_defaults = new ActionSpecial(*group_defaults);
	else as_defaults = new ActionSpecial();
	as_defaults->parse(node);

	// --- Go through all child nodes ---
	for (unsigned a = 0; a < node->nChildren(); a++) {
		ParseTreeNode* child = (ParseTreeNode*)node->getChild(a);

		// Check for 'group'
		if (S_CMPNOCASE(child->getType(), "group"))
			readActionSpecials(child, as_defaults);

		// Action special
		else if (S_CMPNOCASE(child->getType(), "special")) {
			// Get special id as integer
			long special;
			child->getName().ToLong(&special);

			// Reset the action special (in case it's being redefined for whatever reason)
			action_specials[special].reset();

			// Apply group defaults
			action_specials[special].copy(as_defaults);
			action_specials[special].group = groupname;

			// Check for simple definition
			if (child->isLeaf())
				action_specials[special].name = child->getStringValue();
			else
				action_specials[special].parse(child);	// Extended definition
		}
	}

	delete as_defaults;
}

void GameConfiguration::readThingTypes(ParseTreeNode* node, ThingType* group_defaults) {
	// --- Determine current 'group' ---
	ParseTreeNode* group = node;
	string groupname = "";
	while (true) {
		if (group->getName() == "thing_types" || !group)
			break;
		else {
			// Add current node name to group path
			groupname.Prepend(group->getName() + "/");
			group = (ParseTreeNode*)group->getParent();
		}
	}
	if (groupname.EndsWith("/"))
		groupname.RemoveLast();	// Remove last '/'


	// --- Set up group default properties ---
	ParseTreeNode* child = NULL;
	ThingType* tt_defaults = NULL;
	if (group_defaults) tt_defaults = new ThingType(*group_defaults);
	else tt_defaults = new ThingType();
	tt_defaults->parse(node);


	// --- Go through all child nodes ---
	for (unsigned a = 0; a < node->nChildren(); a++) {
		child = (ParseTreeNode*)node->getChild(a);

		// Check for 'group'
		if (S_CMPNOCASE(child->getType(), "group"))
			readThingTypes(child, tt_defaults);

		// Thing type
		else if (S_CMPNOCASE(child->getType(), "thing")) {
			// Get thing type as integer
			long type;
			child->getName().ToLong(&type);

			// Create thing type object if needed
			if (!thing_types[type].type)
				thing_types[type].type = new ThingType();

			// Reset the thing type (in case it's being redefined for whatever reason)
			thing_types[type].type->reset();

			// Apply group defaults
			thing_types[type].type->copy(tt_defaults);
			thing_types[type].type->group = groupname;

			// Check for simple definition
			if (child->isLeaf())
				thing_types[type].type->name = child->getStringValue();
			else
				thing_types[type].type->parse(child);	// Extended definition
		}
	}

	delete tt_defaults;
}

void GameConfiguration::readUDMFProperties(ParseTreeNode* block, vector<UDMFProperty>& plist) {
	// Read block properties
	for (unsigned a = 0; a < block->nChildren(); a++) {
		ParseTreeNode* group = (ParseTreeNode*)block->getChild(a);

		// Group definition
		if (S_CMPNOCASE(group->getType(), "group")) {
			string groupname = group->getName();

			// Go through the group
			for (unsigned b = 0; b < group->nChildren(); b++) {
				ParseTreeNode* def = (ParseTreeNode*)group->getChild(b);

				if (S_CMPNOCASE(def->getType(), "property")) {
					// Check if the property already exists
					bool exists = false;
					for (unsigned p = 0; p < plist.size(); p++) {
						if (plist[p].getProperty() == def->getName()) {
							plist[p].parse(group, groupname);	// Parse group defaults
							plist[p].parse(def, groupname);		// Parse property definition
							exists = true;
							break;
						}
					}

					// Create property if it didn't exist
					if (!exists) {
						plist.push_back(UDMFProperty());
						plist.back().parse(group, groupname);
						plist.back().parse(def, groupname);
					}
				}
			}
		}
	}
}

bool GameConfiguration::readConfiguration(string& cfg, string source) {
	// Testing
	MemChunk mc;
	mc.write(CHR(cfg), cfg.Length());
	mc.exportFile("gctest.txt");

	// Clear current configuration
	name = "Invalid Configuration";
	action_specials.clear();
	thing_types.clear();
	map_names.clear();
	flags_thing.clear();
	flags_line.clear();

	// Parse the full configuration
	Parser parser;
	parser.parseText(cfg, source);

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

		// Valid map names
		else if (S_CMPNOCASE(node->getName(), "map_names")) {
			for (unsigned n = 0; n < node->nValues(); n++)
				map_names.push_back(node->getStringValue(n));
		}

		// Map format
		else if (S_CMPNOCASE(node->getName(), "map_format")) {
			if (S_CMPNOCASE(node->getStringValue(), "doom"))
				map_format = MAP_DOOM;
			else if (S_CMPNOCASE(node->getStringValue(), "hexen"))
				map_format = MAP_HEXEN;
			else if (S_CMPNOCASE(node->getStringValue(), "doom64"))
				map_format = MAP_DOOM64;
			else if (S_CMPNOCASE(node->getStringValue(), "udmf"))
				map_format = MAP_UDMF;
			else {
				wxLogMessage("Warning: Unknown/unsupported map format \"%s\", defaulting to doom format", CHR(node->getStringValue()));
				map_format = MAP_DOOM;
			}
		}

		// Game filter
		else if (S_CMPNOCASE(node->getName(), "game_filter"))
			game_filter = node->getStringValue();

		// Boom extensions
		else if (S_CMPNOCASE(node->getName(), "boom"))
			boom = node->getBoolValue();

		// UDMF namespace
		else if (S_CMPNOCASE(node->getName(), "udmf_namespace"))
			udmf_namespace = node->getStringValue();
	}

	// Go through all other config sections
	ParseTreeNode* node = NULL;
	for (unsigned a = 0; a < base->nChildren(); a++) {
		node = (ParseTreeNode*)base->getChild(a);

		// Skip game section
		if (node == node_game)
			continue;

		// Action specials section
		if (S_CMPNOCASE(node->getName(), "action_specials"))
			readActionSpecials(node);

		// Thing types section
		else if (S_CMPNOCASE(node->getName(), "thing_types"))
			readThingTypes(node);

		// Line flags section
		else if (S_CMPNOCASE(node->getName(), "line_flags")) {
			for (unsigned c = 0; c < node->nChildren(); c++) {
				ParseTreeNode* value = (ParseTreeNode*)node->getChild(c);

				// Check for 'flag' type
				if (!(S_CMPNOCASE(value->getType(), "flag")))
					continue;

				long flag_val;
				value->getName().ToLong(&flag_val);

				// Check if the flag value already exists
				bool exists = false;
				for (unsigned f = 0; f < flags_line.size(); f++) {
					if (flags_line[f].flag == flag_val) {
						exists = true;
						flags_line[f].name = value->getStringValue();
						break;
					}
				}

				// Add flag otherwise
				if (!exists)
					flags_line.push_back(flag_t(flag_val, value->getStringValue()));
			}
		}

		// Line triggers section
		else if (S_CMPNOCASE(node->getName(), "line_triggers")) {
			for (unsigned c = 0; c < node->nChildren(); c++) {
				ParseTreeNode* value = (ParseTreeNode*)node->getChild(c);

				// Check for 'trigger' type
				if (!(S_CMPNOCASE(value->getType(), "trigger")))
					continue;

				long flag_val;
				value->getName().ToLong(&flag_val);

				// Check if the flag value already exists
				bool exists = false;
				for (unsigned f = 0; f < triggers_line.size(); f++) {
					if (triggers_line[f].flag == flag_val) {
						exists = true;
						triggers_line[f].name = value->getStringValue();
						break;
					}
				}

				// Add flag otherwise
				if (!exists)
					triggers_line.push_back(flag_t(flag_val, value->getStringValue()));
			}
		}

		// Thing flags section
		else if (S_CMPNOCASE(node->getName(), "thing_flags")) {
			for (unsigned c = 0; c < node->nChildren(); c++) {
				ParseTreeNode* value = (ParseTreeNode*)node->getChild(c);

				// Check for 'flag' type
				if (!(S_CMPNOCASE(value->getType(), "flag")))
					continue;

				long flag_val;
				value->getName().ToLong(&flag_val);

				// Check if the flag value already exists
				bool exists = false;
				for (unsigned f = 0; f < flags_thing.size(); f++) {
					if (flags_thing[f].flag == flag_val) {
						exists = true;
						flags_thing[f].name = value->getStringValue();
						break;
					}
				}

				// Add flag otherwise
				if (!exists)
					flags_thing.push_back(flag_t(flag_val, value->getStringValue()));
			}
		}

		// Sector types section
		else if (S_CMPNOCASE(node->getName(), "sector_types")) {
			for (unsigned c = 0; c < node->nChildren(); c++) {
				ParseTreeNode* value = (ParseTreeNode*)node->getChild(c);

				// Check for 'type'
				if (!(S_CMPNOCASE(value->getType(), "type")))
					continue;

				long type_val;
				value->getName().ToLong(&type_val);

				// Check if the sector type already exists
				bool exists = false;
				for (unsigned t = 0; t < sector_types.size(); t++) {
					if (sector_types[t].type == type_val) {
						exists = true;
						sector_types[t].name = value->getStringValue();
						break;
					}
				}

				// Add type otherwise
				if (!exists)
					sector_types.push_back(sectype_t(type_val, value->getStringValue()));
			}
		}

		// UDMF properties section
		else if (S_CMPNOCASE(node->getName(), "udmf_properties")) {
			// Parse vertex block properties (if any)
			ParseTreeNode* block = (ParseTreeNode*)node->getChild("vertex");
			if (block) readUDMFProperties(block, udmf_vertex_props);

			// Parse linedef block properties (if any)
			block = (ParseTreeNode*)node->getChild("linedef");
			if (block) readUDMFProperties(block, udmf_linedef_props);

			// Parse sidedef block properties (if any)
			block = (ParseTreeNode*)node->getChild("sidedef");
			if (block) readUDMFProperties(block, udmf_sidedef_props);

			// Parse sector block properties (if any)
			block = (ParseTreeNode*)node->getChild("sector");
			if (block) readUDMFProperties(block, udmf_sector_props);

			// Parse thing block properties (if any)
			block = (ParseTreeNode*)node->getChild("thing");
			if (block) readUDMFProperties(block, udmf_thing_props);
		}

		// Unknown/unexpected section
		else
			wxLogMessage("Warning: Unexpected game configuration section \"%s\", skipping", CHR(node->getName()));
	}

	wxLogMessage("Read game configuration \"%s\"", CHR(this->name));
	return true;
}

bool GameConfiguration::open(string filename) {
	// Build configuration string from file (process #includes, etc)
	string cfg;
	buildConfig(filename, cfg);

	return readConfiguration(cfg, filename);
}

bool GameConfiguration::open(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Build configuration string from entry (process #includes, etc)
	string cfg;
	buildConfig(entry, cfg);

	return readConfiguration(cfg, entry->getName());
}

bool GameConfiguration::openConfig(string name) {
	// Check for file in user config directory
	string fn = appPath("games/", DIR_USER) + name + ".cfg";
	if (wxFileExists(fn)) {
		if (open(fn)) {
			game_configuration = name;
			return true;
		}
		else
			return false;
	}

	// No user config exists, check slade.pk3
	string epath = S_FMT("config/games/%s.cfg", CHR(name));

	Archive* slade_pk3 = theArchiveManager->programResourceArchive();
	ArchiveEntry* entry = slade_pk3->entryAtPath(epath);
	if (open(entry)) {
		game_configuration = name;
		return true;
	}

	// Not found anywhere
	return false;
}

string GameConfiguration::actionSpecialName(int special) {
	// Check special id is valid
	if (special < 0)
		return "Unknown";
	else if (special == 0)
		return "None";

	return action_specials[special].getName();
}

ThingType* GameConfiguration::thingType(unsigned type) {
	tt_t& ttype = thing_types[type];
	if (ttype.type)
		return ttype.type;
	else
		return &ttype_unknown;
}

string GameConfiguration::thingFlagsString(int flags) {
	// Check against all flags
	string ret = "";
	for (unsigned a = 0; a < flags_thing.size(); a++) {
		if (flags & flags_thing[a].flag) {
			// Add flag name to string
			ret += flags_thing[a].name;
			ret += ", ";
		}
	}

	// Remove ending ', ' if needed
	if (ret.Length() > 0)
		ret.RemoveLast(2);

	return ret;
}

string GameConfiguration::lineFlagsString(MapLine* line) {
	if (!line)
		return "";

	// Get raw flags
	int flags = line->prop("flags");
	// TODO: UDMF flags

	// Check against all flags
	string ret = "";
	for (unsigned a = 0; a < flags_line.size(); a++) {
		if (flags & flags_line[a].flag) {
			// Add flag name to string
			ret += flags_line[a].name;
			ret += ", ";
		}
	}

	// Remove ending ', ' if needed
	if (ret.Length() > 0)
		ret.RemoveLast(2);

	return ret;
}

string GameConfiguration::spacTriggerString(MapLine* line) {
	if (!line)
		return "";

	// Get raw flags
	int flags = line->prop("flags");
	// TODO: UDMF flags

	// Get SPAC trigger value from flags
	int trigger = ((flags & 0x1c00) >> 10);

	// Find matching trigger name
	for (unsigned a = 0; a < triggers_line.size(); a++) {
		if (triggers_line[a].flag == trigger)
			return triggers_line[a].name;
	}

	// Unknown trigger
	return "Unknown";
}

string GameConfiguration::sectorTypeName(int type) {
	// Check for zero type
	if (type == 0)
		return "Normal";

	// Deal with generalised flags
	vector<string> gen_flags;
	if (boom) {
		// Check what the map format is (the flag bits differ between doom/hexen format)
		if (map_format == MAP_DOOM && type >= 32) {
			// Damage flags
			if (type & 96)
				gen_flags.push_back("20% Damage");
			else if (type & 32)
				gen_flags.push_back("5% Damage");
			else if (type & 64)
				gen_flags.push_back("10% Damage");

			// Secret
			if (type & 128)
				gen_flags.push_back("Secret");

			// Friction
			if (type & 256)
				gen_flags.push_back("Friction Enabled");

			// Pushers/Pullers
			if (type & 512)
				gen_flags.push_back("Pushers/Pullers Enabled");

			// Remove flag bits from type value
			type = type & 31;
		}
		else if (type >= 256) {
			// Damage flags
			if (type & 768)
				gen_flags.push_back("20% Damage");
			else if (type & 256)
				gen_flags.push_back("5% Damage");
			else if (type & 512)
				gen_flags.push_back("10% Damage");

			// Secret
			if (type & 1024)
				gen_flags.push_back("Secret");

			// Friction
			if (type & 2056)
				gen_flags.push_back("Friction Enabled");

			// Pushers/Pullers
			if (type & 4096)
				gen_flags.push_back("Pushers/Pullers Enabled");

			// Remove flag bits from type value
			type = type & 255;
		}
	}

	// Check if the type only has generalised flags
	if (type == 0 && gen_flags.size() > 0) {
		// Just return flags in this case
		string name = gen_flags[0];
		for (unsigned a = 1; a < gen_flags.size(); a++)
			name += S_FMT(" + %s", CHR(gen_flags[a]));

		return name;
	}

	// Go through sector types
	string name = "Unknown";
	for (unsigned a = 0; a < sector_types.size(); a++) {
		if (sector_types[a].type == type) {
			name = sector_types[a].name;
			break;
		}
	}

	// Add generalised flags to type name
	for (unsigned a = 0; a < gen_flags.size(); a++)
		name += S_FMT(" + %s", CHR(gen_flags[a]));

	return name;
}




void GameConfiguration::dumpActionSpecials() {
	ASpecialMap::iterator i = action_specials.begin();

	while (i != action_specials.end()) {
		wxLogMessage("Action special %d = %s", i->first, CHR(i->second.stringDesc()));
		i++;
	}
}

void GameConfiguration::dumpThingTypes() {
	ThingTypeMap::iterator i = thing_types.begin();

	while (i != thing_types.end()) {
		wxLogMessage("Thing type %d = %s", i->first, CHR(i->second.type->stringDesc()));
		i++;
	}
}

void GameConfiguration::dumpValidMapNames() {
	wxLogMessage("Valid Map Names:");
	for (unsigned a = 0; a < map_names.size(); a++)
		wxLogMessage(map_names[a]);
}

void GameConfiguration::dumpUDMFProperties() {
	// Vertex
	for (unsigned a = 0; a < udmf_vertex_props.size(); a++)
		wxLogMessage(udmf_vertex_props[a].getStringRep());

	// Line
	for (unsigned a = 0; a < udmf_linedef_props.size(); a++)
		wxLogMessage(udmf_linedef_props[a].getStringRep());

	// Side
	for (unsigned a = 0; a < udmf_sidedef_props.size(); a++)
		wxLogMessage(udmf_sidedef_props[a].getStringRep());

	// Sector
	for (unsigned a = 0; a < udmf_sector_props.size(); a++)
		wxLogMessage(udmf_sector_props[a].getStringRep());

	// Thing
	for (unsigned a = 0; a < udmf_thing_props.size(); a++)
		wxLogMessage(udmf_thing_props[a].getStringRep());
}


CONSOLE_COMMAND(testgc, 0) {
	string game = "doom1";

	if (args.size() > 0)
		game = args[0];

	theGameConfiguration->openConfig(game);
}

CONSOLE_COMMAND(dumpactionspecials, 0) {
	theGameConfiguration->dumpActionSpecials();
}

CONSOLE_COMMAND(dumpudmfprops, 0) {
	theGameConfiguration->dumpUDMFProperties();
}
