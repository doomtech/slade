
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
#include <wx/dir.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
GameConfiguration* GameConfiguration::instance = NULL;
CVAR(String, game_configuration, "", CVAR_SAVE)


/*******************************************************************
 * GAMECONFIGURATION CLASS FUNCTIONS
 *******************************************************************/

GameConfiguration::GameConfiguration() {
	setDefaults();
}

GameConfiguration::~GameConfiguration() {
	// Clean up stuff
	ASpecialMap::iterator as = action_specials.begin();
	while (as != action_specials.end()) {
		if (as->second.special) delete as->second.special;
		as++;
	}

	ThingTypeMap::iterator tt = thing_types.begin();
	while (tt != thing_types.end()) {
		if (tt->second.type) delete tt->second.type;
		tt++;
	}
}

void GameConfiguration::setDefaults() {
	game_filter = "";
	udmf_namespace = "";
	ttype_unknown.icon = "unknown";
	ttype_unknown.shrink = true;
	any_map_name = false;
	mix_tex_flats = false;
	tx_textures = false;
	defaults_line.clear();
	defaults_side.clear();
	defaults_sector.clear();
	defaults_thing.clear();
}

string GameConfiguration::readConfigName(MemChunk& mc) {
	Tokenizer tz;
	tz.openMem(&mc, "gameconfig");

	// Parse text
	string token = tz.getToken();
	while (!token.IsEmpty()) {
		// Game section
		if (S_CMPNOCASE(token, "game")) {
			tz.getToken();	// Skip {

			token = tz.getToken();
			while (token != "}") {
				// Config name
				if (S_CMPNOCASE(token, "name")) {
					tz.getToken();	// Skip =
					return tz.getToken();
				}

				token = tz.getToken();
			}
		}

		token = tz.getToken();
	}

	// Name not found (invalid config?)
	return "";
}

void GameConfiguration::init() {
	// Load last configuration if any
	if (!string(game_configuration).IsEmpty())
		openConfig(game_configuration);

	// Add game configurations from user dir
	wxArrayString allfiles;
	wxDir::GetAllFiles(appPath("games", DIR_USER), &allfiles);
	for (unsigned a = 0; a < allfiles.size(); a++) {
		// Read config name
		MemChunk mc;
		mc.importFile(allfiles[a]);
		string name = readConfigName(mc);

		// Add to list if valid
		if (!name.IsEmpty()) {
			gconf_t gc;
			gc.filename = wxFileName(allfiles[a]).GetName();
			gc.title = name;
			game_configs.push_back(gc);
		}
	}

	// Add game configurations from program resource
	ArchiveTreeNode* dir = theArchiveManager->programResourceArchive()->getDir("config/games");
	if (dir) {
		for (unsigned a = 0; a < dir->numEntries(); a++) {
			// Check this game doesn't already exist
			bool exists = false;
			string filename = dir->getEntry(a)->getName(true);
			for (unsigned b = 0; b < game_configs.size(); b++) {
				if (game_configs[b].filename == filename) {
					exists = true;
					break;
				}
			}

			if (exists)
				continue;

			// Read config name
			string name = readConfigName(dir->getEntry(a)->getMCData());

			// Add to list if valid
			if (!name.IsEmpty()) {
				gconf_t gc;
				gc.filename = dir->getEntry(a)->getName(true);
				gc.title = name;
				game_configs.push_back(gc);
			}
		}
	}

	// Sort game configurations list by title
	std::sort(game_configs.begin(), game_configs.end());
	lastDefaultConfig = game_configs.size();
}

string GameConfiguration::configTitle(unsigned index) {
	if (index >= game_configs.size())
		return "";
	else
		return game_configs[index].title;
}

