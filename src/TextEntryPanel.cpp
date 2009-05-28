
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
#include "EntryPanel.h"


/* TextEntryPanel::TextEntryPanel
 * TextEntryPanel class constructor
 *******************************************************************/
TextEntryPanel::TextEntryPanel(wxWindow* parent)
: EntryPanel(parent) {
	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Entry Contents"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);

	// Create the text area
	text_area = new TextEditor(this, -1);
	framesizer->Add(text_area, 1, wxEXPAND | wxALL, 4);

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
	/*
	// Check that the entry exists
	if (!entry) {
		Global::error = _T("Invalid archive entry given");
		return false;
	}

	// Check that the entry has any data, if not do nothing
	if (entry->getSize() == 0)
		return true;

	// Get character entry data
	char* data = (char*)entry->getData();
	if (!data) {
		Global::error = _T("Cannot read entry data (see logfile for info)");
		return false;
	}

	// Load it into the text area
	string istr = wxString::FromAscii(data);
	istr.Truncate(entry->getSize());

	// Add text to the text area
	text_area->SetText(istr);
	 **/

	// Load entry into the text editor
	if (!text_area->loadEntry(entry))
		return false;

	// Update variables
	this->entry = entry;

	return true;
}
