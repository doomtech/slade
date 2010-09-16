
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    TextEditor.cpp
 * Description: The SLADE Text Editor control, does syntax
 *              highlighting etc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "WxStuff.h"
#include "TextEditor.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, txed_trim_whitespace, false, CVAR_SAVE)
CVAR(Int, txed_tab_width, 4, CVAR_SAVE)
CVAR(Bool, txed_auto_indent, true, CVAR_SAVE)
rgba_t col_comment(0, 150, 0, 255);
rgba_t col_string(0, 120, 130, 255);
rgba_t col_keyword(0, 30, 200, 255);
rgba_t col_constant(180, 30, 200, 255);
rgba_t col_function(200, 100, 30, 255);


/*******************************************************************
 * FINDREPLACEDIALOG CLASS FUNCTIONS
 *******************************************************************/

/* FindReplaceDialog::FindReplaceDialog
 * FindReplaceDialog class constructor
 *******************************************************************/
FindReplaceDialog::FindReplaceDialog(wxWindow* parent) : wxMiniFrame(parent, -1, "Find + Replace", wxDefaultPosition, wxDefaultSize, wxCAPTION|wxCLOSE_BOX) {
	// Create backing panel
	wxPanel* panel = new wxPanel(this, -1);
	wxBoxSizer* fsizer = new wxBoxSizer(wxVERTICAL);
	fsizer->Add(panel, 1, wxEXPAND);
	SetSizer(fsizer);

	// Create/set dialog sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(sizer);


	// 'Find' text entry
	sizer->Add(new wxStaticText(panel, -1, "Find:"), 0, wxTOP|wxLEFT|wxRIGHT, 4);
	text_find = new wxTextCtrl(panel, -1);
	sizer->Add(text_find, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Find options checkboxes
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	sizer->AddSpacer(4);
	hbox->AddStretchSpacer(1);

	// 'Match Case' checkbox
	cb_match_case = new wxCheckBox(panel, -1, "Match Case");
	hbox->Add(cb_match_case, 0, wxEXPAND|wxRIGHT, 4);

	// 'Match Whole Word' checkbox
	cb_match_word = new wxCheckBox(panel, -1, "Match Whole Word");
	hbox->Add(cb_match_word, 0, wxEXPAND);


	// 'Replace With' text entry
	sizer->Add(new wxStaticText(panel, -1, "Replace With:"), 0, wxTOP|wxLEFT|wxRIGHT, 4);
	text_replace = new wxTextCtrl(panel, -1);
	sizer->Add(text_replace, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);


	// Buttons
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->AddStretchSpacer(1);

	// 'Find Next'
	btn_find_next = new wxButton(panel, -1, "Find Next");
	hbox->Add(btn_find_next, 0, wxEXPAND|wxRIGHT, 4);

	// 'Replace'
	btn_replace = new wxButton(panel, -1, "Replace");
	hbox->Add(btn_replace, 0, wxEXPAND|wxRIGHT, 4);

	// 'Replace All'
	btn_replace_all = new wxButton(panel, -1, "Replace All");
	hbox->Add(btn_replace_all, 0, wxEXPAND);


	// Init layout
	Layout();
	SetInitialSize(wxSize(400, -1));
	Fit();
}

/* FindReplaceDialog::~FindReplaceDialog
 * FindReplaceDialog class destructor
 *******************************************************************/
FindReplaceDialog::~FindReplaceDialog() {
}


/*******************************************************************
 * TEXTEDITOR CLASS FUNCTIONS
 *******************************************************************/

/* TextEditor::TextEditor
 * TextEditor class constructor
 *******************************************************************/
TextEditor::TextEditor(wxWindow* parent, int id)
: wxStyledTextCtrl(parent, id) {
	// Set default font
	wxFont f(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	StyleSetFont(wxSTC_STYLE_DEFAULT, f);
	SetTabWidth(txed_tab_width);

	// Line numbers by default
	SetMarginType(0, wxSTC_MARGIN_NUMBER);
	SetMarginWidth(0, TextWidth(wxSTC_STYLE_LINENUMBER, "9999"));
	SetMarginWidth(1, 4);

	// General settings
	SetBufferedDraw(true);
	SetUseAntiAliasing(true);

	// Test colours
	StyleSetForeground(wxSTC_C_COMMENT, WXCOL(col_comment));
	StyleSetForeground(wxSTC_C_COMMENTDOC, WXCOL(col_comment));
	StyleSetForeground(wxSTC_C_COMMENTLINE, WXCOL(col_comment));
	StyleSetForeground(wxSTC_C_STRING, WXCOL(col_string));
	StyleSetForeground(wxSTC_C_CHARACTER, WXCOL(col_string));
	StyleSetForeground(wxSTC_C_WORD, WXCOL(col_keyword));
	StyleSetForeground(wxSTC_C_WORD2, WXCOL(col_function));
	StyleSetForeground(wxSTC_C_GLOBALCLASS, WXCOL(col_constant));

	StyleSetBackground(wxSTC_STYLE_BRACELIGHT, WXCOL(rgba_t(170, 255, 170, 255)));
	StyleSetBold(wxSTC_STYLE_BRACELIGHT, true);

	// Temp
	SetLexer(wxSTC_LEX_CPPNOCASE);
	setLanguage(TextLanguage::getLanguage("decorate"));

	// Find+Replace dialog
	dlg_fr = new FindReplaceDialog(this);

	// Bind events
	Bind(wxEVT_KEY_DOWN, &TextEditor::onKeyDown, this);
	Bind(wxEVT_KEY_UP, &TextEditor::onKeyUp, this);
	Bind(wxEVT_STC_CHARADDED, &TextEditor::onCharAdded, this);
	Bind(wxEVT_STC_UPDATEUI, &TextEditor::onUpdateUI, this);
	Bind(wxEVT_STC_CALLTIP_CLICK, &TextEditor::onCalltipClicked, this);
	dlg_fr->getBtnFindNext()->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextEditor::onFRDBtnFindNext, this);
	dlg_fr->getBtnReplace()->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextEditor::onFRDBtnReplace, this);
	dlg_fr->getBtnReplaceAll()->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextEditor::onFRDBtnReplaceAll, this);
}

