
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    PreferencesDialog.cpp
 * Description: The SLADE Preferences dialog. Brings together all the
 *              various preference panels in a single dialog
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
#include "PreferencesDialog.h"
#include "ArchiveManager.h"


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(Bool, wad_force_uppercase)
EXTERN_CVAR(Bool, iwad_lock)
EXTERN_CVAR(Bool, archive_load_data)
EXTERN_CVAR(Int, autosave_entry_changes)
EXTERN_CVAR(Bool, close_archive_with_tab)
EXTERN_CVAR(Bool, gl_tex_enable_np2)


/*******************************************************************
 * PREFERENCESDIALOG CLASS FUNCTIONS
 *******************************************************************/

/* PreferencesDialog::PreferencesDialog
 * PreferencesDialog class constructor
 *******************************************************************/
PreferencesDialog::PreferencesDialog(wxWindow* parent) : wxDialog(parent, -1, "SLADE Preferences") {
	// Setup main sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Setup preferences TreeBook
	tree_prefs = new wxTreebook(this, -1, wxDefaultPosition, wxDefaultSize);
	tree_prefs->AddPage(setupGeneralPrefsPanel(), "General", true);
	tree_prefs->AddPage(setupEditingPrefsPanel(), "Editing");
	tree_prefs->AddSubPage(setupBaseResourceArchivesPanel(), "Base Resource Archive");

	// Expand all tree nodes (so it gets sized properly)
	tree_prefs->ExpandNode(1);

	// Add preferences treebook
	sizer->Add(tree_prefs, 1, wxEXPAND|wxALL, 4);

	// Add buttons
	sizer->Add(CreateButtonSizer(wxOK), 0, wxEXPAND|wxALL, 4);

	// Setup layout
	SetInitialSize(wxSize(-1, 300));
	Layout();

	// Collapse all tree nodes
	tree_prefs->CollapseNode(1);
}

/* PreferencesDialog::~PreferencesDialog
 * PreferencesDialog class destructor
 *******************************************************************/
PreferencesDialog::~PreferencesDialog() {
}

/* PreferencesDialog::setupGeneralPrefsPanel
 * Creates the wxPanel containing 'General' preferences and returns
 * the panel
 *******************************************************************/
wxPanel* PreferencesDialog::setupGeneralPrefsPanel() {
	// Create panel
	wxPanel* panel = new wxPanel(tree_prefs, -1);
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(panel, -1, "General Preferences");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Load on open archive
	cb_archive_load = new wxCheckBox(panel, -1, "Load all archive entry data to memory when opened");
	sizer->Add(cb_archive_load, 0, wxEXPAND|wxALL, 4);

	// Close archive with tab
	cb_archive_close_tab = new wxCheckBox(panel, -1, "Close archive when it's tab is closed");
	sizer->Add(cb_archive_close_tab, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Enable np2 textures
	cb_gl_np2 = new wxCheckBox(panel, -1, "Enable Non-power-of-two textures if supported");
	sizer->Add(cb_gl_np2, 0, wxEXPAND|wxALL, 4);

	// Init controls
	cb_archive_load->SetValue(archive_load_data);
	cb_archive_close_tab->SetValue(close_archive_with_tab);
	cb_gl_np2->SetValue(gl_tex_enable_np2);

	// Bind events
	cb_archive_load->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::onCBArchiveLoadChanged, this);
	cb_archive_close_tab->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::onCBArchiveCloseTabChanged, this);
	cb_gl_np2->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::onCBGLNP2Changed, this);

	return panel;
}

/* PreferencesDialog::setupEditingPrefsPanel
 * Creates the wxPanel containing 'Editing' preferences and returns
 * the panel
 *******************************************************************/
