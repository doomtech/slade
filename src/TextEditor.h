
#ifndef __TEXTEDITOR_H__
#define	__TEXTEDITOR_H__

#include <wx/stc/stc.h>
#include <wx/minifram.h>
#include "ArchiveEntry.h"
#include "TextLanguage.h"

class FindReplaceDialog : public wxMiniFrame {
private:
	wxTextCtrl*	text_find;
	wxTextCtrl*	text_replace;
	wxButton*	btn_find_next;
	wxButton*	btn_replace;
	wxButton*	btn_replace_all;
	wxCheckBox*	cb_match_case;
	wxCheckBox*	cb_match_word;

public:
	FindReplaceDialog(wxWindow* parent);
	~FindReplaceDialog();

	wxButton*	getBtnFindNext() { return btn_find_next; }
	wxButton*	getBtnReplace() { return btn_replace; }
	wxButton*	getBtnReplaceAll() { return btn_replace_all; }
	string		getFindString() { return text_find->GetValue(); }
	string		getReplaceString() { return text_replace->GetValue(); }
	bool		matchCase() { return cb_match_case->GetValue(); }
	bool		matchWord() { return cb_match_word->GetValue(); }
};

class TextEditor : public wxStyledTextCtrl {
private:
	TextLanguage*		language;
	TLFunction*			func_calltip;
	FindReplaceDialog*	dlg_fr;

public:
	TextEditor(wxWindow* parent, int id);
	~TextEditor();

	bool	setLanguage(TextLanguage* lang);
	bool	loadEntry(ArchiveEntry* entry);
	void	getRawText(MemChunk& mc);
	void	trimWhitespace();

	// Find/Replace
	void	showFindReplaceDialog() { dlg_fr->Show(); dlg_fr->CenterOnParent(); }
	bool	findNext(string find);
	bool	replaceCurrent(string find, string replace);
	int		replaceAll(string find, string replace);

	void	checkBraceMatch();
	bool	openCalltip(int pos);
	void	updateCalltip();

	void	onKeyDown(wxKeyEvent& e);
	void	onKeyUp(wxKeyEvent& e);
	void	onCharAdded(wxStyledTextEvent& e);
	void	onUpdateUI(wxStyledTextEvent& e);
	void	onCalltipClicked(wxStyledTextEvent& e);
	void	onFRDBtnFindNext(wxCommandEvent& e);
	void	onFRDBtnReplace(wxCommandEvent& e);
	void	onFRDBtnReplaceAll(wxCommandEvent& e);
};

#endif //__TEXTEDITOR_H__
