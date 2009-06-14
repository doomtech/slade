
#ifndef __WXSTUFF_H__
#define __WXSTUFF_H__

#undef min
#undef max
#undef DWORD
#undef WORD
#undef BYTE

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

#endif //__WXSTUFF_H__
