
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
	SetTabWidth(4);

	// Line numbers by default
	SetMarginType(0, wxSTC_MARGIN_NUMBER);
	SetMarginWidth(0, TextWidth(wxSTC_STYLE_LINENUMBER, _T("9999")));
	SetMarginWidth(1, 4);

	// Test colours
	StyleSetForeground(wxSTC_C_COMMENT, wxColor(0, 150, 0));
	StyleSetForeground(wxSTC_C_COMMENTDOC, wxColor(0, 150, 0));
	StyleSetForeground(wxSTC_C_COMMENTLINE, wxColor(0, 150, 0));
	StyleSetForeground(wxSTC_C_STRING, wxColor(0, 120, 130));
	StyleSetForeground(wxSTC_C_CHARACTER, wxColor(0, 120, 130));

	// Temp
	SetLexer(wxSTC_LEX_CPP);

	Bind(wxEVT_STC_MODIFIED, &TextEditor::onModified, this);
	Bind(wxEVT_STC_CHANGE, &TextEditor::onTextChanged, this);
}

/* TextEditor::~TextEditor
 * TextEditor class destructor
 *******************************************************************/
TextEditor::~TextEditor() {
}

/* TextEditor::loadEntry
 * Reads the contents of <entry> into the text area, returns false
 * if the given entry is invalid
 *******************************************************************/
bool TextEditor::loadEntry(ArchiveEntry* entry) {
	// Clear current text
	ClearAll();

	// Check that the entry exists
	if (!entry) {
		Global::error = _T("Invalid archive entry given");
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

	return true;
}

/* TextEditor::getRawText
 * Writes the raw ASCII text to <mc>
 *******************************************************************/
void TextEditor::getRawText(MemChunk& mc) {
	mc.clear();
	const char* raw_text = GetTextRaw();
	mc.importMem((const uint8_t*)raw_text, GetTextLength());
	//mc.importMem((const uint8_t*)(chr(GetText())), GetText().size());
}

void TextEditor::onModified(wxStyledTextEvent& e) {
	e.Skip();
}

void TextEditor::onTextChanged(wxStyledTextEvent& e) {
	// Update line numbers margin width
	string numlines = s_fmt(_T("0%d"), GetNumberOfLines());
	SetMarginWidth(0, TextWidth(wxSTC_STYLE_LINENUMBER, numlines));
	e.Skip();
}

/* TextEditor::loadHexEntry
 * Temporary function, used just to quickly look at some lumps.
 * Should be completely yanked out and replaced by a real hex editor
 * as soon as possible. This merely displays hex as text!
 *******************************************************************/
bool TextEditor::loadHexEntry(ArchiveEntry* entry) {
	// Clear current text
	ClearAll();

	// Check that the entry exists
	if (!entry) {
		Global::error = _T("Invalid archive entry given");
		return false;
	}

	// Check that the entry has any data, if not do nothing
	if (entry->getSize() == 0 || !entry->getData())
		return true;

	// Compute total size of printed data:
	size_t size = (entry->getSize()) * 3;

	// Get character entry data
	char* data = new char[size];
	const uint8_t* rd = entry->getData();
	const uint8_t mask = 0x0F;
	char view = 0;

	// Convert hex into text. This is horribly ugly but I don't care at the moment.
	for (size_t i = 0; i < entry->getSize(); ++i)
	{
		for (size_t j = 0; j < 2; ++j)
		{
			view = (j ? rd[i] : rd[i] >> 4) & mask;
			view += (view < 10 ? '0' : 'A' - 10);
			data[i*3 + j] = view;
		}
		data[i*3 +2] = ((i+1)%16 ? ' ' : '\n');
	}
	data[size - 1] = '\0';

	// Load text into editor
	SetTextRaw(data);

	// Clean up
	delete[] data;

	return true;
}

