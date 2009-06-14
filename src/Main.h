
#ifndef __MAIN_H__
#define __MAIN_H__

// String stuff
#include <wx/string.h>
typedef wxString string;
#define s_fmt wxString::Format

// macro to convert a wxString to a c string
#define chr(s) ((const char*)(s).ToAscii())

// Vectors
#include <vector>
using std::vector;

// Logfile
#include <wx/log.h>

// Define some variable types
#define DWORD unsigned long
#define WORD unsigned short
#define BYTE unsigned char

// Global internal includes
#include "MemChunk.h"

// Class to hold global variables
namespace Global {
	extern string error;
};

#endif //__MAIN_H__
