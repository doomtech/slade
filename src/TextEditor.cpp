
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
	wxFont f(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	StyleSetFont(wxSTC_STYLE_DEFAULT, f);
	SetTabWidth(4);

	Bind(wxEVT_STC_MODIFIED, &TextEditor::onModified, this);
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
	SetText(wxEmptyString);

	// Check that the entry exists
	if (!entry) {
		Global::error = _T("Invalid archive entry given");
		return false;
	}

	// Check that the entry has any data, if not do nothing
	if (entry->getSize() == 0)
		return true;

	// Get character entry data
	const char* data = (const char*)entry->getData();
	if (!data) {
		Global::error = _T("Cannot read entry data (see logfile for info)");
		return false;
	}

	// Load it into the text area
	string istr = wxString::FromAscii(data);
	istr.Truncate(entry->getSize());

	// Add text to the text area
	SetText(istr);

	return true;
}

/* TextEditor::getRawText
 * Writes the raw ASCII text to <mc>
 *******************************************************************/
void TextEditor::getRawText(MemChunk& mc) {
	mc.clear();
	mc.importMem((const uint8_t*)(chr(GetText())), GetText().size());
}

void TextEditor::onModified(wxStyledTextEvent& e) {
	e.Skip();
}
