
#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

enum CommentTypes {
	CCOMMENTS = 1<<0,		// If true C comments are skipped (// and /* */)
	HCOMMENTS = 1<<1,		// If true hash comments are skipped (##)
	SCOMMENTS = 1<<2,		// If true shell comments are skipped (;)
};

class Tokenizer {
private:
	char*		current;	// Current position
	char*		start;		// Start of text
	uint32_t	size;		// Size of text
	uint32_t	position;	// Current position
	uint8_t		comments;	// See CommentTypes enum
	bool		debug;		// If true every getToken() is printed to the console
	string		special;	// A string defining the 'special characters'. These will always be parsed as separate tokens

public:
	Tokenizer(bool c_comments = true, bool h_comments = true, bool s_comments = false);
	~Tokenizer();

	void setSpecialCharacters(string special) { this->special = special; }

	void enableDebug(bool debug = true) { this->debug = debug; }

	bool openFile(string filename, uint32_t offset = 0, uint32_t length = 0);
	bool openString(string text, uint32_t offset = 0, uint32_t length = 0);
	bool openMem(const char* mem, uint32_t length);
	bool isWhitespace(char p);
	bool isSpecialCharacter(char p);
	bool incrementCurrent();
	void skipLineComment();
	void skipMultilineComment();
	string getToken();
	string peekToken();
	bool checkToken(string check);

	int	getInteger();
	float getFloat();
	double getDouble();
	bool getBool();
};

#endif //__TOKENIZER_H__
