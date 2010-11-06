
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
#include "SplashWindow.h"
#include "EntryDataFormat.h"
#include "TextLanguage.h"
#include "TextStyle.h"
#include <wx/image.h>
#include <wx/stdpaths.h>
#include <wx/ffile.h>
#include <wx/stackwalk.h>
#include <wx/dir.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
namespace Global {
	string error = "";
	string version = "3.0.0";
}
//MainWindow*		main_window = NULL;
string	dir_data = "";
string	dir_user = "";
string	dir_app = "";
MainApp* MainApp::instance = NULL;
CVAR(Bool, temp_use_appdir, false, CVAR_SAVE);


/*******************************************************************
 * EXTERNAL VARIABLES
 *******************************************************************/
extern string main_window_layout;


/*******************************************************************
 * CLASSES
 *******************************************************************/

/* SLADEStackTrace class
 * Extension of the wxStackWalker class that formats stack trace
 * information to a multi-line string, that can be retrieved via
 * getTraceString()
 *******************************************************************/
class SLADEStackTrace : public wxStackWalker {
private:
	string	stack_trace;

public:
	SLADEStackTrace() {
		stack_trace = "Stack Trace:\n";
	}

	~SLADEStackTrace() {
	}

	string getTraceString() {
		return stack_trace;
	}

	void OnStackFrame(const wxStackFrame& frame) {
		string location = wxEmptyString;
		if (frame.HasSourceLocation())
			location = s_fmt("(%s:%d) ", frame.GetFileName().c_str(), frame.GetLine());

		string parameters = wxEmptyString;
		/*
		for (size_t a = 0; a < frame.GetParamCount(); a++) {
			string type = wxEmptyString;
			string name = wxEmptyString;
			string value = wxEmptyString;
			frame.GetParam(a, &type, &name, &value);

			parameters.Append(s_fmt("%s %s = %s", type.c_str(), name.c_str(), value.c_str()));

			if (a < frame.GetParamCount() - 1)
				parameters.Append(", );
		}
		*/

		stack_trace.Append(s_fmt("%d: %s%s(%s)\n", frame.GetLevel(), location.c_str(), frame.GetName().c_str(), parameters.c_str()));
	}
};


/* SLADECrashDialog class
 * A simple dialog that displays a crash message and a scrollable,
 * multi-line textbox with a stack trace
 *******************************************************************/
