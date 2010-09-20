
#ifndef __TEXTLANGUAGE_H__
#define __TEXTLANGUAGE_H__

class TLFunction {
private:
	string			name;
	vector<string>	arg_sets;

public:
	TLFunction(string name = "");
	~TLFunction();

	string		getName() { return name; }
	string		getArgSet(unsigned index);
	unsigned	nArgSets() { return arg_sets.size(); }

	void	setName(string name) { this->name = name; }
	void	addArgSet(string args) { arg_sets.push_back(args); }

	string		generateCallTipString(int arg_set = 0);
	point2_t	getArgTextExtent(int arg, int arg_set = 0);
};

class TextLanguage {
private:
	string				id;
	string				name;
	string				line_comment;
	string				comment_begin;
	string				comment_end;
	string				preprocessor;
	bool				case_sensitive;

	// Keywords
	vector<string>		keywords;
	bool				k_upper;
	bool				k_lower;
	bool				k_caps;

	// Constants
	vector<string>		constants;
	bool				c_upper;
	bool				c_lower;
	bool				c_caps;

	// Functions
	vector<TLFunction*>	functions;
	bool				f_upper;
	bool				f_lower;
	bool				f_caps;

public:
	TextLanguage(string id);
	~TextLanguage();

	void	copyTo(TextLanguage* copy);

	void	setName(string name) { this->name = name; }
	void	setLineComment(string token) { line_comment = token; }
	void	setCommentBegin(string token) { comment_begin = token; }
	void	setCommentEnd(string token) { comment_end = token; }
	void	setPreprocessor(string token) { preprocessor = token; }
	void	setCaseSensitive(bool cs) { case_sensitive = cs; }
	void	addKeyword(string keyword);
	void	addConstant(string constant);
	void	addFunction(string name, string args);

	string	getKeywordsList();
	string	getConstantsList();
	string	getFunctionsList();
	string	getAutocompletionList();

	TLFunction*	getFunction(string name);

	void	clearKeywords() { keywords.clear(); }
	void	clearConstants() { constants.clear(); }
	void	clearFunctions() { functions.clear(); }

	// Static functions
	static bool				readLanguageDefinition(MemChunk& mc);
	static bool				loadLanguages();
	static TextLanguage*	getLanguage(string id);
	static TextLanguage*	getLanguage(unsigned index);
	static TextLanguage*	getLanguageByName(string name);
	static wxArrayString	getLanguageNames();
};

#endif//__TEXTLANGUAGE_H__
