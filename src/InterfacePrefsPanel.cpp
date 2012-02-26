
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    InterfacePrefsPanel.cpp
 * Description: Panel containing interface preference controls
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
#include "InterfacePrefsPanel.h"


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(Bool, size_as_string)
EXTERN_CVAR(Bool, elist_filter_dirs)
EXTERN_CVAR(Bool, show_start_page)
EXTERN_CVAR(Bool, swap_epanel_bars)
EXTERN_CVAR(Bool, context_submenus)
EXTERN_CVAR(Bool, list_font_monospace)


/*******************************************************************
 * INTERFACEPREFSPANEL CLASS FUNCTIONS
 *******************************************************************/

/* InterfacePrefsPanel::InterfacePrefsPanel
 * InterfacePrefsPanel class constructor
 *******************************************************************/
InterfacePrefsPanel::InterfacePrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Create sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "Interface Preferences");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Show entry size as string instead of a number
	cb_size_as_string = new wxCheckBox(this, -1, "Show entry size as a string with units");
	sizer->Add(cb_size_as_string, 0, wxEXPAND|wxALL, 4);

	// Filter directories
	cb_filter_dirs = new wxCheckBox(this, -1, "Ignore directories when filtering by name");
	sizer->Add(cb_filter_dirs, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Monospace list font
	cb_list_monospace = new wxCheckBox(this, -1, "Use monospaced font for lists");
	sizer->Add(cb_list_monospace, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Context menu submenus
	cb_context_submenus = new wxCheckBox(this, -1, "Group related entry context menu items into submenus");
	sizer->Add(cb_context_submenus, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Swap entry panel bars
	cb_swap_epanel_bars = new wxCheckBox(this, -1, "Swap top and bottom bars in entry editors");
	sizer->Add(cb_swap_epanel_bars, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Show startpage
	cb_start_page = new wxCheckBox(this, -1, "Show Start Page on Startup");
	sizer->Add(cb_start_page, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);


	// Init controls
	cb_size_as_string->SetValue(size_as_string);
	cb_filter_dirs->SetValue(!elist_filter_dirs);
	cb_list_monospace->SetValue(list_font_monospace);
	cb_start_page->SetValue(show_start_page);
	cb_swap_epanel_bars->SetValue(swap_epanel_bars);
	cb_context_submenus->SetValue(context_submenus);
}

/* InterfacePrefsPanel::~InterfacePrefsPanel
 * InterfacePrefsPanel class destructor
 *******************************************************************/
InterfacePrefsPanel::~InterfacePrefsPanel() {
}

/* InterfacePrefsPanel::applyPreferences
 * Applies preference values from the controls to CVARs
 *******************************************************************/
void InterfacePrefsPanel::applyPreferences() {
	size_as_string = cb_size_as_string->GetValue();
	elist_filter_dirs = !cb_filter_dirs->GetValue();
	list_font_monospace = cb_list_monospace->GetValue();
	show_start_page = cb_start_page->GetValue();
	swap_epanel_bars = cb_swap_epanel_bars->GetValue();
	context_submenus = cb_context_submenus->GetValue();
}
