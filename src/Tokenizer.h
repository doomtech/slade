
#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

class Tokenizer {
private:
	char*	current;	// Current position
	char*	start;		// Start of text
	DWORD	size;		// Size of text
	DWORD	position;	// Current position
	bool	ccomments;	// If true C comments are skipped (// and /* */)
	bool	hcomments;	// If true hash comments are skipped (##)

public:
	Tokenizer(bool c_comments = true, bool h_comments = true);
	~Tokenizer();

	bool openFile(string filename, DWORD offset = 0, DWORD length = 0);
	bool openString(string text, DWORD offset = 0, DWORD length = 0);
	bool openMem(char* mem, DWORD length);
	bool isWhitespace(char p);
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
