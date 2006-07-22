
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <map>
typedef std::map<string, string> props_t;

#include <algorithm>

// Define some variable types
#ifndef _VAR_TYPES
#define _VAR_TYPES
#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define SWORD signed short
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

string str_upper(string str);
string s_fmt(char *str, ...);
void log_message(string message);
void message_box(string message, string caption = "SLADE");
string c_path(string filename, bool userdir = false);
void redraw_map(bool map = false, bool grid = false);
void update_statusbar();
void change_edit_mode(int mode);
void save_main_config();
