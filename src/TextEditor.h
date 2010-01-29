
#ifndef __TEXTEDITOR_H__
#define	__TEXTEDITOR_H__

#include <wx/stc/stc.h>
#include "ArchiveEntry.h"

class TextEditor : public wxStyledTextCtrl {
private:

public:
	TextEditor(wxWindow* parent, int id);
	~TextEditor();

	bool	loadEntry(ArchiveEntry* entry);
	void	getRawText(MemChunk& mc);

	void	onModified(wxStyledTextEvent& e);
};

#endif //__TEXTEDITOR_H__
