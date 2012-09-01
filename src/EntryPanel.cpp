
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
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
#include "MainWindow.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, swap_epanel_bars, false, CVAR_SAVE)


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
	menu_custom = NULL;

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Create & set sizer & border
	frame = new wxStaticBox(this, -1, "Entry Contents");
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 1, wxEXPAND|wxALL, 4);
	Show(false);

	// Setup sizer positions
	sizer_top = new wxBoxSizer(wxHORIZONTAL);
	sizer_bottom = new wxBoxSizer(wxHORIZONTAL);
	sizer_main = new wxBoxSizer(wxVERTICAL);
	if (swap_epanel_bars) {
		framesizer->Add(sizer_bottom, 0, wxEXPAND|wxALL, 4);
		framesizer->Add(sizer_main, 1, wxEXPAND|wxLEFT|wxRIGHT, 4);
		framesizer->Add(sizer_top, 0, wxEXPAND|wxALL, 4);
	}
	else {
		framesizer->Add(sizer_top, 0, wxEXPAND|wxALL, 4);
		framesizer->Add(sizer_main, 1, wxEXPAND|wxLEFT|wxRIGHT, 4);
		framesizer->Add(sizer_bottom, 0, wxEXPAND|wxALL, 4);
	}

	// Create generic EntryPanel buttons
	btn_save = new wxButton(this, -1, "Save");
	btn_revert = new wxButton(this, -1, "Revert");
	btn_edit_ext = new wxButton(this, -1, "Edit Externally");

	sizer_top->Add(btn_save, 0, wxEXPAND|wxRIGHT, 4);
	sizer_top->Add(btn_revert, 0, wxEXPAND|wxRIGHT, 4);
	sizer_top->Add(btn_edit_ext, 0, wxEXPAND|wxRIGHT, 4);

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
	removeCustomMenu();
	removeCustomToolBar();
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
}

/* EntryPanel::openEntry
 * 'Opens' the given entry (sets the frame label then loads it)
 *******************************************************************/
bool EntryPanel::openEntry(ArchiveEntry* entry) {
	// Check entry was given
	if (!entry)
		return false;

	// Copy current entry content
	entry_data.clear();
	entry_data.importMem(entry->getData(true), entry->getSize());

	// Load the entry
	if (loadEntry(entry)) {
		this->entry = entry;
		updateStatus();
		return true;
	}
	else {
		theMainWindow->SetStatusText("", 1);
		theMainWindow->SetStatusText("", 2);
		return false;
	}
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

/* EntryPanel::refreshPanel
 * Redraws the panel
 *******************************************************************/
void EntryPanel::refreshPanel() {
	Update();
	Refresh();
}

/* EntryPanel::closeEntry
 * 'Closes' the current entry - clean up, save extra info, etc
 *******************************************************************/
void EntryPanel::closeEntry() {
}

/* EntryPanel::updateStatus
 * Updates the main window status bar with info about the current
 * entry
 *******************************************************************/
void EntryPanel::updateStatus() {
	// Basic info
	if (entry)
		theMainWindow->SetStatusText(S_FMT("%s, %d bytes, %s", CHR(entry->getName()), entry->getSize(), CHR(entry->getType()->getName())), 1);
	else
		theMainWindow->SetStatusText("", 1);

	// Extended info
	theMainWindow->SetStatusText(statusString(), 2);
}

/* EntryPanel::addCustomMenu
 * Adds this EntryPanel's custom menu to the main window menubar
 * (if it exists)
 *******************************************************************/
void EntryPanel::addCustomMenu() {
	if (menu_custom)
		theMainWindow->addCustomMenu(menu_custom, custom_menu_name);
}

/* EntryPanel::removeCustomMenu
 * Removes this EntryPanel's custom menu from the main window menubar
 *******************************************************************/
void EntryPanel::removeCustomMenu() {
	theMainWindow->removeCustomMenu(menu_custom);
}

/* EntryPanel::addCustonToolBar
 * Adds this EntryPanel's custom toolbar group to the main window
 * toolbar
 *******************************************************************/
void EntryPanel::addCustomToolBar() {
	// Check any custom actions exist
	if (custom_toolbar_actions.IsEmpty())
		return;

	// Split list of toolbar actions
	wxArrayString actions = wxSplit(custom_toolbar_actions, ';');

	// Add to main window
	theMainWindow->addCustomToolBar("Current Entry", actions);
}

/* EntryPanel::removeCustomMenu
 * Removes this EntryPanel's custom toolbar group from the main
 * window toolbar
 *******************************************************************/
void EntryPanel::removeCustomToolBar() {
	theMainWindow->removeCustomToolBar("Current Entry");
}


/* EntryPanel::isActivePanel
 * Returns true if the entry panel is the Archive Manager Panel's
 * current area. This is needed because the wx function IsShown()
 * is not enough, it will return true if the panel is shown on any
 * tab, even if it is not on the one that is selected...
 *******************************************************************/
bool EntryPanel::isActivePanel() {
	return (IsShown() && theActivePanel == this);
}

/*******************************************************************
 * ENTRYPANEL CLASS EVENTS
 *******************************************************************/

/* EntryPanel::onBtnSave
 * Called when the 'Save Changes' button is clicked
 *******************************************************************/
void EntryPanel::onBtnSave(wxCommandEvent& e) {
	if (modified) {
		if (saveEntry())
			modified = false;
	}
}

/* EntryPanel::onBtnRevert
 * Called when the 'Revert Changes' button is clicked
 *******************************************************************/
void EntryPanel::onBtnRevert(wxCommandEvent& e) {
	revertEntry();
}

/* EntryPanel::onBtnEditExt
 * Called when the 'Edit Externally' button is clicked
 *******************************************************************/
void EntryPanel::onBtnEditExt(wxCommandEvent& e) {
	wxLogMessage("External edit not implemented");
}
