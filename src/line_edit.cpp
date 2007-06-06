

#include "main.h"
#include "line_edit.h"
#include "doom_map.h"
#include "dm_line.h"
#include "dm_side.h"
#include "special_select.h"
#include "args_edit.h"
#include "game_config.h"

#include <wx/list.h>
#include <wx/notebook.h>

extern DoomMap d_map;
extern GameConfig game;

LineEditor::LineEditor()
:	wxDialog(NULL, -1, _T("Edit Linedef"), wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	tabs = new wxNotebook(this, -1);
	m_vbox->Add(tabs, 1, wxEXPAND|wxALL, 4);

	props_page = new LinePropertiesPage(tabs);
	tabs->AddPage(props_page, _T("Properties"));

	side1_page = new SidePropertiesPage(tabs);
	tabs->AddPage(side1_page, _T("Front Side"));

	side2_page = new SidePropertiesPage(tabs, 2);
	tabs->AddPage(side2_page, _T("Back Side"));

	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	Layout();
	SetBestFittingSize();
	Center();
}

LineEditor::~LineEditor()
{
}

void LineEditor::apply_changes()
{
	props_page->apply_changes();
	side1_page->apply_changes();
	side2_page->apply_changes();
	d_map.change_level(MC_LINES);
}

LinePropertiesPage::LinePropertiesPage(wxWindow *parent)
:	wxPanel(parent, -1)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Flags frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Flags"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	// Flags
	wxBoxSizer *vbox = NULL;

	int flags_per_col = 6;
	if (game.n_line_flags() > 12)
		flags_per_col = (game.n_line_flags() / 2) + 1;

	int f = 0;
	int cols = (game.n_line_flags() / flags_per_col) + 1;
	for (int a = 0; a < cols; a++)
	{
		if (f >= game.n_line_flags())
			continue;

		vbox = new wxBoxSizer(wxVERTICAL);
		box->Add(vbox, 1, wxEXPAND);

		for (int b = 0; b < flags_per_col; b++)
		{
			if (f >= game.n_line_flags())
				continue;

			setup_flag_checkbox(game.line_flag(f).value, game.line_flag(f).name, vbox);
			f++;
		}
	}

	// Properties frame
	frame = new wxStaticBox(this, -1, _T("Properties"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	if (d_map.hexen())
	{
		// Action special
		wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
		box->Add(hbox, 0, wxEXPAND);

		hbox->Add(new wxStaticText(this, -1, _T("Action Special:")), 1, wxALIGN_CENTER_VERTICAL);
		entry_special = new wxTextCtrl(this, LP_ENTRY_SPECIAL, _T(""), wxDefaultPosition, wxSize(48, -1));
		hbox->Add(entry_special, 0, wxEXPAND|wxRIGHT, 4);
		btn_changespecial = new wxButton(this, LP_BTN_CHANGESPECIAL, _T("Change"));
		hbox->Add(btn_changespecial, 0, wxEXPAND|wxRIGHT, 4);
		btn_editargs = new wxButton(this, LP_BTN_EDITARGS, _T("Edit Args"));
		hbox->Add(btn_editargs, 0, wxEXPAND);

		label_special = new wxStaticText(this, -1, _T("Special:"));
		box->Add(label_special, 0, wxEXPAND|wxTOP, 2);

		// Trigger
		hbox = new wxBoxSizer(wxHORIZONTAL);
		box->Add(hbox, 0, wxEXPAND|wxTOP, 4);

		wxString triggers[] =
		{
			_T(""),
			_T("Player Crosses Line"),
			_T("Player Uses Line"),
			_T("Monster Crosses Line"),
			_T("Projectile Hits Line"),
			_T("Player Bumps Line"),
			_T("Projectile Crosses Line"),
			_T("Player Uses Line (PassThru)"),
			_T("Projectile Hits OR Crosses Line"),
		};

		hbox->Add(new wxStaticText(this, -1, _T("Activation Trigger:")), 1, wxALIGN_CENTER_VERTICAL);
		combo_trigger = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, 9, triggers);
		hbox->Add(combo_trigger, 0, wxEXPAND);
	}
	else
	{
		// Action special
		wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
		box->Add(hbox, 0, wxEXPAND);

		label_special = new wxStaticText(this, -1, _T("Special:"));
		hbox->Add(label_special, 1, wxALIGN_CENTER_VERTICAL);
		entry_special = new wxTextCtrl(this, LP_ENTRY_SPECIAL, _T(""), wxDefaultPosition, wxSize(32, -1));
		hbox->Add(entry_special, 0, wxEXPAND|wxRIGHT, 4);
		btn_changespecial = new wxButton(this, LP_BTN_CHANGESPECIAL, _T("Change"));
		hbox->Add(btn_changespecial, 0, wxEXPAND);

		// Sector tag
		hbox = new wxBoxSizer(wxHORIZONTAL);
		box->Add(hbox, 0, wxEXPAND|wxTOP, 4);

		hbox->Add(new wxStaticText(this, -1, _T("Sector Tag:")), 1, wxALIGN_CENTER_VERTICAL);
		entry_tag = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(32, -1));
		hbox->Add(entry_tag, 0, wxEXPAND|wxRIGHT, 4);
		btn_getfreetag = new wxButton(this, LP_BTN_GETFREETAG, _T("Free Tag"));
		hbox->Add(btn_getfreetag, 0, wxEXPAND);
	}

	setup_widgets();

	Layout();
}

