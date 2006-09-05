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


#include <wx/filename.h>
#include <wx/image.h>
#include <wx/sysopt.h>

#ifdef __APPLE__
const char *getBundleResourceDir(void);
const char *getTempDir(void);
#elif wxCHECK_VERSION(2, 6, 0)
// Everyone else uses wxWidgets-provided paths
#include <wx/stdpaths.h>
#endif


// Variables ----------------------------- >>
string app_path;
string tmp_path;
string usr_path;

WadManager *wad_manager = NULL;
bool startup = false;

Wad reswad;

CVAR(Bool, reopen_wads, true, CVAR_SAVE)


// External Variables -------------------- >>
extern WadList wads;
extern EditorWindow *editor_window;
extern DoomMap d_map;
extern BindList binds;
extern int edit_mode;


// load_main_config: Loads the main SLADE configuration file
// ------------------------------------------------------ >>
void load_main_config(bool wads_open)
{
	Tokenizer mr;

	if (!mr.open_file(c_path(_T("slade.cfg"), DIR_USR), 0, 0))
	{
		log_message(_T("slade.cfg not found, creating"));
		return;
	}

	string token = mr.get_token();

	while (token != _T("!END"))
	{
		if (token == _T("cvars"))
			load_cvars(&mr);

		else if (token == _T("colours"))
			load_colours(&mr);

		else if (token == _T("iwads"))
			load_game_iwads(&mr);

		else if (token == _T("recent_wads"))
			load_recent_wads(&mr);

		else if (token == _T("open_wads") && reopen_wads && !wads_open)
			load_open_wads(&mr);

		else if (token == _T("key_binds"))
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
	FILE *fp = fopen(chr(c_path(_T("slade.cfg"), DIR_USR)), "wt");

	save_cvars(fp);
	save_game_iwads(fp);
	binds.save(fp);
	save_recent_wads(fp);
	if (reopen_wads) save_open_wads(fp);
	//save_window_properties(fp);
	save_colours(fp);
	fclose(fp);
}

/*
// str: Converts a wxString to a std::string, converting from unicode to ASCII if necessary
// ------------------------------------------------------------------------------------- >>
std::string str(string in)
{
	wxCharBuffer buffer = in.ToAscii();
	std::string ret = (const char*)buffer;
	return ret;
}
*/

// message_box: Pops up a message box
// ------------------------------- >>
void message_box(string message, string caption)
{
	wxMessageBox(message, caption);
}

// log_message: Writes a message to slade.log
// --------------------------------------- >>
void log_message(string message)
{
	wxLogMessage(message);
}

// c_path: Adds a path to a filename
// ------------------------------ >>
string c_path(string filename, BYTE dir)
{
	if (dir == DIR_APP)
		return app_path+filename;
	else if (dir == DIR_TMP)
		return tmp_path+filename;
	else if (dir == DIR_USR)
		return usr_path+filename;
	else 
		return _T("");
}

void update_statusbar()
{
	editor_window->update_statusbar();
}

void update_infobar()
{
	editor_window->update_infobar();
}

void redraw_map(bool map, bool grid)
{
	editor_window->map()->redraw(map, grid);
}

float get_zoom()
{
	return (float)editor_window->map()->get_zoom();
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

void setup_directories()
{
	// Temporary directory
#ifdef UNIX
	tmp_path = _T("/tmp/");
#elif defined(__APPLE__)
	tmp_path = getTempDir();
#else
	tmp_path = app_path;
#endif

	// User directory
#ifdef UNIX
	app_path = wxString::FromAscii(SHARE_DIR);
	usr_path = string(wxFileName::GetHomeDir()) + string(_T("/.slade/"));
#elif defined(__APPLE__)
	app_path = string(getBundleResourceDir()) + _T("/");
	usr_path = wxFileName::GetHomeDir() + string(_T("/Library/Application Support/Slade/"));
#else
	usr_path = app_path;
#endif
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
	setup_directories();

	// Init logfile
	wxLog::SetActiveTarget(new wxLogStderr(fopen(chr(c_path(_T("slade.log"), DIR_TMP)), "wt")));

#ifdef UNIX
	// Create User Directory if it doesn't already exist
	if(!wxFileName::DirExists(usr_path)) {
		if(mkdir(chr(usr_path), S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
			log_message(s_fmt(_T("Could not create %s"), chr(usr_path)));
		}
	}
#endif

	// Allow high-colour toolbar icons
	wxSystemOptions::SetOption(_T("msw.remap"), 0);

	// Load image handlers
	wxImage::AddHandler(new wxPNGHandler);

	if (!reswad.openZip(c_path(_T("slade.pk3"), DIR_APP)))
	{
		message_box(_T("Unable to open slade.pk3!"), _T("Error"));
		save_main_config();
		wxTheApp->ExitMainLoop();
		return false;
	}

	startup = true;

	setup_splash();
	splash(_T("Starting up..."), false);

	// Parse command line
	bool wads_opened = false;

#ifndef UNIX
	// Setup app path
	app_path = argv[0];

	int i = 5;	// 'slade'
#ifdef WIN32
	i = 9;		// 'slade.exe'
#endif

	for (int a = 0; a < i; a++)
		app_path.erase(app_path.end() - 1);
#endif

	// Parse command line args
	for (int a = 1; a < argc; a++)
	{
		string arg = argv[a];

		if (arg.Right(4).CmpNoCase(_T(".wad")) == 0)
		{
			//log_message(s_fmt("Opening wad %s", wx_to_str(arg).c_str()));
			wads.open(arg);
			wads_opened = true;
		}

		if (arg.Right(4).CmpNoCase(_T(".pk3")) == 0 || arg.Right(4).CmpNoCase(_T(".zip")) == 0)
		{
			//log_message(s_fmt("Opening wad %s", wx_to_str(arg).c_str()));
			wads.open(arg, WL_ZIP);
			wads_opened = true;
		}
	}

	load_main_config(wads_opened);

	editor_window = new EditorWindow(_T("SLADE"), -1, -1, 800, 600);
	SetTopWindow(editor_window);

	init_console();

	wad_manager = new WadManager(editor_window);
	splash_hide();

	return true;
}

int MainApp::OnExit()
{
	return 0;
}
