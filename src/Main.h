
#ifndef __MAIN_H__
#define __MAIN_H__

// String stuff
#include <wx/string.h>
typedef wxString string;
#define s_fmt wxString::Format

// the temp object was destroyed before it was used when this was a function
#define chr(s) ((const char*)(s).ToAscii())

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

#endif //__MAIN_H__