LinePropertiesPage::~LinePropertiesPage()
{
}

void LinePropertiesPage::setup_flag_checkbox(WORD flag, wxString label, wxSizer *box)
{
	FlagCheckBox *flag_cb = new FlagCheckBox(this, flag, label);
	flags.push_back(flag_cb);
	box->Add(flag_cb, 0, wxBOTTOM, 2);
}

void LinePropertiesPage::setup_widgets()
{
	// Single item
	if (d_map.selection().size() == 0 && d_map.hilight() != -1)
	{
		// Flags
		for (unsigned int a = 0; a < flags.size(); a++)
		{
			if (d_map.hilight_line()->check_flag(flags[a]->flag))
				flags[a]->Set3StateValue(wxCHK_CHECKED);
		}

		// Properties
		entry_special->SetValue(s_fmt(_T("%d"), d_map.hilight_line()->get_special()));

		if (!d_map.hexen())
			entry_tag->SetValue(s_fmt(_T("%d"), d_map.hilight_line()->get_sector_tag()));
		else
		{
			combo_trigger->Select(d_map.hilight_line()->get_trigger() + 1);

			for (unsigned int a = 0; a < 5; a++)
				args[a] = d_map.hilight_line()->arg(a);
			memset(args_consistent, 1, 5);
		}

		return;
	}

	// Multiple items
	vector<int> &selection = d_map.selection();
	if (selection.size() > 0)
	{
		Line *oline = d_map.line(selection[0]);

		// Flags
		for (unsigned int a = 0; a < flags.size(); a++)
		{
			if (oline->check_flag(flags[a]->flag))
				flags[a]->Set3StateValue(wxCHK_CHECKED);
		}

		// Properties
		entry_special->SetValue(s_fmt(_T("%d"), oline->get_special()));

		if (!d_map.hexen())
			entry_tag->SetValue(s_fmt(_T("%d"), oline->get_sector_tag()));
		else
		{
			combo_trigger->Select(oline->get_trigger() + 1);

			for (unsigned int a = 0; a < 5; a++)
				args[a] = oline->arg(a);
			memset(args_consistent, 1, 5);
		}

		// Check for inconsistency
		for (unsigned int a = 0; a < selection.size(); a++)
		{
			Line* line = d_map.line(selection[a]);

			for (unsigned int b = 0; b < flags.size(); b++)
			{
				if (flags[b]->Get3StateValue() == wxCHK_UNDETERMINED)
					continue;

				bool on = line->check_flag(flags[b]->flag);
				if (on && flags[b]->Get3StateValue() == wxCHK_UNCHECKED ||
					!on && flags[b]->Get3StateValue() == wxCHK_CHECKED)
					flags[b]->Set3StateValue(wxCHK_UNDETERMINED);
			}

			if (line->get_special() != oline->get_special())
				entry_special->SetValue(_T(""));

			if (!d_map.hexen() && line->get_sector_tag() != oline->get_sector_tag())
				entry_tag->SetValue(_T(""));

			if (d_map.hexen() && line->get_trigger() != oline->get_trigger())
				combo_trigger->Select(0);

			if (d_map.hexen())
			{
				for (unsigned int b = 0; b < 5; b++)
				{
					if (line->arg(b) != oline->arg(b))
						args_consistent[b] = false;
				}
			}
		}
	}
}

