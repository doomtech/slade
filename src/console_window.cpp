
#include "main.h"
#include "console_window.h"
#include "console.h"

ConsoleWindow	*console_window;

extern wxString	console_log;
extern string cmd_line;

ConsoleEntry::ConsoleEntry(wxWindow *parent, int id)
:	wxTextCtrl(parent, id, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER)
{
}

ConsoleEntry::~ConsoleEntry()
{
}

BEGIN_EVENT_TABLE(ConsoleEntry, wxTextCtrl)
	EVT_KEY_DOWN(ConsoleEntry::key_down)
END_EVENT_TABLE()

void ConsoleEntry::key_down(wxKeyEvent &event)
{
	if (event.GetKeyCode() == WXK_UP)
	{
		console_prevcommand();
		SetValue(_T(""));
		WriteText(str_to_wx(cmd_line));
		SetInsertionPointEnd();
	}
	else if (event.GetKeyCode() == WXK_DOWN)
	{
		console_nextcommand();
		SetValue(_T(""));
		WriteText(str_to_wx(cmd_line));
		SetInsertionPointEnd();
	}

	event.Skip();
}


ConsoleWindow::ConsoleWindow()
:	wxFrame((wxFrame*)NULL, -1, _T("SLADE Console"))
{
	wxPanel	*back_panel = new wxPanel(this, -1);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	back_panel->SetSizer(vbox);

	log = new wxTextCtrl(back_panel, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_MULTILINE);
	log->SetFont(wxFont(9, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	vbox->Add(log, 1, wxEXPAND);

	entry = new ConsoleEntry(back_panel, CW_ENTRY_COMMAND);
	entry->SetFocus();
	vbox->Add(entry, 0, wxEXPAND|wxTOP, 4);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	SetSizeHints(560, 380);
	Layout();
	Centre();
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::update_log()
{
	log->SetValue(console_log);
	log->SetInsertionPointEnd();
	log->ShowPosition(log->GetInsertionPoint());
}

BEGIN_EVENT_TABLE(ConsoleWindow, wxFrame)
	EVT_CLOSE(ConsoleWindow::close)
	EVT_TEXT_ENTER(CW_ENTRY_COMMAND, ConsoleWindow::cmd_activated)
END_EVENT_TABLE()

void ConsoleWindow::close(wxCloseEvent &event)
{
	if (!event.CanVeto())
	{
		delete this;
		return;
	}

	Hide();
}

void ConsoleWindow::cmd_activated(wxCommandEvent &event)
{
	cmd_line = wx_to_str(entry->GetValue());
	console_parsecommand();
	entry->SetValue(_T(""));
}
