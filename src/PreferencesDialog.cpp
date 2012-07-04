
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
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
#include "NodesPrefsPanel.h"
#include "InputPrefsPanel.h"
#include "Icons.h"
#include "MainWindow.h"
#include <wx/gbsizer.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
PreferencesDialog* PreferencesDialog::instance = NULL;


/*******************************************************************
 * PREFERENCESDIALOG CLASS FUNCTIONS
 *******************************************************************/

/* PreferencesDialog::PreferencesDialog
 * PreferencesDialog class constructor
 *******************************************************************/
PreferencesDialog::PreferencesDialog(wxWindow* parent) : wxDialog(parent, -1, "SLADE Preferences", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	// Setup main sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Set dialog icon
	wxIcon icon;
	icon.CopyFromBitmap(getIcon("t_settings"));
	SetIcon(icon);

	// Create preferences TreeBook
	tree_prefs = new wxTreebook(this, -1, wxDefaultPosition, wxDefaultSize);

	// Setup preferences TreeBook
	PrefsPanelBase* panel;
	panel = new GeneralPrefsPanel(tree_prefs);		tree_prefs->AddPage(panel, "General", true); prefs_pages.push_back(panel);
	panel = new InterfacePrefsPanel(tree_prefs);	tree_prefs->AddPage(panel, "Interface"); prefs_pages.push_back(panel);
	panel = new ColourPrefsPanel(tree_prefs);		tree_prefs->AddSubPage(panel, "Colours"); prefs_pages.push_back(panel);
	panel = new InputPrefsPanel(tree_prefs);		tree_prefs->AddPage(panel, "Input"); prefs_pages.push_back(panel);
	panel = new EditingPrefsPanel(tree_prefs);		tree_prefs->AddPage(panel, "Editing"); prefs_pages.push_back(panel);
	tree_prefs->AddSubPage(setupBaseResourceArchivesPanel(), "Base Resource Archive");
	panel = new TextEditorPrefsPanel(tree_prefs);	tree_prefs->AddPage(panel, "Text Editor"); prefs_pages.push_back(panel);
	panel = new TextStylePrefsPanel(tree_prefs);	tree_prefs->AddSubPage(panel, "Fonts & Colours"); prefs_pages.push_back(panel);
	panel = new GraphicsPrefsPanel(tree_prefs);		tree_prefs->AddPage(panel, "Graphics"); prefs_pages.push_back(panel);
	panel = new PNGPrefsPanel(tree_prefs);			tree_prefs->AddSubPage(panel, "PNG"); prefs_pages.push_back(panel);
	panel = new AudioPrefsPanel(tree_prefs);		tree_prefs->AddPage(panel, "Audio"); prefs_pages.push_back(panel);
	tree_prefs->AddPage(new wxPanel(tree_prefs, -1), "Scripting");
	panel = new ACSPrefsPanel(tree_prefs);			tree_prefs->AddSubPage(panel, "ACS"); prefs_pages.push_back(panel);
	panel = new MapEditorPrefsPanel(tree_prefs);	tree_prefs->AddPage(panel, "Map Editor"); prefs_pages.push_back(panel);
	panel = new MapDisplayPrefsPanel(tree_prefs);	tree_prefs->AddSubPage(panel, "Display"); prefs_pages.push_back(panel);
	panel = new NodesPrefsPanel(tree_prefs);		tree_prefs->AddSubPage(panel, "Node Builders"); prefs_pages.push_back(panel);
	panel = new AdvancedPrefsPanel(tree_prefs);		tree_prefs->AddPage(panel, "Advanced"); prefs_pages.push_back(panel);

	// Expand all tree nodes (so it gets sized properly)
	for (unsigned page = 0; page < tree_prefs->GetPageCount(); page++)
		tree_prefs->ExpandNode(page);

	// Add preferences treebook
	sizer->Add(tree_prefs, 1, wxEXPAND|wxALL, 4);

	// Add buttons
	sizer->Add(CreateButtonSizer(wxOK|wxCANCEL|wxAPPLY), 0, wxEXPAND|wxALL, 4);

	// Bind events
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &PreferencesDialog::onButtonClicked, this);

	// Setup layout
	SetInitialSize(wxSize(-1, -1));
	Layout();
	Fit();
	SetMinSize(GetBestSize());
	CenterOnParent();

	// Collapse all tree nodes
	for (unsigned page = 0; page < tree_prefs->GetPageCount(); page++)
		tree_prefs->CollapseNode(page);
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

/* PreferencesDialog::showPage
 * Shows the preferences page matching [name]
 *******************************************************************/
void PreferencesDialog::showPage(string name) {
	// Go through all pages
	for (unsigned a = 0; a < tree_prefs->GetPageCount(); a++) {
		if (S_CMPNOCASE(tree_prefs->GetPageText(a), name)) {
			tree_prefs->SetSelection(a);
			return;
		}
	}
}

/* PreferencesDialog::applyPreferences
 * Applies preference values from all preference panels
 *******************************************************************/
void PreferencesDialog::applyPreferences() {
	for (unsigned a = 0; a < prefs_pages.size(); a++)
		prefs_pages[a]->applyPreferences();
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

void PreferencesDialog::openPreferences(wxWindow* parent) {
	// Create dialog if needed
	if (!instance)
		instance = new PreferencesDialog(parent);
	else
		instance->SetParent(parent);

	// Show dialog
	instance->CenterOnParent();
	if (instance->ShowModal() == wxID_OK)
		instance->applyPreferences();
	theMainWindow->getArchiveManagerPanel()->refreshAllTabs();
}
