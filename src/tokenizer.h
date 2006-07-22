
#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

// Tokenizer: Reads and parses text
class Tokenizer
{
private:
	char*	current;	// Current position
	char*	start;		// Start of text
	DWORD	size;		// Size of text
	DWORD	position;	// Current position

public:
	Tokenizer() { current = NULL; start = NULL; size = 0; }
	~Tokenizer() { if (start) free(start); }

	bool open_file(string filename, DWORD offset = 0, DWORD length = 0);
	bool open_string(string text, DWORD offset = 0, DWORD length = 0);
	bool open_mem(char* mem, DWORD length);
	bool is_whitespace(char p);
	bool increment_current();
	void skip_line_comment();
	void skip_multiline_comment();
	string get_token();
	string peek_token();
	bool check_token(string check);

	int	get_integer();
	float get_float();
	bool get_bool();
};

#endif
