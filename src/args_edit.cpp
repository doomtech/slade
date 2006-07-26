
#include "main.h"
#include "wx_stuff.h"
#include "args_edit.h"
#include "game_config.h"
#include "archive.h"

extern GameConfig game;

ArgValueDialog::ArgValueDialog(BYTE value, argtype_t *arg_type)
:	wxDialog(NULL, -1, _T("Change Arg Value"), wxDefaultPosition, wxDefaultSize)
{
	this->arg_type = arg_type;
	this->arg = value;

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(vbox);

	// Values
	if (arg_type->has_values)
	{
		wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
		vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

		hbox->Add(new wxStaticText(this, -1, _T("Value:")), 1, wxALIGN_CENTER_VERTICAL);

		wxArrayString values;
		int index = 0;
		for (int a = 0; a < arg_type->values.size(); a++)
		{
			if (!arg_type->values[a].flag)
			{
				values.Add(str_to_wx(arg_type->values[a].name));
				this->values.push_back(arg_type->values[a].value);
				if (arg_type->values[a].value == value)
					index = a;
			}
		}

		combo_value = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, values);
		combo_value->Select(index);
		hbox->Add(combo_value, 0, wxEXPAND|wxLEFT, 4);
	}

	// Flags
	if (arg_type->has_flags)
	{
		wxStaticBox *frame = new wxStaticBox(this, -1, _T("Flags"));
		wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
		vbox->Add(box, 0, wxEXPAND|wxALL, 4);

		for (int a = 0; a < arg_type->values.size(); a++)
		{
			if (arg_type->values[a].flag)
			{
				flags.push_back(arg_type->values[a].value);
				wxCheckBox *cbox = new wxCheckBox(this, -1, str_to_wx(arg_type->values[a].name));
				cbox_flags.push_back(cbox);
				box->Add(cbox, 0, wxEXPAND|wxBOTTOM, 2);

				if (arg_type->values[a].value & value)
					cbox->SetValue(true);
			}
		}
	}

	vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	Layout();
	SetBestFittingSize();
}

ArgValueDialog::~ArgValueDialog()
{
}

BYTE ArgValueDialog::get_value()
{
	BYTE val = 0;
	
	if (combo_value)
		val = values[combo_value->GetSelection()];

	for (int a = 0; a < cbox_flags.size(); a++)
	{
		if (cbox_flags[a]->GetValue())
			val |= flags[a];
	}

	return val;
}

ArgEdit::ArgEdit(wxWindow *parent, int val, string argname, argtype_t *arg_type)
:	wxPanel(parent, -1)
{
	this->argname = argname;
	this->arg_type = arg_type;

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(hbox);

	label_arg = new wxStaticText(this, -1, wxString::Format(_T("%s"), argname.c_str()));
	hbox->Add(label_arg, 1, wxALIGN_CENTER_VERTICAL);

	if (arg_type && val != -1)
		label_arg->SetLabel(wxString::Format(_T("%s: %s"), argname.c_str(), arg_type->get_name(val).c_str()));

#if wxCHECK_VERSION(2, 6, 2)
	label_arg->Wrap(200);
#endif

	entry_arg = new wxTextCtrl(this, AE_ENTRY_ARG, _T(""), wxDefaultPosition, wxSize(32, -1));
	hbox->Add(entry_arg, 0, wxRIGHT, 4);

	if (val != -1)
		entry_arg->SetValue(wxString::Format(_T("%d"), val));

	btn_change = new wxButton(this, AE_BTN_CHANGE, _T("Change"));
	hbox->Add(btn_change, 0);
}

ArgEdit::~ArgEdit()
{
}

BEGIN_EVENT_TABLE(ArgEdit, wxPanel)
	EVT_TEXT(AE_ENTRY_ARG, ArgEdit::entry_arg_changed)
	EVT_BUTTON(AE_BTN_CHANGE, ArgEdit::change_clicked)
END_EVENT_TABLE()

void ArgEdit::entry_arg_changed(wxCommandEvent &event)
{
	if (entry_arg->GetValue() == _T(""))
		label_arg->SetLabel(wxString::Format(_T("%s"), argname.c_str()));
	else if (arg_type)
	{
		int val = atoi(wx_to_str(entry_arg->GetValue()).c_str());
		label_arg->SetLabel(wxString::Format(_T("%s: %s"), argname.c_str(), arg_type->get_name(val).c_str()));
#if wxCHECK_VERSION(2, 6, 2)
		label_arg->Wrap(200);
#endif

		GetParent()->SetBestFittingSize();
		GetParent()->SetSize(360, -1);
		GetParent()->Refresh();
	}
}

void ArgEdit::change_clicked(wxCommandEvent &event)
{
	if (!arg_type)
		return;

	int val = 0;
	if (entry_arg->GetValue() != _T(""))
		val = atoi(wx_to_str(entry_arg->GetValue()).c_str());

	ArgValueDialog av(val, arg_type);
	if (av.ShowModal() == wxID_OK)
		entry_arg->SetValue(wxString::Format(_T("%d"), av.get_value()));
}

int ArgEdit::get_value()
{
	int ret = -1;

	if (entry_arg->GetValue() != _T(""))
	{
		ret = atoi(wx_to_str(entry_arg->GetValue()).c_str());

		if (ret > 255)
			ret = 255;
		if (ret < 0)
			ret = 0;
	}

	return ret;
}

ArgsEditor::ArgsEditor(BYTE *args, string *argnames, string *argtypes, bool *consistent)
:	wxDialog(NULL, -1, _T("Edit Args"), wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	for (int a = 0; a < 5; a++)
	{
		int val = -1;
		if (consistent[a]) val = args[a];

		this->args[a] = new ArgEdit(this, val, argnames[a], game.get_arg_type(argtypes[a]));
		m_vbox->Add(this->args[a], 0, wxEXPAND|wxALL, 4);
	}

	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	SetBestFittingSize();
	SetSize(360, -1);
	Layout();
}

ArgsEditor::~ArgsEditor()
{
}

void ArgsEditor::apply_changes(BYTE* args, bool *consistent)
{
	for (int a = 0; a < 5; a++)
	{
		if (this->args[a]->get_value() == -1)
			consistent[a] = false;
		else
		{
			consistent[a] = true;
			args[a] = this->args[a]->get_value();
		}
	}
}
