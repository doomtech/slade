
#include "Main.h"
#include "WxStuff.h"
#include "ACSPrefsPanel.h"
#include <wx/filedlg.h>

EXTERN_CVAR(String, path_acc)

ACSPrefsPanel::ACSPrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Create sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "ACS Preferences");
	wxStaticBoxSizer *sizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// ACC.exe path
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(new wxStaticText(this, -1, "Location of acc executable:"), 0, wxALL, 4);
	text_accpath = new wxTextCtrl(this, -1, wxString(path_acc));
	hbox->Add(text_accpath, 1, wxEXPAND|wxRIGHT, 4);
	btn_browse_accpath = new wxButton(this, -1, "Browse");
	hbox->Add(btn_browse_accpath, 0, wxEXPAND);
	sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Bind events
	btn_browse_accpath->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ACSPrefsPanel::onBtnBrowseACCPath, this);
}

ACSPrefsPanel::~ACSPrefsPanel() {
}

void ACSPrefsPanel::applyPreferences() {
	path_acc = text_accpath->GetValue();
}



void ACSPrefsPanel::onBtnBrowseACCPath(wxCommandEvent& e) {
	// Setup acc executable file string
	string acc_exe = "acc";
#ifdef WIN32
	acc_exe += ".exe";	// exe extension in windows
#endif

	// Open file dialog
	wxFileDialog fd(this, "Browse for ACC Executable", wxEmptyString, acc_exe, acc_exe);
	if (fd.ShowModal() == wxID_OK)
		text_accpath->SetValue(fd.GetPath());
}