void LinePropertiesPage::apply_changes()
{
	vector<Line*> items;

	// Get items to modify
	if (d_map.selection().size() == 0 && d_map.hilight() != -1)
		items.push_back(d_map.hilight_line());

	if (d_map.selection().size() > 0)
	{
		for (unsigned int a = 0; a < d_map.selection().size(); a++)
			items.push_back(d_map.line(d_map.selection()[a]));
	}

	// Flags
	for (unsigned int a = 0; a < items.size(); a++)
	{
		d_map.index(items[a], true);

		WORD nflags = (WORD)items[a]->get_flags();
		for (unsigned int b = 0; b < flags.size(); b++)
			flags[b]->apply_flag(&nflags);
		items[a]->set_flags(nflags);
	}

	// Action special
	if (entry_special->GetValue().size())
	{
		for (unsigned int a = 0; a < items.size(); a++)
			items[a]->set_special(atoi(chr(entry_special->GetValue())));
	}

	if (!d_map.hexen())
	{
		// Sector tag
		if (entry_tag->GetValue().size())
		{
			for (unsigned int a = 0; a < items.size(); a++)
				items[a]->set_sector_tag(atoi(chr(entry_tag->GetValue())));
		}
	}
	else
	{
		// Trigger
		if (combo_trigger->GetSelection() > 0)
		{
			for (unsigned int a = 0; a < items.size(); a++)
				items[a]->set_trigger(combo_trigger->GetSelection() - 1);
		}

		// Args
		for (unsigned int a = 0; a < 5; a++)
		{
			if (args_consistent[a])
			{
				for (unsigned int b = 0; b < items.size(); b++)
					items[b]->set_arg(a, args[a]);
			}
		}
	}
}

BEGIN_EVENT_TABLE(LinePropertiesPage, wxPanel)
	EVT_BUTTON(LP_BTN_CHANGESPECIAL, LinePropertiesPage::change_special_clicked)
	EVT_BUTTON(LP_BTN_GETFREETAG, LinePropertiesPage::get_free_tag_clicked)
	EVT_BUTTON(LP_BTN_EDITARGS, LinePropertiesPage::edit_args_clicked)
	EVT_TEXT(LP_ENTRY_SPECIAL, LinePropertiesPage::entry_special_changed)
END_EVENT_TABLE()

void LinePropertiesPage::change_special_clicked(wxCommandEvent &event)
{
	int val = 0;
	if (entry_special->GetValue().size())
		val = atoi(chr(entry_special->GetValue()));

	SpecialSelectDialog sd(val);
	if (sd.ShowModal() == wxID_OK)
	{
		if (sd.get_special() != -1)
			entry_special->SetValue(s_fmt(_T("%d"), sd.get_special()));
	}
}

void LinePropertiesPage::get_free_tag_clicked(wxCommandEvent &event)
{
	int tag = d_map.get_free_tag();
	entry_tag->SetValue(wxString::Format(_T("%d"), tag));
}

