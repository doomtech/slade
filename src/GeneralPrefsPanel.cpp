
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GeneralPrefsPanel.cpp
 * Description: Panel containing general preference controls
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
#include "GeneralPrefsPanel.h"


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
EXTERN_CVAR(Bool, close_archive_with_tab)
EXTERN_CVAR(Bool, archive_load_data)
EXTERN_CVAR(Bool, gl_tex_enable_np2)
EXTERN_CVAR(Bool, temp_use_appdir)


/*******************************************************************
 * GENERALPREFSPANEL CLASS FUNCTIONS
 *******************************************************************/

/* GeneralPrefsPanel::GeneralPrefsPanel
 * GeneralPrefsPanel class constructor
 *******************************************************************/
GeneralPrefsPanel::GeneralPrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Create sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "General Preferences");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Load on open archive
	cb_archive_load = new wxCheckBox(this, -1, "Load all archive entry data to memory when opened");
	sizer->Add(cb_archive_load, 0, wxEXPAND|wxALL, 4);

	// Close archive with tab
	cb_archive_close_tab = new wxCheckBox(this, -1, "Close archive when its tab is closed");
	sizer->Add(cb_archive_close_tab, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Enable np2 textures
	cb_gl_np2 = new wxCheckBox(this, -1, "Enable Non-power-of-two textures if supported");
	sizer->Add(cb_gl_np2, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Use app dir for temp files
#ifdef WIN32
	cb_temp_dir = new wxCheckBox(this, -1, "Write temp files to SLADE directory rather than system temp folder");
	sizer->Add(cb_temp_dir, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	cb_temp_dir->SetValue(temp_use_appdir);
#endif

	// Init controls
	cb_archive_load->SetValue(archive_load_data);
	cb_archive_close_tab->SetValue(close_archive_with_tab);
	cb_gl_np2->SetValue(gl_tex_enable_np2);
}

/* GeneralPrefsPanel::~GeneralPrefsPanel
 * GeneralPrefsPanel class destructor
 *******************************************************************/
GeneralPrefsPanel::~GeneralPrefsPanel() {
}

/* GeneralPrefsPanel::applyPreferences
 * Applies preference values from the controls to CVARs
 *******************************************************************/
void GeneralPrefsPanel::applyPreferences() {
	archive_load_data = cb_archive_load->GetValue();
	close_archive_with_tab = cb_archive_close_tab->GetValue();
	gl_tex_enable_np2 = cb_gl_np2->GetValue();
#ifdef WIN32
	temp_use_appdir = cb_temp_dir->GetValue();
#else
	temp_use_appdir = false;
#endif

	// Create temp dir if necessary
	if (temp_use_appdir) {
		string dir_temp = appPath("", DIR_TEMP);
		if (!wxDirExists(dir_temp)) {
			if (!wxMkdir(dir_temp)) {
				// Unable to create it, just use system temp dir
				wxMessageBox(S_FMT("Unable to create temp directory \"%s\"", dir_temp.c_str()), "Error", wxICON_ERROR);
				temp_use_appdir = false;
			}
		}
	}
}
