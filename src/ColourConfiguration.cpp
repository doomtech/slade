
#include "Main.h"
#include "ColourConfiguration.h"
#include "Parser.h"
#include "ArchiveManager.h"
#include "Console.h"
#include <wx/hashmap.h>

WX_DECLARE_STRING_HASH_MAP(cc_col_t, ColourHashMap);
ColourHashMap	cc_colours;

rgba_t ColourConfiguration::getColour(string name) {
	cc_col_t& col = cc_colours[name];
	if (col.exists)
		return col.colour;
	else
		return COL_WHITE;
}

void ColourConfiguration::setColour(string name, int red, int green, int blue, int alpha, int blend) {
	cc_col_t& col = cc_colours[name];
	if (red >= 0)
		col.colour.r = red;
	if (green >= 0)
		col.colour.g = green;
	if (blue >= 0)
		col.colour.b = blue;
	if (alpha >= 0)
		col.colour.a = alpha;
	if (blend >= 0)
		col.colour.blend = blend;
}

bool ColourConfiguration::readConfiguration(MemChunk& mc) {
	// Parse text
	Parser parser;
	parser.parseText(mc);

	// Get 'colours' block
	ParseTreeNode* colours = (ParseTreeNode*)parser.parseTreeRoot()->getChild("colours");
	if (!colours)
		return false;

	// Read all colour definitions
	for (unsigned a = 0; a < colours->nChildren(); a++) {
		ParseTreeNode* def = (ParseTreeNode*)colours->getChild(a);

		// Read properties
		for (unsigned b = 0; b < def->nChildren(); b++) {
			ParseTreeNode* prop = (ParseTreeNode*)def->getChild(b);
			cc_col_t& col = cc_colours[def->getName()];
			col.exists = true;

			// Colour name
			if (prop->getName() == "name")
				col.name = prop->getStringValue();

			// Colour group (for config ui)
			else if (prop->getName() == "group")
				col.group = prop->getStringValue();

			// Colour
			else if (prop->getName() == "rgb")
				col.colour.set(prop->getIntValue(0), prop->getIntValue(1), prop->getIntValue(2));

			// Alpha
			else if (prop->getName() == "alpha")
				col.colour.a = prop->getIntValue();

			// Additive
			else if (prop->getName() == "additive") {
				if (prop->getBoolValue())
					col.colour.blend = 1;
				else
					col.colour.blend = 0;
			}

			else
				wxLogMessage("Warning: unknown colour definition property \"%s\"", CHR(prop->getName()));
		}
	}

	return true;
}

bool ColourConfiguration::writeConfigration(MemChunk& mc) {
	return false;
}

bool ColourConfiguration::init() {
	Archive* pres = theArchiveManager->programResourceArchive();
	ArchiveEntry* entry_default_cc = pres->entryAtPath("config/colours/default.txt");

	if (!entry_default_cc)
		return false;
	else
		return readConfiguration(entry_default_cc->getMCData());
}

void ColourConfiguration::getColourNames(vector<string>& list) {
	ColourHashMap::iterator i = cc_colours.begin();
	while (i != cc_colours.end()) {
		list.push_back(i->first);
		i++;
	}
}




CONSOLE_COMMAND(ccfg, 1) {
	// Check for 'list'
	if (S_CMPNOCASE(args[0], "list")) {
		// Get (sorted) list of colour names
		vector<string> list;
		ColourConfiguration::getColourNames(list);
		sort(list.begin(), list.end());

		// Dump list to console
		theConsole->logMessage(S_FMT("%d Colours:", list.size()));
		for (unsigned a = 0; a < list.size(); a++)
			theConsole->logMessage(list[a]);
	}
	else {
		// Check for enough args to set the colour
		if (args.size() >= 4) {
			// Read RGB
			long red, green, blue;
			args[1].ToLong(&red);
			args[2].ToLong(&green);
			args[3].ToLong(&blue);

			// Read alpha (if specified)
			long alpha = -1;
			if (args.size() >= 5)
				args[4].ToLong(&alpha);

			// Read blend (if specified)
			long blend = -1;
			if (args.size() >= 6)
				args[5].ToLong(&blend);

			// Set colour
			ColourConfiguration::setColour(args[0], red, green, blue, alpha, blend);
		}

		// Print colour
		rgba_t col = ColourConfiguration::getColour(args[0]);
		theConsole->logMessage(S_FMT("Colour \"%s\" = %d %d %d %d %d", CHR(args[0]), col.r, col.g, col.b, col.a, col.blend));
	}
}

CONSOLE_COMMAND(load_ccfg, 1) {
	// Determine full path string to entry
	string path = S_FMT("config/colours/%s.txt", CHR(args[0]));
	Archive* pres = theArchiveManager->programResourceArchive();
	ArchiveEntry* entry_cc = pres->entryAtPath(path);

	if (entry_cc)
		ColourConfiguration::readConfiguration(entry_cc->getMCData());
}