void LinePropertiesPage::edit_args_clicked(wxCommandEvent &event)
{
	int type = 0;
	if (entry_special->GetValue().size())
		type = atoi(chr(entry_special->GetValue()));

	ActionSpecial *as = game.get_special(type);

	ArgsEditor ae(args, as->args, as->arg_types, args_consistent);
	if (ae.ShowModal() == wxID_OK)
		ae.apply_changes(args, args_consistent);
}

void LinePropertiesPage::entry_special_changed(wxCommandEvent &event)
{
	if (entry_special->GetValue().empty())
		label_special->SetLabel(_T("Special:"));
	else
	{
		ActionSpecial *as = game.get_special(atoi(chr(entry_special->GetValue())));
		label_special->SetLabel(_T("Special: ") + as->name);
	}
}

FlagCheckBox::FlagCheckBox(wxWindow *parent, int flag, wxString label)
:	wxCheckBox(parent, -1, label, wxDefaultPosition, wxDefaultSize, wxCHK_3STATE)
{
	this->flag = flag;
}

FlagCheckBox::~FlagCheckBox()
{
}

void FlagCheckBox::apply_flag(WORD *flags)
{
	if (Get3StateValue() == wxCHK_UNCHECKED)
	{
		//wxLogMessage(_T("Removing flag %d"), flag);
		*flags = (*flags & ~flag);
	}

	if (Get3StateValue() == wxCHK_CHECKED)
	{
		//wxLogMessage(_T("Applying flag %d"), flag);
		*flags |= flag;
	}
}

SidePropertiesPage::SidePropertiesPage(wxWindow *parent, int side)
:	wxPanel(parent, -1)
{
	this->side = side;

	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Textures frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Textures"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	// Upper
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox, 1, wxEXPAND);
	box->AddSpacer(4);

	vbox->Add(new wxStaticText(this, -1, _T("Upper"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), 0, wxEXPAND);
	entry_upper = new wxTextCtrl(this, SP_ENTRY_UPPER, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	tex_upper = new ClickableTexBox(this, 96, 96, entry_upper, _T("_none_"), 1, 5.0f, SP_TEX_UPPER);
	tex_upper->Enable(true);
	vbox->Add(tex_upper, 0, wxEXPAND|wxTOP, 4);
	vbox->Add(entry_upper, 0, wxEXPAND|wxTOP, 4);

	// Middle
	vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox, 1, wxEXPAND);
	box->AddSpacer(4);

	vbox->Add(new wxStaticText(this, -1, _T("Middle"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), 0, wxEXPAND);
	entry_middle = new wxTextCtrl(this, SP_ENTRY_MIDDLE, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	tex_middle = new ClickableTexBox(this, 96, 96, entry_middle, _T("_none_"), 1, 5.0f, SP_TEX_MIDDLE);
	tex_middle->Enable(true);
	vbox->Add(tex_middle, 0, wxEXPAND|wxTOP, 4);
	vbox->Add(entry_middle, 0, wxEXPAND|wxTOP, 4);

	// Lower
	vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox, 1, wxEXPAND);

	vbox->Add(new wxStaticText(this, -1, _T("Lower"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER), 0, wxEXPAND);
	entry_lower = new wxTextCtrl(this, SP_ENTRY_LOWER, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	tex_lower = new ClickableTexBox(this, 96, 96, entry_lower, _T("_none_"), 1, 5.0f, SP_TEX_LOWER);
	tex_lower->Enable(true);
	vbox->Add(tex_lower, 0, wxEXPAND|wxTOP, 4);
	vbox->Add(entry_lower, 0, wxEXPAND|wxTOP, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND);

	// Offsets frame
	frame = new wxStaticBox(this, -1, _T("Offsets"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	hbox->Add(box, 1, wxEXPAND|wxALL, 4);

	// X
	wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox2, 0, wxEXPAND);
	entry_xoff = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(56, -1));
	hbox2->Add(new wxStaticText(this, -1, _T("X Offset:")), 0, wxALIGN_CENTER_VERTICAL);
	hbox2->AddStretchSpacer();
	hbox2->Add(entry_xoff, 0, wxEXPAND|wxALIGN_RIGHT);

	// Y
	hbox2 = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox2, 0, wxEXPAND|wxTOP, 4);
	entry_yoff = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(56, -1));
	hbox2->Add(new wxStaticText(this, -1, _T("Y Offset:")), 0, wxALIGN_CENTER_VERTICAL);
	hbox2->AddStretchSpacer();
	hbox2->Add(entry_yoff, 0, wxEXPAND|wxALIGN_RIGHT);

	// Side frame
	frame = new wxStaticBox(this, -1, _T("Side"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	hbox->Add(box, 1, wxEXPAND|wxALL, 4);

	// Sector reference
	hbox2 = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox2, 0, wxEXPAND);
	entry_sector = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(56, -1));
	hbox2->Add(new wxStaticText(this, -1, _T("Sector Reference:")), 0, wxALIGN_CENTER_VERTICAL);
	hbox2->AddStretchSpacer();
	hbox2->Add(entry_sector, 0, wxEXPAND|wxALIGN_RIGHT);

	// Create/Delete buttons
	hbox2 = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox2, 0, wxEXPAND|wxTOP, 4);
	btn_create = new wxButton(this, -1, _T("Create"));
	btn_delete = new wxButton(this, -1, _T("Delete"));
	//hbox2->AddStretchSpacer();
	hbox2->Add(btn_create, 0, wxEXPAND);
	hbox2->Add(btn_delete, 0, wxEXPAND|wxLEFT, 4);

	setup_widgets();
}

