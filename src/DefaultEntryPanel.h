
#ifndef __DEFAULTENTRYPANEL_H__
#define	__DEFAULTENTRYPANEL_H__

#include "EntryPanel.h"
#include "TextEditor.h"

class DefaultEntryPanel : public EntryPanel {
private:
	wxStaticText*	label_type;
	wxStaticText*	label_size;
	wxButton*		btn_edit_text;
	wxButton*		btn_view_hex;
	TextEditor*		text_area;
	bool			view_text;

public:
	DefaultEntryPanel(wxWindow* parent);
	~DefaultEntryPanel();

	bool loadEntry(ArchiveEntry* entry);
	bool saveEntry();
	void showTextEditor();
	void showEntryInfo(bool show_btn_edittext = false);

	void onEditTextClicked(wxCommandEvent& e);
	void onViewHexClicked(wxCommandEvent& e);
	void onTextModified(wxStyledTextEvent& e);
};

#endif //__DEFAULTENTRYPANEL_H__
