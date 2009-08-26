
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ConsolePanel.cpp
 * Description: UI Frontend panel for the console
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
#include "Console.h"
#include "WxStuff.h"
#include "ConsolePanel.h"


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/


/* ConsolePanel::ConsolePanel
 * ConsolePanel class constructor
 *******************************************************************/
ConsolePanel::ConsolePanel(wxWindow *parent, int id)
: wxPanel(parent, id) {
	// Setup layout
	initLayout();

	// Listen to the console
	listenTo(&Console::getInstance());

	// Load the current contents of the console log
	text_log->AppendText(Console::getInstance().dumpLog());
}

/* ConsolePanel::~ConsolePanel
 * ConsolePanel class destructor
 *******************************************************************/
ConsolePanel::~ConsolePanel() {
}

/* ConsolePanel::initLayout
 * Sets up the panel layout
 *******************************************************************/
void ConsolePanel::initLayout() {
	// Create and set the sizer for the panel
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	// Create and add the message log textbox
	text_log = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	vbox->Add(text_log, 1, wxEXPAND | wxALL, 4);

	// Create and add the command entry textbox
	text_command = new wxTextCtrl(this, CP_TEXT_COMMAND, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	vbox->Add(text_command, 0, wxEXPAND | wxBOTTOM | wxLEFT | wxRIGHT, 4);

	Layout();

	// Set console font to default+monospace
	wxFont f = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	wxFont console_font(f.GetPointSize(), wxFONTFAMILY_MODERN, f.GetStyle(), f.GetWeight());
	text_log->SetFont(console_font);
	text_command->SetFont(console_font);
}

/* ConsolePanel::onAnnouncement
 * Handles any announcement events
 *******************************************************************/
void ConsolePanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// New console log message added
	if (event_name == _T("console_logmessage")) {
		text_log->AppendText(Console::getInstance().lastLogLine());
	}
}

/*******************************************************************
 * WXWIDGETS EVENTS & HANDLERS
 *******************************************************************/
BEGIN_EVENT_TABLE(ConsolePanel, wxPanel)
EVT_TEXT_ENTER(CP_TEXT_COMMAND, ConsolePanel::onCommandEnter)
END_EVENT_TABLE()

void ConsolePanel::onCommandEnter(wxCommandEvent &e) {
	Console::getInstance().execute(e.GetString());
	text_command->Clear();
}
