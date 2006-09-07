
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

#endif
