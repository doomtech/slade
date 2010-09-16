
#include "Main.h"
#include "TextLanguage.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "ArchiveManager.h"

vector<TextLanguage*>	text_languages;


TLFunction::TLFunction(string name) {
	this->name = name;
}

TLFunction::~TLFunction() {
}

string TLFunction::generateCallTipString(int arg_set) {
	// Check requested arg set exists
	if (arg_set < 0 || (unsigned)arg_set >= arg_sets.size())
		return "<invalid argset index>";

	string calltip;

	// Add extra buttons for selection if there is more than one arg set
	if (arg_sets.size() > 1)
		calltip += s_fmt("\001 %d of %d \002 ", arg_set+1, arg_sets.size());

	// Generate scintilla-format calltip string
	calltip += name + "(";
	calltip += arg_sets[arg_set];
	calltip += ")";

	return calltip;
}

point2_t TLFunction::getArgTextExtent(int arg, int arg_set) {
	point2_t extent(-1, -1);

	// Check requested arg set exists
	if (arg_set < 0 || (unsigned)arg_set >= arg_sets.size())
		return extent;

	// Get start position of args list
	int start_pos = name.Length() + 1;
	if (arg_sets.size() > 1) {
		string temp = s_fmt("\001 %d of %d \002 ", arg_set+1, arg_sets.size());
		start_pos += temp.Length();
	}

	// Check arg
	string args = arg_sets[arg_set];
	if (arg < 0) {
		extent.set(start_pos, start_pos + args.Length());
		return extent;
	}

	// Go through arg set string
	int current_arg = 0;
	extent.x = start_pos;
	extent.y = start_pos + args.Length();
	for (unsigned a = 0; a < args.Length(); a++) {
		// Check for ,
		if (args.at(a) == ',') {
			// ',' found, so increment current arg
			current_arg++;

			// If we're at the start of the arg we want
			if (current_arg == arg)
				extent.x = start_pos + a+1;

			// If we've reached the end of the arg we want
			if (current_arg > arg) {
				extent.y = start_pos + a;
				break;
			}
		}
	}

	return extent;
}




TextLanguage::TextLanguage(string id) {
	// Init variables
	this->id = id;

	// Add to languages list
	text_languages.push_back(this);
}

TextLanguage::~TextLanguage() {
	// Remove from languages list
	for (size_t a = 0; a < text_languages.size(); a++) {
		if (text_languages[a] == this)
			text_languages.erase(text_languages.begin() + a);
	}
}

void TextLanguage::addFunction(string name, string args) {
	// Check if the function exists
	TLFunction* func = getFunction(name);

	// If it doesn't, create it
	if (!func) {
		func = new TLFunction(name);
		functions.push_back(func);
	}

	// Add the arg set
	func->addArgSet(args);
}

string TextLanguage::getKeywordsList() {
	// Init return string
	string ret = "";

	// Add each keyword to return string (separated by spaces)
	for (size_t a = 0; a < keywords.size(); a++)
		ret += keywords[a] + " ";

	return ret;
}

string TextLanguage::getConstantsList() {
	// Init return string
	string ret = "";

	// Add each constant to return string (separated by spaces)
	for (size_t a = 0; a < constants.size(); a++)
		ret += constants[a] + " ";

	return ret;
}

string TextLanguage::getFunctionsList() {
	// Init return string
	string ret = "";

	// Add each function name to return string (separated by spaces)
	for (unsigned a = 0; a < functions.size(); a++)
		ret += functions[a]->getName() + " ";

	return ret;
}

TLFunction* TextLanguage::getFunction(string name) {
	// Find function matching [name]
	for (unsigned a = 0; a < functions.size(); a++) {
		if (functions[a]->getName() == name)
			return functions[a];
	}

	// Not found
	return NULL;
}

