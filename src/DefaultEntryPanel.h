
#ifndef __DEFAULTENTRYPANEL_H__
#define	__DEFAULTENTRYPANEL_H__

#include "EntryPanel.h"
#include "TextEditor.h"

class DefaultEntryPanel : public EntryPanel {
private:
	wxStaticText*	label_type;
	wxStaticText*	label_size;
	wxButton*		btn_edit_text;
	TextEditor*		text_area;
	bool			view_text;

public:
	enum {
		BTN_EDITTEXT,
	};

	DefaultEntryPanel(wxWindow* parent);
	~DefaultEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
	void showTextEditor();
	void showEntryInfo(bool show_btn_edittext = false);

	void onEditTextClicked(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif //__DEFAULTENTRYPANEL_H__
