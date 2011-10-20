
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
#include "BaseResourceArchivesPanel.h"
#include "TextEditorPrefsPanel.h"
#include "TextStylePrefsPanel.h"
#include "GeneralPrefsPanel.h"
#include "InterfacePrefsPanel.h"
#include "EditingPrefsPanel.h"
#include "ACSPrefsPanel.h"
#include "GraphicsPrefsPanel.h"
#include "PNGPrefsPanel.h"
#include "AudioPrefsPanel.h"
#include "ColourPrefsPanel.h"
#include "MapEditorPrefsPanel.h"
#include "MapDisplayPrefsPanel.h"
#include "AdvancedPrefsPanel.h"
#include "ArchiveManager.h"
#include "TextEditorPrefsPanel.h"
#include "Icons.h"
#include <wx/gbsizer.h>


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

	// Set dialog icon
	wxIcon icon;
	icon.CopyFromBitmap(getIcon("t_settings"));
	SetIcon(icon);

	// Create preferences TreeBook
	tree_prefs = new wxTreebook(this, -1, wxDefaultPosition, wxDefaultSize);

	// Create separate preferences panels
	panel_general = new GeneralPrefsPanel(tree_prefs);
	panel_interface = new InterfacePrefsPanel(tree_prefs);
	panel_editing = new EditingPrefsPanel(tree_prefs);
	panel_text_editor = new TextEditorPrefsPanel(tree_prefs);
	panel_text_styles = new TextStylePrefsPanel(tree_prefs);
	panel_script_acs = new ACSPrefsPanel(tree_prefs);
	panel_gfx_prefs = new GraphicsPrefsPanel(tree_prefs);
	panel_gfx_png = new PNGPrefsPanel(tree_prefs);
	panel_audio = new AudioPrefsPanel(tree_prefs);
	panel_colours = new ColourPrefsPanel(tree_prefs);
	panel_maped = new MapEditorPrefsPanel(tree_prefs);
	panel_map_display = new MapDisplayPrefsPanel(tree_prefs);
	panel_advanced = new AdvancedPrefsPanel(tree_prefs);

	// Setup preferences TreeBook
	tree_prefs->AddPage(panel_general, "General", true);
	tree_prefs->AddPage(panel_interface, "Interface");
	tree_prefs->AddSubPage(panel_colours, "Colours");
	tree_prefs->AddPage(panel_editing, "Editing");
	tree_prefs->AddSubPage(setupBaseResourceArchivesPanel(), "Base Resource Archive");
	tree_prefs->AddPage(panel_text_editor, "Text Editor");
	tree_prefs->AddSubPage(panel_text_styles, "Fonts & Colours");
	tree_prefs->AddPage(panel_gfx_prefs, "Graphics");
	tree_prefs->AddSubPage(panel_gfx_png, "PNG");
	tree_prefs->AddPage(panel_audio, "Audio");
	tree_prefs->AddPage(new wxPanel(tree_prefs, -1), "Scripting");
	tree_prefs->AddSubPage(panel_script_acs, "ACS");
	tree_prefs->AddPage(panel_maped, "Map Editor");
	tree_prefs->AddSubPage(panel_map_display, "Display");
	tree_prefs->AddPage(panel_advanced, "Advanced");

	// Expand all tree nodes (so it gets sized properly)
	tree_prefs->ExpandNode(2);

	// Add preferences treebook
	sizer->Add(tree_prefs, 1, wxEXPAND|wxALL, 4);

	// Add buttons
	sizer->Add(CreateButtonSizer(wxOK|wxCANCEL|wxAPPLY), 0, wxEXPAND|wxALL, 4);

	// Bind events
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PreferencesDialog::onButtonClicked, this);

	// Setup layout
	SetInitialSize(wxSize(-1, -1));
	Layout();

	// Collapse all tree nodes
	tree_prefs->CollapseNode(2);
}

/* PreferencesDialog::~PreferencesDialog
 * PreferencesDialog class destructor
 *******************************************************************/
PreferencesDialog::~PreferencesDialog() {
}

/* PreferencesDialog::setupBaseResourceArchivesPanel
 * Creates the wxPanel containing the Base Resource Archives panel,
 * plus some extra stuff, and returns it
 *******************************************************************/
wxPanel* PreferencesDialog::setupBaseResourceArchivesPanel() {
	// Create panel
	wxPanel* panel = new wxPanel(tree_prefs, -1);
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

/* PreferencesDialog::applyPreferences
 * Applies preference values from all preference panels
 *******************************************************************/
void PreferencesDialog::applyPreferences() {
	panel_general->applyPreferences();
	panel_interface->applyPreferences();
	panel_editing->applyPreferences();
	panel_text_editor->applyPreferences();
	panel_text_styles->applyPreferences();
	panel_script_acs->applyPreferences();
	panel_gfx_prefs->applyPreferences();
	panel_gfx_png->applyPreferences();
	panel_audio->applyPreferences();
	panel_colours->applyPreferences();
	panel_maped->applyPreferences();
	panel_map_display->applyPreferences();
	panel_advanced->applyPreferences();
}


/*******************************************************************
 * PREFERENCESDIALOG CLASS EVENTS
 *******************************************************************/

/* PreferencesDialog::onBtnBRAOpenClicked
 * Called when the 'Open Selected BRA' button is clicked
 *******************************************************************/
void PreferencesDialog::onBtnBRAOpenClicked(wxCommandEvent& e) {
	theArchiveManager->openBaseResource(panel_bra->getSelectedPath());
}

/* PreferencesDialog::onButtonClicked
 * Called when a button is clicked
 *******************************************************************/
void PreferencesDialog::onButtonClicked(wxCommandEvent& e) {
	// Check if it was the apply button
	if (e.GetId() == wxID_APPLY)
		applyPreferences();
	else
		e.Skip();
}