bool TextLanguage::readLanguageDefinition(MemChunk& mc) {
	Tokenizer tz;

	// Open the given text data
	if (!tz.openMem((const char*)mc.getData(), mc.getSize())) {
		wxLogMessage("Unable to open file");
		return false;
	}

	// Parse the definition text
	ParseTreeNode root;
	if (!root.parse(tz))
		return false;

	// Get parsed data
	for (unsigned a = 0; a < root.nChildren(); a++) {
		ParseTreeNode* node = (ParseTreeNode*)root.getChild(a);

		// Create language
		TextLanguage* lang = new TextLanguage(node->getName());

		// Parse language info
		for (unsigned c = 0; c < node->nChildren(); c++) {
			ParseTreeNode* child = (ParseTreeNode*)node->getChild(c);

			// Language name
			if (s_cmpnocase(child->getName(), "name"))
				lang->setName(child->getStringValue());

			// Comment begin
			else if (s_cmpnocase(child->getName(), "comment_begin"))
				lang->setCommentBegin(child->getStringValue());

			// Comment end
			else if (s_cmpnocase(child->getName(), "comment_end"))
				lang->setCommentEnd(child->getStringValue());

			// Line comment
			else if (s_cmpnocase(child->getName(), "line_comment"))
				lang->setLineComment(child->getStringValue());

			// Preprocessor
			else if (s_cmpnocase(child->getName(), "preprocessor"))
				lang->setPreprocessor(child->getStringValue());

			// Case sensitive
			else if (s_cmpnocase(child->getName(), "case_sensitive"))
				lang->setCaseSensitive(child->getBoolValue());

			// Keywords
			else if (s_cmpnocase(child->getName(), "keywords")) {
				// Go through values
				for (unsigned v = 0; v < child->nValues(); v++) {
					string val = child->getStringValue(v);

					// Check for '$override'
					if (s_cmpnocase(val, "$override")) {
						// Clear any inherited keywords
						lang->clearKeywords();
					}

					// Not a special symbol, add as keyword
					else
						lang->addKeyword(val);
				}
			}

			// Constants
			else if (s_cmpnocase(child->getName(), "constants")) {
				// Go through values
				for (unsigned v = 0; v < child->nValues(); v++) {
					string val = child->getStringValue(v);

					// Check for '$override'
					if (s_cmpnocase(val, "$override")) {
						// Clear any inherited constants
						lang->clearConstants();
					}

					// Not a special symbol, add as constant
					else
						lang->addConstant(val);
				}
			}

			// Functions
			else if (s_cmpnocase(child->getName(), "functions")) {
				// Go through children (functions)
				for (unsigned f = 0; f < child->nChildren(); f++) {
					ParseTreeNode* child_func = (ParseTreeNode*)child->getChild(f);

					// Add function
					lang->addFunction(child_func->getName(), child_func->getStringValue(0));

					// Add args
					for (unsigned v = 1; v < child_func->nValues(); v++) {
						lang->addFunction(child_func->getName(), child_func->getStringValue(v));
					}
				}
			}
		}
	}

	return true;
}

bool TextLanguage::loadLanguages() {
	// Get slade resource archive
	Archive* res_archive = theArchiveManager->programResourceArchive();

	// Read language definitions from resource archive
	if (res_archive) {
		// Get 'config/languages' directlry
		ArchiveTreeNode* dir = res_archive->getDir("config/languages");

		if (dir) {
			// Read all entries in this dir
			for (unsigned a = 0; a < dir->numEntries(); a++)
				readLanguageDefinition(dir->getEntry(a)->getMCData());
		}
		else
			wxLogMessage("Warning: 'config/languages' not found in slade.pk3, no builtin text language definitions loaded");
	}

	return true;
}

TextLanguage* TextLanguage::getLanguage(string id) {
	// Find text language matching [id]
	for (unsigned a = 0; a < text_languages.size(); a++) {
		if (text_languages[a]->id == id)
			return text_languages[a];
	}

	// Not found
	return NULL;
}

TextLanguage* TextLanguage::getLanguage(unsigned index) {
	// Check index
	if (index >= text_languages.size())
		return NULL;

	return text_languages[index];
}

wxArrayString TextLanguage::getLanguageNames() {
	wxArrayString ret;

	for (unsigned a = 0; a < text_languages.size(); a++)
		ret.push_back(text_languages[a]->name);

	return ret;
}