/* TextEditor::~TextEditor
 * TextEditor class destructor
 *******************************************************************/
TextEditor::~TextEditor() {
}

bool TextEditor::setLanguage(TextLanguage* lang) {
	// Check language was given
	if (!lang)
		return false;

	// Load word lists
	SetKeyWords(0, lang->getKeywordsList().Lower());
	SetKeyWords(1, lang->getFunctionsList().Lower());
	SetKeyWords(3, lang->getConstantsList().Lower());

	// Update variables
	this->language = lang;

	return true;
}

/* TextEditor::loadEntry
 * Reads the contents of [entry] into the text area, returns false
 * if the given entry is invalid
 *******************************************************************/
bool TextEditor::loadEntry(ArchiveEntry* entry) {
	// Clear current text
	ClearAll();

	// Check that the entry exists
	if (!entry) {
		Global::error = "Invalid archive entry given";
		return false;
	}

	// Check that the entry has any data, if not do nothing
	if (entry->getSize() == 0 || !entry->getData())
		return true;

	// Get character entry data
	char* data = new char[entry->getSize() + 1];
	memcpy(data, entry->getData(), entry->getSize());
	data[entry->getSize()] = 0;

	// Load text into editor
	SetTextRaw(data);

	// Clean up
	delete[] data;

	// Update line numbers margin width
	string numlines = s_fmt("0%d", GetNumberOfLines());
	SetMarginWidth(0, TextWidth(wxSTC_STYLE_LINENUMBER, numlines));

	return true;
}

/* TextEditor::getRawText
 * Writes the raw ASCII text to [mc]
 *******************************************************************/
void TextEditor::getRawText(MemChunk& mc) {
	mc.clear();
	const char* raw_text = GetTextRaw();
	mc.importMem((const uint8_t*)raw_text, GetTextLength());
}

void TextEditor::trimWhitespace() {
}

/* TextEditor::findNext
 * Finds the next occurrence of the [find] after the caret position,
 * selects it and scrolls to it if needed. Returns false if the
 * [find] was invalid or no match was found, true otherwise
 *******************************************************************/
bool TextEditor::findNext(string find) {
	// Check search string
	if (find.IsEmpty())
		return false;

	// Setup target range
	SetTargetEnd(GetTextLength());
	SetTargetStart(GetSelectionEnd());

	// Search within current target range
	if (SearchInTarget(find) < 0) {
		// None found, search again from start
		SetTargetStart(0);
		SetTargetEnd(GetTextLength());
		if (SearchInTarget(find) < 0) {
			// No matches found in entire text
			return false;
		}
	}

	// Select matched text
	SetSelection(GetTargetStart(), GetTargetEnd());

	// Scroll to selection
	EnsureCaretVisible();

	return true;
}

