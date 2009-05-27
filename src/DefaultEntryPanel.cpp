
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    DefaultEntryPanel.cpp
 * Description: DefaultEntryPanel class. Used for entries that don't
 *              have their own specific editor, or entries of an
 *              unknown type. Has the option to open/edit the entry
 *              as text.
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


/* DefaultEntryPanel::DefaultEntryPanel
 * DefaultEntryPanel class constructor
 *******************************************************************/
DefaultEntryPanel::DefaultEntryPanel(wxWindow* parent)
: EntryPanel(parent) {
	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Entry Contents"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);

	Layout();
}

/* DefaultEntryPanel::~DefaultEntryPanel
 * DefaultEntryPanel class destructor
 *******************************************************************/
DefaultEntryPanel::~DefaultEntryPanel() {
}

/* DefaultEntryPanel::loadEntry
 * Loads entry info into the panel
 *******************************************************************/
bool DefaultEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Check that the entry exists
	if (!entry) {
		Global::error = _T("Invalid archive entry given");
		return false;
	}

	// Update variables
	this->entry = entry;

	return true;
}
