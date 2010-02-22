
#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

class Tokenizer {
private:
	char*		current;	// Current position
	char*		start;		// Start of text
	uint32_t	size;		// Size of text
	uint32_t	position;	// Current position
	bool		ccomments;	// If true C comments are skipped (// and /* */)
	bool		hcomments;	// If true hash comments are skipped (##)
	bool		debug;		// If true every getToken() is printed to the console

public:
	Tokenizer(bool c_comments = true, bool h_comments = true);
	~Tokenizer();

	void enableDebug(bool debug = true) { this->debug = debug; }

	bool openFile(string filename, uint32_t offset = 0, uint32_t length = 0);
	bool openString(string text, uint32_t offset = 0, uint32_t length = 0);
	bool openMem(char* mem, uint32_t length);
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
