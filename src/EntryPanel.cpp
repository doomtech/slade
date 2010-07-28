
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


/*******************************************************************
 * ENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* EntryPanel::EntryPanel
 * EntryPanel class constructor
 *******************************************************************/
EntryPanel::EntryPanel(wxWindow* parent, string id)
: wxPanel(parent, -1) {
	// Init variables
	modified = false;
	entry = NULL;
	this->id = id;

	// Create & set sizer & border
	frame = new wxStaticBox(this, -1, "Entry Contents");
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	SetSizer(framesizer);
	Show(false);

	sizer_main = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(sizer_main, 1, wxEXPAND|wxALL, 4);

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	framesizer->Add(hbox, 0, wxEXPAND, 0);

	sizer_bottom = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(sizer_bottom, 1, wxEXPAND|wxBOTTOM, 4);

	// Create generic EntryPanel buttons
	btn_save = new wxButton(this, -1, "Save Changes");
	btn_revert = new wxButton(this, -1, "Revert Changes");
	btn_edit_ext = new wxButton(this, -1, "Edit Externally");

	hbox->Add(btn_edit_ext, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
	hbox->Add(btn_revert, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
	hbox->Add(btn_save, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Bind button events
	btn_save->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &EntryPanel::onBtnSave, this);
	btn_revert->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &EntryPanel::onBtnRevert, this);
	btn_edit_ext->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &EntryPanel::onBtnEditExt, this);

	// Disable unimplemented buttons
	btn_edit_ext->Show(false);
}

/* EntryPanel::~EntryPanel
 * EntryPanel class destructor
 *******************************************************************/
EntryPanel::~EntryPanel() {
}

/* EntryPanel::setModified
 * Sets the modified flag. If the entry is locked modified will
 * always be false
 *******************************************************************/
void EntryPanel::setModified(bool c) {
	if (!entry) {
		modified = c;
		return;
	}
	else {
		if (entry->isLocked())
			modified = false;
		else
			modified = c;
	}

	// Set frame label
	//if (modified)
	//	frame->SetLabel(s_fmt("Entry Contents (%s, unsaved changes)", entry->getName().c_str()));
	//else
	//	frame->SetLabel(s_fmt("Entry Contents (%s)", entry->getName().c_str()));
}

/* EntryPanel::openEntry
 * 'Opens' the given entry (sets the frame label then loads it)
 *******************************************************************/
bool EntryPanel::openEntry(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Set frame label
	//frame->SetLabel(s_fmt("Entry Contents (%s)", entry->getName().c_str()));

	// Copy current entry content
	entry_data.clear();
	entry_data.importMem(entry->getData(true), entry->getSize());

	// Load the entry
	if (loadEntry(entry)) {
		this->entry = entry;
		return true;
	}
	else
		return false;
}

/* EntryPanel::loadEntry
 * Loads an entry into the entry panel (does nothing here, to be
 * overridden by child classes)
 *******************************************************************/
bool EntryPanel::loadEntry(ArchiveEntry* entry) {
	Global::error = "Cannot open an entry with the base EntryPanel class";
	return false;
}

/* EntryPanel::saveEntry
 * Saves the entrypanel content to the entry (does nothing here, to
 * be overridden by child classes)
 *******************************************************************/
bool EntryPanel::saveEntry() {
	Global::error = "Cannot save an entry with the base EntryPanel class";
	return false;
}

/* EntryPanel::revertEntry
 * Reverts any changes made to the entry since it was loaded into
 * the EntryPanel. Returns false if no changes have been made or
 * if the entry data wasn't saved
 *******************************************************************/
bool EntryPanel::revertEntry() {
	if (modified) {
		if (entry_data.hasData()) {
			entry->importMemChunk(entry_data);
			EntryType::detectEntryType(entry);
			loadEntry(entry);

			return true;
		}
	}

	return false;
}


void EntryPanel::onBtnSave(wxCommandEvent& e) {
	if (modified) {
		if (saveEntry()) {
			modified = false;
			EntryType::detectEntryType(entry);
		}
	}
}

void EntryPanel::onBtnRevert(wxCommandEvent& e) {
	revertEntry();
}

void EntryPanel::onBtnEditExt(wxCommandEvent& e) {
	wxLogMessage("External edit not implemented");
}
