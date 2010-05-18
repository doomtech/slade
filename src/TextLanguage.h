
#ifndef __TEXTLANGUAGE_H__
#define __TEXTLANGUAGE_H__

struct tl_function_t {
	string			name;
	vector<string>	args;
};

class TextLanguage {
private:
	string					id;
	string					name;
	string					line_comment;
	string					comment_begin;
	string					comment_end;
	string					preprocessor;
	vector<string>			keywords;
	vector<string>			constants;
	vector<tl_function_t>	functions;

public:
	TextLanguage(string id);
	~TextLanguage();

	void	setName(string name) { this->name = name; }
	void	setLineComment(string token) { line_comment = token; }
	void	setCommentBegin(string token) { comment_begin = token; }
	void	setCommentEnd(string token) { comment_end = token; }
	void	setPreprocessor(string token) { preprocessor = token; }
	void	addKeyword(string keyword) { keywords.push_back(keyword); }
	void	addConstant(string constant) { constants.push_back(constant); }
	void	addFunction(string name, vector<string> args);

	string	getKeywordsList();
	string	getConstantsList();
	string	getFunctionsList();

	// Static functions
	static bool	readLanguageDefinition(MemChunk& mc);
};

#endif//__TEXTLANGUAGE_H__