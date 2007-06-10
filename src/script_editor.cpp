
#include "main.h"
#include "wx_stuff.h"
#include "script_editor.h"
#include "doom_map.h"

#include <wx/filename.h>

CVAR(String, path_acc, _T(""), CVAR_SAVE)

extern DoomMap d_map;

///////////////////////////////////////////////////////////////////
// wxScintilla Script Editor //////////////////////////////////////
///////////////////////////////////////////////////////////////////
#ifdef USE_WXSCINTILLA

ScriptView::ScriptView(wxWindow *parent, int id)
:	wxScintilla(parent, id)
{
	// Set monospace font
	wxFont tmpFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	StyleSetFont(wxSCI_STYLE_DEFAULT, tmpFont);

	// Set tab stuff
	SetTabWidth(4);
	SetUseTabs(true);
    SetTabIndents(false);
    SetBackSpaceUnIndents(true);
	SetIndent(0);

	// Show line numbers in margin
	SetMarginType(1, 1);
	set_margin_widths();
}

ScriptView::~ScriptView()
{
}

void ScriptView::set_margin_widths()
{
	SetMarginWidth(1, TextWidth(wxSCI_STYLE_LINENUMBER, wxString::Format(_T("_%d"), GetLineCount())));
}

void ScriptView::onTextChanged(wxScintillaEvent &event)
{
	char chr = event.GetKey();

	// Auto indent
    int current_line = GetCurrentLine();
    if (chr == '\n')
	{
        int line_ind = 0;

        if (current_line > 0)
            line_ind = GetLineIndentation(current_line - 1);

        if (line_ind != 0)
		{
			SetLineIndentation(current_line, line_ind);
			GotoPos(GetLineEndPosition(current_line));
		}

		set_margin_widths();
    }

	/*
	// Calltips
	if (chr == '(')
	{
		int pos1 = WordStartPosition(GetCurrentPos()-1, false);
		string token = GetTextRange(WordStartPosition(pos1, true), 
									WordEndPosition(pos1, true));

		funcdef_t *f = cur_lang->searchFunction(token);
		if (f)
			CallTipShow(GetCurrentPos(), f->func_args);
	}

	if (chr == ')')
		CallTipCancel();
		*/
}

BEGIN_EVENT_TABLE(ScriptView, wxScintilla)
	EVT_KEY_DOWN(ScriptView::key_down)
	EVT_SCI_CHARADDED(ScriptEditor::SE_SCRIPTVIEW, ScriptView::onTextChanged)
END_EVENT_TABLE()

void ScriptView::key_down(wxKeyEvent &event)
{
	/*
	if (event.ControlDown() && event.GetKeyCode() == WXK_SPACE)
	{
		// Get token previous to cursor
		wxString token = GetTextRange(WordStartPosition(GetCurrentPos(), true), GetCurrentPos());

		if (cur_lang->langName() != _T("None"))
			AutoCompShow(token.size(), cur_lang->getFunctions(false) + cur_lang->getKeywords(false) + cur_lang->getConstants(false));
	}
	*/

	event.Skip();
}



///////////////////////////////////////////////////////////////////
// wxTextCtrl Script Editor ///////////////////////////////////////
///////////////////////////////////////////////////////////////////
#else

