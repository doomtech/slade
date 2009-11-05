
#ifndef __MAIN_H__
#define __MAIN_H__

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
};

enum { DIR_USER, DIR_DATA, DIR_APP, DIR_TEMP };
string appPath(string filename, int dir);

#endif //__MAIN_H__
