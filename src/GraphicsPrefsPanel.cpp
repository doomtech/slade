
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GraphicsPrefsPanel.cpp
 * Description: Panel containing graphics preference controls
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
#include "GraphicsPrefsPanel.h"
#include "GLTexture.h"
#include "MainWindow.h"
#include <wx/filedlg.h>


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(String, bgtx_colour1)
EXTERN_CVAR(String, bgtx_colour2)

/*******************************************************************
 * GRAPHICSPREFSPANEL CLASS FUNCTIONS
 *******************************************************************/

/* GraphicsPrefsPanel::GraphicsPrefsPanel
 * GraphicsPrefsPanel class constructor
 *******************************************************************/
GraphicsPrefsPanel::GraphicsPrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Create sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "Graphics Preferences");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Colours for the chequered background texture
	sizer->Add(new wxStaticText(this, -1, "Transparent background colours:"), 0, wxALL, 4);
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	cp_colour1 = new wxColourPickerCtrl(this, -1, WXCOL(COL_BLACK), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	cp_colour1->SetColour(wxColour(bgtx_colour1));
	vbox->Add(cp_colour1, 0, wxEXPAND|wxBOTTOM, 4);
	cp_colour2 = new wxColourPickerCtrl(this, -1, WXCOL(COL_BLACK), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL|wxCLRP_USE_TEXTCTRL);
	cp_colour2->SetColour(wxColour(bgtx_colour2));
	vbox->Add(cp_colour2, 0, wxEXPAND|wxBOTTOM, 4);

	// Quick colour presets
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	b_reset		= new wxButton(this, -1, "Default");
	b_black		= new wxButton(this, -1, "Black");
	b_cyan		= new wxButton(this, -1, "Cyan");
	b_magenta	= new wxButton(this, -1, "Magenta");
	b_white		= new wxButton(this, -1, "White");
	hbox->Add(b_reset, 0, wxEXPAND|wxBOTTOM, 4);
	hbox->Add(b_black, 0, wxEXPAND|wxBOTTOM, 4);
	hbox->Add(b_cyan, 0, wxEXPAND|wxBOTTOM, 4);
	hbox->Add(b_magenta, 0, wxEXPAND|wxBOTTOM, 4);
	hbox->Add(b_white, 0, wxEXPAND|wxBOTTOM, 4);
	vbox->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);

	sizer->Add(vbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Bind events
	b_reset->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GraphicsPrefsPanel::onButtonRClicked, this);
	b_cyan->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GraphicsPrefsPanel::onButtonCClicked, this);
	b_magenta->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GraphicsPrefsPanel::onButtonMClicked, this);
	b_black->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GraphicsPrefsPanel::onButtonBClicked, this);
	b_white->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GraphicsPrefsPanel::onButtonWClicked, this);
}

/* GraphicsPrefsPanel::~GraphicsPrefsPanel
 * GraphicsPrefsPanel class destructor
 *******************************************************************/
GraphicsPrefsPanel::~GraphicsPrefsPanel() {
}

/* GraphicsPrefsPanel::applyPreferences
 * Applies preferences from the panel controls
 *******************************************************************/
void GraphicsPrefsPanel::applyPreferences() {
	wxColour wxc = cp_colour1->GetColour();
	bgtx_colour1 = wxc.GetAsString();
	wxc = cp_colour2->GetColour();
	bgtx_colour2 = wxc.GetAsString();
	GLTexture::resetBgTex();
	theMainWindow->Refresh();
}

/* GraphicsPrefsPanel::onButtonRClicked
 * Called when the 'Default' button is clicked
 *******************************************************************/
void GraphicsPrefsPanel::onButtonRClicked(wxCommandEvent& e) {
	cp_colour1->SetColour(wxColour(64, 64, 80));
	cp_colour2->SetColour(wxColour(80, 80, 96));
	applyPreferences();
}

/* GraphicsPrefsPanel::onButtonBClicked
 * Called when the 'Black' button is clicked
 *******************************************************************/
void GraphicsPrefsPanel::onButtonBClicked(wxCommandEvent& e) {
	cp_colour1->SetColour(wxColour(0, 0, 0));
	cp_colour2->SetColour(wxColour(0, 0, 0));
	applyPreferences();
}

/* GraphicsPrefsPanel::onButtonCClicked
 * Called when the 'Cyan' button is clicked
 *******************************************************************/
void GraphicsPrefsPanel::onButtonCClicked(wxCommandEvent& e) {
	cp_colour1->SetColour(wxColour(0, 255, 255));
	cp_colour2->SetColour(wxColour(0, 255, 255));
	applyPreferences();
}

/* GraphicsPrefsPanel::onButtonMClicked
 * Called when the 'Magenta' button is clicked
 *******************************************************************/
void GraphicsPrefsPanel::onButtonMClicked(wxCommandEvent& e) {
	cp_colour1->SetColour(wxColour(255, 0, 255));
	cp_colour2->SetColour(wxColour(255, 0, 255));
	applyPreferences();
}

/* GraphicsPrefsPanel::onButtonWClicked
 * Called when the 'White' button is clicked
 *******************************************************************/
void GraphicsPrefsPanel::onButtonWClicked(wxCommandEvent& e) {
	cp_colour1->SetColour(wxColour(255, 255, 255));
	cp_colour2->SetColour(wxColour(255, 255, 255));
	applyPreferences();
}