SidePropertiesPage::~SidePropertiesPage()
{
}

void SidePropertiesPage::setup_widgets()
{
	bool missingside = false;

	// Get list of relevant sidedefs
	vector<Line*> selection;
	d_map.get_selection(selection, true);
	vector<Side*> sides;

	if (selection.size() > 0)
	{
		for (unsigned int a = 0; a < selection.size(); a++)
		{
			Side* sd;

			if (side == 1)
				sd = selection[a]->side1();
			else
				sd = selection[a]->side2();

			if (d_map.valid(sd))
				sides.push_back(sd);
			else
				missingside = true;
		}
	}

	// If there are no sides in the list, disable all widgets except the 'create' button
	if (sides.size() == 0)
	{
		wxNode *node = (wxNode*)(GetChildren().GetFirst());

		while (node)
		{
			((wxWindow*)(node->GetData()))->Enable(false);
			node = node->GetNext();
		}

		btn_create->Enable(true);
		return;
	}
	else if (!missingside)
		btn_create->Enable(false);


	// Upper texture
	entry_upper->SetValue(sides[0]->get_texname(TEX_UPPER));
	if (sides[0]->get_texname(TEX_UPPER) == _T("-"))
		tex_upper->set_texture(_T("_none_"));
	else
		tex_upper->set_texture(sides[0]->get_texname(TEX_UPPER), 1);

	// Middle texture
	entry_middle->SetValue(sides[0]->get_texname(TEX_MIDDLE));
	if (sides[0]->get_texname(TEX_MIDDLE) == _T("-"))
		tex_middle->set_texture(_T("_none_"));
	else
		tex_middle->set_texture(sides[0]->get_texname(TEX_MIDDLE), 1);

	// Lower texture
	entry_lower->SetValue(sides[0]->get_texname(TEX_LOWER));
	if (sides[0]->get_texname(TEX_LOWER) == _T("-"))
		tex_lower->set_texture(_T("_none_"));
	else
		tex_lower->set_texture(sides[0]->get_texname(TEX_LOWER), 1);

	// Offsets
	entry_xoff->SetValue(wxString::Format(_T("%d"), sides[0]->get_xoff()));
	entry_yoff->SetValue(wxString::Format(_T("%d"), sides[0]->get_yoff()));

	// Sector ref
	entry_sector->SetValue(wxString::Format(_T("%d"), selection[0]->sector_index(side == 1)));

	// Check for inconsistency
	for (unsigned int a = 0; a < sides.size(); a++)
	{
		d_map.index(sides[a], true);

		if (entry_upper->GetValue().size())
		{
			if (sides[a]->get_texname(TEX_UPPER) != sides[0]->get_texname(TEX_UPPER))
			{
				entry_upper->SetValue(_T(""));
				tex_upper->set_texture(_T("_none_"), 1);
			}
		}

		if (entry_middle->GetValue().size())
		{
			if (sides[a]->get_texname(TEX_MIDDLE) != sides[0]->get_texname(TEX_MIDDLE))
			{
				entry_middle->SetValue(_T(""));
				tex_middle->set_texture(_T("_none_"), 1);
			}
		}

		if (entry_lower->GetValue().size())
		{
			if (sides[a]->get_texname(TEX_LOWER) != sides[0]->get_texname(TEX_LOWER))
			{
				entry_lower->SetValue(_T(""));
				tex_lower->set_texture(_T("_none_"), 1);
			}
		}

		if (sides[a]->get_xoff() != sides[0]->get_xoff())
			entry_xoff->SetValue(_T(""));

		if (sides[a]->get_yoff() != sides[0]->get_yoff())
			entry_yoff->SetValue(_T(""));

		if (sides[a]->get_sector() != sides[0]->get_sector())
			entry_sector->SetValue(_T(""));
	}
}

