
#ifndef ANIMATEDENTRYPANEL_H
#define ANIMATEDENTRYPANEL_H

#include "EntryPanel.h"
#include "BinaryControlLump.h"
#include "AnimatedList.h"
#include "ListView.h"
#include <wx/radiobut.h>

class AnimatedEntryPanel : public EntryPanel {
private:
	AnimatedList	animated;

	ListView*		list_entries;
	wxTextCtrl*		text_firstname;
	wxTextCtrl*		text_lastname;
	wxTextCtrl*		text_speed;
	wxRadioButton*	rbtn_flat;
	wxRadioButton*	rbtn_texture;
	wxCheckBox*		cbox_decals;
	wxCheckBox*		cbox_swirl;
	wxButton*		butn_convert;

public:
	AnimatedEntryPanel(wxWindow* parent);
	~AnimatedEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	void	populateEntryList();
};

#endif//ANIMATEDENTRYPANEL_H
