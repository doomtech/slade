
#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>
#include <stdlib.h>

// Use QString as the program 'string' class
#include <QString>
typedef QString string;

// Some convenience defines
#define chr(x) x.toAscii().constData()	// Similar to std::string.c_str()
#define s_fmt QString().sprintf			// String formatting

// For now
#define _T(x) x
#define log_message printf

// Program-wide headers
#include "MemChunk.h"

#endif//__MAIN_H__