void SidePropertiesPage::apply_changes()
{
	// Get list of relevant sidedefs
	vector<Line*> selection;
	d_map.get_selection(selection, true);
	vector<Side*> sides;

	if (selection.size() > 0)
	{
		for (unsigned int a = 0; a < selection.size(); a++)
		{
			Side* sd;

			if (side == 1)
				sd = selection[a]->side1();
			else
				sd = selection[a]->side2();

			if (d_map.valid(sd))
				sides.push_back(sd);
		}
	}

	for (unsigned int a = 0; a < sides.size(); a++)
	{
		// Textures
		if (entry_upper->GetValue().size())
			sides[a]->set_texture(entry_upper->GetValue(), TEX_UPPER);

		if (entry_middle->GetValue().size())
			sides[a]->set_texture(entry_middle->GetValue(), TEX_MIDDLE);

		if (entry_lower->GetValue().size())
			sides[a]->set_texture(entry_lower->GetValue(), TEX_LOWER);

		// Offsets
		if (entry_xoff->GetValue().size())
			sides[a]->set_xoff(atoi(chr(entry_xoff->GetValue())));

		if (entry_yoff->GetValue().size())
			sides[a]->set_yoff(atoi(chr(entry_yoff->GetValue())));

		// Sector ref
		if (entry_sector->GetValue().size())
			sides[a]->set_sector(atoi(chr(entry_sector->GetValue())));
	}
}

BEGIN_EVENT_TABLE(SidePropertiesPage, wxPanel)
	EVT_TEXT(SP_ENTRY_UPPER, SidePropertiesPage::upper_tex_entry_changed)
	EVT_TEXT(SP_ENTRY_MIDDLE, SidePropertiesPage::middle_tex_entry_changed)
	EVT_TEXT(SP_ENTRY_LOWER, SidePropertiesPage::lower_tex_entry_changed)
END_EVENT_TABLE()

void SidePropertiesPage::upper_tex_entry_changed(wxCommandEvent &event)
{
	wxString str = entry_upper->GetValue();
	str.UpperCase();
	tex_upper->set_texture(str, 1);
}

void SidePropertiesPage::middle_tex_entry_changed(wxCommandEvent &event)
{
	wxString str = entry_middle->GetValue();
	str.UpperCase();
	tex_middle->set_texture(str, 1);
}

void SidePropertiesPage::lower_tex_entry_changed(wxCommandEvent &event)
{
	wxString str = entry_lower->GetValue();
	str.UpperCase();
	tex_lower->set_texture(str, 1);
}

void SidePropertiesPage::upper_tex_clicked(wxMouseEvent &event)
{
}