class SLADECrashDialog : public wxDialog {
private:
	wxTextCtrl*	text_stack;

public:
	SLADECrashDialog(SLADEStackTrace& st) : wxDialog(wxTheApp->GetTopWindow(), -1, "SLADE3 Application Crash") {
		// Setup sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(sizer);

		// Add general crash method
		string message = "SLADE3 has crashed unexpectedly. To help fix the problem that caused this crash,\nplease copy+paste the information from the window below to a text file, and email\nit to <sirjuddington@gmail.com> along with a description of what you were\ndoing at the time of the crash. Sorry for the inconvenience.";
		sizer->Add(new wxStaticText(this, -1, message), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 4);

		// Add stack trace text area
		text_stack = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);
		text_stack->SetValue(st.getTraceString());
		text_stack->SetFont(wxFont(8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
		sizer->Add(text_stack, 1, wxEXPAND|wxALL, 4);

		// Dump stack trace to a file (just in case)
		wxFile file(appPath("slade3_crash.log", DIR_APP), wxFile::write);
		file.Write(st.getTraceString());
		file.Close();

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
	string sep = "\\";
#else
	string sep = "/";
#endif

	if (dir == DIR_DATA)
		return dir_data + sep + filename;
	else if (dir == DIR_USER)
		return dir_user + sep + filename;
	else if (dir == DIR_APP)
		return dir_app + sep + filename;
	else if (dir == DIR_TEMP) {
		if (temp_use_appdir)
			return dir_app + sep + "temp" + sep + filename;
		else
			return wxStandardPaths::Get().GetTempDir().Append(sep).Append(filename);
	}
	else
		return filename;
}


/*******************************************************************
 * SLADELOG CLASS FUNCTIONS
 *******************************************************************/

// How fun. DoLogString() does not work with 2.9.1, and DoLogText() with 2.9.0.
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION == 9 && wxRELEASE_NUMBER == 0)
void SLADELog::DoLogString(const wxString& msg, time_t time) {
#else
void SLADELog::DoLogText(const wxString& msg) {
#endif
	theConsole->logMessage(msg);
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
	// Setup separator character
#ifdef WIN32
	string sep = "\\";
#else
	string sep = "/";
#endif

	// Setup app dir
	dir_app = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();

	// Check for portable install
	if (wxFileExists(appPath("portable", DIR_APP))) {
		// Setup portable user/data dirs
		dir_data = dir_app;
		dir_user = dir_app + sep + "config";
	}
	else {
		// Setup standard user/data dirs
		dir_user = wxStandardPaths::Get().GetUserDataDir();
		dir_data = wxStandardPaths::Get().GetDataDir();
	}

	// Create user dir if necessary
	if (!wxDirExists(dir_user)) {
		if (!wxMkdir(dir_user)) {
			wxMessageBox(s_fmt("Unable to create user directory \"%s\"", dir_user.c_str()), "Error", wxICON_ERROR);
			return false;
		}
	}

	// Create temp dir if necessary
	string dir_temp = dir_app + sep + "temp";
	if (!wxDirExists(dir_temp)) {
		if (!wxMkdir(dir_temp)) {
			// Unable to create it, just use system temp dir
			wxMessageBox(s_fmt("Unable to create temp directory \"%s\", using system temp directory instead", dir_temp.c_str()), "Error", wxICON_ERROR);
			temp_use_appdir = false;
		}
	}

	// Check data dir
	if (!wxDirExists(dir_data))
		dir_data = dir_app;	// Use app dir if data dir doesn't exist

	return true;
}

/* MainApp::OnInit
 * Application initialization, run when program is started
 *******************************************************************/
bool MainApp::OnInit() {
	// Init global variables
	setInstance(this);
	Global::error = "";
	ArchiveManager::getInstance();

	// Set application name (for wx directory stuff)
	wxApp::SetAppName("slade3");

	// Handle exceptions using wxDebug stuff
	wxHandleFatalExceptions(true);

	// Init application directories
	if (!initDirectories())
		return false;

	// Load image handlers
	wxInitAllImageHandlers();

	// Init logfile
	initLogFile();

	wxLogMessage("Compiled with wxWidgets %i.%i.%i", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER);

	// Load configuration file
	wxLogMessage("Loading configuration");
	readConfigFile();

	// Check that SLADE.pk3 can be found
	wxLogMessage("Loading resources");
	theArchiveManager->init();
	if (!theArchiveManager->resArchiveOK())
		return false;

	// Show splash screen
	theSplashWindow->init();
	theSplashWindow->show("Starting up...");

	// Load program icons
	wxLogMessage("Loading icons");
	loadIcons();

	// Load entry types
	wxLogMessage("Loading entry types");
	EntryDataFormat::initBuiltinFormats();
	EntryType::loadEntryTypes();

	// Load text languages
	wxLogMessage("Loading text languages");
	TextLanguage::loadLanguages();

	// Init text stylesets
	wxLogMessage("Loading text style sets");
	StyleSet::loadResourceStyles();
	StyleSet::loadCustomStyles();

	// Show the main window
	theMainWindow->Show(true);
	theSplashWindow->SetParent(theMainWindow);
	theSplashWindow->CentreOnParent();

	// Open any archives on the command line
	// argv[0] is normally the executable itself (i.e. Slade.exe)
	// and opening it as an archive should not be attempted...
	for (int a = 1; a < argc; a++) {
		string arg = argv[a];
		theArchiveManager->openArchive(arg);
	}

	// Hide splash screen
	theSplashWindow->hide();

	wxLogMessage("SLADE Initialisation OK");

	return true;
}

/* MainApp::OnExit
 * Application shutdown, run when program is closed
 *******************************************************************/
int MainApp::OnExit() {
	// Save configuration
	saveConfigFile();

	// Save text style configuration
	StyleSet::saveCurrent();

	// Close all open archives
	theArchiveManager->closeAll();

	// Clean up
	EntryType::cleanupEntryTypes();
	ArchiveManager::deleteInstance();
	Console::deleteInstance();
	SplashWindow::deleteInstance();

	// Clear temp folder
	wxDir temp;
	temp.Open(appPath("", DIR_TEMP));
	string filename = wxEmptyString;
	bool files = temp.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
	while (files) {
		wxRemoveFile(appPath(filename, DIR_TEMP));
		files = temp.GetNext(&filename);
	}

	return 0;
}

void MainApp::OnFatalException() {
#ifndef _DEBUG
	SLADEStackTrace st;
	st.WalkFromException();
	SLADECrashDialog sd(st);
	sd.ShowModal();
#endif
}

/* MainApp::initLogFile
 * Sets up the SLADE log file
 *******************************************************************/
void MainApp::initLogFile() {
	// Set wxLog target(s)
	wxLog::SetActiveTarget(new SLADELog());
	FILE* log_file = fopen(chr(appPath("slade3.log", DIR_DATA)), "wt");
	new wxLogChain(new wxLogStderr(log_file));

	// Write logfile header
	string year = wxNow().Right(4);
	wxLogMessage("SLADE - It's a Doom Editor");
	wxLogMessage("Version %s", Global::version.c_str());
	wxLogMessage("Written by Simon Judd, 2008-%s", year.c_str());
	wxLogMessage("--------------------------------");
}

/* MainApp::readConfigFile
 * Reads and parses the SLADE configuration file
 *******************************************************************/
void MainApp::readConfigFile() {
	// Open SLADE.cfg
	Tokenizer tz;
	if (!tz.openFile(appPath("slade3.cfg", DIR_USER)))
		return;

	// Go through the file with the tokenizer
	string token = tz.getToken();
	while (token.Cmp("")) {
		// If we come across a 'cvars' token, read in the cvars section
		if (!token.Cmp("cvars")) {
			token = tz.getToken();	// Skip '{'

			// Keep reading name/value pairs until we hit the ending '}'
			string cvar_name = tz.getToken();
			while (cvar_name.Cmp("}")) {
				string cvar_val = tz.getToken();
				read_cvar(cvar_name, cvar_val);
				cvar_name = tz.getToken();
			}
		}

		// Read saved main window AUI layout
		if (!token.Cmp("main_window_layout")) {
			main_window_layout = tz.getToken();
		}

		// Read base resource archive paths
		if (!token.Cmp("base_resource_paths")) {
			// Skip {
			token = tz.getToken();

			// Read paths until closing brace found
			token = tz.getToken();
			while (token.Cmp("}")) {
				theArchiveManager->addBaseResourcePath(token);
				token = tz.getToken();
			}
		}

		// Read recent files list
		if (token == "recent_files") {
			// Skip {
			token = tz.getToken();

			// Read files until closing brace found
			token = tz.getToken();
			while (token != "") {
				theArchiveManager->addRecentFile(token);
				token = tz.getToken();
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
	wxFile file(appPath("slade3.cfg", DIR_USER), wxFile::write);

	// Do nothing if it didn't open correctly
	if (!file.IsOpened())
		return;

	// Write cfg header
	file.Write("/*****************************************************\n");
	file.Write(" * SLADE Configuration File\n");
	file.Write(" * Don't edit this unless you know what you're doing\n");
	file.Write(" *****************************************************/\n\n");

	// Write cvars
	save_cvars(file);

	// Write main window AUI layout
	string layout = s_fmt("%3d%s", MW_LAYOUT_VERS, main_window_layout.c_str());
	file.Write(s_fmt("main_window_layout \"%s\"\n", chr(layout)));

	// Write base resource archive paths
	file.Write("\nbase_resource_paths\n{\n");
	for (size_t a = 0; a < theArchiveManager->numBaseResourcePaths(); a++)
		file.Write(s_fmt("\t\"%s\"\n", theArchiveManager->getBaseResourcePath(a)));
	file.Write("}\n");

	// Write recent files list (in reverse to keep proper order when reading back)
	file.Write("\nrecent_files\n{\n");
	for (int a = theArchiveManager->numRecentFiles()-1; a >= 0; a--)
		file.Write(s_fmt("\t\"%s\"\n", theArchiveManager->recentFile(a)));
	file.Write("}\n");

	// Close configuration file
	file.Write("\n// End Configuration File\n\n");
}


CONSOLE_COMMAND (crash, 0) {
	if (wxMessageBox("Yes, this command does actually exist and *will* crash the program. Do you really want it to crash?", "...Really?", wxYES_NO|wxCENTRE) == wxYES) {
		uint8_t* test = NULL;
		test[123] = 5;
	}
}
