
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
typedef unsigned __int64 uint64_t;
#endif

// String stuff
#include <wx/string.h>
typedef wxString string;
#define S_FMT wxString::Format
#define S_CMP(s1, s2) s1.Cmp(s2) == 0
#define S_CMPNOCASE(s1, s2) s1.CmpNoCase(s2) == 0

// Macro to convert a wxString to a c string
#define CHR(s) ((const char*)(s).ToAscii())


// Vectors
#include <vector>
#include <algorithm>
#define vector std::vector

// A macro to check if a value exists in a vector
#define VECTOR_EXISTS(vec, val) find(vec.begin(), vec.end(), val) != vec.end()

// A macro to add a value to a vector if the value doesn't already exist in the vector
#define VECTOR_ADD_UNIQUE(vec, val) if (!(vector_exists(vec, val))) vec.push_back(val)

// A macro to remove an item with a given value from a vector
#define VECTOR_REMOVE(vec, val) vec.erase(find(vec.begin(), vec.end(), val))


// Logfile
#include <wx/log.h>

// OpenGL
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif _WINDOWS
#ifndef GLEW_STATIC	// For some reason vs2010 doesn't like this not being defined in the project settings
#define GLEW_STATIC
#endif
#include "glew/glew.h"
#include "glew/wglew.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// File handling
#include <wx/file.h>

// Global internal includes
#include "MemChunk.h"
#include "CVar.h"
#include "Structs.h"

// Namespace to hold 'global' variables
namespace Global {
	extern string error;
	extern string version;
};

enum Directory { DIR_USER, DIR_DATA, DIR_APP, DIR_TEMP };
string appPath(string filename, int dir);

#ifdef _MSC_VER
#ifdef _DEBUG
    #include <crtdbg.h>
    #define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
    #define new DEBUG_NEW
#else
    #define DEBUG_NEW new
#endif
#endif

// Random useful defines
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))


#endif //__MAIN_H__
