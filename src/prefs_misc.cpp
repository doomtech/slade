
#include "main.h"
#include "prefs_dialog.h"

EXTERN_CVAR(String, path_doomexe)
EXTERN_CVAR(String, run_parameters)
EXTERN_CVAR(String, nodebuilder_path)
EXTERN_CVAR(String, nodebuilder_params)
EXTERN_CVAR(Bool, nodebuilder_useinternal)

MiscPrefs::MiscPrefs(wxWindow *parent)
:	wxPanel(parent, -1)
{
	init = false;

	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Running frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Map Running"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);

	string ini = path_doomexe;
	hbox->Add(new wxStaticText(this, -1, _T("Executable: ")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	entry_path_doomexe = new wxTextCtrl(this, MP_ENTRY_DOOMEXE, ini);
	hbox->Add(entry_path_doomexe, 1, wxEXPAND);
	btn_browse_doomexe = new wxButton(this, MP_BTN_BROWSE_DOOMEXE, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_browse_doomexe, 0, wxEXPAND|wxLEFT, 4);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND);

	ini = run_parameters;
	hbox->Add(new wxStaticText(this, -1, _T("Parameters: ")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	entry_runparams = new wxTextCtrl(this, MP_ENTRY_RUNPARAMS, ini);
	hbox->Add(entry_runparams, 1, wxEXPAND);
	box->Add(new wxStaticText(this, -1, _T("%i = Current IWAD, %w = Current PWAD,\n%a = All Open PWADs, %m = Current Map Name")), 0, wxEXPAND);

	// Node builder frame
	frame = new wxStaticBox(this, -1, _T("Node Builder"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	cb_nodes_internal = new wxCheckBox(this, MP_CB_NODES_INTERNAL, _T("Use Internal Nodebuilder (ZDBSP)"));
	box->Add(cb_nodes_internal, 0, wxEXPAND|wxALL, 4);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND);

	ini = nodebuilder_path;
	hbox->Add(new wxStaticText(this, -1, _T("External Nodebuilder: ")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	entry_path_nodebuilder = new wxTextCtrl(this, MP_ENTRY_PATH_NODES, ini);
	hbox->Add(entry_path_nodebuilder, 1, wxEXPAND);
	btn_browse_nodebuilder = new wxButton(this, MP_BTN_BROWSE_NODES, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_browse_nodebuilder, 0, wxEXPAND|wxLEFT, 4);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxTOP, 4);

	ini = nodebuilder_params;
	hbox->Add(new wxStaticText(this, -1, _T("Parameters: ")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	entry_nodes_params = new wxTextCtrl(this, MP_ENTRY_NODES_PARAMS, ini);
	hbox->Add(entry_nodes_params, 1, wxEXPAND);
	box->Add(new wxStaticText(this, -1, _T("%w = Wadfile Path, %t = Tempfile Path, %m = Current Map Name")), 0, wxEXPAND|wxTOP, 4);

	init = true;

	if (nodebuilder_useinternal) cb_nodes_internal->SetValue(true);

	if (cb_nodes_internal->IsChecked())
	{
		entry_path_nodebuilder->Enable(false);
		entry_nodes_params->Enable(false);
		btn_browse_nodebuilder->Enable(false);
	}
	else
	{
		entry_path_nodebuilder->Enable(true);
		entry_nodes_params->Enable(true);
		btn_browse_nodebuilder->Enable(true);
	}
}

MiscPrefs::~MiscPrefs()
{
}

BEGIN_EVENT_TABLE(MiscPrefs, wxPanel)
	EVT_TEXT(MP_ENTRY_DOOMEXE, MiscPrefs::entry_doomexe_changed)
	EVT_TEXT(MP_ENTRY_RUNPARAMS, MiscPrefs::entry_runparams_changed)
	EVT_BUTTON(MP_BTN_BROWSE_DOOMEXE, MiscPrefs::btn_doomexe_clicked)

	EVT_CHECKBOX(MP_CB_NODES_INTERNAL, MiscPrefs::cb_nodes_internal_toggled)
	EVT_TEXT(MP_ENTRY_PATH_NODES, MiscPrefs::entry_nodepath_changed)
	EVT_TEXT(MP_ENTRY_NODES_PARAMS, MiscPrefs::entry_nodeparams_changed)
	EVT_BUTTON(MP_BTN_BROWSE_NODES, MiscPrefs::btn_nodepath_clicked)
END_EVENT_TABLE()

void MiscPrefs::entry_doomexe_changed(wxCommandEvent &event)
{
	if (init)
		path_doomexe = entry_path_doomexe->GetValue();
}

void MiscPrefs::entry_runparams_changed(wxCommandEvent &event)
{
	if (init)
		run_parameters = entry_runparams->GetValue();
}

void MiscPrefs::btn_doomexe_clicked(wxCommandEvent &event)
{
	wxString path = wxFileSelector(_T("Browse For Doom Executable"), entry_path_doomexe->GetValue(),
									_T(""), _T("*.exe"), _T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxFILE_MUST_EXIST|wxOPEN);

	entry_path_doomexe->SetValue(path);
}

void MiscPrefs::entry_nodepath_changed(wxCommandEvent &event)
{
	if (init)
		nodebuilder_path = entry_path_nodebuilder->GetValue();
}

void MiscPrefs::entry_nodeparams_changed(wxCommandEvent &event)
{
	if (init)
		nodebuilder_params = entry_nodes_params->GetValue();
}

void MiscPrefs::btn_nodepath_clicked(wxCommandEvent &event)
{
	wxString path = wxFileSelector(_T("Browse For Node Builder"), entry_path_nodebuilder->GetValue(),
									_T(""), _T("*.exe"), _T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxFILE_MUST_EXIST|wxOPEN);

	entry_path_nodebuilder->SetValue(path);
}

void MiscPrefs::cb_nodes_internal_toggled(wxCommandEvent &event)
{
	if (!init) return;

	if (cb_nodes_internal->IsChecked())
	{
		nodebuilder_useinternal = true;
		entry_path_nodebuilder->Enable(false);
		entry_nodes_params->Enable(false);
		btn_browse_nodebuilder->Enable(false);
	}
	else
	{
		nodebuilder_useinternal = false;
		entry_path_nodebuilder->Enable(true);
		entry_nodes_params->Enable(true);
		btn_browse_nodebuilder->Enable(true);
	}
}
