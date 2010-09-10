
#ifndef __WXSTUFF_H__
#define __WXSTUFF_H__

#undef min
#undef max

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

// Some misc wx-related defines
#define WXCOL(rgba) wxColor(rgba.r, rgba.g, rgba.b, rgba.a)

// Some misc wx-related functions
wxMenuItem* createMenuItem(wxMenu* menu, int id, string label, string help = wxEmptyString, string icon = wxEmptyString);

#endif //__WXSTUFF_H__
