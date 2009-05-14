
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    WadPanel.cpp
 * Description: WadPanel class. The base wxWidgets panel for wadfile
 *              editing. One of these is opened in a tab for each
 *              open wadfile.
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
#include "WadPanel.h"


/* WadPanel::WadPanel
 * WadPanel class constructor
 *******************************************************************/
WadPanel::WadPanel(wxWindow* parent, Archive* wad)
: wxPanel(parent, -1) {
	this->wad = wad;

	// Create & set sizer
	wxBoxSizer *m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Lump list panel
	lump_list = new EntryListPanel(this, ENTRY_LIST_PANEL, wad);
	m_hbox->Add(lump_list, 0, wxEXPAND | wxALL, 4);

	lump_list->populateEntryList();

	// Create lump areas
	lump_area = new LumpArea(this);
	text_area = new TextLumpArea(this);

	// Add default lump area to the panel
	cur_area = lump_area;
	m_hbox->Add(cur_area, 1, wxEXPAND | wxALL, 4);
	cur_area->Show(true);

	Layout();
}

/* WadPanel::~WadPanel
 * WadPanel class destructor
 *******************************************************************/
WadPanel::~WadPanel() {
}

/* WadPanel::save
 * Saves the wad file
 *******************************************************************/
void WadPanel::save() {
	// Check the wad exists
	if (!wad)
		return;

	// Save the wad
	if (!wad->save()) {
		// If there was an error pop up a message box
		wxMessageBox(s_fmt(_T("Error: %s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
	}
}

/* WadPanel::saveAs
 * Saves the wad file to a new file
 *******************************************************************/
void WadPanel::saveAs() {
	// Check the wad exists
	if (!wad)
		return;

	// Setup file filters (temporary, should go through all archive types somehow)
	string formats = _T("Doom Wad File (*.wad)|*.wad");
	string deftype = _T("*.wad");
	string filename = wxFileSelector(_T("Save Wad ") + wad->getFileName(false) + _T(" As"), _T(""), _T(""), deftype, formats, wxSAVE | wxOVERWRITE_PROMPT);

	// Check a filename was selected
	if (!filename.empty()) {
		// Save the wad
		if (!wad->save(filename)) {
			// If there was an error pop up a message box
			wxMessageBox(s_fmt(_T("Error: %s"), Global::error.c_str()), _T("Error"), wxICON_ERROR);
		}
	}
}


BEGIN_EVENT_TABLE(WadPanel, wxPanel)
EVT_LIST_ITEM_FOCUSED(WadPanel::ENTRY_LIST_PANEL, WadPanel::onEntryListChange)
END_EVENT_TABLE()

/* WadPanel::onEntryListChange
 * Called when the current focus on the list control in the
 * entry list panel is changed (ie when the user selects an entry
 * in the list)
 *******************************************************************/
void WadPanel::onEntryListChange(wxListEvent& event) {
	// Get the panel sizer
	wxSizer* sizer = GetSizer();

	// Hide the current lump area, replace it with the new
	// lump area, and show that
	cur_area->Show(false);
	sizer->Replace(cur_area, text_area);
	cur_area = text_area;
	cur_area->loadEntry(lump_list->getFocusedEntry());
	cur_area->Show(true);

	Layout();
}
