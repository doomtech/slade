
#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef _WINDOWS
#include <windows.h>
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint64_t;
#endif

// String stuff
#include <wx/string.h>
typedef wxString string;
#define s_fmt wxString::Format

// Property list (string:string)
#include <wx/hashmap.h>
WX_DECLARE_STRING_HASH_MAP(wxString, PropertyList);

// Macro to convert a wxString to a c string
#define chr(s) ((const char*)(s).ToAscii())

// Vectors
#include <vector>
#define vector std::vector

// Logfile
#include <wx/log.h>

// Opengl
#include <GL/gl.h>

// Global internal includes
#include "MemChunk.h"
#include "CVar.h"
#include "Structs.h"

// Class to hold global variables
namespace Global {
	extern string error;
	extern string version;
};

enum { DIR_USER, DIR_DATA, DIR_APP, DIR_TEMP };
string appPath(string filename, int dir);

#endif //__MAIN_H__
