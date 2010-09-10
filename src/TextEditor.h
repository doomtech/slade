
#ifndef __TEXTEDITOR_H__
#define	__TEXTEDITOR_H__

#include <wx/stc/stc.h>
#include "ArchiveEntry.h"
#include "TextLanguage.h"

class TextEditor : public wxStyledTextCtrl {
private:
	TextLanguage*	language;

public:
	TextEditor(wxWindow* parent, int id);
	~TextEditor();

	bool	setLanguage(TextLanguage* lang);
	bool	loadEntry(ArchiveEntry* entry);
	void	getRawText(MemChunk& mc);
	void	trimWhitespace();

	void	onModified(wxStyledTextEvent& e);
	void	onTextChanged(wxStyledTextEvent& e);
};

#endif //__TEXTEDITOR_H__
