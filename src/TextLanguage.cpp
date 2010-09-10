
#include "Main.h"
#include "TextLanguage.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "ArchiveManager.h"

vector<TextLanguage*>	text_languages;

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

void TextLanguage::addFunction(string name, vector<string> args) {
	// Init new function
	tl_function_t func;

	// Set name
	func.name = name;

	// Set args
	for (size_t a = 0; a < args.size(); a++)
		func.args.push_back(args[a]);

	// Add to list
	functions.push_back(func);
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
	for (size_t a = 0; a < functions.size(); a++)
		ret += functions[a].name + " ";

	return ret;
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
		text_languages.push_back(lang);

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

					// Get name
					tl_function_t fn;
					fn.name = child_func->getName();

					// Get args
					for (unsigned v = 0; v < child_func->nValues(); v++)
						fn.args.push_back(child_func->getStringValue(v));

					// Add function
					lang->addFunction(fn.name, fn.args);
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
