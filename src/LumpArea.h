
#ifndef __LUMPAREA_H__
#define __LUMPAREA_H__

#include "Lump.h"
//#include <wx/stc/stc.h>

class LumpArea : public wxPanel
{
protected:
	Lump*	lump;

public:
	LumpArea(wxWindow* parent);
	~LumpArea();
};

class TextLumpArea : public LumpArea
{
private:

public:
	TextLumpArea(wxWindow* parent);
	~TextLumpArea();
};

#endif //__LUMPAREA_H__
