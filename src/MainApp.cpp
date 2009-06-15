
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
#include "ArchiveManager.h"
#include "Tokenizer.h"
#include <wx/image.h>
#include <wx/glcanvas.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
namespace Global {
	string error = _("");
}
wxGLContext*	gl_context = NULL;
MainWindow*		main_window = NULL;


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

	// Load configuration file
	readConfigFile();

	// Create a MainWindow and show it
	main_window = new MainWindow();
	main_window->Show(true);

	// Open any archives on the command line
	for (int a = 0; a < argc; a++) {
		string arg = argv[a];
		theArchiveManager.openArchive(arg);
	}

	return true;
}

/* MainApp::OnExit
 * Application shutdown, run when program is closed
 *******************************************************************/
int MainApp::OnExit() {
	saveConfigFile();
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

/* MainApp::readConfigFile
 * Reads and parses the SLADE configuration file
 *******************************************************************/
void MainApp::readConfigFile() {
	// Open SLADE.cfg
	Tokenizer tz;
	if (!tz.openFile(_T("SLADE.cfg")))
		return;

	// Go through the file with the tokenizer
	string token = tz.getToken();
	while (token.Cmp(_T(""))) {
		// If we come across a 'cvars' token, read in the cvars section
		if (!token.Cmp(_T("cvars"))) {
			token = tz.getToken();	// Skip '{'

			// Keep reading name/value pairs until we hit the ending '}'
			string cvar_name = tz.getToken();
			while (cvar_name.Cmp(_T("}"))) {
				string cvar_val = tz.getToken();
				read_cvar(cvar_name, cvar_val);
				cvar_name = tz.getToken();
			}
		}

		// Get next token
		token = tz.getToken();
	}
}

/* MainApp::saveConfigFile
 * Saves the SLADE configuration file
 *******************************************************************/
void MainApp::saveConfigFile() {
	// Open SLADE.cfg for writing text
	FILE* fp = fopen("SLADE.cfg", "wt");

	// Do nothing if it didn't open correctly
	if (!fp)
		return;

	// Write cfg header
	fprintf(fp, "/*****************************************************\n");
	fprintf(fp, " * SLADE Configuration File\n");
	fprintf(fp, " * Don't edit this unless you know what you're doing\n");
	fprintf(fp, " *****************************************************/\n\n");

	// Write cvars
	save_cvars(fp);

	// Close configuration file
	fprintf(fp, "\n// End Configuration File");
	fclose(fp);
}
