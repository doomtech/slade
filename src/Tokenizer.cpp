
/*******************************************************************
 * SLADE - It's a Map Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Tokenizer.cpp
 * Description: My trusty old string tokenizer class.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "Tokenizer.h"


/* Tokenizer::Tokenizer
 * Tokenizer class constructor
 *******************************************************************/
Tokenizer::Tokenizer(bool c_comments, bool h_comments)
{
	current = NULL;
	start = NULL;
	size = 0;
	ccomments = c_comments;
	hcomments = h_comments;
}

/* Tokenizer::~Tokenizer
 * Tokenizer class destructor
 *******************************************************************/
Tokenizer::~Tokenizer()
{
	if (start) free(start);
}

/* Tokenizer::openFile
 * Reads a portion of a file to the Tokenizer
 *******************************************************************/
bool Tokenizer::openFile(string filename, DWORD offset, DWORD length)
{
	FILE *fp = fopen(chr(filename), "rb");

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

/* Tokenizer::openString
 * Reads a portion of a string to the Tokenizer
 *******************************************************************/
bool Tokenizer::openString(string text, DWORD offset, DWORD length)
{
	if (length == 0)
		length = (DWORD)text.length();

	size = length;
	position = 0;
	start = current = (char *)malloc(size);
	memcpy(start, text.c_str(), size);

	return true;
}

/* Tokenizer::openMem
 * Reads a chunk of memory to the Tokenizer
 *******************************************************************/
bool Tokenizer::openMem(char* mem, DWORD length)
{
	if (length <= 0)
		return false;

	size = length;
	position = 0;
	start = current = (char*)malloc(size);
	memcpy(start, mem, size);

	return true;
}

/* Tokenizer::isWhitespace
 * Checks if a character is 'whitespace'
 *******************************************************************/
bool Tokenizer::isWhitespace(char p)
{
	if (p == '\n' || p == 13 || p == ' ' || p == '\t')
		return true;
	else
		return false;
}

/* Tokenizer::incrementCurrent
 * Increments the position pointer, returns false on end of block
 *******************************************************************/
bool Tokenizer::incrementCurrent()
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

/* Tokenizer::skipLineComment
 * Skips a '//' comment
 *******************************************************************/
void Tokenizer::skipLineComment()
{
	while (current[0] != '\n' && current[0] != 13)
	{
		if (!incrementCurrent())
			return;
	}

	incrementCurrent();
}

/* Tokenizer::skipMultilineComment
 * Skips a multiline comment (like this one :P)
 *******************************************************************/
void Tokenizer::skipMultilineComment()
{
	while (!(current[0] == '*' && current[1] == '/'))
	{
		if (!incrementCurrent())
			return;
	}

	// Skip the ending '*/'
	incrementCurrent();
	incrementCurrent();
}

/* Tokenizer::getToken
 * Gets the next 'token' from the text & moves past it
 *******************************************************************/
string Tokenizer::getToken()
{
	string ret_str = _T("");
	bool ready = false;

	// Increment pointer to next token
	while (!ready)
	{
		ready = true;

		// Increment pointer until non-whitespace is found
		while (isWhitespace(current[0]))
		{
			if (!incrementCurrent())
			{
				ret_str = _T("!END");
				return ret_str;
			}
		}

		// Skip C-style comments
		if (ccomments)
		{
			// Check if we have a line comment
			if (current[0] == '/' && current[1] == '/' && current[2] != '$')
			{
				skipLineComment();	// Skip it
				ready = false;
			}

			// Check if we have a multiline comment
			if (current[0] == '/' && current[1] == '*')
			{
				skipMultilineComment();	// Skip it
				ready = false;
			}
		}

		// Skip '##' comments
		if (hcomments)
		{
			if (current[0] == '#' && current[1] == '#')
			{
				skipLineComment();	// Skip it
				ready = false;
			}
		}

		// Check for end of text
		if (position == size)
		{
			ret_str = _T("!END");
			return ret_str;
		}
	}

	// Check for literal string (enclosed with "")
	if (current[0] == '\"')
	{
		// Skip opening "
		incrementCurrent();

		// Read literal string (include whitespace)
		while (current[0] != '\"')
		{
			ret_str += current[0];

			if (!incrementCurrent())
				return ret_str;
		}

		// Skip closing "
		incrementCurrent();
	}
	else
	{
		// Read token (don't include whitespace)
		while (!isWhitespace(current[0]))
		{
			ret_str += current[0];

			if (!incrementCurrent())
				return ret_str;
		}
	}

	return ret_str;
}

/* Tokenizer::peekToken
 * Returns the next token without actually moving past it
 *******************************************************************/
string Tokenizer::peekToken()
{
	string token;

	char* c = current;
	DWORD p = position;

	token = getToken();

	current = c;
	position = p;

	return token;
}

/* Tokenizer::checkToken
 * Compares the current token with a string (unused)
 *******************************************************************/
bool Tokenizer::checkToken(string check)
{
	return getToken() == check;
}

/* Tokenizer::getInteger
 * Reads a token and returns it's integer value
 *******************************************************************/
int Tokenizer::getInteger()
{
	string token = getToken();

	return atoi(chr(token));
}

/* Tokenizer::getFloat
 * Reads a token and returns it's floating point value
 *******************************************************************/
float Tokenizer::getFloat()
{
	string token = getToken();

	return (float)atof(chr(token));
}

/* Tokenizer::getDouble
 * Reads a token and returns it's double-precision
 * floating point value
 *******************************************************************/
double Tokenizer::getDouble()
{
	string token = getToken();

	return atof(chr(token));
}

/* Tokenizer::getBool
 * Reads a token and returns it's boolean value
 *******************************************************************/
bool Tokenizer::getBool()
{
	string token = getToken();

	if (!stricmp(chr(token), "no") || !stricmp(chr(token), "false"))
		return false;

	return !!atoi(chr(token));
}
