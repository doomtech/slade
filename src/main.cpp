// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-06               >>
// << ------------------------------------ >>
// << Main.cpp - All the important stuff   >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "wx_stuff.h"
#include "wad.h"
#include "splash.h"
#include "doom_map.h"
#include "colours.h"
#include "editor_window.h"
#include "wad_manager.h"
#include "game_config.h"
#include "keybind.h"
#include "console.h"


#include "archive.h"

#include <wx/image.h>
#include <wx/sysopt.h>

#ifdef __APPLE__
// Doing our own thing for Apple path handling, since 10.4 only ships
// with wxWidgets 2.5, and besides we need to make sure to write to
// the user's path instead of /Applications
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFBundle.h>
#include <cstdlib>
string userPath;
#elif wxCHECK_VERSION(2, 6, 0)
// Everyone else uses wxWidgets-provided paths
#include <wx/stdpaths.h>
#endif


// Variables ----------------------------- >>
string app_path;
WadManager *wad_manager = NULL;
bool startup = false;

CVAR(Bool, reopen_wads, true, CVAR_SAVE)


// External Variables -------------------- >>
extern WadList wads;
extern EditorWindow *editor_window;
extern DoomMap d_map;
extern BindList binds;
extern int edit_mode;


// str_upper: Returns a string in uppercase
// ------------------------------------- >>
string str_upper(string str)
{
	for(unsigned i = 0; i < str.length(); ++i)
		str[i] = (char)toupper(str[i]);

	string retval = str;
	return retval;
}

string s_fmt(char *str, ...)
{
	char text[512] = "";
	va_list ap;

	va_start(ap, str);
	vsprintf(text, str, ap);
	va_end(ap);

	string ret = text;
	return ret;
}

void log_message(string message)
{
	if (!startup)
		return;

	wxLogMessage(str_to_wx(message));
}

void message_box(string message, string caption)
{
	wxMessageBox(str_to_wx(message), str_to_wx(caption));
}

// load_main_config: Loads the main SLADE configuration file
// ------------------------------------------------------ >>
void load_main_config(bool wads_open)
{
	Tokenizer mr;

	if (!mr.open_file(c_path("slade.cfg", true), 0, 0))
		return;

	string token = mr.get_token();

	while (token != "!END")
	{
		if (token == "cvars")
			load_cvars(&mr);

		if (token == "colours")
			load_colours(&mr);

		if (token == "iwads")
			load_game_iwads(&mr);

		if (token == "recent_wads")
			load_recent_wads(&mr);

		if (token == "open_wads" && reopen_wads && !wads_open)
			load_open_wads(&mr);

		if (token == "key_binds")
			binds.load(&mr);
		
		/*
		if (token == "window_props")
			load_window_properties(&mr);
			*/

		token = mr.get_token();
	}
}

// save_main_config: Saves the main SLADE configuration file
// ------------------------------------------------------ >>
void save_main_config()
{
	FILE *fp = fopen(c_path("slade.cfg", true).c_str(), "wt");

	save_cvars(fp);
	save_game_iwads(fp);
	binds.save(fp);
	save_recent_wads(fp);
	if (reopen_wads) save_open_wads(fp);
	//save_window_properties(fp);
	save_colours(fp);
	fclose(fp);
}

string c_path(string filename, bool userdir)
{
	string prefix = app_path;

	if (userdir)
	{
#ifdef __APPLE__
		prefix = userDir;
#endif
	}

	return s_fmt("%s%s", prefix.c_str(), filename.c_str());
}

void update_statusbar()
{
	editor_window->update_statusbar();
}

void redraw_map(bool map, bool grid)
{
	editor_window->map()->redraw(map, grid);
}

void change_edit_mode(int mode)
{
	if (mode == edit_mode)
	{
		d_map.clear_selection();
		return;
	}

	edit_mode = mode;
	redraw_map();
	editor_window->change_infobar();
	update_statusbar();
	d_map.clear_selection();
}


class MainApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
};

IMPLEMENT_APP(MainApp)

bool MainApp::OnInit()
{
	srand(wxGetLocalTime());

	// Setup working directory
#ifdef __APPLE__
	CFURLRef bundleRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleRef, kCFURLPOSIXPathStyle);
	wxString dataDir = str_to_wx(string(CFStringGetCStringPtr(bundlePath, CFStringGetSystemEncoding())) + "/Contents/Resources/");
	userPath = string(getenv("HOME")) + string("/Library/Application Support/Slade");
	if(!wxDirExists(str_to_wx(userPath)))
		wxMkdir(str_to_wx(userPath), 0700);
#elif wxCHECK_VERSION(2, 6, 0)
	wxStandardPaths sp;
	wxString dataDir = sp.GetDataDir();
#endif

	wxSetWorkingDirectory(dataDir);

	// Init logfile
	wxLog::SetActiveTarget(new wxLogStderr(fopen(c_path("slade.log", true).c_str(), "wt")));

	// Allow high-colour toolbar icons
	wxSystemOptions::SetOption(wxT("msw.remap"), 0);

	// Load image handlers
	wxImage::AddHandler(new wxPNGHandler);

	// Setup app_path
	app_path = wx_to_str(dataDir);

#ifdef WIN32
	app_path += "\\";
#else
	app_path += "/";
#endif

	startup = true;

	setup_splash();
	splash("Starting up...", false);

	// Parse command line
	bool wads_opened = false;
	if (argc > 0)
	{
		for (int a = 0; a < argc; a++)
			wxLogMessage(argv[a]);

		for (int a = 1; a < argc; a++)
		{
			wxString arg = argv[a];

			if (arg.Right(4).CmpNoCase(_T(".wad")))
			{
				//log_message(s_fmt("Opening wad %s", wx_to_str(arg).c_str()));
				wads.open_wad(wx_to_str(arg));
				wads_opened = true;
			}
		}
	}

	load_main_config(wads_opened);

	editor_window = new EditorWindow(_T("SLADE"), -1, -1, 800, 600);
	SetTopWindow(editor_window);

	init_console();

	wad_manager = new WadManager(editor_window);
	splash_hide();

	testy();

	return true;
}

int MainApp::OnExit()
{
	return 0;
}
