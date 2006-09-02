// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-05               >>
// << ------------------------------------ >>
// << console.cpp - Console functions      >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "wx_stuff.h"
#include "console.h"
#include "colours.h"
#include "textures.h"
//#include "tex_browser.h"
#include "splash.h"
#include "version.h"
#include "console_window.h"
#include "doom_map.h"
#include "dm_vertex.h"

// Variables ----------------------------- >>
//GtkTextBuffer	*console_log;
wxString		console_log;
string			cmd_line;
vector<string>	cmd_history;
int				console_hcmd = -1;

string qdb[] = {
	_T("SLADE: SlayeR's LeetAss Doom Editor"),
	_T("SLADE: Some Lame Ancient Doom Editor"),
	_T("Having fun playing with useless console commands?"),
	_T("Initializing Nickbakery..."),
	_T("Truth is beautiful, without doubt; but so are lies."),
	_T("Initializing Doom Builder..."),
	_T("...And the lord said, 'lo, there shall only be case or default labels inside a switch statement'"),
	_T("Call me paranoid but finding '/*' inside this comment makes me suspicious."),
	_T("My word, whats wrong with that mans bottom?"),
	_T("Subliminal (kill) messaging (your) is (parents) awesome!"),
	_T("Yeah it's a map editor, what of it?"),
	_T("Installing Gator..."),
	_T("Formatting C:/..."),
	_T("Remember that when you reach for the stars, they are too far away, so it's hopeless."),
	_T("http://slade.mancubus.net -- FREE PORN!!"),
	_T("You're trying to say you like DOS better than me right? (Press Y to quit)"),
	_T("I've hidden some pr0n somewhere in the SLADE resources, find it and win a prize!"),
	_T("\"splash hide\" in the console to hide this, by the way."),
	_T("<SoM> you should call it SLAID || <rf`> slaid gets you laid"),
	_T("<Bloodshedder> you've got it made with slade"),
	_T("SLADE: it slices, it dices, it makes julian fries!"),
	_T("SLADE: this slogan isn't cliched"),
	_T("SLADE: more popular than dennis quaid"),
	_T("SLADE: If you were a map editor, it would use you"),
	_T("SLADE: coming to your aid"),
};
int p_q = 0;
int n_quotes = sizeof(qdb) / sizeof(string);

// External Variables -------------------- >>
extern vector<Texture*>	textures;
extern vector<Texture*>	flats;
extern bool dev_log;
extern ConsoleWindow *console_window;
extern DoomMap d_map;

void console_prevcommand()
{
	if (console_hcmd != -1)
		console_hcmd--;

	if (console_hcmd == -1)
	{
		if (cmd_history.size() > 0)
			console_hcmd = cmd_history.size() - 1;
	}

	if (console_hcmd != -1)
		cmd_line = cmd_history[console_hcmd];
}

void console_nextcommand()
{
	if (console_hcmd == -1)
		return;

	if (console_hcmd != -1)
		console_hcmd++;

	if (console_hcmd >= cmd_history.size())
	{
		cmd_line = "";
		console_hcmd = -1;
	}
	else
		cmd_line = cmd_history[console_hcmd];
}

// init_console: Initialises the console
// ---------------------------------- >>
void init_console()
{
	console_window = new ConsoleWindow();

	string line = _T("<< S.L.A.D.E. -- \"SlayeR's LeetAss Doom Editor\" (");
	line += _T(__SLADEVERS);
	line += _T(")");
	for (int a = 0; a < 21 - sizeof(__SLADEVERS); a++)
		line += _T(" ");
	line += _T(">>");
	console_print(_T("<< ------------------------------------------------------------------ >>"));
	console_print(line);
	console_print(_T("<< ------------------------------------------------------------------ >>"));
	console_print(_T("<< By Simon \"SlayeR\" Judd, 2005-06                                    >>"));
	console_print(_T("<< web: \"http://slade.mancubus.net\" email: \"veilofsorrow@gmail.com\"   >>"));
	console_print(_T("<< ------------------------------------------------------------------ >>"));
	console_print(_T("<< Type 'cmdlist' for a list of valid commands.                       >>"));
	console_print(_T("<< ------------------------------------------------------------------ >>"));
	console_print(_T(" "));
}

// console_print: Prints a message to the console
// ------------------------------------------- >>
void console_print(string message)
{
	if (message[message.size() - 1] != '\n')
		message += _T("\n");

	console_log.insert(console_log.size(), str_to_wx(message));

	wxLogMessage(str_to_wx(message));

	console_window->update_log();
}

