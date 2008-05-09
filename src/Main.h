
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

// Define some variable types
#ifndef _VAR_TYPES
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef   signed short SWORD;
#ifdef _WIN32
typedef unsigned long DWORD;
#else
typedef uint32_t DWORD;
#endif
#endif

// Global internal includes
#include "MemChunk.h"

#endif //__MAIN_H__
