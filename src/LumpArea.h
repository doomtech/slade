
#ifndef __LUMPAREA_H__
#define __LUMPAREA_H__

#include "ArchiveEntry.h"
#include <wx/stc/stc.h>

class LumpArea : public wxPanel
{
protected:
	ArchiveEntry*	lump;

public:
	LumpArea(wxWindow* parent);
	~LumpArea();
};

class TextLumpArea : public LumpArea
{
private:
	wxStyledTextCtrl*	text_area;

public:
	TextLumpArea(wxWindow* parent);
	~TextLumpArea();
};

#endif //__LUMPAREA_H__
