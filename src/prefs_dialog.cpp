
#include "main.h"
#include "prefs_dialog.h"

PrefsDialog::PrefsDialog()
:	wxDialog(NULL, -1, _T("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	tabs = new wxNotebook(this, -1);
	m_vbox->Add(tabs, 1, wxEXPAND|wxALL, 2);

	tabs->AddPage(new VisualPrefs(tabs), _T("Visual"));
	tabs->AddPage(new ColourPrefs(tabs), _T("Colours"));
	tabs->AddPage(new InputPrefs(tabs), _T("Input"));
	tabs->AddPage(new RenderPrefs(tabs), _T("3d Mode"));
	//tabs->AddPage(new MiscPrefs(tabs), _T("Misc"));

	m_vbox->Add(CreateButtonSizer(wxOK), 0, wxEXPAND|wxALL, 4);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	Layout();
	SetBestFittingSize();
}

PrefsDialog::~PrefsDialog()
{
}