string GameConfiguration::configName(unsigned index) {
	if (index >= game_configs.size())
		return "";
	else
		return game_configs[index].filename;
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
 * in the entry text recursively. This will search in the resource
 * folder and archive as well as in the parent archive. The resulting 
 * 'expanded' text is written to [out]
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
			string inc_name = tz.getToken();
			string name = entry->getPath() + inc_name;

			// Get the entry
			bool done = false;
			ArchiveEntry* entry_inc = entry->getParent()->entryAtPath(name);
			if (entry_inc) {
				buildConfig(entry_inc, out);
				done = true;
			}

			// Look in resource pack
			if (!done && theArchiveManager->programResourceArchive()) {
				name = "config/games/" + inc_name;
				entry_inc = theArchiveManager->programResourceArchive()->entryAtPath(name);
				if (entry_inc) {
					buildConfig(entry_inc, out);
					done = true;
				}
			}

			// Okay, we've exhausted all possibilities
			if (!done)
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

			// Create action special object if needed
			if (!action_specials[special].special) {
				action_specials[special].special = new ActionSpecial();
				action_specials[special].index = action_specials.size();
			}

			// Reset the action special (in case it's being redefined for whatever reason)
			action_specials[special].special->reset();

			// Apply group defaults
			action_specials[special].special->copy(as_defaults);
			action_specials[special].special->group = groupname;

			// Check for simple definition
			if (child->isLeaf())
				action_specials[special].special->name = child->getStringValue();
			else
				action_specials[special].special->parse(child);	// Extended definition
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
			if (!thing_types[type].type) {
				thing_types[type].type = new ThingType();
				thing_types[type].index = thing_types.size();
			}

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

void GameConfiguration::readUDMFProperties(ParseTreeNode* block, UDMFPropMap& plist) {
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
					// Create property if needed
					if (!plist[def->getName()].property)
						plist[def->getName()].property = new UDMFProperty();

					// Parse group defaults
					plist[def->getName()].property->parse(group, groupname);

					// Parse definition
					plist[def->getName()].property->parse(def, groupname);

					// Set index
					plist[def->getName()].index = plist.size();
				}
			}
		}
	}
}

