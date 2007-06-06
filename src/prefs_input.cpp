
#include "main.h"
#include "prefs_dialog.h"
#include "wad.h"

extern BindList binds;
extern Wad reswad;

InputTextCtrl::InputTextCtrl(wxWindow *parent, int id)
:	wxTextCtrl(parent, id, _T(""), wxDefaultPosition, wxSize(64, -1))
{
	btn_shift = NULL;
	btn_ctrl = NULL;
	btn_alt = NULL;
}

InputTextCtrl::~InputTextCtrl()
{
}

void InputTextCtrl::set_buttons(wxToggleButton *s, wxToggleButton *c, wxToggleButton *a)
{
	btn_shift = s;
	btn_ctrl = c;
	btn_alt = a;
}

BEGIN_EVENT_TABLE(InputTextCtrl, wxTextCtrl)
	EVT_KEY_DOWN(InputTextCtrl::key_down)
END_EVENT_TABLE()

void InputTextCtrl::key_down(wxKeyEvent &event)
{
	SetValue(get_key_name(event.GetKeyCode()));

	btn_shift->SetValue(event.ShiftDown());
	btn_ctrl->SetValue(event.ControlDown());
	btn_alt->SetValue(event.AltDown());
}

InputControlDialog::InputControlDialog(wxWindow *parent, keybind_t *init)
:	wxDialog(parent, -1, _T("Set Key Binding"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	text_key = new InputTextCtrl(this, ICD_TEXT_KEY);
	vbox->Add(text_key, 0, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	hbox->Add(new wxStaticText(this, -1, _T("Other: ")), 0, wxEXPAND|wxRIGHT, 4);

	string okeys[] = {
		_T("Mouse1"),
		_T("Mouse2"),
		_T("Mouse3"),
		_T("MWheelUp"),
		_T("MWheelDown"),
		_T("Return"),
		_T("Tab"),
	};

	combo_keys = new wxChoice(this, ICD_COMBO_KEYS, wxDefaultPosition, wxDefaultSize, 7, okeys);
	hbox->Add(combo_keys, 1, wxEXPAND);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	btn_shift = new wxToggleButton(this, -1, _T("Shift"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_shift, 1, wxEXPAND|wxRIGHT, 4);

	btn_ctrl = new wxToggleButton(this, -1, _T("Ctrl"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_ctrl, 1, wxEXPAND|wxRIGHT, 4);

	btn_alt = new wxToggleButton(this, -1, _T("Alt"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_alt, 1, wxEXPAND);

	vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	if (init)
	{
		text_key->SetValue(init->key);
		btn_shift->SetValue(!!(init->mods & KMOD_SHIFT));
		btn_ctrl->SetValue(!!(init->mods & KMOD_CTRL));
		btn_alt->SetValue(!!(init->mods & KMOD_ALT));
	}

	text_key->set_buttons(btn_shift, btn_ctrl, btn_alt);

	SetBestFittingSize();
}

InputControlDialog::~InputControlDialog()
{
}

string InputControlDialog::get_key()
{
	return text_key->GetValue();
}

BYTE InputControlDialog::get_mods()
{
	BYTE ret = 0;
	if (btn_shift->GetValue())
		ret |= KMOD_SHIFT;
	if (btn_ctrl->GetValue())
		ret |= KMOD_CTRL;
	if (btn_alt->GetValue())
		ret |= KMOD_ALT;

	return ret;
}

BEGIN_EVENT_TABLE(InputControlDialog, wxDialog)
	EVT_CHOICE(ICD_COMBO_KEYS, InputControlDialog::combo_keys_changed)
END_EVENT_TABLE()

void InputControlDialog::combo_keys_changed(wxCommandEvent &event)
{
	text_key->SetValue(event.GetString());
}

InputPrefs::InputPrefs(wxWindow *parent)
:	wxPanel(parent, -1)
{
	wxBoxSizer *m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Controls frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Controls"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	m_hbox->Add(box, 0, wxEXPAND|wxALL, 4);

	list_controls = new wxListBox(this, IP_LIST_CONTROLS);
	box->Add(list_controls, 1, wxEXPAND|wxALL, 4);

	for (unsigned int a = 0; a < binds.n_keys(); a++)
		list_controls->Append(binds.get_bind(a)->name);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	m_hbox->Add(vbox, 0, wxEXPAND);

	// Binding frame
	frame = new wxStaticBox(this, -1, _T("Binding(s)"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	list_binds = new wxListBox(this, -1);
	box->Add(list_binds, 1, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND);

	btn_addbind = new wxButton(this, IP_BTN_ADDBIND, _T("Add"), wxDefaultPosition, wxDefaultSize);
	hbox->Add(btn_addbind, 1, wxEXPAND|wxRIGHT, 4);

	btn_removebind = new wxButton(this, IP_BTN_REMOVEBIND, _T("Del"), wxDefaultPosition, wxDefaultSize);
	hbox->Add(btn_removebind, 1, wxEXPAND);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxTOP, 4);

	btn_changebind = new wxButton(this, IP_BTN_CHANGEBIND, _T("Change"), wxDefaultPosition, wxDefaultSize);
	hbox->Add(btn_changebind, 1, wxEXPAND|wxRIGHT, 4);

	btn_defaultbind = new wxButton(this, IP_BTN_DEFAULTBIND, _T("Default"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_defaultbind, 1, wxEXPAND);

	// Config frame
	frame = new wxStaticBox(this, -1, _T("Configuration"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	btn_readconfig = new wxButton(this, IP_BTN_READCONFIG, _T("Load Config"));
	box->Add(btn_readconfig, 0, wxEXPAND|wxBOTTOM, 4);

	btn_loadconfig = new wxButton(this, IP_BTN_LOADCONFIG, _T("Load Custom Config"));
	box->Add(btn_loadconfig, 0, wxEXPAND|wxBOTTOM, 4);

	btn_saveconfig = new wxButton(this, IP_BTN_SAVECONFIG, _T("Save Custom Config"));
	box->Add(btn_saveconfig, 0, wxEXPAND|wxBOTTOM, 4);
}

InputPrefs::~InputPrefs()
{
}

BEGIN_EVENT_TABLE(InputPrefs, wxPanel)
	EVT_LISTBOX(IP_LIST_CONTROLS, InputPrefs::list_controls_changed)
	EVT_BUTTON(IP_BTN_READCONFIG, InputPrefs::btn_readconfig_clicked)
	EVT_BUTTON(IP_BTN_SAVECONFIG, InputPrefs::btn_saveconfig_clicked)
	EVT_BUTTON(IP_BTN_LOADCONFIG, InputPrefs::btn_loadconfig_clicked)
	EVT_BUTTON(IP_BTN_ADDBIND, InputPrefs::btn_addbind_clicked)
	EVT_BUTTON(IP_BTN_REMOVEBIND, InputPrefs::btn_removebind_clicked)
	EVT_BUTTON(IP_BTN_CHANGEBIND, InputPrefs::btn_changebind_clicked)
	EVT_BUTTON(IP_BTN_DEFAULTBIND, InputPrefs::btn_defaultbind_clicked)
END_EVENT_TABLE()

void InputPrefs::list_controls_changed(wxCommandEvent &event)
{
	list_binds->Clear();

	control_t *c = binds.get_bind(list_controls->GetSelection());

	if (!c)
		return;

	for (unsigned int a = 0; a < c->keys.size(); a++)
		list_binds->Append(c->keys[a].get_string());
}

void InputPrefs::btn_readconfig_clicked(wxCommandEvent &event)
{
	//binds.set_defaults();
	//list_controls_changed(event);

	vector<Lump*> config_lumps;
	wxArrayString configs;
	configs.Add(_T("SLADE Defaults"));

	for (DWORD a = 0; a < reswad.numLumps(); a++)
	{
		if (reswad.lumpAt(a)->getDir(1) == _T("keys"))
		{
			configs.Add(reswad.lumpAt(a)->getName(false, false));
			config_lumps.push_back(reswad.lumpAt(a));
		}
	}

	int index = wxGetSingleChoiceIndex(_T("Select a configuration to load"), _T("Load Config"), configs);

	if (index == 0)
	{
		binds.set_defaults();
		list_controls_changed(event);
	}
	else if (index > 0)
	{
		config_lumps[index-1]->dumpToFile(c_path(_T("sladetemp"), DIR_TMP));
		Tokenizer tz;
		tz.open_file(c_path(_T("sladetemp"), DIR_TMP));
		binds.load(&tz);
		remove(chr(c_path(_T("sladetemp"), DIR_TMP)));
	}
}

void InputPrefs::btn_loadconfig_clicked(wxCommandEvent &event)
{
	string filename = wxFileSelector(_T("Open Key Configuration"), c_path(_T("keys/"), DIR_USR), _T(""), _T("*.cfg"),
									_T("Configuration Files|*.cfg"), wxOPEN|wxFILE_MUST_EXIST);

	if (filename.size())
	{
		Tokenizer tz;
		tz.open_file(filename, 0, 0);
		binds.load(&tz);
	}
}

void InputPrefs::btn_saveconfig_clicked(wxCommandEvent &event)
{
	string filename = wxFileSelector(_T("Save Key Configuration"), c_path(_T("keys/"), DIR_USR), _T(""), _T("*.cfg"),
									_T("Configuration Files|*.cfg"), wxSAVE|wxOVERWRITE_PROMPT);

	if (filename.size())
	{
		FILE *fp = fopen(chr(filename), "wt");
		binds.save(fp);
		fclose(fp);
	}
}

void InputPrefs::btn_addbind_clicked(wxCommandEvent &event)
{
	control_t *c = binds.get_bind(list_controls->GetSelection());

	if (!c)
		return;

	InputControlDialog icd(this);
	if (icd.ShowModal() == wxID_OK && icd.get_key() != _T(""))
	{
		keybind_t kb;
		kb.key = icd.get_key();
		kb.mods = icd.get_mods();
		c->keys.push_back(kb);
		list_controls_changed(event);
	}
}

void InputPrefs::btn_removebind_clicked(wxCommandEvent &event)
{
	if (list_binds->GetSelection() == -1)
		return;

	control_t *c = binds.get_bind(list_controls->GetSelection());

	if (!c)
		return;

	if (list_binds->GetSelection() == 0 && c->keys.size() == 1)
		return;
	else
	{
		c->keys.erase(c->keys.begin() + list_binds->GetSelection());
		list_controls_changed(event);
	}
}

void InputPrefs::btn_changebind_clicked(wxCommandEvent &event)
{
	if (list_binds->GetSelection() == -1)
		return;

	control_t *c = binds.get_bind(list_controls->GetSelection());

	if (!c)
		return;

	InputControlDialog icd(this, &c->keys[list_binds->GetSelection()]);
	if (icd.ShowModal() == wxID_OK && icd.get_key() != _T(""))
	{
		c->keys[list_binds->GetSelection()].key = icd.get_key();
		c->keys[list_binds->GetSelection()].mods = icd.get_mods();
		list_controls_changed(event);
	}
}

void InputPrefs::btn_defaultbind_clicked(wxCommandEvent &event)
{
	control_t *c = binds.get_bind(list_controls->GetSelection());

	if (!c)
		return;

	binds.change_default(c->name);
	list_controls_changed(event);
}
