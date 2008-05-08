
#include "main.h"
#include "byteswap.h"
#include "script_editor.h"
#include "map.h"

CVAR(String, path_acc, "acc", CVAR_SAVE)
CVAR(Int, script_font_size, 10, CVAR_SAVE)

wxString functions;
wxArrayString func_names;
wxArrayString func_defs;

vector<style_t> styles;
style_t style_default =
{
	"default",		// Name
	100, 100, 100,	// Foreground
	255, 255, 255,	// Background
	0				// Flags
};

extern string app_path;

void load_styles()
{
	styles.clear();

	Tokenizer tz;
	tz.open_file("config/scripts/styles.cfg", 0, 0);

	string token = tz.get_token();

	while (token != "!END")
	{
		if (token == "style")
		{
			style_t newstyle = style_default;
			newstyle.name = tz.get_token();

			tz.check_token("{");
			token = tz.get_token();

			while (token != "}")
			{
				if (token == "foreground")
				{
					tz.check_token("{");
					newstyle.fred = tz.get_integer();
					newstyle.fgreen = tz.get_integer();
					newstyle.fblue = tz.get_integer();
					tz.check_token("}");
				}

				if (token == "background")
				{
					tz.check_token("{");
					newstyle.bred = tz.get_integer();
					newstyle.bgreen = tz.get_integer();
					newstyle.bblue = tz.get_integer();
					tz.check_token("}");
				}

				if (token == "bold")
					newstyle.flags |= STYLE_BOLD;

				if (token == "italic")
					newstyle.flags |= STYLE_ITALIC;

				if (token == "underlined")
					newstyle.flags |= STYLE_UNDERLINE;

				if (token == "lowercase")
					newstyle.flags |= STYLE_LOWERCASE;

				if (token == "uppercase")
					newstyle.flags |= STYLE_UPPERCASE;

				token = tz.get_token();
			}

			if (newstyle.name == "default")
				memcpy(&style_default, &newstyle, sizeof(style_t));

			styles.push_back(newstyle);
		}

		token = tz.get_token();
	}
}

style_t *get_style(string name)
{
	for (int a = 0; a < styles.size(); a++)
	{
		if (styles[a].name == name)
			return &styles[a];
	}

	return &style_default;
}

ScriptText::ScriptText(wxWindow *parent, int id)
:	wxScintilla(parent, id)
{
	// Set monospace font
	//StyleSetFont(wxSCI_STYLE_DEFAULT, wxFont(script_font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	wxFont monoFont(script_font_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	StyleSetFont(wxSCI_STYLE_DEFAULT, monoFont);

	// Set tab stuff
	SetTabWidth(4);
	SetUseTabs(true);
    SetTabIndents(false);
    SetBackSpaceUnIndents(true);
	SetIndent(0);

	StyleClearAll();
	SetLexer(wxSCI_LEX_CPPNOCASE);
	CallTipUseStyle(0);

	// Setup hilight colours
	load_styles();

	setup_style(0, "default");
	setup_style(1, "comment");
	setup_style(2, "comment");
	setup_style(3, "comment");
	setup_style(4, "number");
	setup_style(5, "keyword");
	setup_style(6, "string");
	setup_style(7, "string");
	setup_style(8, "default");
	setup_style(9, "preprocessor");
	setup_style(10, "operator");
	setup_style(11, "default");
	setup_style(12, "default");
	setup_style(13, "default");
	setup_style(14, "default");
	setup_style(15, "comment");
	setup_style(16, "function");
	setup_style(17, "keyword");
	setup_style(18, "default");
	setup_style(32, "default");
	setup_style(wxSCI_STYLE_CALLTIP, "number");
	setup_style(wxSCI_STYLE_BRACEBAD, "function");
	setup_style(wxSCI_STYLE_BRACELIGHT, "keyword");

	//StyleSetFont(wxSCI_STYLE_CALLTIP, wxFont(8, 0, 0, 0));
	wxFont tmpFont(8, 0, 0, 0);
	StyleSetFont(wxSCI_STYLE_CALLTIP, tmpFont); 

	// Keywords
	SetKeyWords(0, _T("script int fixed world global if for while do return function \
						void open enter respawn death lightning unloading disconnect \
						const continue default else restart str suspend terminate until"));

	// Functions (only read the names for now)
	Tokenizer tz;
	tz.open_file("config/scripts/acs.cfg", 0, 0);

	string token = tz.get_token();

	while (token != "!END")
	{
		// Read function name
		functions += str_to_wx(token);
		functions += _T(" ");
		func_names.Add(str_to_wx(token));

		// skip "="
		tz.get_token();

		// skip args
		//tz.get_token();
		func_defs.Add(str_to_wx(tz.get_token()));

		token = tz.get_token();
	}

	SetKeyWords(1, functions.Lower());

	// Show line numbers in margin
	SetMarginType(1, 1);

	AutoCompSetIgnoreCase(true);
}

