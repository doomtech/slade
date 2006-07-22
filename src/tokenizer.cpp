// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-06               >>
// << ------------------------------------ >>
// << Tokenizer.cpp - Tokenizer class      >>
// << functions                            >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "tokenizer.h"

// Variables ----------------------------- >>

// External Variables -------------------- >>

// Tokenizer::open_file: Reads a portion of a file to the Tokenizer
// ------------------------------------------------------------- >>
bool Tokenizer::open_file(string filename, DWORD offset, DWORD length)
{
	FILE *fp = fopen(filename.c_str(), "rb");

	if (!fp)
		return false;

	if (length == 0)
	{
		while (1)
		{
			getc(fp);

			if (feof(fp))
				break;
			else
				length++;
		}
	}

	size = length;
	position = 0;
	start = current = (char *)malloc(size);

	fseek(fp, offset, SEEK_SET);
	fread(start, 1, size, fp);
	fclose(fp);

	return true;
}

// Tokenizer::open_string: Reads a portion of a string to the Tokenizer
// ----------------------------------------------------------------- >>
bool Tokenizer::open_string(string text, DWORD offset, DWORD length)
{
	if (length == 0)
		length = text.length();

	size = length;
	position = 0;
	start = current = (char *)malloc(size);
	memcpy(start, text.c_str(), size);

	return true;
}

// Tokenizer::open_mem: Reads a chunk of memory to the Tokenizer
// ---------------------------------------------------------- >>
bool Tokenizer::open_mem(char* mem, DWORD length)
{
	if (length <= 0)
		return false;

	size = length;
	position = 0;
	start = current = (char*)malloc(size);
	memcpy(start, mem, size);

	return true;
}

// Tokenizer::is_whitespace: Checks if a character is 'whitespace'
// ------------------------------------------------------------ >>
bool Tokenizer::is_whitespace(char p)
{
	if (p == '\n' || p == 13 || p == ' ' || p == '\t')
		return true;
	else
		return false;
}

// Tokenizer::increment_current: Increments the position pointer, returns false on end of block
// ----------------------------------------------------------------------------------------- >>
bool Tokenizer::increment_current()
{
	if (position >= size - 1)
	{
		position = size;
		return false;
	}
	else
	{
		position++;
		current++;
		return true;
	}
}

// Tokenizer::skip_line_comment: Skips a '//' comment
void Tokenizer::skip_line_comment()
{
	while (current[0] != '\n' && current[0] != 13)
	{
		if (!increment_current())
			return;
	}

	increment_current();
}

// Tokenizer::skip_multiline_comment: Skips a '/* ... */' comment
void Tokenizer::skip_multiline_comment()
{
	while (!(current[0] == '*' && current[1] == '/'))
	{
		if (!increment_current())
			return;
	}

	// Skip the ending '*/'
	increment_current();
	increment_current();
}

// Tokenizer::get_token: Gets the next 'token' from the text
// ------------------------------------------------------ >>
string Tokenizer::get_token()
{
	string ret_str = "";
	bool ready = false;

	// Increment pointer to next token
	while (!ready)
	{
		ready = true;

		// Increment pointer until non-whitespace is found
		while (is_whitespace(current[0]))
		{
			if (!increment_current())
			{
				ret_str = "!END";
				return ret_str;
			}
		}

		// Check if we have a line comment
		if (current[0] == '/' && current[1] == '/' && current[2] != '$')
		{
			skip_line_comment();	// Skip it
			ready = false;
		}

		// Also skip '##' comments
		if (current[0] == '#' && current[1] == '#')
		{
			skip_line_comment();	// Skip it
			ready = false;
		}

		// Check if we have a multiline comment
		if (current[0] == '/' && current[1] == '*')
		{
			skip_multiline_comment();	// Skip it
			ready = false;
		}

		// Check for end of text
		if (position == size)
		{
			ret_str = "!END";
			return ret_str;
		}
	}

	// Check for literal string (enclosed with "")
	if (current[0] == '\"')
	{
		// Skip opening "
		increment_current();

		// Read literal string (include whitespace)
		while (current[0] != '\"')
		{
			ret_str += current[0];

			if (!increment_current())
				return ret_str;
		}

		// Skip closing "
		increment_current();
	}
	else
	{
		// Read token (don't include whitespace
		while (!is_whitespace(current[0]))
		{
			ret_str += current[0];

			if (!increment_current())
				return ret_str;
		}
	}

	return ret_str;
}

// Tokenizer::peek_token: Returns the next token without actually moving past it
// -------------------------------------------------------------------------- >>
string Tokenizer::peek_token()
{
	string token;

	char* c = current;
	DWORD p = position;

	token = get_token();

	current = c;
	position = p;

	return token;
}

// Tokenizer::check_token: Checks the current token
// --------------------------------------------- >>
bool Tokenizer::check_token(string check)
{
	if (get_token() == check)
		return true;
	else
	{
		//message_box("Error: \"" + check + "\" expected!", GTK_MESSAGE_ERROR);
		return false;
	}
}

// Tokenizer::get_integer: Reads a token and returns it's integer value
// ----------------------------------------------------------------- >>
int Tokenizer::get_integer()
{
	string token = get_token();

	return atoi(token.c_str());
}

// Tokenizer::get_float: Reads a token and returns it's floating point value
// ---------------------------------------------------------------------- >>
float Tokenizer::get_float()
{
	string token = get_token();

	return (float)atof(token.c_str());
}

// Tokenizer::get_bool: Reads a token and returns it's boolean value
// -------------------------------------------------------------- >>
bool Tokenizer::get_bool()
{
	string token = get_token();

	if (token == "no" || token == "No" || token == "NO")
		return false;

	if (token == "false" || token == "False" || token == "FALSE")
		return false;

	return !!atoi(token.c_str());
}
