
#include "main.h"
#include "line_edit.h"
#include "thing_edit.h"
#include "doom_map.h"
#include "dm_thing.h"
#include "special_select.h"
#include "ttype_select.h"
//#include "checks.h"
#include "args_edit.h"
#include "game_config.h"

extern Thing last_thing;
extern DoomMap d_map;
extern GameConfig game;

ThingEditor::ThingEditor(int type)
:	wxDialog(NULL, -1, _T("Edit Thing"), wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Flags frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Flags"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	// Flags
	wxBoxSizer *vbox = NULL;

	int flags_per_col = 5;
	if (game.n_thing_flags() > 15)
		flags_per_col = (game.n_thing_flags() / 3) + 1;

	int f = 0;
	int cols = (game.n_thing_flags() / flags_per_col) + 1;
	for (int a = 0; a < cols; a++)
	{
		if (f >= game.n_thing_flags())
			continue;

		vbox = new wxBoxSizer(wxVERTICAL);
		box->Add(vbox, 1, wxEXPAND);

		for (int b = 0; b < flags_per_col; b++)
		{
			if (f >= game.n_thing_flags())
				continue;

			setup_flag_checkbox(game.thing_flag(f).value, game.thing_flag(f).name, vbox);
			f++;
		}
	}

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND);

	// Type frame
	frame = new wxStaticBox(this, -1, _T("Type"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	hbox->Add(box, 1, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox2, 1, wxEXPAND);

	type_entry = new wxTextCtrl(this, TE_ENTRY_TYPE, _T(""));
	sprite = new ClickableTexBox(this, 96, 96, type_entry, _T("_none_"), 3, 2.0f);
	sprite->Enable(true);
	hbox2->Add(sprite, 1, wxEXPAND|wxALL, 4);

	vbox = new wxBoxSizer(wxVERTICAL);
	hbox2->Add(vbox, 0, wxEXPAND);

	vbox->Add(type_entry, 1, wxALL, 4);

	change_type = new wxButton(this, TE_BTN_CHANGETYPE, _T("Change"));
	vbox->Add(change_type, 1, wxEXPAND|wxALL, 4);

	browse_type = new wxButton(this, TE_BTN_BROWSETYPE, _T("Browse"));
	vbox->Add(browse_type, 1, wxEXPAND|wxALL, 4);

	type_label = new wxStaticText(this, -1, _T("Type"));
	box->Add(type_label, 0, wxEXPAND|wxALL, 4);

	// Angle frame
	frame = new wxStaticBox(this, -1, _T("Angle"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	hbox->Add(box, 0, wxEXPAND|wxALL, 4);

	angle_n = new wxRadioButton(this, TE_RADIO_N, _T(""), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	angle_ne = new wxRadioButton(this, TE_RADIO_NE, _T(""));
	angle_e = new wxRadioButton(this, TE_RADIO_E, _T(""));
	angle_se = new wxRadioButton(this, TE_RADIO_SE, _T(""));
	angle_s = new wxRadioButton(this, TE_RADIO_S, _T(""));
	angle_sw = new wxRadioButton(this, TE_RADIO_SW, _T(""));
	angle_w = new wxRadioButton(this, TE_RADIO_W, _T(""));
	angle_nw = new wxRadioButton(this, TE_RADIO_NW, _T(""));
	angle_unknown = new wxRadioButton(this, -1, _T(""));
	angle_unknown->Hide();

	wxGridSizer *grid = new wxGridSizer(5, 5, 4, 4);
	box->Add(grid, 1, wxEXPAND|wxALL, 4);

	// Yeah, I know its ugly. I can't find a better way to do this
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->Add(angle_n);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->Add(angle_nw);
	grid->AddSpacer(0);
	grid->Add(angle_ne);
	grid->AddSpacer(0);
	grid->Add(angle_w);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->Add(angle_e);
	grid->AddSpacer(0);
	grid->Add(angle_sw);
	grid->AddSpacer(0);
	grid->Add(angle_se);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->AddSpacer(0);
	grid->Add(angle_s);

	angle_entry = new wxTextCtrl(this, TE_ENTRY_ANGLE, _T(""));
	box->Add(angle_entry, 0, wxEXPAND|wxALL, 4);

	if (d_map.hexen())
	{
		wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
		m_vbox->Add(hbox, 0, wxEXPAND);
		
		// TID frame
		frame = new wxStaticBox(this, -1, _T("Thing ID"));
		box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
		hbox->Add(box, 0, wxEXPAND|wxALL, 4);

		tid_entry = new wxTextCtrl(this, TE_ENTRY_TID, _T(""), wxDefaultPosition, wxSize(56, -1));
		box->Add(tid_entry, 1, wxEXPAND|wxRIGHT, 4);

		find_tid = new wxButton(this, TE_BTN_FINDTID, _T("Find Free"));
		box->Add(find_tid, 0, wxEXPAND);

		// Z Height frame
		frame = new wxStaticBox(this, -1, _T("Z Height"));
		box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
		hbox->Add(box, 0, wxEXPAND|wxALL, 4);

		z_entry = new wxTextCtrl(this, TE_ENTRY_Z, _T(""), wxDefaultPosition, wxSize(56, -1));
		box->Add(z_entry, 1, wxEXPAND);

		// Special frame
		frame = new wxStaticBox(this, -1, _T("Action Special"));
		box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
		hbox->Add(box, 0, wxEXPAND|wxALL, 4);

		special_entry = new wxTextCtrl(this, TE_ENTRY_SPECIAL, _T(""), wxDefaultPosition, wxSize(56, -1));
		box->Add(special_entry, 1, wxEXPAND|wxRIGHT, 4);

		change_special = new wxButton(this, TE_BTN_CHANGESPECIAL, _T("Change"));
		box->Add(change_special, 0, wxEXPAND);

		// Args frame
		frame = new wxStaticBox(this, -1, _T("Args"));
		box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
		hbox->Add(box, 0, wxEXPAND|wxALL, 4);

		edit_args = new wxButton(this, TE_BTN_EDITARGS, _T("Edit Args"));
		box->Add(edit_args, 0, wxEXPAND);
	}

	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	setup_widgets();
	Layout();
	SetBestFittingSize();
	Center();
}

ThingEditor::~ThingEditor()
{
}

void ThingEditor::setup_flag_checkbox(WORD flag, wxString label, wxSizer *box)
{
	FlagCheckBox *flag_cb = new FlagCheckBox(this, flag, label);
	flags.push_back(flag_cb);
	box->Add(flag_cb, 0, wxBOTTOM, 2);
}

void ThingEditor::setup_widgets()
{
	vector<int> selection = d_map.selection();

	// Single item
	if (selection.size() == 0 && d_map.hilight() != -1)
	{
		Thing *t = d_map.hilight_thing();

		// Flags
		for (int a = 0; a < flags.size(); a++)
		{
			if (t->check_flag(flags[a]->flag))
				flags[a]->Set3StateValue(wxCHK_CHECKED);
		}

		// Type
		type_entry->SetValue(wxString::Format(_T("%d"), t->get_type()));

		// Angle
		angle_entry->SetValue(wxString::Format(_T("%d"), t->get_angle()));

		if (d_map.hexen())
		{
			// TID
			tid_entry->SetValue(wxString::Format(_T("%d"), t->get_tid()));

			// Z height
			z_entry->SetValue(wxString::Format(_T("%d"), t->z_pos()));

			// Special
			special_entry->SetValue(wxString::Format(_T("%d"), t->get_special()));

			// Args
			for (int a = 0; a < 5; a++)
				args[a] = t->arg(a);
			memset(args_consistent, 1, 5);
		}
	}

	// Multiple items
	if (selection.size() > 0)
	{
		Thing *othing = d_map.thing(selection[0]);

		// Flags
		for (int a = 0; a < flags.size(); a++)
		{
			if (othing->check_flag(flags[a]->flag))
				flags[a]->Set3StateValue(wxCHK_CHECKED);
		}

		// Type
		type_entry->SetValue(wxString::Format(_T("%d"), othing->get_type()));

		// Angle
		angle_entry->SetValue(wxString::Format(_T("%d"), othing->get_angle()));

		if (d_map.hexen())
		{
			// TID
			tid_entry->SetValue(wxString::Format(_T("%d"), othing->get_tid()));

			// Z height
			z_entry->SetValue(wxString::Format(_T("%d"), othing->z_pos()));

			// Special
			special_entry->SetValue(wxString::Format(_T("%d"), othing->get_special()));

			// Args
			for (int a = 0; a < 5; a++)
				args[a] = othing->arg(a);
			memset(args_consistent, 1, 5);
		}

		// Check for inconsistency
		for (int a = 0; a < selection.size(); a++)
		{
			Thing* thing = d_map.thing(selection[a]);

			for (int b = 0; b < flags.size(); b++)
			{
				if (flags[b]->Get3StateValue() == wxCHK_UNDETERMINED)
					continue;

				bool on = thing->check_flag(flags[b]->flag);
				if (on && flags[b]->Get3StateValue() == wxCHK_UNCHECKED ||
					!on && flags[b]->Get3StateValue() == wxCHK_CHECKED)
					flags[b]->Set3StateValue(wxCHK_UNDETERMINED);
			}

			if (thing->get_type() != othing->get_type())
				type_entry->SetValue(_T(""));

			if (thing->get_angle() != othing->get_angle())
				angle_entry->SetValue(_T(""));

			if (d_map.hexen())
			{
				if (thing->get_tid() != othing->get_tid())
					tid_entry->SetValue(_T(""));

				if (thing->z_pos() != othing->z_pos())
					z_entry->SetValue(_T(""));

				if (thing->get_special() != othing->get_special())
					special_entry->SetValue(_T(""));

				for (int b = 0; b < 5; b++)
				{
					if (thing->arg(b) != othing->arg(b))
						args_consistent[b] = false;
				}
			}
		}
	}
}

void ThingEditor::apply_changes()
{
	vector<int> selection = d_map.selection();
	vector<Thing*> items;

	// Get items to modify
	items.push_back(&last_thing);

	if (selection.size() == 0 && d_map.hilight() != -1)
		items.push_back(d_map.hilight_thing());

	if (selection.size() > 0)
	{
		for (int a = 0; a < selection.size(); a++)
			items.push_back(d_map.thing(selection[a]));
	}

	for (int a = 0; a < items.size(); a++)
	{
		// Flags
		WORD nflags = (WORD)items[a]->get_flags();
		for (int b = 0; b < flags.size(); b++)
			flags[b]->apply_flag(&nflags);
		items[a]->set_flags(nflags);

		// Type
		if (type_entry->GetValue().size())
			items[a]->set_type(atoi(chr(type_entry->GetValue())));

		// Angle
		if (angle_entry->GetValue().size())
			items[a]->set_angle(atoi(chr(angle_entry->GetValue())));

		if (d_map.hexen())
		{
			// Args
			for (int b = 0; b < 5; b++)
			{
				if (args_consistent[b])
					items[a]->set_arg(b, args[b]);
			}

			// TID
			if (tid_entry->GetValue().size())
				items[a]->set_tid(atoi(chr(tid_entry->GetValue())));

			// Z height
			if (z_entry->GetValue().size())
				items[a]->set_z(atoi(chr(z_entry->GetValue())));

			// Special
			for (int a = 0; a < items.size(); a++)
				items[a]->set_special(atoi(chr(special_entry->GetValue())));
		}

		items[a]->update_ttype();
	}

	d_map.change_level(MC_THINGS);
}

BEGIN_EVENT_TABLE(ThingEditor, wxDialog)
	EVT_RADIOBUTTON(TE_RADIO_N, ThingEditor::angle_radio_changed)
	EVT_RADIOBUTTON(TE_RADIO_NE, ThingEditor::angle_radio_changed)
	EVT_RADIOBUTTON(TE_RADIO_E, ThingEditor::angle_radio_changed)
	EVT_RADIOBUTTON(TE_RADIO_SE, ThingEditor::angle_radio_changed)
	EVT_RADIOBUTTON(TE_RADIO_S, ThingEditor::angle_radio_changed)
	EVT_RADIOBUTTON(TE_RADIO_SW, ThingEditor::angle_radio_changed)
	EVT_RADIOBUTTON(TE_RADIO_W, ThingEditor::angle_radio_changed)
	EVT_RADIOBUTTON(TE_RADIO_NW, ThingEditor::angle_radio_changed)
	EVT_TEXT(TE_ENTRY_TYPE, ThingEditor::type_entry_changed)
	EVT_TEXT(TE_ENTRY_ANGLE, ThingEditor::angle_entry_changed)
	EVT_BUTTON(TE_BTN_CHANGETYPE, ThingEditor::change_type_clicked)
	EVT_BUTTON(TE_BTN_FINDTID, ThingEditor::find_tid_clicked)
	EVT_BUTTON(TE_BTN_CHANGESPECIAL, ThingEditor::change_special_clicked)
	EVT_BUTTON(TE_BTN_EDITARGS, ThingEditor::edit_args_clicked)
END_EVENT_TABLE()

void ThingEditor::angle_radio_changed(wxCommandEvent &event)
{
	int angle = 0;
	switch (event.GetId())
	{
	case TE_RADIO_N:
		angle = 90; break;
	case TE_RADIO_NE:
		angle = 45; break;
	case TE_RADIO_E:
		angle = 0; break;
	case TE_RADIO_SE:
		angle = 315; break;
	case TE_RADIO_S:
		angle = 270; break;
	case TE_RADIO_SW:
		angle = 225; break;
	case TE_RADIO_W:
		angle = 180; break;
	case TE_RADIO_NW:
		angle = 135; break;
	default:
		angle = -1; break;
	}

	if (angle != -1)
		angle_entry->SetValue(s_fmt(_T("%d"), angle));
	else
		angle_entry->SetValue(_T(""));
}

void ThingEditor::type_entry_changed(wxCommandEvent &event)
{
	if (type_entry->GetValue().size())
	{
		ThingType *ttype = game.get_ttype(atoi(chr(type_entry->GetValue())));
		type_label->SetLabel(ttype->name);
		sprite->set_texture(ttype->spritename, 3);
	}
	else
	{
		type_label->SetLabel(_T("(Inconsistent / Don't Change)"));
		sprite->set_texture(_T("_none_"), 3);
	}
}

void ThingEditor::angle_entry_changed(wxCommandEvent &event)
{
	angle_unknown->SetValue(true);
	if (angle_entry->GetValue().size())
	{
		int angle = atoi(chr(angle_entry->GetValue()));

		if (angle == 0)
			angle_e->SetValue(true);
		if (angle == 45)
			angle_ne->SetValue(true);
		if (angle == 90)
			angle_n->SetValue(true);
		if (angle == 135)
			angle_nw->SetValue(true);
		if (angle == 180)
			angle_w->SetValue(true);
		if (angle == 225)
			angle_sw->SetValue(true);
		if (angle == 270)
			angle_s->SetValue(true);
		if (angle == 315)
			angle_se->SetValue(true);
	}
}

void ThingEditor::change_type_clicked(wxCommandEvent &event)
{
	int val = 0;
	if (type_entry->GetValue().size())
		val = atoi(chr(type_entry->GetValue()));

	TTypeSelectDialog td(val);
	if (td.ShowModal() == wxID_OK)
	{
		if (td.get_type() != -1)
			type_entry->SetValue(s_fmt(_T("%d"), td.get_type()));
	}
}

void ThingEditor::find_tid_clicked(wxCommandEvent &event)
{
	/*
	int tid = get_free_tid();
	tid_entry->SetValue(wxString::Format(_T("%d"), tid));
	*/
}

void ThingEditor::change_special_clicked(wxCommandEvent &event)
{
	int val = 0;
	if (special_entry->GetValue().size())
		val = atoi(chr(special_entry->GetValue()));

	SpecialSelectDialog sd(val);
	if (sd.ShowModal() == wxID_OK)
	{
		if (sd.get_special() != -1)
			special_entry->SetValue(wxString::Format(_T("%d"), sd.get_special()));
	}
}

void ThingEditor::edit_args_clicked(wxCommandEvent &event)
{
	int type = -1;
	if (type_entry->GetValue().size())
		type = atoi(chr(type_entry->GetValue()));

	ThingType *tt = game.get_ttype(type);
	int result;

	if (tt->flags & TTYPE_HASARGS)
	{
		ArgsEditor ae(args, tt->args, tt->arg_types, args_consistent);
		if (ae.ShowModal() == wxID_OK)
			ae.apply_changes(args, args_consistent);
	}
	else
	{
		int special = 0;
		if (special_entry->GetValue().size())
			special = atoi(chr(special_entry->GetValue()));

		ActionSpecial *as = game.get_special(special);

		ArgsEditor ae(args, as->args, as->arg_types, args_consistent);
		if (ae.ShowModal() == wxID_OK)
			ae.apply_changes(args, args_consistent);
	}
}
