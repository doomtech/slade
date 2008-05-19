
#ifndef __LUMPAREA_H__
#define __LUMPAREA_H__

#include "Lump.h"

class LumpArea : public wxPanel
{
protected:
	Lump*	lump;

public:
	LumpArea(wxWindow* parent);
	~LumpArea();
};

#endif //__LUMPAREA_H__
