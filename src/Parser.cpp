
#include "Main.h"
#include "Parser.h"

Parser::Parser() {
	// Create parse tree root node
	pt_root = new ParseTreeNode();
}

Parser::~Parser() {
	// Clean up
	delete pt_root;
}

bool Parser::parseText(MemChunk& mc) {
	Tokenizer tz;

	// Open the given text data
	if (!tz.openMem((const char*)mc.getData(), mc.getSize())) {
		wxLogMessage(_T("Unable to open text data for parsing"));
		return false;
	}
}
