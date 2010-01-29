
#ifndef __TEXTENTRYPANEL_H__
#define	__TEXTENTRYPANEL_H__

#include "EntryPanel.h"
#include "TextEditor.h"

class TextEntryPanel : public EntryPanel {
private:
	TextEditor*	text_area;

public:
	TextEntryPanel(wxWindow* parent);
	~TextEntryPanel();

	bool	loadEntry(ArchiveEntry* entry);
	bool	saveEntry();

	// Events
	void	onTextModified(wxStyledTextEvent& e);
};


#endif //__TEXTENTRYPANEL_H__

