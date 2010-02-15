
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
: EntryPanel(parent) {
	// Create the text area
	text_area = new TextEditor(this, -1);
	sizer_main->Add(text_area, 1, wxEXPAND | wxALL, 4);

	// Bind events
	text_area->Bind(wxEVT_STC_MODIFIED, &TextEntryPanel::onTextModified, this);

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

	// Copy current entry content
	entry_data.clear();
	entry_data.importMem(entry->getData(true), entry->getSize());

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
	MemChunk raw_text;
	text_area->getRawText(raw_text);
	entry->importMemChunk(raw_text);

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
