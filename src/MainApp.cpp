
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
#include "Icons.h"
#include <wx/image.h>
#include <wx/stdpaths.h>
#include <wx/ffile.h>
#include <wx/stackwalk.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
namespace Global {
	string error = _T("");
	string version = _T("3.0 alpha");
}
MainWindow*		main_window = NULL;
string	dir_data = _T("");
string	dir_user = _T("");
string	dir_app = _T("");


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
extern string main_window_layout;


class SLADEStackTrace : public wxStackWalker {
private:
	string	stack_trace;
	
public:
	SLADEStackTrace() {
		stack_trace = _T("Stack Trace:\n");
	}

	~SLADEStackTrace() {
	}

	string getTraceString() {
		return stack_trace;
	}

	void OnStackFrame(const wxStackFrame& frame) {
		string location = wxEmptyString;
		if (frame.HasSourceLocation())
			location = s_fmt(_T("(%s:%d) "), frame.GetFileName().c_str(), frame.GetLine());

		string parameters = wxEmptyString;
		/*
		for (size_t a = 0; a < frame.GetParamCount(); a++) {
			string type = wxEmptyString;
			string name = wxEmptyString;
			string value = wxEmptyString;
			frame.GetParam(a, &type, &name, &value);

			parameters.Append(s_fmt(_T("%s %s = %s"), type.c_str(), name.c_str(), value.c_str()));

			if (a < frame.GetParamCount() - 1)
				parameters.Append(_T(", "));
		}
		*/

		stack_trace.Append(s_fmt(_T("%d: %s%s(%s)\n"), frame.GetLevel(), location.c_str(), frame.GetName().c_str(), parameters.c_str()));
	}
};

class SLADECrashDialog : public wxDialog {
private:
	wxTextCtrl*	text_stack;

public:
	SLADECrashDialog(SLADEStackTrace& st) : wxDialog(wxTheApp->GetTopWindow(), -1, _T("SLADE3 Application Crash")) {
		// Setup sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(sizer);

		// Add general crash method
		string message = _T("SLADE3 has crashed unexpectedly. To help fix the problem that caused this crash,\nplease copy+paste the information from the window below to a text file, and email\nit to <sirjuddington@gmail.com>. Sorry for the inconvenience.");
		sizer->Add(new wxStaticText(this, -1, message), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 4);

		// Add stack trace text area
		text_stack = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);
		text_stack->SetValue(st.getTraceString());
		text_stack->SetFont(wxFont(8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
		sizer->Add(text_stack, 1, wxEXPAND|wxALL, 4);

		// Add standard 'OK' button
		sizer->Add(CreateStdDialogButtonSizer(wxOK), 0, wxEXPAND|wxALL, 4);

		// Setup layout
		Layout();
		SetInitialSize(wxSize(500, 500));
	}

	~SLADECrashDialog() {
	}
};


/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

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
		return dir_data + sep + filename;
	else if (dir == DIR_USER)
		return dir_user + sep + filename;
	else if (dir == DIR_APP)
		return dir_app + sep + filename;
	else if (dir == DIR_TEMP)
		return wxStandardPaths::Get().GetTempDir().Append(sep).Append(filename);
	else
		return filename;
}


/*******************************************************************
 * SLADELOG CLASS FUNCTIONS
 *******************************************************************/

void SLADELog::DoLog(wxLogLevel level, const wxChar* str, time_t t) {
	theConsole->logMessage(wxString(str));
}


/*******************************************************************
 * MAINAPP CLASS FUNCTIONS
 *******************************************************************/
IMPLEMENT_APP(MainApp)

/* MainApp::initDirectories
 * Checks for and creates necessary application directories. Returns
 * true if all directories existed and were created successfully if
 * needed, false otherwise
 *******************************************************************/
bool MainApp::initDirectories() {
	// Setup or create user directory if necessary
	dir_user = wxStandardPaths::Get().GetUserDataDir();
	if (!wxDirExists(dir_user)) {
		if (!wxMkdir(dir_user)) {
			wxMessageBox(s_fmt(_T("Unable to create user directory \"%s\""), dir_user.c_str()), _T("Error"), wxICON_ERROR);
			return false;
		}
	}

	// Setup app dir
	dir_app = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();

	// Setup data dir
	dir_data = wxStandardPaths::Get().GetDataDir();
	if (!wxDirExists(dir_data))
		dir_data = dir_app;	// Use app dir if data dir doesn't exist

	return true;
}

/* MainApp::OnInit
 * Application initialization, run when program is started
 *******************************************************************/
bool MainApp::OnInit() {
	// Init global variables
	Global::error = _T("");

	// Set application name (for wx directory stuff)
	wxApp::SetAppName(_T("slade3"));

	// Handle exceptions using wxDebug stuff
	wxHandleFatalExceptions(true);

	// Init application directories
	if (!initDirectories())
		return false;

	// Init logfile
	initLogFile();

	// Check that SLADE.pk3 can be found
	if (!theArchiveManager->resArchiveOK())
		return false;

	// Load image handlers
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxICOHandler);

	// Load configuration file
	readConfigFile();

	// Load program icons
	loadIcons();

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

void MainApp::OnFatalException() {
	SLADEStackTrace st;
	st.WalkFromException();
	SLADECrashDialog sd(st);
	sd.ShowModal();
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



void c_crash(vector<string> args) {
	uint8_t* test = NULL;
	test[123] = 5;
}
ConsoleCommand crash(_T("crash"), &c_crash, 0);
