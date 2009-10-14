
#ifndef __WXSTUFF_H__
#define __WXSTUFF_H__

#undef min
#undef max
#undef WXWIN_COMPATIBILITY_2_6
#undef WXWIN_COMPATIBILITY_2_4

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

#endif //__WXSTUFF_H__
