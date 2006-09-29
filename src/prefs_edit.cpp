
#include "main.h"
#include "prefs_dialog.h"



EditPrefs::EditPrefs(wxWindow *parent)
:	wxPanel(parent, -1)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Options frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Options"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);
}

EditPrefs::~EditPrefs()
{
}

BEGIN_EVENT_TABLE(EditPrefs, wxPanel)
END_EVENT_TABLE()
