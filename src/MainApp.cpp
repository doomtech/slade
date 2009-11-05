
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
#include "Console.h"
#include <wx/image.h>
#include <wx/stdpaths.h>
#include <wx/ffile.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
namespace Global {
	string error = _("");
	string dir_user = _T("");
	string dir_data = _T("");
}
MainWindow*		main_window = NULL;


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
extern string main_window_layout;

void SLADELog::DoLog(wxLogLevel level, const wxChar* str, time_t t) {
	theConsole->logMessage(wxString(str));
}


/* appPath
 * Prepends an application-related path to a filename,
 * DIR_DATA: SLADE application data directory (for SLADE.pk3)
 * DIR_USER: User configuration and resources directory
 * DIR_APP: Directory of the SLADE executable
 * DIR_TEMP: Temporary files directory
 *******************************************************************/
string appPath(string filename, int dir) {
	// Setup separator character
#ifdef WIN32
	string sep = _T("\\");
#else
	string sep = _T("/");
#endif

	if (dir == DIR_DATA)
		return wxStandardPaths::Get().GetDataDir().Append(sep).Append(filename);
	else if (dir == DIR_USER)
		return wxStandardPaths::Get().GetUserDataDir().Append(sep).Append(filename);
	else if (dir == DIR_APP) {
		wxFileName fn(wxStandardPaths::Get().GetExecutablePath());
		return fn.GetPath().Append(sep).Append(filename);
	}
	else if (dir == DIR_TEMP)
		return wxStandardPaths::Get().GetTempDir().Append(sep).Append(filename);
	else
		return filename;
}


IMPLEMENT_APP(MainApp)

/* MainApp::initDirectories
 * Checks for and creates necessary application directories. Returns
 * true if all directories existed and were created successfully if
 * needed, false otherwise
 *******************************************************************/
bool MainApp::initDirectories() {
	// Create user directory if necessary
	string dir_user = wxStandardPaths::Get().GetUserDataDir();
	if (!wxDirExists(dir_user)) {
		if (!wxMkdir(dir_user)) {
			wxMessageBox(s_fmt(_T("Unable to create user directory \"%s\""), dir_user.c_str()), _T("Error"), wxICON_ERROR);
			return false;
		}
	}

	// Check the data directory exists
	string dir_data = wxStandardPaths::Get().GetDataDir();
	if (!wxDirExists(dir_data)) {
		wxMessageBox(s_fmt(_T("SLADE data directory \"%s\" does not exist. Is SLADE installed correctly?"), dir_data.c_str()), _T("Error"), wxICON_ERROR);
		return false;
	}

	return true;
}

/* MainApp::OnInit
 * Application initialization, run when program is started
 *******************************************************************/
bool MainApp::OnInit() {
	// Init global variables
	Global::error = _T("");

	// Set application name (for wx directory stuff)
	wxApp::SetAppName(_T("SLADE3"));

	// Init application directories
	if (!initDirectories())
		return false;

	// Init logfile
	initLogFile();

	// Load image handlers
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxICOHandler);

	// Load configuration file
	readConfigFile();

	// Create the main window
	main_window = new MainWindow();

	// Open any archives on the command line
	for (int a = 0; a < argc; a++) {
		string arg = argv[a];
		theArchiveManager->openArchive(arg);
	}

	// Show the main window
	main_window->Show(true);

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
	// Set wxLog target(s)
	wxLog::SetActiveTarget(new SLADELog());
	FILE* log_file = fopen(chr(appPath(_T("slade3.log"), DIR_USER)), "wt");
	new wxLogChain(new wxLogStderr(log_file));

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
	if (!tz.openFile(appPath(_T("slade3.cfg"), DIR_USER)))
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

		// Read saved main window AUI layout
		if (!token.Cmp(_T("main_window_layout"))) {
			main_window_layout = tz.getToken();
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
	FILE* fp = fopen(chr(appPath(_T("slade3.cfg"), DIR_USER)), "wt");

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

	// Write main window AUI layout
	fprintf(fp, "main_window_layout \"%s\"\n", chr(main_window_layout));

	// Close configuration file
	fprintf(fp, "\n// End Configuration File\n\n");
	fclose(fp);
}
