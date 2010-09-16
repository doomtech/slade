
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    TextEntryPanel.cpp
 * Description: TextEntryPanel class. The UI for editing text entries.
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
#include "TextEntryPanel.h"


/*******************************************************************
 * TEXTENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* TextEntryPanel::TextEntryPanel
 * TextEntryPanel class constructor
 *******************************************************************/
TextEntryPanel::TextEntryPanel(wxWindow* parent)
: EntryPanel(parent, "text") {
	// Setup top bar sizer
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer_main->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Add 'Text Language' choice
	wxArrayString languages = TextLanguage::getLanguageNames();
	languages.Insert("None", 0, 1);
	choice_text_language = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, languages);
	choice_text_language->Select(0);
	hbox->Add(new wxStaticText(this, -1, "Text Language:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->Add(choice_text_language, 0, wxEXPAND|wxRIGHT, 4);


	// Create the text area
	text_area = new TextEditor(this, -1);
	sizer_main->Add(text_area, 1, wxEXPAND | wxALL, 4);


	// Add 'Find/Replace' button to bottom sizer
	sizer_bottom->AddStretchSpacer();
	btn_find_replace = new wxButton(this, -1, "Find + Replace");
	sizer_bottom->Add(btn_find_replace, 0, wxEXPAND, 4);


	// Bind events
	choice_text_language->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &TextEntryPanel::onChoiceLanguageChanged, this);
	text_area->Bind(wxEVT_STC_CHANGE, &TextEntryPanel::onTextModified, this);
	btn_find_replace->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextEntryPanel::onBtnFindReplace, this);

	Layout();
}

/* TextEntryPanel::~TextEntryPanel
 * TextEntryPanel class destructor
 *******************************************************************/
TextEntryPanel::~TextEntryPanel() {
}

/* TextEntryPanel::loadEntry
 * Loads an entry into the panel as text
 *******************************************************************/
bool TextEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Load entry into the text editor
	if (!text_area->loadEntry(entry))
		return false;

	// Enable save changes button depending on if the entry is locked
	if (entry->isLocked())
		btn_save->Enable(false);
	else
		btn_save->Enable(true);

	// Update variables
	this->entry = entry;
	setModified(false);

	return true;
}

/* TextEntryPanel::saveEntry
 * Saves any changes to the entry
 *******************************************************************/
bool TextEntryPanel::saveEntry() {
	// Write raw text to the entry
	wxCharBuffer text_raw = text_area->GetTextRaw();
	entry->importMem(text_raw, text_raw.length());

	// Re-detect entry type
	EntryType::detectEntryType(entry);

	// Update variables
	setModified(false);

	return true;
}


/*******************************************************************
 * TEXTENTRYPANEL CLASS EVENTS
 *******************************************************************/

/* TextEntryPanel::onTextModified
 * Called when the text in the TextEditor is modified
 *******************************************************************/
void TextEntryPanel::onTextModified(wxStyledTextEvent& e) {
	setModified();
}

void TextEntryPanel::onBtnFindReplace(wxCommandEvent& e) {
	text_area->showFindReplaceDialog();
}

void TextEntryPanel::onChoiceLanguageChanged(wxCommandEvent& e) {
	int index = choice_text_language->GetSelection();
	text_area->setLanguage(TextLanguage::getLanguage(index - 1));
}