ScriptView::ScriptView(wxWindow *parent, int id)
:	wxTextCtrl(parent, id, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_TAB|wxTE_MULTILINE|wxTE_RICH2)
{
	SetFont(wxFont(8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
}

ScriptView::~ScriptView()
{
}

#endif


ScriptEditor::ScriptEditor(wxWindow *parent)
:	wxFrame(parent, -1, _T("Script Editor"))
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	wxPanel *back_panel = new wxPanel(this, -1);
	m_vbox->Add(back_panel, 1, wxEXPAND);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	back_panel->SetSizer(vbox);

	script_view = new ScriptView(back_panel, SE_SCRIPTVIEW);
	vbox->Add(script_view, 1, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->AddStretchSpacer();
	vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	btn_save = new wxButton(back_panel, SE_BTN_SAVE, _T("Save Changes"));
	hbox->Add(btn_save, 0, wxEXPAND|wxRIGHT, 4);

	btn_compile = new wxButton(back_panel, SE_BTN_COMPILE, _T("Save and Compile"));
	hbox->Add(btn_compile, 0, wxEXPAND);

	SetSize(640, 480);
}

ScriptEditor::~ScriptEditor()
{
}

void ScriptEditor::load_scripts()
{
	Lump* scriptlump = d_map.get_scripts();

	script_view->Clear();

	string istr = wxString::FromAscii((char*)scriptlump->getData());
	istr.Truncate(scriptlump->getSize());

#ifdef USE_WXSCINTILLA
	script_view->SetText(istr);
	script_view->set_margin_widths();
#else
	script_view->SetValue(istr);
#endif
}

BEGIN_EVENT_TABLE(ScriptEditor, wxFrame)
	EVT_CLOSE(ScriptEditor::close)
	EVT_BUTTON(SE_BTN_SAVE, ScriptEditor::btn_save_clicked)
	EVT_BUTTON(SE_BTN_COMPILE, ScriptEditor::btn_compile_clicked)
END_EVENT_TABLE()

void ScriptEditor::close(wxCloseEvent &event)
{
	Hide();
}

void ScriptEditor::btn_save_clicked(wxCommandEvent &event)
{
	Lump* scriptlump = d_map.get_scripts();

#ifdef USE_WXSCINTILLA
	scriptlump->loadData((BYTE*)chr(script_view->GetText()), script_view->GetText().size());
#else
	scriptlump->loadData((BYTE*)chr(script_view->GetValue()), script_view->GetValue().size());
#endif
}

void ScriptEditor::btn_compile_clicked(wxCommandEvent &event)
{
	// Save scripts
	Lump* scriptlump = d_map.get_scripts();

#ifdef USE_WXSCINTILLA
	scriptlump->loadData((BYTE*)chr(script_view->GetText()), script_view->GetText().size());
#else
	scriptlump->loadData((BYTE*)chr(script_view->GetValue()), script_view->GetValue().size());
#endif

	// Check for acc path
	if (path_acc == _T(""))
	{
		wxMessageBox(_T("You must browse for acc.exe"), _T("ACC.exe Path Undefined"));

		wxFileDialog *OpenDialog = new wxFileDialog(this, _T("Choose file(s) to open"), wxEmptyString, wxEmptyString, 
			_T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxOPEN|wxFILE_MUST_EXIST, wxDefaultPosition);

		if (OpenDialog->ShowModal() == wxID_OK)
		{
			if (OpenDialog->GetPath() != _T(""))
				path_acc = OpenDialog->GetPath();
			else
				return;
		}
	}

	wxFileName fn(path_acc);
	string acc_dir = fn.GetPath(true);

	string acs_path = s_fmt(_T("%ssladetemp.acs"), acc_dir);
	string o_path = s_fmt(_T("%ssladetemp.o"), acc_dir);
	string err_path = s_fmt(_T("%sacs.err"), acc_dir);

	scriptlump->dumpToFile(acs_path);

	// Compile the script
	string temp = path_acc;
	if (wxExecute(wxString::Format(_T("\"%s\" \"%s\" \"%s\""), temp, acs_path, o_path), wxEXEC_SYNC) == -1)
	{
		wxMessageBox(_T("Acc(.exe) not found! Please check the path"));
		return;
	}

	// Remove temp acs file
	remove(chr(acs_path));

	// Open the output
	FILE* fp = fopen(chr(o_path), "rb");

	// If output doesn't exist some kind of error occurred
	if (!fp)
	{
		fp = fopen(chr(err_path), "rt");

		if (!fp) // acs.err not found, show generic error message
			wxMessageBox(_T("Compile failed, can't find acs.err"), _T("Error"), wxICON_ERROR);
		else
		{
			// If acs.err is found, Open a dialog with the contents of acs.err
			fseek(fp, 0, SEEK_END);
			int len = ftell(fp);
			char* text = (char*)malloc(len);
			fseek(fp, 0, SEEK_SET);
			fread(text, 1, len, fp);
			string error = wxString(text, wxConvISO8859_1);;
			free(text);

			wxDialog dlg(NULL, -1, _T("ACC Error Message"), wxDefaultPosition, wxDefaultSize);
			wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
			dlg.SetSizer(vbox);
			wxTextCtrl *error_text = new wxTextCtrl(&dlg, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
			error_text->SetFont(wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
			error_text->SetValue(error);
			vbox->Add(error_text, 1, wxEXPAND|wxALL, 4);
			vbox->Add(dlg.CreateButtonSizer(wxOK), 0, wxEXPAND|wxALL, 4);

			dlg.ShowModal();

			fclose(fp);

			remove(chr(err_path));
		}
	}
	else // Otherwise we compiled successfully
	{
		// Read the output and get it's length
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		BYTE* data = (BYTE*)malloc(len);
		fseek(fp, 0, SEEK_SET);
		fread(data, 1, len, fp);

		// Save data to behavior lump
		Lump* behavior = d_map.get_behavior();
		behavior->loadData(data, len);

		// Close & remove etc
		fclose(fp);
		remove(chr(o_path));

		wxMessageBox(_T("Compiled Successfully"));
	}
}
