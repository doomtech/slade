
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Parser.cpp
 * Description: Parser/Parse tree classes. Parses formatted text
 *              data and generates a tree of ParseTreeNodes containing
 *              the parsed data. Currently supports SLADE/DB/UDMF
 *              formatting style.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "Parser.h"
#include "Console.h"
#include <wx/regex.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
wxRegEx re_integer("[+-]?[1-9]+[0-9]* | 0[0-9]+ | 0x[0-9A-Fa-f]+");
wxRegEx re_float("[+-]?[0-9]+'.'[0-9]*([eE][+-]?[0-9]+)?");


/*******************************************************************
 * PARSETREENODE CLASS FUNCTIONS
 *******************************************************************/

/* ParseTreeNode::ParseTreeNode
 * ParseTreeNode class constructor
 *******************************************************************/
ParseTreeNode::ParseTreeNode(ParseTreeNode* parent) : STreeNode(parent) {
	allowDup(true);
}

/* ParseTreeNode::~ParseTreeNode
 * ParseTreeNode class destructor
 *******************************************************************/
ParseTreeNode::~ParseTreeNode() {
}

Property ParseTreeNode::getValue(unsigned index) {
	// Check index
	if (index >= values.size())
		return Property(false);

	return values[index];
}

/* ParseTreeNode::getStringValue
 * Returns the node's value at [index] as a string. If [index] is
 * out of range, returns an empty string
 *******************************************************************/
string ParseTreeNode::getStringValue(unsigned index) {
	// Check index
	if (index >= values.size())
		return wxEmptyString;

	return values[index].getStringValue();
}

/* ParseTreeNode::getStringValue
 * Returns the node's value at [index] as an integer. If [index] is
 * out of range, returns 0
 *******************************************************************/
int ParseTreeNode::getIntValue(unsigned index) {
	// Check index
	if (index >= values.size())
		return 0;

	return (int)values[index];
}

/* ParseTreeNode::getStringValue
 * Returns the node's value at [index] as a boolean. If [index] is
 * out of range, returns false
 *******************************************************************/
bool ParseTreeNode::getBoolValue(unsigned index) {
	// Check index
	if (index >= values.size())
		return false;

	return (bool)values[index];
}

/* ParseTreeNode::getStringValue
 * Returns the node's value at [index] as a float. If [index] is
 * out of range, returns 0.0f
 *******************************************************************/
double ParseTreeNode::getFloatValue(unsigned index) {
	// Check index
	if (index >= values.size())
		return 0.0f;

	return (double)values[index];
}

/* ParseTreeNode::parse
 * Parses formatted text data. Current valid formatting is:
 * (type) child = value;
 * (type) child = value1, value2, ...;
 * (type) child = { value1, value2, ... }
 * (type) child { grandchild = value; etc... }
 * (type) child : inherited { ... }
 * All values are read as strings, but can be retrieved as string,
 * int, bool or float.
 *******************************************************************/
