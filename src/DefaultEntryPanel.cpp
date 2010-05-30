
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
#include "DefaultEntryPanel.h"


/*******************************************************************
 * DEFAULTENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* DefaultEntryPanel::DefaultEntryPanel
 * DefaultEntryPanel class constructor
 *******************************************************************/
DefaultEntryPanel::DefaultEntryPanel(wxWindow* parent)
: EntryPanel(parent, "default") {
	// Create widgets
	label_type = new wxStaticText(this, -1, "Entry Type:");
	label_size = new wxStaticText(this, -1, "Entry Size:");
	btn_edit_text = new wxButton(this, -1, "Edit as Text");
	btn_view_hex = new wxButton(this, -1, "View as Hex");
	text_area = new TextEditor(this, -1);

	// Bind Events
	btn_edit_text->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DefaultEntryPanel::onEditTextClicked, this);
	btn_view_hex->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DefaultEntryPanel::onViewHexClicked, this);
	text_area->Bind(wxEVT_STC_MODIFIED, &DefaultEntryPanel::onTextModified, this);

	// Show entry info stuff
	view_text = true;
	showEntryInfo(true);
}

/* DefaultEntryPanel::~DefaultEntryPanel
 * DefaultEntryPanel class destructor
 *******************************************************************/
DefaultEntryPanel::~DefaultEntryPanel() {
}

/* DefaultEntryPanel::showEntryInfo
 * Shows entry info stuff on the panel
 *******************************************************************/
void DefaultEntryPanel::showEntryInfo(bool show_btn_edittext) {
	// Hide the text editor
	text_area->Show(false);

	// Clear the sizer
	sizer_main->Clear(false);

	// Show entry info stuff
	label_type->Show(true);
	label_size->Show(true);
	btn_edit_text->Show(show_btn_edittext);
	btn_view_hex->Show(show_btn_edittext);

	// Add entry info stuff to the panel sizer
	sizer_main->AddStretchSpacer();
	sizer_main->Add(label_type, 0, wxALIGN_CENTER|wxALL, 4);
	sizer_main->Add(label_size, 0, wxALIGN_CENTER|wxALL, 4);
	sizer_main->AddSpacer(8);
	sizer_main->Add(btn_edit_text, 0, wxALIGN_CENTER|wxALL, 4);
	sizer_main->Add(btn_view_hex, 0, wxALIGN_CENTER|wxALL, 4);
	sizer_main->AddStretchSpacer();

	// Update variables etc
	view_text = false;

	Layout();
}

/* DefaultEntryPanel::openTextEntry
 * Open an entry and show it on the text editor panel
 *******************************************************************/
void DefaultEntryPanel::openTextEntry(ArchiveEntry * text_entry) {
	showTextEditor();
	text_area->loadEntry(text_entry);
	setModified(false);
}

/* DefaultEntryPanel::showTextEditor
 * Shows the text editor on the panel
 *******************************************************************/
void DefaultEntryPanel::showTextEditor() {
	// Hide entry info stuff
	label_type->Show(false);
	label_size->Show(false);
	btn_edit_text->Show(false);
	btn_view_hex->Show(false);

	// Clear the sizer
	sizer_main->Clear(false);

	// Show text editor
	text_area->Show(true);

	// Add text editor to the panel sizer
	sizer_main->Add(text_area, 1, wxEXPAND|wxALL, 4);

	// Update variables etc
	view_text = true;

	Layout();

}

/* DefaultEntryPanel::loadEntry
 * Loads entry info into the panel
 *******************************************************************/
bool DefaultEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Check that the entry exists
	if (!entry) {
		Global::error = "Invalid archive entry given";
		return false;
	}

	// Set labels
	label_type->SetLabel(s_fmt("Entry Type: %s", entry->getTypeString().c_str()));
	label_size->SetLabel(s_fmt("Entry Size: %d bytes", entry->getSize()));

	// Update variables
	this->entry = entry;
	setModified(false);

	// Check whether to show the 'edit as text' button
	bool show_btn_edittext = true;
	if (entry->getType() == EntryType::folderType())
		show_btn_edittext = false;

	// Show entry info stuff
	showEntryInfo(show_btn_edittext);

	// Enable save changes button depending on if the entry is locked
	if (entry->isLocked())
		btn_save->Enable(false);
	else
		btn_save->Enable(true);

	return true;
}

/* DefaultEntryPanel::saveEntry
 * Saves any changes to the entry
 *******************************************************************/
bool DefaultEntryPanel::saveEntry() {
	// Write raw text to the entry
	wxCharBuffer text_raw = text_area->GetTextRaw();
	entry->importMem(text_raw, text_raw.length());

	// Set entry type to text
	entry->setType(EntryType::getType("text"));
	entry->setState(1);

	return true;
}


/*******************************************************************
 * DEFAULTENTRYPANEL CLASS EVENTS
 *******************************************************************/

/* DefaultEntryPanel::onEditTextClicked
 * Called when the 'Edit as Text' button is clicked. Shows the text
 * editor on the panel and loads entry contents into it
 *******************************************************************/
void DefaultEntryPanel::onEditTextClicked(wxCommandEvent& event) {
	// Show the text editor
	showTextEditor();

	// Enable saving (in case it was disabled earlier)
	btn_save->Enable();

	// Load entry data into the text editor
	text_area->loadEntry(entry);
	setModified(false);
}

/* DefaultEntryPanel::onTextModified
 * Called when the text in the TextEditor is modified
 *******************************************************************/
void DefaultEntryPanel::onTextModified(wxStyledTextEvent& e) {
	if (btn_save->IsEnabled())
		setModified();
}

void DefaultEntryPanel::onViewHexClicked(wxCommandEvent& event) {
	// Show the text editor
	showTextEditor();

	// Disable saving (it's unsafe!)
	btn_save->Disable();

	// Load entry data into the text editor
	text_area->loadHexEntry(entry);
	setModified(false);
}
