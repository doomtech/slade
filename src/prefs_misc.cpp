
#include "main.h"
#include "prefs_dialog.h"

EXTERN_CVAR(Int, node_builder)

EXTERN_CVAR(String, path_doomexe)
EXTERN_CVAR(String, path_zdbsp)
EXTERN_CVAR(String, path_zennode)
EXTERN_CVAR(String, path_bsp)
EXTERN_CVAR(String, path_glbsp)
EXTERN_CVAR(String, run_parameters)

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
	entry_path_doomexe = new wxTextCtrl(this, MP_ENTRY_DOOMEXE, str_to_wx(ini));
	hbox->Add(entry_path_doomexe, 1, wxEXPAND);
	btn_browse_doomexe = new wxButton(this, MP_BTN_BROWSE_DOOMEXE, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_browse_doomexe, 0, wxEXPAND|wxLEFT, 4);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND);

	ini = run_parameters;
	hbox->Add(new wxStaticText(this, -1, _T("Parameters: ")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	entry_runparams = new wxTextCtrl(this, MP_ENTRY_RUNPARAMS, str_to_wx(ini));
	hbox->Add(entry_runparams, 1, wxEXPAND);
	box->Add(new wxStaticText(this, -1, _T("%i = Current IWAD, %w = Current PWAD,\n%a = All Open PWADs, %m = Current Map Name")), 0, wxEXPAND);

	// Node builder frame
	frame = new wxStaticBox(this, -1, _T("Node Builder"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);

	hbox->Add(new wxStaticText(this, -1, _T("Node Builder:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);

	wxString nodebuilders[] = {
		_T("ZDBSP"),
		_T("ZenNode"),
		_T("BSP"),
	};

	combo_nodebuilder = new wxChoice(this, MP_COMBO_NODEBUILDER, wxDefaultPosition, wxDefaultSize, 3, nodebuilders);
	combo_nodebuilder->SetSelection(node_builder);
	hbox->Add(combo_nodebuilder, 1, wxEXPAND);

	// Zdbsp
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);

	ini = path_zdbsp;
	hbox->Add(new wxStaticText(this, -1, _T("ZDBSP: "), wxDefaultPosition, wxSize(80, -1)), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	entry_path_zdbsp = new wxTextCtrl(this, MP_ENTRY_ZDBSP, str_to_wx(ini));
	hbox->Add(entry_path_zdbsp, 1, wxEXPAND);
	btn_browse_zdbsp = new wxButton(this, MP_BTN_BROWSE_ZDBSP, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_browse_zdbsp, 0, wxEXPAND|wxLEFT, 4);

	// Zennode
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);

	ini = path_zennode;
	hbox->Add(new wxStaticText(this, -1, _T("ZenNode: "), wxDefaultPosition, wxSize(80, -1)), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	entry_path_zennode = new wxTextCtrl(this, MP_ENTRY_ZENNODE, str_to_wx(ini));
	hbox->Add(entry_path_zennode, 1, wxEXPAND);
	btn_browse_zennode = new wxButton(this, MP_BTN_BROWSE_ZENNODE, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_browse_zennode, 0, wxEXPAND|wxLEFT, 4);

	// BSP
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);

	ini = path_bsp;
	hbox->Add(new wxStaticText(this, -1, _T("BSP: "), wxDefaultPosition, wxSize(80, -1)), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	entry_path_bsp = new wxTextCtrl(this, MP_ENTRY_BSP, str_to_wx(ini));
	hbox->Add(entry_path_bsp, 1, wxEXPAND);
	btn_browse_bsp = new wxButton(this, MP_BTN_BROWSE_BSP, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_browse_bsp, 0, wxEXPAND|wxLEFT, 4);

	// glBSP
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxBOTTOM, 4);

	ini = path_glbsp;
	hbox->Add(new wxStaticText(this, -1, _T("GLBSP: "), wxDefaultPosition, wxSize(80, -1)), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	entry_path_glbsp = new wxTextCtrl(this, MP_ENTRY_GLBSP, str_to_wx(ini));
	hbox->Add(entry_path_glbsp, 1, wxEXPAND);
	btn_browse_glbsp = new wxButton(this, MP_BTN_BROWSE_GLBSP, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_browse_glbsp, 0, wxEXPAND|wxLEFT, 4);

	init = true;
}

MiscPrefs::~MiscPrefs()
{
}

BEGIN_EVENT_TABLE(MiscPrefs, wxPanel)
	EVT_TEXT(MP_ENTRY_DOOMEXE, MiscPrefs::entry_doomexe_changed)
	EVT_TEXT(MP_ENTRY_RUNPARAMS, MiscPrefs::entry_runparams_changed)
	EVT_BUTTON(MP_BTN_BROWSE_DOOMEXE, MiscPrefs::btn_doomexe_clicked)

	EVT_CHOICE(MP_COMBO_NODEBUILDER, MiscPrefs::combo_nodebuilder_changed)
	EVT_TEXT(MP_ENTRY_ZDBSP, MiscPrefs::entry_zdbsp_changed)
	EVT_TEXT(MP_ENTRY_ZENNODE, MiscPrefs::entry_zennode_changed)
	EVT_TEXT(MP_ENTRY_BSP, MiscPrefs::entry_bsp_changed)
	EVT_TEXT(MP_ENTRY_GLBSP, MiscPrefs::entry_glbsp_changed)
	EVT_BUTTON(MP_BTN_BROWSE_ZDBSP, MiscPrefs::btn_zdbsp_clicked)
	EVT_BUTTON(MP_BTN_BROWSE_ZENNODE, MiscPrefs::btn_zennode_clicked)
	EVT_BUTTON(MP_BTN_BROWSE_BSP, MiscPrefs::btn_bsp_clicked)
	EVT_BUTTON(MP_BTN_BROWSE_GLBSP, MiscPrefs::btn_glbsp_clicked)
END_EVENT_TABLE()

void MiscPrefs::entry_doomexe_changed(wxCommandEvent &event)
{
	if (init)
		path_doomexe = wx_to_str(entry_path_doomexe->GetValue());
}

void MiscPrefs::entry_runparams_changed(wxCommandEvent &event)
{
	if (init)
		run_parameters = wx_to_str(entry_runparams->GetValue());
}

void MiscPrefs::btn_doomexe_clicked(wxCommandEvent &event)
{
	wxString path = wxFileSelector(_T("Browse For Doom Executable"), entry_path_doomexe->GetValue(),
									_T(""), _T("*.exe"), _T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxFILE_MUST_EXIST|wxOPEN);

	entry_path_doomexe->SetValue(path);
}

void MiscPrefs::combo_nodebuilder_changed(wxCommandEvent &event)
{
	node_builder = combo_nodebuilder->GetSelection();
}

void MiscPrefs::entry_zdbsp_changed(wxCommandEvent &event)
{
	if (init)
		path_zdbsp = wx_to_str(entry_path_zdbsp->GetValue());
}

void MiscPrefs::entry_zennode_changed(wxCommandEvent &event)
{
	if (init)
		path_zennode = wx_to_str(entry_path_zennode->GetValue());
}

void MiscPrefs::entry_bsp_changed(wxCommandEvent &event)
{
	if (init)
		path_bsp = wx_to_str(entry_path_bsp->GetValue());
}

void MiscPrefs::entry_glbsp_changed(wxCommandEvent &event)
{
	if (init)
		path_glbsp = wx_to_str(entry_path_glbsp->GetValue());
}

void MiscPrefs::btn_zdbsp_clicked(wxCommandEvent &event)
{
	wxString path = wxFileSelector(_T("Browse For ZDBSP"), entry_path_zdbsp->GetValue(),
									_T(""), _T("*.exe"), _T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxFILE_MUST_EXIST|wxOPEN);

	entry_path_zdbsp->SetValue(path);
}

void MiscPrefs::btn_zennode_clicked(wxCommandEvent &event)
{
	wxString path = wxFileSelector(_T("Browse For ZenNode"), entry_path_zennode->GetValue(),
									_T(""), _T("*.exe"), _T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxFILE_MUST_EXIST|wxOPEN);

	entry_path_zennode->SetValue(path);
}

void MiscPrefs::btn_bsp_clicked(wxCommandEvent &event)
{
	wxString path = wxFileSelector(_T("Browse For BSP"), entry_path_bsp->GetValue(),
									_T(""), _T("*.exe"), _T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxFILE_MUST_EXIST|wxOPEN);

	entry_path_bsp->SetValue(path);
}

void MiscPrefs::btn_glbsp_clicked(wxCommandEvent &event)
{
	wxString path = wxFileSelector(_T("Browse For GLBSP"), entry_path_glbsp->GetValue(),
									_T(""), _T("*.exe"), _T("Executable files (*.exe)|*.exe|All Files (*.*)|*.*"), wxFILE_MUST_EXIST|wxOPEN);

	entry_path_glbsp->SetValue(path);
}
