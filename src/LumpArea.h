
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

	virtual void loadEntry(ArchiveEntry* entry);
};

class TextLumpArea : public LumpArea
{
private:
	wxStyledTextCtrl*	text_area;

public:
	TextLumpArea(wxWindow* parent);
	~TextLumpArea();

	void loadEntry(ArchiveEntry* entry);
};

#endif //__LUMPAREA_H__
