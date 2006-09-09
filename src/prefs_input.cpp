
#include "main.h"
#include "prefs_dialog.h"

extern BindList binds;

/*
InputTextCtrl::InputTextCtrl(wxWindow *parent, int id)
:	wxTextCtrl(parent, id, _T(""), wxDefaultPosition, wxSize(112, -1), wxTE_CENTRE|wxWANTS_CHARS|wxTE_PROCESS_TAB|wxTE_PROCESS_ENTER)
{
	Enable(false);
}

InputTextCtrl::~InputTextCtrl()
{
}

void InputTextCtrl::update()
{
	SetValue(key);

	string smods = _T("Modifiers: ");

	bool none = true;
	if (mods & KMOD_SHIFT)
	{
		none = false;
		smods += _T("Shift");
	}

	if (mods & KMOD_CTRL)
	{
		if (none)
			smods += _T("Ctrl");
		else
			smods += _T("+Ctrl");

		none = false;
	}

	if (mods & KMOD_ALT)
	{
		if (none)
			smods += _T("Alt");
		else
			smods += _T("+Alt");

		none = false;
	}

	if (none)
		smods += _T("None");

	mod_label->SetLabel(smods);
}

void InputTextCtrl::set_bind(keybind_t *b)
{
	bind = b;
	key = b->key;
	mods = b->mods;
	update();
}

void InputTextCtrl::accept_changes()
{
	bind->key = key;
	bind->mods = mods;
}

void InputTextCtrl::cancel_changes()
{
	key = bind->key;
	mods = bind->mods;
	update();
}

void InputTextCtrl::set_default()
{
	//bind->key = bind->defaultkey;
	//bind->mods = bind->defaultmods;
	cancel_changes();
}

BEGIN_EVENT_TABLE(InputTextCtrl, wxTextCtrl)
	EVT_KEY_DOWN(InputTextCtrl::key_down)
	EVT_CHAR(InputTextCtrl::key_char)
	EVT_MOUSE_EVENTS(InputTextCtrl::mouse_event)
END_EVENT_TABLE()

void InputTextCtrl::key_down(wxKeyEvent &event)
{
	key = get_key_name(event.GetKeyCode());

	mods = 0;

	if (event.ShiftDown())
		mods |= KMOD_SHIFT;

	if (event.ControlDown())
		mods |= KMOD_CTRL;

	if (event.AltDown())
		mods |= KMOD_ALT;

	update();
	event.Skip();
}

void InputTextCtrl::key_char(wxKeyEvent &event)
{
	if (event.GetKeyCode() == WXK_TAB)
	{
		key = get_key_name(WXK_TAB);

		mods = 0;

		if (event.ShiftDown())
			mods |= KMOD_SHIFT;

		if (event.ControlDown())
			mods |= KMOD_CTRL;

		if (event.AltDown())
			mods |= KMOD_ALT;
	}

	update();
	event.Skip();
}

void InputTextCtrl::mouse_event(wxMouseEvent &event)
{
	SetFocus();

	// Mouse buttons
	if (event.ButtonDown())
	{
		if (event.Button(wxMOUSE_BTN_LEFT))
			key = _T("Mouse1");

		if (event.Button(wxMOUSE_BTN_RIGHT))
			key = _T("Mouse3");

		if (event.Button(wxMOUSE_BTN_MIDDLE))
			key = _T("Mouse2");

		if (event.ShiftDown())
			mods |= KMOD_SHIFT;

		if (event.ControlDown())
			mods |= KMOD_CTRL;

		if (event.AltDown())
			mods |= KMOD_ALT;

		update();
		return;
	}

	// Mouse wheel
	if (event.GetWheelRotation() > 0)
	{
		key = _T("MWheel Up");

		if (event.ShiftDown())
			mods |= KMOD_SHIFT;

		if (event.ControlDown())
			mods |= KMOD_CTRL;

		if (event.AltDown())
			mods |= KMOD_ALT;

		update();
		return;
	}
	else if (event.GetWheelRotation() < 0)
	{
		key = _T("MWheel Down");

		if (event.ShiftDown())
			mods |= KMOD_SHIFT;

		if (event.ControlDown())
			mods |= KMOD_CTRL;

		if (event.AltDown())
			mods |= KMOD_ALT;

		update();
		return;
	}
}
*/


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
		btn_shift->SetValue(init->mods & KMOD_SHIFT);
		btn_ctrl->SetValue(init->mods & KMOD_CTRL);
		btn_alt->SetValue(init->mods & KMOD_ALT);
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

	for (int a = 0; a < binds.n_keys(); a++)
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

	btn_addbind = new wxButton(this, IP_BTN_ADDBIND, _T("Add"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_addbind, 0, wxEXPAND|wxALL, 4);

	btn_removebind = new wxButton(this, IP_BTN_REMOVEBIND, _T("Del"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_removebind, 0, wxEXPAND|wxALL, 4);

	btn_changebind = new wxButton(this, IP_BTN_CHANGEBIND, _T("Change"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_changebind, 0, wxEXPAND|wxALL, 4);

	btn_defaultbind = new wxButton(this, IP_BTN_DEFAULTBIND, _T("Default"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_defaultbind, 0, wxEXPAND|wxALL, 4);

	// Config frame
	frame = new wxStaticBox(this, -1, _T("Configuration"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	btn_defaults = new wxButton(this, IP_BTN_DEFAULTS, _T("Restore Defaults"));
	box->Add(btn_defaults, 0, wxEXPAND|wxBOTTOM, 4);

	btn_loadconfig = new wxButton(this, IP_BTN_LOADCONFIG, _T("Load Config"));
	box->Add(btn_loadconfig, 0, wxEXPAND|wxBOTTOM, 4);

	btn_saveconfig = new wxButton(this, IP_BTN_SAVECONFIG, _T("Save Config"));
	box->Add(btn_saveconfig, 0, wxEXPAND|wxBOTTOM, 4);
}

InputPrefs::~InputPrefs()
{
}

BEGIN_EVENT_TABLE(InputPrefs, wxPanel)
	EVT_LISTBOX(IP_LIST_CONTROLS, InputPrefs::list_controls_changed)
	EVT_BUTTON(IP_BTN_DEFAULTS, InputPrefs::btn_defaults_clicked)
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

	for (int a = 0; a < c->keys.size(); a++)
		list_binds->Append(c->keys[a].get_string());
}

void InputPrefs::btn_defaults_clicked(wxCommandEvent &event)
{
	binds.set_defaults();
	list_controls_changed(event);
}

void InputPrefs::btn_loadconfig_clicked(wxCommandEvent &event)
{
	string filename = wxFileSelector(_T("Open Key Configuration"), _T("./config/keys"), _T(""), _T("*.cfg"),
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
	string filename = wxFileSelector(_T("Save Key Configuration"), _T("./config/keys"), _T(""), _T("*.cfg"),
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