// console_parsecommand: Parses the current console command
// ----------------------------------------------------- >>
void console_parsecommand()
{
	if (cmd_line == _T(""))
		return;

	bool parsed = false;
	Tokenizer tz;
	tz.open_string(cmd_line, 0, 0);

	string token = tz.get_token();

	console_print(cmd_line);
	cmd_history.push_back(cmd_line);

	// Check for cvar
	CVar* cvar = get_cvar(token);
	if (cvar)
	{
		bool changed = false;

		// Check if we want to change the value
		if (tz.peek_token() != _T("!END"))
		{
			if (cvar->type == CVAR_INTEGER)
				*((CIntCVar *)cvar) = tz.get_integer();

			if (cvar->type == CVAR_BOOLEAN)
				*((CBoolCVar *)cvar) = !!(tz.get_integer());

			if (cvar->type == CVAR_FLOAT)
				*((CFloatCVar *)cvar) = tz.get_float();

			if (cvar->type == CVAR_STRING)
				*((CStringCVar *)cvar) = tz.get_token();

			changed = true;
		}

		// Display the value
		string msg = _T("- \"");
		msg += cvar->name;

		if (changed)
			msg += _T("\" set to ");
		else
			msg += _T("\" is ");

		char val[16] = _T("");

		if (cvar->type == CVAR_INTEGER)
			sprintf(val, "\"%d\"", cvar->GetValue().Int);

		if (cvar->type == CVAR_STRING)
			sprintf(val, "\"%s\"", ((CStringCVar *)cvar)->value.c_str());

		if (cvar->type == CVAR_BOOLEAN)
			sprintf(val, "\"%d\"", cvar->GetValue().Bool);

		if (cvar->type == CVAR_FLOAT)
			sprintf(val, "\"%1.2f\"", cvar->GetValue().Float);

		console_print(msg + val);

		parsed = true;
	}

	// "cvarlist" command
	if (token == _T("cvarlist"))
	{
		console_print(_T("- All CVars:"));

		vector<string> l_cvars = get_cvar_list();

		for (DWORD s = 0; s < l_cvars.size(); s++)
		{
			char temp[128] = "";
			sprintf(temp, "\"%s\"", l_cvars[s].c_str());
			console_print(temp);
		}

		parsed = true;
	}

	// "dump_textures" command
	if (token == _T("dump_textures"))
	{
		for (int a = 0; a < textures.size(); a++)
			console_print(s_fmt(_T("%s, %dx%d"), chr(textures[a]->name), textures[a]->width, textures[a]->height));

		parsed = true;
	}

	// "dump_flats" command
	if (token == _T("dump_flats"))
	{
		for (int a = 0; a < flats.size(); a++)
			console_print(flats[a]->name);

		parsed = true;
	}

	// "tex_browse" command
	if (token == _T("tex_browse"))
	{
		//string stex = open_texture_browser(true, true, true);
		//console_print(parse_string("Selected \"%s\"", stex.c_str()));
		parsed = true;
	}

	// "splash" command
	// If msg isn't specified, a random quote is shown instead
	if (token == _T("splash"))
	{
		// Get a random number that isn't what was previously shown :P
		int q = p_q;
		while (q == p_q) q = float((float)rand() / RAND_MAX) * n_quotes;
		p_q = q;

		string msg = qdb[q];
		float prog = 0.0f;

		if (tz.peek_token() != _T("!END"))
		{
			msg = tz.get_token();

			if (tz.peek_token() != _T("!END"))
				prog = tz.get_float();
		}

		if (msg != _T("hide"))
		{
			splash(msg);
			splash_progress(prog);
		}
		else
			splash_hide();

		parsed = true;
	}

	// "cmdlist" command
	if (token == _T("cmdlist"))
	{
		console_print(_T("Available Commands:"));
		console_print(_T("cmdlist"));
		console_print(_T("cvarlist"));
		console_print(_T("dump_flats"));
		console_print(_T("dump_textures"));
		console_print(_T("splash"));
		parsed = true;
	}

	if (token == "listverts")
	{
		for (int a = 0; a < d_map.n_verts(); a++)
		{
			Vertex* v = d_map.vertex(a);
			console_print(s_fmt("Vertex %d: i%d x%d y%d r%d", a, d_map.index(v), v->x_pos(), v->y_pos(), v->refs()));
		}
	}

	// Unknown command
	if (!parsed)
	{
		char temp[276] = "";
		sprintf(temp, "- Unknown Command \"%s\"", chr(cmd_line));
		console_print(temp);
	}

	// Finish up
	cmd_line = _T("");
	console_hcmd = -1;
}
