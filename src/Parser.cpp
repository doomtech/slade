
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


/*******************************************************************
 * PARSETREENODE CLASS FUNCTIONS
 *******************************************************************/

/* ParseTreeNode::ParseTreeNode
 * ParseTreeNode class constructor
 *******************************************************************/
ParseTreeNode::ParseTreeNode(ParseTreeNode* parent) : STreeNode(parent) {
}

/* ParseTreeNode::~ParseTreeNode
 * ParseTreeNode class destructor
 *******************************************************************/
ParseTreeNode::~ParseTreeNode() {
}

/* ParseTreeNode::getStringValue
 * Returns the node's value at [index] as a string. If [index] is
 * out of range, returns an empty string
 *******************************************************************/
string ParseTreeNode::getStringValue(unsigned index) {
	// Check index
	if (index >= values.size())
		return wxEmptyString;

	return values[index];
}

/* ParseTreeNode::getStringValue
 * Returns the node's value at [index] as an integer. If [index] is
 * out of range, returns 0
 *******************************************************************/
int ParseTreeNode::getIntValue(unsigned index) {
	// Check index
	if (index >= values.size())
		return 0;

	long val;
	values[index].ToLong(&val);
	return val;
}

/* ParseTreeNode::getStringValue
 * Returns the node's value at [index] as a boolean. If [index] is
 * out of range, returns false
 *******************************************************************/
bool ParseTreeNode::getBoolValue(unsigned index) {
	// Check index
	if (index >= values.size())
		return false;

	if (s_cmpnocase(values[index], _T("false")) ||
		s_cmpnocase(values[index], _T("0")) ||
		s_cmpnocase(values[index], _T("no")))
		return false;
	else
		return true;
}

/* ParseTreeNode::getStringValue
 * Returns the node's value at [index] as a float. If [index] is
 * out of range, returns 0.0f
 *******************************************************************/
float ParseTreeNode::getFloatValue(unsigned index) {
	// Check index
	if (index >= values.size())
		return 0.0f;

	double val;
	values[index].ToDouble(&val);
	return (float)val;
}

/* ParseTreeNode::parse
 * Parses formatted text data. Current valid formatting is:
 * child = value;
 * child = value1, value2, ...;
 * child { grandchild = value; etc... }
 * child : inherited { ... }
 * All values are read as strings, but can be retrieved as string,
 * int, bool or float.
 *******************************************************************/
// TODO: Specify line number of errors
bool ParseTreeNode::parse(Tokenizer& tz) {
	// Get first token
	string token = tz.getToken();

	// Keep parsing until final } is reached (or end of file)
	while (!(s_cmp(token, _T("}"))) && !token.IsEmpty()) {
		// If it's a special character (ie not a valid name), parsing fails
		if (tz.isSpecialCharacter(token.at(0))) {
			wxLogMessage(_T("Parsing error: Unexpected special character"));
			return false;
		}

		// So we have either a node or property name
		string name = token;

		// Check next token to determine what we're doing
		string next = tz.peekToken();

		// Assignment
		if (s_cmp(next, _T("="))) {
			// Skip =
			tz.getToken();

			// Create item node
			ParseTreeNode* child = (ParseTreeNode*)addChild(name);
			//wxLogMessage(_T("Item %s"), chr(name));

			// Parse until ;
			token = tz.getToken();
			while (!(s_cmp(token, _T(";")))) {
				// Add value
				child->values.push_back(token);
				//wxLogMessage(_T("Value %s"), chr(token));

				// Check for ,
				if (s_cmp(tz.peekToken(), _T(",")))
					tz.getToken();	// Skip it
				else if (!(s_cmp(tz.peekToken(), _T(";")))) {
					wxLogMessage(_T("Parsing error: Expected \",\" or \";\", got \"%s\""), chr(tz.getToken()));
					return false;
				}

				token = tz.getToken();
			}
		}

		// Child node
		else if (s_cmp(next, _T("{"))) {
			// Add child node
			ParseTreeNode* child = (ParseTreeNode*)addChild(name);
			//wxLogMessage(_T("Child %s"), chr(name));

			// Skip {
			tz.getToken();

			// Parse child node
			if (!child->parse(tz))
				return false;
		}

		// Child node + inheritance
		else if (s_cmp(next, _T(":"))) {
			// Skip :
			tz.getToken();

			// Read inherited name
			string inherit = tz.getToken();

			// Check for opening brace
			if (tz.checkToken(_T("{"))) {
				// Add child node
				ParseTreeNode* child = (ParseTreeNode*)addChild(name);

				// Set its inheritance
				child->inherit = inherit;
				//wxLogMessage(_T("Child %s inherits %s"), chr(name), chr(inherit));

				// Parse child node
				if (!child->parse(tz))
					return false;
			}
		}

		// Unexpected token
		else {
			wxLogMessage(_T("Parsing error: \"%s\" unexpected"), chr(next));
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
bool Parser::parseText(MemChunk& mc) {
	Tokenizer tz;

	// Open the given text data
	if (!tz.openMem((const char*)mc.getData(), mc.getSize())) {
		wxLogMessage(_T("Unable to open text data for parsing"));
		return false;
	}

	// Do parsing
	return pt_root->parse(tz);
}



// Console test command
void c_testparse(vector<string> args) {
	string test = _T("root { item1 = value1; item2 = value1, value2; child1 { item1 = value1, value2, value3; item2 = value4; } child2 : child1 { item1 = value1; child3 { item1 = value1, value2; } } }");
	Parser tp;
	MemChunk mc((const uint8_t*)chr(test), test.Length());
	tp.parseText(mc);
}
ConsoleCommand testparse(_T("testparse"), &c_testparse, 0);
