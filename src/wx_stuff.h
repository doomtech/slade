
#ifndef __WX_STUFF_H__
#define __WX_STUFF_H__

#include "wx/wxprec.h"

#undef min
#undef max

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b)) 

string wx_to_str(wxString str);
wxString str_to_wx(string str);

#endif
