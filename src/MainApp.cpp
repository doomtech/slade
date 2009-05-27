
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MainApp.cpp
 * Description: MainApp class functions.
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
#include "MainApp.h"
#include "MainWindow.h"
#include <wx/image.h>
#include <wx/glcanvas.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
namespace Global {
	string error = _("");
}
wxGLContext *gl_context = NULL;


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/

IMPLEMENT_APP(MainApp)

/* MainApp::OnInit
 * Application initialization, run when program is started
 *******************************************************************/
bool MainApp::OnInit() {
	// Init global variables
	Global::error = _T("");

	// Init logfile
	initLogFile();

	// Load image handlers
	wxImage::AddHandler(new wxPNGHandler);

	// Create a MainWindow and show it
	MainWindow *heh = new MainWindow();
	heh->Show(true);

	return true;
}

/* MainApp::OnExit
 * Application shutdown, run when program is closed
 *******************************************************************/
int MainApp::OnExit() {
	return 0;
}

/* MainApp::initLogFile
 * Sets up the SLADE log file
 *******************************************************************/
void MainApp::initLogFile() {
	// Set wxLog target
	wxLog::SetActiveTarget(new wxLogStderr(fopen("slade.log", "wt")));

	// Write logfile header
	wxLogMessage(_T("SLADE - It's a Doom Editor"));
	wxLogMessage(_T("Written by Simon Judd, 2008"));
	wxLogMessage(_T("---------------------------"));
}
