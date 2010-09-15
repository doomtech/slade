
#ifndef __TEXTLANGUAGE_H__
#define __TEXTLANGUAGE_H__

class TLFunction {
private:
	string			name;
	vector<string>	arg_sets;

public:
	TLFunction(string name = "");
	~TLFunction();

	string	getName() { return name; }

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
	vector<string>		keywords;
	vector<string>		constants;
	vector<TLFunction*>	functions;

public:
	TextLanguage(string id);
	~TextLanguage();

	void	setName(string name) { this->name = name; }
	void	setLineComment(string token) { line_comment = token; }
	void	setCommentBegin(string token) { comment_begin = token; }
	void	setCommentEnd(string token) { comment_end = token; }
	void	setPreprocessor(string token) { preprocessor = token; }
	void	setCaseSensitive(bool cs) { case_sensitive = cs; }
	void	addKeyword(string keyword) { keywords.push_back(keyword); }
	void	addConstant(string constant) { constants.push_back(constant); }
	void	addFunction(string name, string args);

	string	getKeywordsList();
	string	getConstantsList();
	string	getFunctionsList();

	TLFunction*	getFunction(string name);

	void	clearKeywords() { keywords.clear(); }
	void	clearConstants() { constants.clear(); }
	void	clearFunctions() { functions.clear(); }

	// Static functions
	static bool				readLanguageDefinition(MemChunk& mc);
	static bool				loadLanguages();
	static TextLanguage*	getLanguage(string id);
};

#endif//__TEXTLANGUAGE_H__