bool ParseTreeNode::parse(Tokenizer& tz) {
	// Get first token
	string token = tz.getToken();

	// Keep parsing until final } is reached (or end of file)
	while (!(S_CMP(token, "}")) && !token.IsEmpty()) {
		// If it's a special character (ie not a valid name), parsing fails
		if (tz.isSpecialCharacter(token.at(0))) {
			wxLogMessage("Parsing error: Unexpected special character '%s' in %s (line %d)", CHR(token), CHR(tz.getName()), tz.lineNo());
			return false;
		}

		// So we have either a node or property name
		string name = token;

		// Check next token to determine what we're doing
		string next = tz.peekToken();

		// Check for type+name pair
		string type = "";
		if (next != "=" && next != "{" && next != ";" && next != ":") {
			type = name;
			name = tz.getToken();
			next = tz.peekToken();
		}

		// Assignment
		if (S_CMP(next, "=")) {
			// Skip =
			tz.getToken();

			// Create item node
			ParseTreeNode* child = (ParseTreeNode*)addChild(name);
			child->type = type;

			// Check type of assignment list
			token = tz.getToken();
			string list_end = ";";
			if (token == "{") {
				list_end = "}";
				token = tz.getToken();
			}

			// Parse until ; or }
			while (1) {
				// Check for list end
				if (S_CMP(token, list_end) && !tz.quotedString())
					break;

				// Setup value
				Property value;
				
				// Detect value type
				if (tz.quotedString())					// Quoted string
					value = token;
				else if (S_CMPNOCASE(token, "true"))	// Boolean (true)
					value = true;
				else if (S_CMPNOCASE(token, "false"))	// Boolean (false)
					value = false;
				else if (re_integer.Matches(token)) {	// Integer
					long val;
					token.ToLong(&val);
					value = (int)val;
				}
				else if (re_float.Matches(token)) {		// Floating point
					double val;
					token.ToDouble(&val);
					value = (double)val;
				}
				else									// Unknown, just treat as string
					value = token;
				
				// Add value
				child->values.push_back(value);

				// Check for ,
				if (S_CMP(tz.peekToken(), ","))
					tz.getToken();	// Skip it
				else if (!(S_CMP(tz.peekToken(), list_end))) {
					string token = tz.getToken();
					string name = tz.getName();
					wxLogMessage("Parsing error: Expected \",\" or \"%s\", got \"%s\" in %s (line %d)", CHR(list_end), CHR(token), CHR(name), tz.lineNo());
					return false;
				}

				token = tz.getToken();
			}
		}

		// Child node
		else if (S_CMP(next, "{")) {
			// Add child node
			ParseTreeNode* child = (ParseTreeNode*)addChild(name);
			child->type = type;

			// Skip {
			tz.getToken();

			// Parse child node
			if (!child->parse(tz))
				return false;
		}

		// Child node (with no values/children)
		else if (S_CMP(next, ";")) {
			// Add child node
			ParseTreeNode* child = (ParseTreeNode*)addChild(name);
			child->type = type;

			// Skip ;
			tz.getToken();
		}

		// Child node + inheritance
		else if (S_CMP(next, ":")) {
			// Skip :
			tz.getToken();

			// Read inherited name
			string inherit = tz.getToken();

			// Check for opening brace
			if (tz.checkToken("{")) {
				// Add child node
				ParseTreeNode* child = (ParseTreeNode*)addChild(name);
				child->type = type;

				// Set its inheritance
				child->inherit = inherit;

				// Parse child node
				if (!child->parse(tz))
					return false;
			}
		}

		// Unexpected token
		else {
			wxLogMessage("Parsing error: \"%s\" unexpected in %s (line %d)", CHR(next), CHR(tz.getName()), tz.lineNo());
			return false;
		}

		// Continue parsing
		token = tz.getToken();
	}

	return true;
}


/*******************************************************************
 * PARSER CLASS FUNCTIONS
 *******************************************************************/

/* Parser::Parser
 * Parser class constructor
 *******************************************************************/
Parser::Parser() {
	// Create parse tree root node
	pt_root = new ParseTreeNode();
}

/* Parser::~Parser
 * Parser class destructor
 *******************************************************************/
Parser::~Parser() {
	// Clean up
	delete pt_root;
}

/* Parser::parseText
 * Parses the given text data to build a tree of ParseTreeNodes.
 * Example:
 * 	base {
 * 		child1 = value1;
 * 		child2 = value2, value3, value4;
 * 		child3 {
 * 			grandchild1 = value5;
 * 			grandchild2 = value6;
 * 		}
 * 		child4 {
 * 			grandchild3 = value7, value8;
 * 		}
 * 	}
 *
 * will generate this tree (represented in xml-like format, node names within <>):
 * 	<root>
 * 		<base>
 * 			<child1>value1</child1>
 * 			<child2>value2, value3, value4</child2>
 * 			<child3>
 * 				<grandchild1>value5</grandchild1>
 * 				<grandchild2>value6</grandchild2>
 * 			</child3>
 * 			<child4>
 * 				<grandchild3>value7, value8</grandchild3>
 * 			</child4>
 * 		</base>
 * 	</root>
 *******************************************************************/
bool Parser::parseText(MemChunk& mc, string source) {
	Tokenizer tz;

	// Open the given text data
	if (!tz.openMem(&mc, source)) {
		wxLogMessage("Unable to open text data for parsing");
		return false;
	}

	// Do parsing
	return pt_root->parse(tz);
}
bool Parser::parseText(string& text, string source) {
	// Open the given text data
	Tokenizer tz;
	if (!tz.openString(text, 0, 0, source)) {
		wxLogMessage("Unable to open text data for parsing");
		return false;
	}

	// Do parsing
	return pt_root->parse(tz);
}



// Console test command
/*
CONSOLE_COMMAND (testparse, 0) {
	string test = "root { item1 = value1; item2 = value1, value2; child1 { item1 = value1, value2, value3; item2 = value4; } child2 : child1 { item1 = value1; child3 { item1 = value1, value2; } } }";
	Parser tp;
	MemChunk mc((const uint8_t*)CHR(test), test.Length());
	tp.parseText(mc);
}
*/