/* TextEditor::replaceCurrent
 * Replaces the currently selected occurrence of [find] with
 * [replace], then selects and scrolls to the next occurrence of
 * [find] in the text. Returns false if [find] is invalid or the
 * current selection does not match it, true otherwise
 *******************************************************************/
bool TextEditor::replaceCurrent(string find, string replace) {
	// Check search string
	if (find.IsEmpty())
		return false;

	// Check that we've done a find previously
	// (by searching for the find string within the current selection)
	if (GetSelectedText().Length() != find.Length())
		return false;
	SetTargetStart(GetSelectionStart());
	SetTargetEnd(GetSelectionEnd());
	if (SearchInTarget(find) < 0)
		return false;

	// Do the replace
	ReplaceTarget(replace);

	// Update selection
	SetSelection(GetTargetStart(), GetTargetEnd());

	// Do find next
	findNext(find);

	return true;
}

/* TextEditor::replaceAll
 * Replaces all occurrences of [find] in the text with [replace].
 * Returns the number of occurrences replaced
 *******************************************************************/
int TextEditor::replaceAll(string find, string replace) {
	// Check search string
	if (find.IsEmpty())
		return false;

	// Init search target to entire text
	SetTargetStart(0);
	SetTargetEnd(GetTextLength());

	// Loop of death
	int replaced = 0;
	while (1) {
		if (SearchInTarget(find) < 0)
			break;	// No matches, finished
		else {
			// Replace text & increment counter
			ReplaceTarget(replace);
			replaced++;

			// Continue search from end of replaced text to end of text
			SetTargetStart(GetTargetEnd());
			SetTargetEnd(GetTextLength());
		}
	}

	// Return number of instances replaced
	return replaced;
}

void TextEditor::checkBraceMatch() {
	int bracematch = BraceMatch(GetCurrentPos());

	if (bracematch == wxSTC_INVALID_POSITION)
		BraceHighlight(-1, -1);
	else {
		BraceHighlight(GetCurrentPos(), bracematch);
	}
}

bool TextEditor::openCalltip(int pos) {
	// Get start of word before bracket
	int start = WordStartPosition(pos - 1, false);

	// Get word before bracket
	string word = GetTextRange(WordStartPosition(start, true), WordEndPosition(start, true));

	// Get matching language function (if any)
	TLFunction* func = language->getFunction(word);

	// Show calltip if it's a function
	if (func && func->nArgSets() > 0) {
		CallTipShow(GetCurrentPos(), func->generateCallTipString());
		func_calltip = func;

		// Highlight first arg
		point2_t arg_ext = func_calltip->getArgTextExtent(0);
		CallTipSetHighlight(arg_ext.x, arg_ext.y);

		return true;
	}
	else {
		func_calltip = NULL;
		return false;
	}
}

void TextEditor::updateCalltip() {
	if (!CallTipActive()) {
		// No calltip currently showing, check if we're in a function
		int pos = GetCurrentPos() - 1;
		while (1) {
			// Exit if we get to the start of the text
			if (pos < 0)
				return;

			// Get character
			int chr = GetCharAt(pos);

			// If we find a closing bracket, exit
			if (chr == ')')
				return;

			// If we find an opening bracket, try to open a calltip
			if (chr == '(') {
				if (!openCalltip(pos))
					return;
				else
					break;
			}

			// Go to previous character
			pos--;
		}
	}

	if (func_calltip) {
		// Calltip currently showing, determine what arg we're at
		int pos = GetCurrentPos() - 1;
		int arg = 0;
		while (1) {
			// Exit if we get to the start of the text
			if (pos < 0)
				break;

			// Get character
			int chr = GetCharAt(pos);

			// If we find a comma, increment the arg number
			if (chr == ',')
				arg++;

			// If we find a starting brace, exit
			if (chr == '(')
				break;

			// If we find an ending brace, we're outside a function, so cancel the calltip
			if (chr == ')') {
				CallTipCancel();
				func_calltip = NULL;
				return;
			}

			// Go to previous character
			pos--;
		}

		// Update calltip string with the current arg highlighted
		point2_t arg_ext = func_calltip->getArgTextExtent(arg);
		CallTipSetHighlight(arg_ext.x, arg_ext.y);
	}
}