bool GameConfiguration::readConfiguration(string& cfg, string source) {
	// Testing
	/*MemChunk mc;
	mc.write(CHR(cfg), cfg.Length());
	mc.exportFile("gctest.txt");*/

	// Clear current configuration
	setDefaults();
	name = "Invalid Configuration";
	action_specials.clear();
	thing_types.clear();
	map_names.clear();
	flags_thing.clear();
	flags_line.clear();
	udmf_vertex_props.clear();
	udmf_linedef_props.clear();
	udmf_sidedef_props.clear();
	udmf_sector_props.clear();
	udmf_thing_props.clear();

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

		// Allow any map name
		else if (S_CMPNOCASE(node->getName(), "map_name_any"))
			any_map_name = node->getBoolValue();

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

		// Mixed Textures and Flats
		else if (S_CMPNOCASE(node->getName(), "mix_tex_flats"))
			mix_tex_flats = node->getBoolValue();

		// TX_/'textures' namespace enabled
		else if (S_CMPNOCASE(node->getName(), "tx_textures"))
			tx_textures = node->getBoolValue();
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

		// Defaults section
		else if (S_CMPNOCASE(node->getName(), "defaults")) {
			// Go through defaults blocks
			for (unsigned b = 0; b < node->nChildren(); b++) {
				ParseTreeNode* block = (ParseTreeNode*)node->getChild(b);

				// Linedef defaults
				if (S_CMPNOCASE(block->getName(), "linedef")) {
					for (unsigned c = 0; c < block->nChildren(); c++) {
						ParseTreeNode* def = (ParseTreeNode*)block->getChild(c);
						defaults_line[def->getName()] = def->getValue();
					}
				}

				// Sidedef defaults
				else if (S_CMPNOCASE(block->getName(), "sidedef")) {
					for (unsigned c = 0; c < block->nChildren(); c++) {
						ParseTreeNode* def = (ParseTreeNode*)block->getChild(c);
						defaults_side[def->getName()] = def->getValue();
					}
				}

				// Sector defaults
				else if (S_CMPNOCASE(block->getName(), "sector")) {
					for (unsigned c = 0; c < block->nChildren(); c++) {
						ParseTreeNode* def = (ParseTreeNode*)block->getChild(c);
						defaults_sector[def->getName()] = def->getValue();
					}
				}

				// Thing defaults
				else if (S_CMPNOCASE(block->getName(), "thing")) {
					for (unsigned c = 0; c < block->nChildren(); c++) {
						ParseTreeNode* def = (ParseTreeNode*)block->getChild(c);
						defaults_thing[def->getName()] = def->getValue();
					}
				}

				else
					wxLogMessage("Unknown defaults block \"%s\"", CHR(block->getName()));
			}
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

bool GameConfiguration::openEmbeddedConfig(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Build configuration string from entry (process #includes, etc)
	string cfg;
	buildConfig(entry, cfg);

	if (readConfiguration(cfg, entry->getName())) {
		string name = readConfigName(entry->getMCData());

		// Add to list if valid
		if (!name.IsEmpty()) {
			gconf_t gc;
			gc.filename = entry->getParent()->getFilename();
			gc.title = name;
			game_configs.push_back(gc);
		}

		return true;
	}
	return false;
}

bool GameConfiguration::removeEmbeddedConfig(string name) {
	if (game_configs.size() > lastDefaultConfig) {
		vector<gconf_t>::iterator it, stop = game_configs.begin() + lastDefaultConfig;
		for (it = game_configs.end() - 1; it >= stop; --it) {
			if (!name.Cmp(it->filename)) {
				game_configs.erase(it);
				return true;
			}
		}
	}
	return false;
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

	// No user config exists, check embedded archives
	ArchiveEntry* entry = NULL;
	Archive* archive = theArchiveManager->getArchive(name);
	if (archive) {
		Archive::search_options_t search;
		search.match_name = "sladecfg";
		entry = archive->findLast(search);
	}

	// Still not found, check slade.pk3
	if (!entry) {
		string epath = S_FMT("config/games/%s.cfg", CHR(name));

		archive = theArchiveManager->programResourceArchive();
		entry = archive->entryAtPath(epath);
	}

	if (open(entry)) {
		game_configuration = name;
		return true;
	}

	// Not found anywhere
	return false;
}

ActionSpecial* GameConfiguration::actionSpecial(unsigned id) {
	as_t& as = action_specials[id];
	if (as.special)
		return as.special;
	else
		return &as_unknown;
}

string GameConfiguration::actionSpecialName(int special) {
	// Check special id is valid
	if (special < 0)
		return "Unknown";
	else if (special == 0)
		return "None";

	if (action_specials[special].special)
		return action_specials[special].special->getName();
	else
		return "Unknown";
}

vector<as_t> GameConfiguration::allActionSpecials() {
	vector<as_t> ret;

	// Build list
	ASpecialMap::iterator i = action_specials.begin();
	while (i != action_specials.end()) {
		if (i->second.special) {
			as_t as(i->second.special);
			as.number = i->first;
			ret.push_back(as);
		}

		i++;
	}

	return ret;
}

ThingType* GameConfiguration::thingType(unsigned type) {
	tt_t& ttype = thing_types[type];
	if (ttype.type)
		return ttype.type;
	else
		return &ttype_unknown;
}

vector<tt_t> GameConfiguration::allThingTypes() {
	vector<tt_t> ret;

	ThingTypeMap::iterator i = thing_types.begin();
	while (i != thing_types.end()) {
		if (i->second.type) {
			tt_t tt(i->second.type);
			tt.number = i->first;
			ret.push_back(tt);
		}

		i++;
	}

	return ret;
}

string GameConfiguration::thingFlag(unsigned index) {
	// Check index
	if (index >= flags_thing.size())
		return "";

	return flags_thing[index].name;
}

bool GameConfiguration::thingFlagSet(unsigned index, MapThing* thing) {
	// Check index
	if (index >= flags_thing.size())
		return false;

	// Check if flag is set
	int flags = thing->prop("flags");
	if (flags & flags_thing[index].flag)
		return true;
	else
		return false;
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

void GameConfiguration::setThingFlag(unsigned index, MapThing* thing, bool set) {
	// Check index
	if (index >= flags_thing.size())
		return;

	// Determine new flags value
	int flags = thing->prop("flags");
	if (set)
		flags |= flags_thing[index].flag;
	else
		flags = (flags & ~flags_thing[index].flag);

	// Update thing flags
	thing->setIntProperty("flags", flags);
}

string GameConfiguration::lineFlag(unsigned index) {
	// Check index
	if (index >= flags_line.size())
		return "";

	return flags_line[index].name;
}

bool GameConfiguration::lineFlagSet(unsigned index, MapLine* line) {
	// Check index
	if (index >= flags_line.size())
		return false;

	// Check if flag is set
	int flags = line->prop("flags");
	if (flags & flags_line[index].flag)
		return true;
	else
		return false;
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

void GameConfiguration::setLineFlag(unsigned index, MapLine* line, bool set) {
	// Check index
	if (index >= flags_line.size())
		return;

	// Determine new flags value
	int flags = line->prop("flags");
	if (set)
		flags |= flags_line[index].flag;
	else
		flags = (flags & ~flags_line[index].flag);

	// Update line flags
	line->setIntProperty("flags", flags);
}

void GameConfiguration::setLineBasicFlag(string flag, MapLine* line, bool set) {
	// If UDMF, just set the bool value
	if (map_format == MAP_UDMF) {
		line->setBoolProperty(flag, set);
		return;
	}

	// Get current flags
	int flags = line->intProperty("flags");
	int fval = 0;

	// Impassible
	if (flag == "blocking")
		fval = 1;

	// Two Sided
	else if (flag == "twosided")
		fval = 4;

	// Upper unpegged
	else if (flag == "dontpegtop")
		fval = 8;

	// Lower unpegged
	else if (flag == "dontpegbottom")
		fval = 16;

	// Set/unset flag
	if (set)
		line->setIntProperty("flags", flags|fval);
	else
		line->setIntProperty("flags", flags & ~fval);
}

string GameConfiguration::spacTriggerString(MapLine* line) {
	if (!line)
		return "";

	// Hexen format
	if (map_format == MAP_HEXEN) {
		// Get raw flags
		int flags = line->prop("flags");

		// Get SPAC trigger value from flags
		int trigger = ((flags & 0x1c00) >> 10);

		// Find matching trigger name
		for (unsigned a = 0; a < triggers_line.size(); a++) {
			if (triggers_line[a].flag == trigger)
				return triggers_line[a].name;
		}
	}

	// UDMF format
	else if (map_format == MAP_UDMF) {
		// Go through all line UDMF properties
		string trigger = "";
		vector<udmfp_t> props = allUDMFProperties(MOBJ_LINE);
		for (unsigned a = 0; a < props.size(); a++) {
			// Check for trigger property
			if (props[a].property->isTrigger()) {
				// Check if the line has this property
				if (line->boolProperty(props[a].property->getProperty())) {
					// Add to trigger line
					if (!trigger.IsEmpty())
						trigger += ", ";
					trigger += props[a].property->getName();
				}
			}
		}

		// Check if there was any trigger
		if (trigger.IsEmpty())
			return "None";
		else
			return trigger;
	}

	// Unknown trigger
	return "Unknown";
}

wxArrayString GameConfiguration::allSpacTriggers() {
	wxArrayString ret;

	for (unsigned a = 0; a < triggers_line.size(); a++)
		ret.Add(triggers_line[a].name);

	return ret;
}

void GameConfiguration::setLineSpacTrigger(unsigned index, MapLine* line) {
	// Check index
	if (index >= triggers_line.size())
		return;

	// Get trigger value
	int trigger = triggers_line[index].flag;

	// Get raw line flags
	int flags = line->prop("flags");

	// Apply trigger to flags
	trigger = trigger << 10;
	flags &= ~0x1c00;
	flags |= trigger;

	// Update line flags
	line->setIntProperty("flags", flags);
}

UDMFProperty* GameConfiguration::getUDMFProperty(string name, int type) {
	if (type == MOBJ_VERTEX)
		return udmf_vertex_props[name].property;
	else if (type == MOBJ_LINE)
		return udmf_linedef_props[name].property;
	else if (type == MOBJ_SIDE)
		return udmf_sidedef_props[name].property;
	else if (type == MOBJ_SECTOR)
		return udmf_sector_props[name].property;
	else if (type == MOBJ_THING)
		return udmf_thing_props[name].property;
	else
		return NULL;
}

vector<udmfp_t> GameConfiguration::allUDMFProperties(int type) {
	vector<udmfp_t> ret;

	// Build list depending on type
	UDMFPropMap* map = NULL;
	if (type == MOBJ_VERTEX)
		map = &udmf_vertex_props;
	else if (type == MOBJ_LINE)
		map = &udmf_linedef_props;
	else if (type == MOBJ_SIDE)
		map = &udmf_sidedef_props;
	else if (type == MOBJ_SECTOR)
		map = &udmf_sector_props;
	else if (type == MOBJ_THING)
		map = &udmf_thing_props;
	else
		return ret;

	UDMFPropMap::iterator i = map->begin();
	while (i != map->end()) {
		if (i->second.property) {
			udmfp_t up(i->second.property);
			up.index = i->second.index;
			ret.push_back(up);
		}

		i++;
	}

	return ret;
}

void GameConfiguration::cleanObjectUDMFProps(MapObject* object) {
	// Get UDMF properties list for type
	UDMFPropMap* map = NULL;
	int type = object->getObjType();
	if (type == MOBJ_VERTEX)
		map = &udmf_vertex_props;
	else if (type == MOBJ_LINE)
		map = &udmf_linedef_props;
	else if (type == MOBJ_SIDE)
		map = &udmf_sidedef_props;
	else if (type == MOBJ_SECTOR)
		map = &udmf_sector_props;
	else if (type == MOBJ_THING)
		map = &udmf_thing_props;
	else
		return;

	// Go through properties
	UDMFPropMap::iterator i = map->begin();
	while (i != map->end()) {
		if (!i->second.property) {
			i++;
			continue;
		}

		// Check if the object even has this property
		if (!object->hasProp(i->first)) {
			i++;
			continue;
		}

		// Remove the property from the object if it is the default value
		//Property& def = i->second.property->getDefaultValue();
		if (i->second.property->getDefaultValue().getType() == PROP_BOOL) {
			if (i->second.property->getDefaultValue().getBoolValue() == object->boolProperty(i->first))
				object->props().removeProperty(i->first);
		}
		else if (i->second.property->getDefaultValue().getType() == PROP_INT) {
			if (i->second.property->getDefaultValue().getIntValue() == object->intProperty(i->first))
				object->props().removeProperty(i->first);
		}
		else if (i->second.property->getDefaultValue().getType() == PROP_FLOAT) {
			if (i->second.property->getDefaultValue().getFloatValue() == object->floatProperty(i->first))
				object->props().removeProperty(i->first);
		}
		else if (i->second.property->getDefaultValue().getType() == PROP_STRING) {
			if (i->second.property->getDefaultValue().getStringValue() == object->stringProperty(i->first))
				object->props().removeProperty(i->first);
		}

		i++;
	}
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
			if ((type & 96) == 96)
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
			if ((type & 768) == 768)
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

string GameConfiguration::getDefaultString(int type, string property) {
	switch (type) {
	case MOBJ_LINE:
		return defaults_line[property].getStringValue(); break;
	case MOBJ_SIDE:
		return defaults_side[property].getStringValue(); break;
	case MOBJ_SECTOR:
		return defaults_sector[property].getStringValue(); break;
	case MOBJ_THING:
		return defaults_thing[property].getStringValue(); break;
	default:
		return "";
	}
}

int GameConfiguration::getDefaultInt(int type, string property) {
	switch (type) {
	case MOBJ_LINE:
		return defaults_line[property].getIntValue(); break;
	case MOBJ_SIDE:
		return defaults_side[property].getIntValue(); break;
	case MOBJ_SECTOR:
		return defaults_sector[property].getIntValue(); break;
	case MOBJ_THING:
		return defaults_thing[property].getIntValue(); break;
	default:
		return 0;
	}
}

double GameConfiguration::getDefaultFloat(int type, string property) {
	switch (type) {
	case MOBJ_LINE:
		return defaults_line[property].getFloatValue(); break;
	case MOBJ_SIDE:
		return defaults_side[property].getFloatValue(); break;
	case MOBJ_SECTOR:
		return defaults_sector[property].getFloatValue(); break;
	case MOBJ_THING:
		return defaults_thing[property].getFloatValue(); break;
	default:
		return 0;
	}
}

bool GameConfiguration::getDefaultBool(int type, string property) {
	switch (type) {
	case MOBJ_LINE:
		return defaults_line[property].getBoolValue(); break;
	case MOBJ_SIDE:
		return defaults_side[property].getBoolValue(); break;
	case MOBJ_SECTOR:
		return defaults_sector[property].getBoolValue(); break;
	case MOBJ_THING:
		return defaults_thing[property].getBoolValue(); break;
	default:
		return false;
	}
}

void GameConfiguration::applyDefaults(MapObject* object) {
	// Get all defaults for the object type
	vector<string> prop_names;
	vector<Property> prop_vals;

	// Line defaults
	if (object->getObjType() == MOBJ_LINE) {
		defaults_line.allProperties(prop_vals);
		defaults_line.allPropertyNames(prop_names);
	}

	// Side defaults
	else if (object->getObjType() == MOBJ_SIDE) {
		defaults_side.allProperties(prop_vals);
		defaults_side.allPropertyNames(prop_names);
	}

	// Sector defaults
	else if (object->getObjType() == MOBJ_SECTOR) {
		defaults_sector.allProperties(prop_vals);
		defaults_sector.allPropertyNames(prop_names);
	}

	// Thing defaults
	else if (object->getObjType() == MOBJ_THING) {
		defaults_thing.allProperties(prop_vals);
		defaults_thing.allPropertyNames(prop_names);
	}

	// Apply defaults to object
	for (unsigned a = 0; a < prop_names.size(); a++) {
		if (prop_vals[a].getType() == PROP_BOOL)
			object->setBoolProperty(prop_names[a], prop_vals[a].getBoolValue());
		else if (prop_vals[a].getType() == PROP_INT)
			object->setIntProperty(prop_names[a], prop_vals[a].getIntValue());
		else if (prop_vals[a].getType() == PROP_FLOAT)
			object->setFloatProperty(prop_names[a], prop_vals[a].getFloatValue());
		else if (prop_vals[a].getType() == PROP_STRING)
			object->setStringProperty(prop_names[a], prop_vals[a].getStringValue());
	}
}




void GameConfiguration::dumpActionSpecials() {
	ASpecialMap::iterator i = action_specials.begin();

	while (i != action_specials.end()) {
		wxLogMessage("Action special %d = %s", i->first, CHR(i->second.special->stringDesc()));
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
	wxLogMessage("\nVertex properties:");
	UDMFPropMap::iterator i = udmf_vertex_props.begin();
	while (i != udmf_vertex_props.end()) {
		wxLogMessage(i->second.property->getStringRep());
		i++;
	}

	// Line
	wxLogMessage("\nLine properties:");
	i = udmf_linedef_props.begin();
	while (i != udmf_linedef_props.end()) {
		wxLogMessage(i->second.property->getStringRep());
		i++;
	}

	// Side
	wxLogMessage("\nSide properties:");
	i = udmf_sidedef_props.begin();
	while (i != udmf_sidedef_props.end()) {
		wxLogMessage(i->second.property->getStringRep());
		i++;
	}

	// Sector
	wxLogMessage("\nSector properties:");
	i = udmf_sector_props.begin();
	while (i != udmf_sector_props.end()) {
		wxLogMessage(i->second.property->getStringRep());
		i++;
	}

	// Thing
	wxLogMessage("\nThing properties:");
	i = udmf_thing_props.begin();
	while (i != udmf_thing_props.end()) {
		wxLogMessage(i->second.property->getStringRep());
		i++;
	}
}


CONSOLE_COMMAND(testgc, 0) {
	string game = "doomu";

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
