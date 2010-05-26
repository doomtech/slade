
#include "Main.h"
#include "TextLanguage.h"
#include "Tokenizer.h"

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

	// Get first token
	string token = tz.getToken();

	// If it's an entry_types definition, read it
	if (!token.Cmp("language")) {
		// Read language id
		string id = tz.getToken();

		// Create new language
		TextLanguage* language = new TextLanguage(id);

		// Check for ":" (inheritance)
		if (tz.peekToken() == ":") {
			string parent = tz.getToken();

			// Do inheritance stuff
		}

		// Skip opening brace
		if (!tz.checkToken("{"))
			return false;

		// Read language definition
		token = tz.getToken();
		while (token != "}") {
			// Language name
			if (s_cmpnocase(token, "name")) {
				tz.getToken();						// Skip =
				language->setName(tz.getToken());	// Set name
				tz.getToken();						// Skip ;
			}

			// Line comment
			if (s_cmpnocase(token, "line_comment")) {
				tz.getToken();								// Skip =
				language->setLineComment(tz.getToken());	// Set line comment
				tz.getToken();								// Skip ;
			}

			// Comment begin
			if (s_cmpnocase(token, "comment_begin")) {
				tz.getToken();								// Skip =
				language->setCommentBegin(tz.getToken());	// Set comment begin
				tz.getToken();								// Skip ;
			}

			// Comment end
			if (s_cmpnocase(token, "comment_end")) {
				tz.getToken();							// Skip =
				language->setCommentEnd(tz.getToken());	// Set comment end
				tz.getToken();							// Skip ;
			}

			// Preprocessor
			if (s_cmpnocase(token, "preprocessor")) {
				tz.getToken();								// Skip =
				language->setPreprocessor(tz.getToken());	// Set preprocessor
				tz.getToken();								// Skip ;
			}

			// Keywords list
			if (s_cmpnocase(token, "keywords")) {
				tz.getToken();		// Skip {
				token = tz.getToken();
				while (!(s_cmp(token, "}"))) {
					language->addKeyword(token);	// Add keyword
					token = tz.getToken();
				}
			}

			// Constants list
			if (s_cmpnocase(token, "constants")) {
				tz.getToken();		// Skip {
				token = tz.getToken();
				while (!(s_cmp(token, "}"))) {
					language->addConstant(token);	// Add constant
					token = tz.getToken();
				}
			}

			// Functions list
			if (s_cmpnocase(token, "functions")) {
				tz.getToken();		// Skip {
				token = tz.getToken();
				while (!(s_cmp(token, "}"))) {
					// Read function name
					string func_name = token;
					token = tz.getToken();

					// If next token is =, read parameter list
					if (s_cmp(token, "=")) {
						vector<string> args;
						tz.getToken();	// Skip {
						token = tz.getToken();
						while (!(s_cmp(token, "}"))) {
							args.push_back(token);
							token = tz.getToken();
						}
						tz.getToken();	// Skip }

						// Add new function
						language->addFunction(func_name, args);
					}

					// If next token is ;, function has no parameters
					else if (s_cmp(token, ";")) {
						// Add new function
						language->addFunction(func_name, vector<string>());
					}

					token = tz.getToken();
				}
			}

			token = tz.getToken();
		}
	}

	return false;
}
