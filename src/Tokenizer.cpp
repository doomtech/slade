
/*******************************************************************
 * SLADE - It's a Doom Editor
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
#include <wx/log.h>


/* Tokenizer::Tokenizer
 * Tokenizer class constructor
 *******************************************************************/
Tokenizer::Tokenizer(bool c_comments, bool h_comments)
{
	// Initialize variables
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
	// Free memory if used
	if (start) free(start);
}

/* Tokenizer::openFile
 * Reads a portion of a file to the Tokenizer
 *******************************************************************/
bool Tokenizer::openFile(string filename, DWORD offset, DWORD length)
{
	// Open the file
	FILE *fp = fopen(chr(filename), "rb");

	if (!fp)
	{
		wxLogMessage(_T("Tokenizer::openFile: Unable to open file %s"), filename.c_str());
		return false;
	}

	// Get file length
	DWORD flen = 0;
	fseek(fp, 0, SEEK_END);
	flen = (DWORD)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// If length isn't specified or exceeds the file length,
	// only read to the end of the file
	if (offset + length > flen || length == 0)
		length = flen - offset;

	// Setup variables & allocate memory
	size = length;
	position = 0;
	start = current = (char *)malloc(size);

	// Read the file portion
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
	// If length isn't specified or exceeds the string's length,
	// only copy to the end of the string
	if (offset + length > (DWORD)text.length() || length == 0)
		length = (DWORD)text.length() - offset;

	// Setup variables & allocate memory
	size = length;
	position = 0;
	start = current = (char *)malloc(size);

	// Copy the string portion
	memcpy(start, text.c_str() + offset, size);

	return true;
}

/* Tokenizer::openMem
 * Reads a chunk of memory to the Tokenizer
 *******************************************************************/
bool Tokenizer::openMem(char* mem, DWORD length)
{
	// Length must be specified
	if (length == 0)
	{
		wxLogMessage(_T("Tokenizer::openMem: length not specified"));
		return false;
	}

	// Setup variables & allocate memory
	size = length;
	position = 0;
	start = current = (char*)malloc(size);

	// Copy the data
	memcpy(start, mem, size);

	return true;
}

/* Tokenizer::isWhitespace
 * Checks if a character is 'whitespace'
 *******************************************************************/
bool Tokenizer::isWhitespace(char p)
{
	// Whitespace is either a newline, tab character or space
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
		// At end of text, return false
		position = size;
		return false;
	}
	else
	{
		// Increment position & current pointer
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
	// Increment position until a newline or end is found
	while (current[0] != '\n' && current[0] != 13)
	{
		if (!incrementCurrent())
			return;
	}

	// Skip the newline character also
	incrementCurrent();
}

/* Tokenizer::skipMultilineComment
 * Skips a multiline comment (like this one :P)
 *******************************************************************/
void Tokenizer::skipMultilineComment()
{
	// Increment position until '*/' or end is found
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
 * Gets the next 'token' from the text & moves past it, returns
 * a blank string if we're at the end of the text
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
			// Return if end of text found
			if (!incrementCurrent())
				return ret_str;
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
			return ret_str;
	}

	// Now read the token
	if (current[0] == '\"') // If we have a literal string (enclosed with "")
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
			// Add current character to the token
			ret_str += current[0];

			// Return if end of text found
			if (!incrementCurrent())
				return ret_str;
		}
	}

	// Return the token
	return ret_str;
}

/* Tokenizer::peekToken
 * Returns the next token without actually moving past it
 *******************************************************************/
string Tokenizer::peekToken()
{
	// Backup current position
	char* c = current;
	DWORD p = position;

	// Read the next token
	string token = getToken();

	// Go back to original position
	current = c;
	position = p;

	// Return the token
	return token;
}

/* Tokenizer::checkToken
 * Compares the current token with a string (unused)
 *******************************************************************/
bool Tokenizer::checkToken(string check)
{
	return !!(getToken().CompareTo(check));
}

/* Tokenizer::getInteger
 * Reads a token and returns it's integer value
 *******************************************************************/
int Tokenizer::getInteger()
{
	// Get token
	string token = getToken();

	// Return integer value
	return atoi(chr(token));
}

/* Tokenizer::getFloat
 * Reads a token and returns it's floating point value
 *******************************************************************/
float Tokenizer::getFloat()
{
	// Get token
	string token = getToken();

	// Return float value
	return (float)atof(chr(token));
}

/* Tokenizer::getDouble
 * Reads a token and returns it's double-precision floating point
 * value
 *******************************************************************/
double Tokenizer::getDouble()
{
	// Get token
	string token = getToken();

	// Return double value
	return atof(chr(token));
}

/* Tokenizer::getBool
 * Reads a token and returns it's boolean value, anything except
 * "0", "no", or "false" will return true
 *******************************************************************/
bool Tokenizer::getBool()
{
	// Get token
	string token = getToken();

	// If the token is a string "no" or "false", the value is false
	//if (!stricmp(chr(token), "no") || !stricmp(chr(token), "false"))
	if (!token.compare(_T("no")) || !token.compare(_T("false")))
		return false;

	// Returns true ("1") or false ("0")
	return !!atoi(chr(token));
}
