
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    AdvancedPrefsPanel.cpp
 * Description: Panel containing 'advanced' preference controls,
 *              basically a way to edit raw cvar values outside the
 *              slade.cfg file
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
#include "AdvancedPrefsPanel.h"


/*******************************************************************
 * ADVANCEDPREFSPANEL CLASS FUNCTIONS
 *******************************************************************/

/* AdvancedPrefsPanel::AdvancedPrefsPanel
 * AdvancedPrefsPanel class constructor
 *******************************************************************/
AdvancedPrefsPanel::AdvancedPrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Create sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "Advanced Settings");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Add warning label
	sizer->Add(new wxStaticText(this, -1, "Warning: Only modify these values if you know what you are doing!\nMost of these settings can be changed more safely from the other sections."), 0, wxEXPAND|wxALL, 4);

	// Add property grid
	pg_cvars = new wxPropertyGrid(this, -1, wxDefaultPosition, wxDefaultSize, wxPG_BOLD_MODIFIED|wxPG_SPLITTER_AUTO_CENTER|wxPG_TOOLTIPS|wxPG_HIDE_MARGIN);
	sizer->Add(pg_cvars, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Init property grid
	refreshPropGrid();

	Layout();
}

/* AdvancedPrefsPanel::~AdvancedPrefsPanel
 * AdvancedPrefsPanel class destructor
 *******************************************************************/
AdvancedPrefsPanel::~AdvancedPrefsPanel() {
}

/* AdvancedPrefsPanel::refreshPropGrid
 * Refreshes the cvars wxPropertyGrid
 *******************************************************************/
void AdvancedPrefsPanel::refreshPropGrid() {
	// Clear
	pg_cvars->Clear();

	// Get list of cvars
	vector<string> cvars;
	get_cvar_list(cvars);
	std::sort(cvars.begin(), cvars.end());

	for (unsigned a = 0; a < cvars.size(); a++) {
		// Get cvar
		CVar* cvar = get_cvar(cvars[a]);

		// Add to grid depending on type
		if (cvar->type == CVAR_BOOLEAN)
			pg_cvars->Append(new wxBoolProperty(cvars[a], cvars[a], cvar->GetValue().Bool));
		else if (cvar->type == CVAR_INTEGER)
			pg_cvars->Append(new wxIntProperty(cvars[a], cvars[a], cvar->GetValue().Int));
		else if (cvar->type == CVAR_FLOAT)
			pg_cvars->Append(new wxFloatProperty(cvars[a], cvars[a], cvar->GetValue().Float));
		else if (cvar->type == CVAR_STRING)
			pg_cvars->Append(new wxStringProperty(cvars[a], cvars[a], S_FMT("%s", CHR(((CStringCVar *)cvar)->value))));
	}

	// Set all bool properties to use checkboxes
	pg_cvars->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, true);
}

/* AdvancedPrefsPanel::applyPreferences
 * Applies preferences from the panel controls
 *******************************************************************/
void AdvancedPrefsPanel::applyPreferences() {
	// Get list of cvars
	vector<string> cvars;
	get_cvar_list(cvars);

	for (unsigned a = 0; a < cvars.size(); a++) {
		// Get cvar
		CVar* cvar = get_cvar(cvars[a]);

		// Read value from grid depending on type
		wxVariant value = pg_cvars->GetPropertyValue(cvars[a]);
		if (cvar->type == CVAR_INTEGER)
			*((CIntCVar *) cvar) = value.GetInteger();
		else if (cvar->type == CVAR_BOOLEAN)
			*((CBoolCVar *) cvar) = value.GetBool();
		else if (cvar->type == CVAR_FLOAT)
			*((CFloatCVar *) cvar) = value.GetDouble();
		else if (cvar->type == CVAR_STRING)
			*((CStringCVar *) cvar) = value.GetString();

		pg_cvars->GetProperty(cvars[a])->SetModifiedStatus(false);
		pg_cvars->Refresh();
		pg_cvars->RefreshEditor();
	}
}