/*******************************************************************
 * TEXTEDITOR CLASS EVENTS
 *******************************************************************/

void TextEditor::onKeyDown(wxKeyEvent& e) {
	// Check for Ctrl+Shift+Space (invoke calltip)
	if (e.GetModifiers() & wxMOD_SHIFT && e.GetModifiers() & wxMOD_CONTROL && e.GetKeyCode() == WXK_SPACE)
		updateCalltip();

	e.Skip();
}

void TextEditor::onKeyUp(wxKeyEvent& e) {
	e.Skip();
}

void TextEditor::onCharAdded(wxStyledTextEvent& e) {
	// Update line numbers margin width
	string numlines = s_fmt("0%d", GetNumberOfLines());
	SetMarginWidth(0, TextWidth(wxSTC_STYLE_LINENUMBER, numlines));

	// Auto indent
    int currentLine = GetCurrentLine();
    if (txed_auto_indent && e.GetKey() == '\n') {
		// Get indentation amount
        int lineInd = 0;
        if (currentLine > 0)
            lineInd = GetLineIndentation(currentLine - 1);

		// Do auto-indent if needed
        if (lineInd != 0) {
			SetLineIndentation(currentLine, lineInd);

			// Skip to end of tabs
			while (1) {
				int chr = GetCharAt(GetCurrentPos());
				if (chr == '\t' || chr == ' ')
					GotoPos(GetCurrentPos()+1);
				else
					break;
			}
		}
    }

	// Call tip
	if (e.GetKey() == '(') {
		openCalltip(GetCurrentPos());
	}

	// End call tip
	if (e.GetKey() == ')') {
		CallTipCancel();
	}

	// Comma, possibly update calltip
	if (e.GetKey() == ',') {
		if (CallTipActive())
			updateCalltip();
	}

	// Continue
	e.Skip();
}

void TextEditor::onUpdateUI(wxStyledTextEvent& e) {
	//checkBraceMatch();	// Disabled for now, wxSTC bug makes it not redraw properly

	// If a calltip is open, update it
	if (CallTipActive())
		updateCalltip();

	e.Skip();
}

void TextEditor::onCalltipClicked(wxStyledTextEvent& e) {
}

/* TextEditor::onFRDBtnFindNext
 * Called when the 'Find Next' button on the Find+Replace frame is
 * clicked
 *******************************************************************/
void TextEditor::onFRDBtnFindNext(wxCommandEvent& e) {
	// Check find string
	string find = dlg_fr->getFindString();
	if (find.IsEmpty())
		return;

	// Set search options
	int flags = 0;
	if (dlg_fr->matchCase()) flags |= wxSTC_FIND_MATCHCASE;
	if (dlg_fr->matchWord()) flags |= wxSTC_FIND_WHOLEWORD;
	SetSearchFlags(flags);

	// Do find
	if (!findNext(find))
		wxLogMessage(s_fmt("No text matching \"%s\" found.", chr(find)));
}

/* TextEditor::onFRDBtnReplace
 * Called when the 'Replace' button on the Find+Replace frame is
 * clicked
 *******************************************************************/
void TextEditor::onFRDBtnReplace(wxCommandEvent& e) {
	// Set search options
	int flags = 0;
	if (dlg_fr->matchCase()) flags |= wxSTC_FIND_MATCHCASE;
	if (dlg_fr->matchWord()) flags |= wxSTC_FIND_WHOLEWORD;
	SetSearchFlags(flags);

	// Do replace
	replaceCurrent(dlg_fr->getFindString(), dlg_fr->getReplaceString());
}

/* TextEditor::onFRDBtnReplaceAll
 * Called when the 'Replace All' button on the Find+Replace frame is
 * clicked
 *******************************************************************/
void TextEditor::onFRDBtnReplaceAll(wxCommandEvent& e) {
	// Set search options
	int flags = 0;
	if (dlg_fr->matchCase()) flags |= wxSTC_FIND_MATCHCASE;
	if (dlg_fr->matchWord()) flags |= wxSTC_FIND_WHOLEWORD;
	SetSearchFlags(flags);

	// Do replace all
	int replaced = replaceAll(dlg_fr->getFindString(), dlg_fr->getReplaceString());
	wxMessageBox(s_fmt("Replaced %d occurrences", replaced), "Replace All");
}
