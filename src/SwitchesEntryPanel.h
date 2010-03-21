
#ifndef SWITCHESENTRYPANEL_H
#define SWITCHESENTRYPANEL_H

#include "EntryPanel.h"
#include "BinaryControlLump.h"
#include "SwitchesList.h"
#include "ListView.h"

class SwitchesEntryPanel : public EntryPanel {
private:
	SwitchesList	switches;

	ListView*		list_entries;
	wxTextCtrl*		text_offname;
	wxTextCtrl*		text_onname;
	wxComboBox*		cbox_range;

public:
	SwitchesEntryPanel(wxWindow* parent);
	~SwitchesEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	void	populateEntryList();
};

#endif//SWITCHESENTRYPANEL_H
