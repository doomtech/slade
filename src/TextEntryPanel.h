
#ifndef __TEXTENTRYPANEL_H__
#define	__TEXTENTRYPANEL_H__

#include "EntryPanel.h"
#include "TextEditor.h"

class TextEntryPanel : public EntryPanel {
private:
	TextEditor*	text_area;
	wxButton*	btn_find_replace;
	wxChoice*	choice_text_language;

public:
	TextEntryPanel(wxWindow* parent);
	~TextEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();
	void	refreshPanel();
	void	closeEntry();

	// Events
	void	onTextModified(wxStyledTextEvent& e);
	void	onBtnFindReplace(wxCommandEvent& e);
	void	onChoiceLanguageChanged(wxCommandEvent& e);
};


#endif //__TEXTENTRYPANEL_H__