ScriptText::~ScriptText()
{
}

void ScriptText::setup_style(int style, string name)
{
	style_t *s = get_style(name);
	StyleSetForeground(style, wxColor(s->fred, s->fgreen, s->fblue));
	StyleSetBackground(style, wxColor(s->bred, s->bgreen, s->bblue));
	StyleSetBold(style, s->flags & STYLE_BOLD);
	StyleSetItalic(style, s->flags & STYLE_ITALIC);
	StyleSetUnderline(style, s->flags & STYLE_UNDERLINE);
	
	if (s->flags & STYLE_UPPERCASE)
		StyleSetCase(style, 1);

	if (s->flags & STYLE_LOWERCASE)
		StyleSetCase(style, 2);
}

void ScriptText::set_margin_widths()
{
	SetMarginWidth(1, TextWidth(wxSCI_STYLE_LINENUMBER, wxString::Format(_T("_%d"), GetLineCount())));
}

void ScriptText::char_added(wxScintillaEvent &event)
{
	char chr = event.GetKey();

	// Get token previous to cursor
	int pos1 = GetCurrentPos() - 1;
	int pos2 = GetCurrentPos() - 1;

	bool done = false;
	while (!done)
	{
		pos1--;
		char c = GetCharAt(pos1);
		if (c == ' ' ||
			c == '\n' ||
			c == '\t' ||
			pos1 <= 0)
			done = true;
	}

	wxString token = GetTextRange(pos1, pos2);

	// Auto indent
    int currentLine = GetCurrentLine();
    if (chr == '\n')
	{
        int lineInd = 0;

        if (currentLine > 0)
            lineInd = GetLineIndentation(currentLine - 1);

        if (lineInd == 0)
			return;

        SetLineIndentation(currentLine, lineInd);
		LineEnd();
        //GotoPos(PositionFromLine(currentLine) + lineInd);
    }

	if (chr == '(')
	{
		if (pos1 >= 0 && pos2 >= 0)
		{
			token = GetTextRange(pos1+1, pos2);
			//CallTipShow(GetCurrentPos(), token);

			for (int a = 0; a < func_names.size(); a++)
			{
				if (func_names[a].Upper() == token.Upper())
				{
					CallTipShow(GetCurrentPos(), func_defs[a]);
					break;
				}
			}
		}
	}

	if (chr == ')')
		CallTipCancel();

	// Brace hilight
	/*
	if (chr == '(' || chr == '{' || chr == '[' || chr == '<' ||
		chr == ')' || chr == '}' || chr == ']' || chr == '>')
	{
		int pos2 = BraceMatch(GetCurrentPos()-1);

		if (pos2 == wxSCI_INVALID_POSITION)
			BraceBadLight(GetCurrentPos()-1);
		else
			BraceHighlight(GetCurrentPos()-1, pos2);
	}
	else
		BraceHighlight(wxSCI_INVALID_POSITION, wxSCI_INVALID_POSITION);
		*/
}

BEGIN_EVENT_TABLE(ScriptText, wxScintilla)
	EVT_KEY_DOWN(ScriptText::key_down)
END_EVENT_TABLE()

void ScriptText::key_down(wxKeyEvent &event)
{
	if (event.ControlDown() && event.GetKeyCode() == WXK_SPACE)
	{
		// Get token previous to cursor
		int pos1 = GetCurrentPos() - 1;
		int pos2 = GetCurrentPos() - 1;

		bool done = false;
		while (!done)
		{
			pos1--;
			char c = GetCharAt(pos1);
			if (c == ' ' ||
				c == '\n' ||
				c == '\t' ||
				pos1 <= 0)
				done = true;
		}

		wxString token = GetTextRange(pos1, pos2);
		AutoCompShow(token.size(), functions);
	}

	event.Skip();
}


