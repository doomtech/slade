
#ifndef __ENTRYPANEL_H__
#define __ENTRYPANEL_H__

#include "ArchiveEntry.h"

class EntryPanel : public wxPanel {
protected:
	ArchiveEntry*	entry;
	bool			changed;

public:
	EntryPanel(wxWindow* parent);
	~EntryPanel();

	ArchiveEntry*	getEntry() { return entry; }
	bool			unsavedChanges() { return changed; }

	virtual bool	loadEntry(ArchiveEntry* entry);
	virtual bool	saveEntry();
};

#endif //__ENTRYPANEL_H__
