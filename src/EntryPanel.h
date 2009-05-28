
#ifndef __ENTRYPANEL_H__
#define __ENTRYPANEL_H__

#include "ArchiveEntry.h"

class EntryPanel : public wxPanel {
protected:
	ArchiveEntry*	entry;

public:
	EntryPanel(wxWindow* parent);
	~EntryPanel();

	ArchiveEntry* getEntry() { return entry; }
	virtual bool loadEntry(ArchiveEntry* entry);
};

#endif //__ENTRYPANEL_H__
