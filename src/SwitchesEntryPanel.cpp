
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    SwitchesEntryPanel.cpp
 * Description: SwitchesEntryPanel class. The UI for editing Boom
 *              SWITCHES lumps.
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
#include "SwitchesEntryPanel.h"
#include "Archive.h"
#include "ArchiveManager.h"
#include "Misc.h"
#include <wx/listctrl.h>


/*******************************************************************
 * TEXTENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* SwitchesEntryPanel::SwitchesEntryPanel
 * SwitchesEntryPanel class constructor
 *******************************************************************/
SwitchesEntryPanel::SwitchesEntryPanel(wxWindow* parent)
: EntryPanel(parent, _T("switches")) {
	// Setup panel sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer_main->Add(sizer, 1, wxEXPAND, 0);
	
	// Add editing controls
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer_bottom->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
 
	// Add entry list
	wxStaticBox* frame = new wxStaticBox(this, -1, _T("Switches"));
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	list_entries = new ListView(this, -1);
	list_entries->showIcons(false);
	framesizer->Add(list_entries, 1, wxEXPAND|wxALL, 4);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	Layout();
}

/* SwitchesEntryPanel::~SwitchesEntryPanel
 * SwitchesEntryPanel class destructor
 *******************************************************************/
SwitchesEntryPanel::~SwitchesEntryPanel() {
}

/* SwitchesEntryPanel::loadEntry
 * Loads an entry into the SWITCHES entry panel
 *******************************************************************/
bool SwitchesEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Do nothing if entry is already open
	if (this->entry == entry)
		return true;

	// Read SWITCHES entry into list
	switches.readSWITCHESData(entry);

	// Update variables
	this->entry = entry;
	setModified(false);

	// Refresh controls
	populateEntryList();
	Layout();

	return true;
}

/* SwitchesEntryPanel::saveEntry
 * Saves any changes made to the entry
 *******************************************************************/
bool SwitchesEntryPanel::saveEntry() {
	return false;
}

/* SwitchesEntryPanel::populateEntryList
 * Clears and adds all entries to the entry list
 *******************************************************************/
void SwitchesEntryPanel::populateEntryList() {
	// Clear current list
	list_entries->ClearAll();

	// Add columns
	list_entries->InsertColumn(0, _T("Off Texture"));
	list_entries->InsertColumn(1, _T("On Texture"));
	list_entries->InsertColumn(2, _T("Range"));

	// Add each switch to the list
	list_entries->enableSizeUpdate(false);
	for (uint32_t a = 0; a < switches.nEntries(); a++) {
		SwitchesEntry* ent = switches.getEntry(a);
		string cols[] = { ent->getOff(), ent->getOn(), 
			ent->getType() >= SWCH_COMM ? _T("Commercial")
			: ent->getType() == SWCH_FULL ? _T("Registered")
			: ent->getType() == SWCH_DEMO ? _T("Shareware")
			: _T("BugBugBug") };
		list_entries->addItem(a, wxArrayString(3, cols));
	}

	// Update list width
	list_entries->enableSizeUpdate(true);
	list_entries->updateSize();
}

/* TODO, MAYBE:
 * Allow in-place editing in the list?
 * Add a preview window displaying the texture toggling back and forth?
 * Add a "Convert to ANIMDEFS" button?
 */