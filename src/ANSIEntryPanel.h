
#ifndef __ANSI_ENTRY_PANEL__
#define __ANSI_ENTRY_PANEL__

#include "EntryPanel.h"
#include <wx/grid.h>

class ANSIEntryPanel : public EntryPanel {
private:
	wxGrid*		grid_ansi;

public:
	ANSIEntryPanel(wxWindow* parent);
	~ANSIEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
};

#endif//__ANSI_ENTRY_PANEL__
