
#include "Main.h"
#include "ColourConfiguration.h"
#include "Parser.h"
#include "ArchiveManager.h"
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
