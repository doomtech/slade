
#include "main.h"
#include "wx_stuff.h"
#include "script_editor.h"

ScriptView::ScriptView(wxWindow *parent)
:	wxScintilla(parent, -1)
{
}

ScriptView::~ScriptView()
{
}




ScriptEditor::ScriptEditor(wxWindow *parent)
:	wxFrame(parent, -1, _T("Script Editor"))
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	script_view = new ScriptView(this);
	m_vbox->Add(script_view, 1, wxEXPAND|wxALL, 4);
}

ScriptEditor::~ScriptEditor()
{
}
