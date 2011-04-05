
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
 * VARIABLES
 *******************************************************************/
CVAR(Bool, txed_trim_whitespace, false, CVAR_SAVE)
wxArrayString languages;


/*******************************************************************
 * TEXTENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* TextEntryPanel::TextEntryPanel
 * TextEntryPanel class constructor
 *******************************************************************/
TextEntryPanel::TextEntryPanel(wxWindow* parent)
: EntryPanel(parent, "text") {
	// Create the text area
	text_area = new TextEditor(this, -1);
	sizer_main->Add(text_area, 1, wxEXPAND, 0);

	// Add 'Text Language' choice
	languages = TextLanguage::getLanguageNames();
	languages.Sort();
	languages.Insert("None", 0, 1);
	choice_text_language = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, languages);
	choice_text_language->Select(0);
	sizer_bottom->Add(new wxStaticText(this, -1, "Text Language:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	sizer_bottom->Add(choice_text_language, 0, wxEXPAND);


	// Add 'Find/Replace' button to top sizer
	sizer_top->AddStretchSpacer();
	btn_find_replace = new wxButton(this, -1, "Find + Replace");
	sizer_top->Add(btn_find_replace, 0, wxEXPAND, 0);


	// Bind events
	choice_text_language->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &TextEntryPanel::onChoiceLanguageChanged, this);
	text_area->Bind(wxEVT_STC_CHANGE, &TextEntryPanel::onTextModified, this);
	btn_find_replace->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextEntryPanel::onBtnFindReplace, this);
	text_area->Bind(wxEVT_STC_UPDATEUI, &TextEntryPanel::onUpdateUI, this);

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

	// Scroll to previous position (if any)
	if (entry->exProps().propertyExists("TextPosition"))
		text_area->GotoPos((int)(entry->exProp("TextPosition")));

	// --- Attempt to determine text language ---
	TextLanguage* tl = NULL;

	// From entry language hint
	if (entry->exProps().propertyExists("TextLanguage")) {
		string lang_id = entry->exProp("TextLanguage");
		tl = TextLanguage::getLanguage(lang_id);
	}

	// Or, from entry type
	if (!tl && entry->getType()->extraProps().propertyExists("text_language")) {
		string lang_id = entry->getType()->extraProps()["text_language"];
		tl = TextLanguage::getLanguage(lang_id);
	}

	// Or, from entry's parent directory
	if (!tl) {
		// ZDoom DECORATE (within 'actors' or 'decorate' directories)
		if (S_CMPNOCASE(wxString("/actors/"), entry->getPath().Left(8)) ||
			S_CMPNOCASE(wxString("/decorate/"), entry->getPath().Left(10)))
			tl = TextLanguage::getLanguage("decorate");
	}

	// Load language
	text_area->setLanguage(tl);

	// Select it in the choice box
	if (tl) {
		for (unsigned a = 0; a < languages.size(); a++) {
			if (S_CMPNOCASE(tl->getName(), languages[a])) {
				choice_text_language->Select(a);
				break;
			}
		}
	}
	else
		choice_text_language->Select(0);


	// Update variables
	this->entry = entry;
	setModified(false);

	return true;
}

/* TextEntryPanel::saveEntry
 * Saves any changes to the entry
 *******************************************************************/
bool TextEntryPanel::saveEntry() {
	// Trim whitespace
	if (txed_trim_whitespace)
		text_area->trimWhitespace();

	// Write raw text to the entry
	wxCharBuffer text_raw = text_area->GetTextRaw();
	entry->importMem(text_raw, text_raw.length());

	// Re-detect entry type
	EntryType::detectEntryType(entry);

	// Update variables
	setModified(false);

	return true;
}

/* TextEntryPanel::refreshPanel
 * Updates the text editor options and redraws it
 *******************************************************************/
void TextEntryPanel::refreshPanel() {
	// Update text editor
	text_area->setup();

	Refresh();
	Update();
}

/* TextEntryPanel::closeEntry
 * Performs any actions required on closing the entry
 *******************************************************************/
void TextEntryPanel::closeEntry() {
	// Check any entry is open
	if (!entry)
		return;

	// Save current caret position
	entry->exProp("TextPosition") = text_area->GetCurrentPos();
}

/* TextEntryPanel::statusString
 * Returns a string with extended editing/entry info for the status
 * bar
 *******************************************************************/
string TextEntryPanel::statusString() {
	// Setup status string
	int line = text_area->GetCurrentLine()+1;
	int pos = text_area->GetCurrentPos();
	int col = text_area->GetColumn(pos)+1;
	string status = S_FMT("Ln %d, Col %d, Pos %d", line, col, pos);

	return status;
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

/* TextEntryPanel::onBtnFindReplace
 * Called when the 'Find+Replace' button is clicked
 *******************************************************************/
void TextEntryPanel::onBtnFindReplace(wxCommandEvent& e) {
	text_area->showFindReplaceDialog();
}

/* TextEntryPanel::onChoiceLanguageChanged
 * Called when the language in the dropdown is changed
 *******************************************************************/
void TextEntryPanel::onChoiceLanguageChanged(wxCommandEvent& e) {
	int index = choice_text_language->GetSelection();
	// Get selected language
	TextLanguage* tl = TextLanguage::getLanguageByName(choice_text_language->GetStringSelection());

	// Set text editor language
	text_area->setLanguage(tl);

	// Set entry language hint
	if (tl)
		entry->exProp("TextLanguage") = tl->getId();
	else
		entry->exProps().removeProperty("TextLanguage");
}

void TextEntryPanel::onUpdateUI(wxStyledTextEvent& e) {
	updateStatus();
	e.Skip();
}
