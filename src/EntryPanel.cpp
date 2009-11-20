
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    EntryPanel.cpp
 * Description: EntryPanel class. Different UI panels for editing
 *              different entry types extend from this class.
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

/* EntryPanel::EntryPanel
 * EntryPanel class constructor
 *******************************************************************/
EntryPanel::EntryPanel(wxWindow* parent)
: wxPanel(parent, -1) {
	// Create & set sizer & border
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Entry Contents"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);
	Show(false);

	changed = false;
}

/* EntryPanel::~EntryPanel
 * EntryPanel class destructor
 *******************************************************************/
EntryPanel::~EntryPanel() {
}

/* EntryPanel::loadEntry
 * Loads an entry into the entry panel (does nothing here, to be
 * overridden by child classes)
 *******************************************************************/
bool EntryPanel::loadEntry(ArchiveEntry* entry) {
	Global::error = _T("Cannot open an entry with the base EntryPanel class");
	return false;
}

/* EntryPanel::saveEntry
 * Saves the entrypanel content to the entry (does nothing here, to
 * be overridden by child classes)
 *******************************************************************/
bool EntryPanel::saveEntry() {
	Global::error = _T("Cannot save an entry with the base EntryPanel class");
	return false;
}
