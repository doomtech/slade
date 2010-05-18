
#include "Main.h"
#include "Parser.h"
#include "Console.h"

ParseTreeNode::ParseTreeNode(ParseTreeNode* parent) : STreeNode(parent) {
}

ParseTreeNode::~ParseTreeNode() {
}

// TODO: Specify line number of errors
bool ParseTreeNode::parse(Tokenizer& tz) {
	// Get first token
	string token = tz.getToken();

	// Keep parsing until final } is reached (or end of file)
	while (!(s_cmp(token, _T("}"))) && !token.IsEmpty()) {
		// If it's a special character (ie not a valid name), parsing fails
		if (tz.isSpecialCharacter(token.at(0))) {
			wxLogMessage(_T("Parsing error: Unexpected special character"));
			
			// Attempt to continue parsing
			token = tz.getToken();
			continue;
		}

		// So we have either a node or property name
		string name = token;

		// Check next token to determine what we're doing
		string next = tz.peekToken();

		// Assignment
		if (s_cmp(next, _T("="))) {
			// Skip =
			tz.getToken();

			// Parse until ;
			token = tz.getToken();
			while (!(s_cmp(token, _T(";")))) {
				// Add value
				values.push_back(Property(token));

				// Check for ,
				if (s_cmp(tz.peekToken(), _T(",")))
					tz.getToken();	// Skip it

				token = tz.getToken();
			}
		}

		// Child node
		else if (s_cmp(next, _T("{"))) {
			// Add child node
			ParseTreeNode* child = (ParseTreeNode*)addChild(name);

			// Skip {
			tz.getToken();

			// Parse child node
			child->parse(tz);
		}

		// Flag
		//else if (s_cmp(next, _T(";"))) {
		//}

		// Unexpected token
		else {
			wxLogMessage(_T("Parsing error: Unexpected token"));

			// Attempt to continue parsing
			token = tz.getToken();
			continue;
		}

		// Continue parsing
		token = tz.getToken();
	}

	return true;
}


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

	// Do parsing
	pt_root->parse(tz);

	return true;
}



void c_testparse(vector<string> args) {
	string test = _T("root { item1 = value1; item2 = value1, value2; child1 { item1 = value1, value2, value3; item2 = value4; } child2 { item1 = value1; child3 { item1 = value1, value2; } } }");
	Parser tp;
	MemChunk mc((const uint8_t*)chr(test), test.Length());
	tp.parseText(mc);
}
ConsoleCommand testparse(_T("testparse"), &c_testparse, 0);