wxPanel* PreferencesDialog::setupEditingPrefsPanel() {
	// Create panel
	wxPanel* panel = new wxPanel(tree_prefs, -1);
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(panel, -1, "Editing Preferences");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Force uppercase
	cb_wad_force_uppercase = new wxCheckBox(panel, -1, "Force uppercase entry names in Wad Archives");
	sizer->Add(cb_wad_force_uppercase, 0, wxEXPAND|wxALL, 4);

	// Lock IWAD
	cb_wad_lock_iwad = new wxCheckBox(panel, -1, "Lock all IWAD entries");
	sizer->Add(cb_wad_lock_iwad, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Unsaved entry changes
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxALL, 4);
	string choices[] = { "Don't Save", "Save", "Ask" };
	choice_entry_mod = new wxChoice(panel, -1, wxDefaultPosition, wxDefaultSize, 3, choices);
	hbox->Add(new wxStaticText(panel, -1, "Action on unsaved entry changes:"), 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->Add(choice_entry_mod, 0, wxEXPAND, 0);

	// Init controls
	cb_wad_force_uppercase->SetValue(wad_force_uppercase);
	cb_wad_lock_iwad->SetValue(iwad_lock);
	choice_entry_mod->SetSelection(autosave_entry_changes);

	// Bind events
	cb_wad_force_uppercase->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::onCBWadForceUppercaseChanged, this);
	cb_wad_lock_iwad->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &PreferencesDialog::onCBWadLockIWAD, this);
	choice_entry_mod->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &PreferencesDialog::onChoiceEntryModChanged, this);

	return panel;
}

/* PreferencesDialog::setupBaseResourceArchivesPanel
 * Creates the wxPanel containing the Base Resource Archives panel,
 * plus some extra stuff, and returns it
 *******************************************************************/
wxPanel* PreferencesDialog::setupBaseResourceArchivesPanel() {
	// Create panel
	wxPanel* panel = new wxPanel(this, -1);
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(panel, -1, "Base Resource Archive");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Add BRA panel
	panel_bra = new BaseResourceArchivesPanel(panel);
	sizer->Add(panel_bra, 1, wxEXPAND|wxALL, 4);

	// Add 'Open BRA' button
	btn_bra_open = new wxButton(panel, -1, "Open Selected Archive");
	sizer->Add(btn_bra_open, 0, wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Bind events
	btn_bra_open->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PreferencesDialog::onBtnBRAOpenClicked, this);

	return panel;
}


/*******************************************************************
 * PREFERENCESDIALOG CLASS EVENTS
 *******************************************************************/

/* PreferencesDialog::onCBArchiveLoadChanged
 * Called when the 'Load Archive Data' checkbox is changed
 *******************************************************************/
void PreferencesDialog::onCBArchiveLoadChanged(wxCommandEvent& e) {
	archive_load_data = cb_archive_load->GetValue();
}

/* PreferencesDialog::onCBArchiveCloseTabChanged
 * Called when the 'Close Archive with Tab' checkbox is changed
 *******************************************************************/
void PreferencesDialog::onCBArchiveCloseTabChanged(wxCommandEvent& e) {
	close_archive_with_tab = cb_archive_close_tab->GetValue();
}

/* PreferencesDialog::onCBGLNP2Changed
 * Called when the 'Enable NP2 Textures' checkbox is changed
 *******************************************************************/
void PreferencesDialog::onCBGLNP2Changed(wxCommandEvent& e) {
	gl_tex_enable_np2 = cb_gl_np2->GetValue();
}

/* PreferencesDialog::onCBWadForceUppercaseChanged
 * Called when the 'Force Uppercase Entry Names' checkbox is changed
 *******************************************************************/
void PreferencesDialog::onCBWadForceUppercaseChanged(wxCommandEvent& e) {
	wad_force_uppercase = cb_wad_force_uppercase->GetValue();
}

/* PreferencesDialog::onCBWadLockIWAD
 * Called when the 'Lock IWAD Entries' checkbox is changed
 *******************************************************************/
void PreferencesDialog::onCBWadLockIWAD(wxCommandEvent& e) {
	iwad_lock = cb_wad_lock_iwad->GetValue();
}

/* PreferencesDialog::onChoiceEntryModChanged
 * Called when the 'Unsaved Entry Changes' choice is changed
 *******************************************************************/
void PreferencesDialog::onChoiceEntryModChanged(wxCommandEvent& e) {
	autosave_entry_changes = choice_entry_mod->GetSelection();
}

/* PreferencesDialog::onBtnBRAOpenClicked
 * Called when the 'Open Selected BRA' button is clicked
 *******************************************************************/
void PreferencesDialog::onBtnBRAOpenClicked(wxCommandEvent& e) {
	theArchiveManager->openBaseResource(panel_bra->getSelectedPath());
}
