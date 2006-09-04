
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// String stuff
#include <wx/string.h>
typedef wxString string;
#define s_fmt wxString::Format

const char* chr(const wxString &str);

#if wxUSE_UNICODE
#define atoi(s)	((int)wcstol(s, NULL, 10))
#define atof(s) ((float)wcstod(s, NULL))
#endif

#include <vector>
using std::vector;

#include <map>
typedef std::map<string, string> props_t;

#include <algorithm>

#ifdef _MSC_VER
typedef unsigned __int32 uint32_t;
typedef __int32 int32_t;
#else
#include <stdint.h>
#endif

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

// A macro to check if a value exists in a vector
#define vector_exists(vec, val) find(vec.begin(), vec.end(), val) != vec.end()

// A macro to add a value to a vector if the value doesn't already exist in the vector
#define vector_add_nodup(vec, val) if (!(vector_exists(vec, val))) vec.push_back(val)

// Define min and max
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

// Editor stuff
#define MAJOR_UNIT 32

#include "cvar.h"

string c_path(string filename, BYTE dir = 0);
void redraw_map(bool map = false, bool grid = false);
void update_statusbar();
void update_infobar();
void change_edit_mode(int mode);
void save_main_config();
void message_box(string message, string caption = _T(""));
void log_message(string message);
float get_zoom();

#define DIR_APP	0
#define DIR_TMP	1
#define DIR_USR	2