ScriptEditor::ScriptEditor(wxWindow *parent)
:	wxFrame(parent, -1, _T("Script Editor"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE|wxFRAME_FLOAT_ON_PARENT)
{
	wxPanel	*back_panel = new wxPanel(this, -1);
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	back_panel->SetSizer(m_vbox);

	text_script = new ScriptText(back_panel, SE_TEXT_SCRIPT);

	if (map.opened)
		text_script->SetText(str_to_wx(string((char*)map.scripts->Data(), map.scripts->Size())));

	text_script->set_margin_widths();

	m_vbox->Add(text_script, 1, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	hbox->AddStretchSpacer();

	btn_savechanges = new wxButton(back_panel, SE_BTN_SAVECHANGES, _T("Save Changes"));
	hbox->Add(btn_savechanges, 0, wxRIGHT|wxEXPAND, 4);

	btn_compile = new wxButton(back_panel, SE_BTN_COMPILE, _T("Compile Script"));
	hbox->Add(btn_compile, 0, wxEXPAND);

	SetIcon(wxIcon(_T("res/slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	SetSize(wxSize(640, 480));
}

ScriptEditor::~ScriptEditor()
{
}

BEGIN_EVENT_TABLE(ScriptEditor, wxFrame)
	EVT_CLOSE(ScriptEditor::close)
	EVT_BUTTON(SE_BTN_SAVECHANGES, ScriptEditor::save_changes_clicked)
	EVT_BUTTON(SE_BTN_COMPILE, ScriptEditor::compile_clicked)
	EVT_SCI_CHARADDED(SE_TEXT_SCRIPT, ScriptEditor::text_changed)
END_EVENT_TABLE()

void ScriptEditor::close(wxCloseEvent &event)
{
	if (!event.CanVeto())
	{
		delete this;
		return;
	}

	Hide();
}

void ScriptEditor::save_changes_clicked(wxCommandEvent &event)
{
	if (!map.opened)
		return;

	//string content = wx_to_str(text_script->GetValue());
	string content = wx_to_str(text_script->GetText());
	map.scripts->LoadData((BYTE*)content.c_str(), content.size());
}

void ScriptEditor::compile_clicked(wxCommandEvent &event)
{
	if (!map.opened)
		return;

	string o_path = parse_string("%ssladetemp.o", app_path.c_str());
	string acs_path = parse_string("%ssladetemp.acs", app_path.c_str());

	remove(o_path.c_str());

	// Save the script
	//string content = wx_to_str(text_script->GetValue());
	string content = wx_to_str(text_script->GetText());
	map.scripts->LoadData((BYTE*)content.c_str(), content.size());
	map.scripts->DumpToFile(acs_path);

	// Compile it
	string path = path_acc;
	if (wxExecute(wxString::Format(_T("%s %s %s"), path.c_str(), acs_path.c_str(), o_path.c_str()), wxEXEC_SYNC) == -1)
	{
		wxLogMessage(_T("Acc(.exe) not found! Please check the path (edit->preferences->paths)"));
		return;
	}

	//system("acc sladetemp.acs sladetemp.o");

	// Open the output
	FILE* fp = fopen(o_path.c_str(), "rb");

	// If output doesn't exist some kind of error occurred
	if (!fp)
	{
		fp = fopen(parse_string("%sacs.err", app_path.c_str()).c_str(), "rt");

		if (!fp)
			wxMessageBox(_T("Compile failed"), _T("Error"), wxICON_ERROR);
		else
		{
			// Open a dialog with the contents of acs.err
			fseek(fp, 0, SEEK_END);
			int len = ftell(fp);
			char* text = (char*)malloc(len);
			fseek(fp, 0, SEEK_SET);
			fread(text, 1, len, fp);
			string error = text;
			free(text);

			wxDialog dlg(NULL, -1, _T("ACC Error Message"), wxDefaultPosition, wxDefaultSize);
			wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
			dlg.SetSizer(vbox);
			wxTextCtrl *error_text = new wxTextCtrl(&dlg, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
			error_text->SetFont(wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
			error_text->SetValue(str_to_wx(string(error.c_str(), error.size())));
			vbox->Add(error_text, 1, wxEXPAND|wxALL, 4);
			vbox->Add(dlg.CreateButtonSizer(wxOK), 0, wxEXPAND|wxALL, 4);

			dlg.ShowModal();

			fclose(fp);
		}
	}
	else
	{
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		BYTE* data = (BYTE*)malloc(len);
		fseek(fp, 0, SEEK_SET);
		fread(data, 1, len, fp);

		map.behavior->LoadData(data, len);

		fclose(fp);
		remove(o_path.c_str());

		wxMessageBox(_T("Compiled Successfully"));
	}
}

void ScriptEditor::text_changed(wxScintillaEvent &event)
{
	text_script->set_margin_widths();
	text_script->char_added(event);
}
