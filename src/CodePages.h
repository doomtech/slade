
#ifndef __CODEPAGES_H__
#define __CODEPAGES_H__

#include "Main.h"
#include <wx/colour.h>

namespace CP {
	string		fromASCII(uint8_t val);
	string		fromCP437(uint8_t val);
	wxColour	ansiColor(uint8_t val);
};

#endif //__CODEPAGES_H__