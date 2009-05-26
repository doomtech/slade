
#ifndef __LUMPAREA_H__
#define __LUMPAREA_H__

#include "ArchiveEntry.h"
#include <wx/stc/stc.h>

class EntryPanel : public wxPanel {
protected:
	ArchiveEntry*	entry;

public:
	EntryPanel(wxWindow* parent);
	~EntryPanel();

	ArchiveEntry* getEntry() { return entry; }
	virtual bool loadEntry(ArchiveEntry* entry);
};

class TextEntryPanel : public EntryPanel {
private:
	wxStyledTextCtrl*	text_area;

public:
	TextEntryPanel(wxWindow* parent);
	~TextEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
};

#endif //__LUMPAREA_H__
