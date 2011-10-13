
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ColourPrefsPanel.cpp
 * Description: Panel containing colour preference controls
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
#include "ColourPrefsPanel.h"
#include "ColourConfiguration.h"
#include "MainWindow.h"


/*******************************************************************
 * COLOURPREFSPANEL CLASS FUNCTIONS
 *******************************************************************/

/* ColourPrefsPanel::ColourPrefsPanel
 * ColourPrefsPanel class constructor
 *******************************************************************/
ColourPrefsPanel::ColourPrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Create sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "Colour Preferences");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Colours for entry lists
	sizer->Add(new wxStaticText(this, -1, "Entry list colours:"), 0, wxALL, 4);
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

	// Error
	vbox->Add(new wxStaticText(this, -1, "Error:"), 1, wxALIGN_CENTER_VERTICAL);
	cp_err = new wxColourPickerCtrl(this, -1, wxColour(230, 30, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	cp_err->SetColour(WXCOL(ColourConfiguration::getColour("error")));
	vbox->Add(cp_err, 0, wxEXPAND|wxBOTTOM, 4);

	// Locked
	vbox->Add(new wxStaticText(this, -1, "Locked:"), 1, wxALIGN_CENTER_VERTICAL);
	cp_loc = new wxColourPickerCtrl(this, -1, wxColour(180, 50, 0), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	cp_loc->SetColour(WXCOL(ColourConfiguration::getColour("locked")));
	vbox->Add(cp_loc, 0, wxEXPAND|wxBOTTOM, 4);

	// Modified
	vbox->Add(new wxStaticText(this, -1, "Modified:"), 1, wxALIGN_CENTER_VERTICAL);
	cp_mod = new wxColourPickerCtrl(this, -1, wxColour(0, 80, 180), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	cp_mod->SetColour(WXCOL(ColourConfiguration::getColour("modified")));
	vbox->Add(cp_mod, 0, wxEXPAND|wxBOTTOM, 4);

	// New
	vbox->Add(new wxStaticText(this, -1, "New:"), 1, wxALIGN_CENTER_VERTICAL);
	cp_new = new wxColourPickerCtrl(this, -1, wxColour(0, 150,  0), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	cp_new->SetColour(WXCOL(ColourConfiguration::getColour("new")));
	vbox->Add(cp_new, 0, wxEXPAND|wxBOTTOM, 4);

	// Reset button
	button_reset = new wxButton(this, -1, "Reset to Default");
	vbox->Add(button_reset, 0, wxEXPAND|wxBOTTOM, 4);

	// Complete the sizer
	sizer->Add(vbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
		
	// Bind events
	button_reset->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ColourPrefsPanel::onReset, this);
}

/* ColourPrefsPanel::~ColourPrefsPanel
 * ColourPrefsPanel class destructor
 *******************************************************************/
ColourPrefsPanel::~ColourPrefsPanel() {
}

/* ColourPrefsPanel::applyPreferences
 * Applies preferences from the panel controls
 *******************************************************************/
void ColourPrefsPanel::applyPreferences() {
	wxColour wxc = cp_err->GetColour();
	ColourConfiguration::setColour("error", wxc.Red(), wxc.Green(), wxc.Blue(), wxc.Alpha());
	wxc = cp_loc->GetColour();
	ColourConfiguration::setColour("locked", wxc.Red(), wxc.Green(), wxc.Blue(), wxc.Alpha());
	wxc = cp_mod->GetColour();
	ColourConfiguration::setColour("modified", wxc.Red(), wxc.Green(), wxc.Blue(), wxc.Alpha());
	wxc = cp_new->GetColour();
	ColourConfiguration::setColour("new", wxc.Red(), wxc.Green(), wxc.Blue(), wxc.Alpha());

	theMainWindow->Refresh();
}

/* ColourPrefsPanel::onReset
 * Called when the 'reset' button is clicked
 *******************************************************************/
void ColourPrefsPanel::onReset(wxCommandEvent& e) {
	cp_err->SetColour(wxColour(230, 30, 0));
	cp_loc->SetColour(wxColour(180, 50, 0));
	cp_mod->SetColour(wxColour(0, 80, 180));
	cp_new->SetColour(wxColour(0, 150,  0));

	applyPreferences();
}
