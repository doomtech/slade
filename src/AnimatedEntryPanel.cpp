
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    AnimatedEntryPanel.cpp
 * Description: AnimatedEntryPanel class. The UI for editing Boom
 *              ANIMATED lumps.
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
#include "AnimatedEntryPanel.h"
#include "Archive.h"
#include "ArchiveManager.h"
#include "Misc.h"
#include <wx/listctrl.h>


/*******************************************************************
 * ANIMATEDENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* AnimatedEntryPanel::TextureXEntryPanel
 * AnimatedEntryPanel class constructor
 *******************************************************************/
AnimatedEntryPanel::AnimatedEntryPanel(wxWindow* parent)
: EntryPanel(parent, "animated") {
	// Setup panel sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer_main->Add(sizer, 1, wxEXPAND, 0);

	// Add entry list
	wxStaticBox* frame = new wxStaticBox(this, -1, "Animations");
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	list_entries = new ListView(this, -1);
	list_entries->showIcons(false);
	framesizer->Add(list_entries, 1, wxEXPAND|wxALL, 4);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// Add editing controls
	frame = new wxStaticBox(this, -1, "Selection");
	framesizer = new wxStaticBoxSizer(frame, wxHORIZONTAL);

	wxStaticBox* textframe;
	wxStaticBoxSizer* textframesizer;

	textframe = new wxStaticBox(this, -1, "First frame");
	textframesizer = new wxStaticBoxSizer(textframe, wxHORIZONTAL);
	text_firstname = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(80, -1));
	textframesizer->Add(text_firstname, 1, wxTILE, 4);
	framesizer->Add(textframesizer, 1, wxTILE, 4);

	textframe = new wxStaticBox(this, -1, "Last frame");
	textframesizer = new wxStaticBoxSizer(textframe, wxHORIZONTAL);
	text_lastname = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(80, -1));
	textframesizer->Add(text_lastname, 1, wxTILE, 4);
	framesizer->Add(textframesizer, 1, wxTILE, 4);

	textframe = new wxStaticBox(this, -1, "Speed");
	textframesizer = new wxStaticBoxSizer(textframe, wxHORIZONTAL);
	text_speed = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(80, -1));
	textframesizer->Add(text_speed, 1, wxTILE, 4);
	framesizer->Add(textframesizer, 1, wxTILE, 4);

/*	wxRadioButton*	rbtn_flat;
	wxRadioButton*	rbtn_texture;
	wxCheckBox*		cbox_decals;
	wxCheckBox*		cbox_swirl;*/
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	Layout();
}

/* AnimatedEntryPanel::~AnimatedEntryPanel
 * AnimatedEntryPanel class destructor
 *******************************************************************/
AnimatedEntryPanel::~AnimatedEntryPanel() {
}

/* AnimatedEntryPanel::loadEntry
 * Loads an entry into the ANIMATED entry panel
 *******************************************************************/
bool AnimatedEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Do nothing if entry is already open
	if (this->entry == entry)
		return true;

	// Read ANIMATED entry into texturexlist
	animated.readANIMATEDData(entry);

	// Update variables
	this->entry = entry;
	setModified(false);

	// Refresh controls
	populateEntryList();
	Layout();

	return true;
}

/* AnimatedEntryPanel::saveEntry
 * Saves any changes made to the entry
 *******************************************************************/
bool AnimatedEntryPanel::saveEntry() {
	return false;
}

/* AnimatedEntryPanel::populateEntryList
 * Clears and adds all entries to the entry list
 *******************************************************************/
void AnimatedEntryPanel::populateEntryList() {
	// Clear current list
	list_entries->ClearAll();

	// Add columns
	list_entries->InsertColumn(0, "Type");
	list_entries->InsertColumn(1, "Fist frame");
	list_entries->InsertColumn(2, "Last frame");
	list_entries->InsertColumn(3, "Speed");
	list_entries->InsertColumn(4, "Decals");

	// Add each animation to the list
	list_entries->enableSizeUpdate(false);
	for (uint32_t a = 0; a < animated.nEntries(); a++) {
		AnimatedEntry* ent = animated.getEntry(a);
		string cols[] = { ent->getType() ? "Texture" : "Flat",
			ent->getFirst(), ent->getLast(),
			ent->getSpeed() < 65535 ? s_fmt("%d tics", ent->getSpeed()) : "Swirling",
			ent->getDecals()? "Allowed" : " " };
		list_entries->addItem(a, wxArrayString(5, cols));
	}

	// Update list width
	list_entries->enableSizeUpdate(true);
	list_entries->updateSize();
}

/* TODO, MAYBE:
 * Allow in-place editing in the list?
 * Add a preview window displaying the texture or flat cycling at the proper speed?
 * Add a "Convert to ANIMDEFS" button?
 * panel_archivemanager->newEntry();
 */
