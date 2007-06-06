
#include "main.h"
#include "sector_edit.h"
#include "doom_map.h"
#include "dm_sector.h"
#include "game_config.h"
#include "stype_select.h"

extern DoomMap d_map;
extern GameConfig game;

SectorEditor::SectorEditor()
:	wxDialog(NULL, -1, _T("Edit Sector"), wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND);

	// Floor frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Floor"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	hbox->Add(box, 0, wxEXPAND|wxALL, 4);

	entry_ftex = new wxTextCtrl(this, SE_ENTRY_FTEX, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	tbox_ftex = new ClickableTexBox(this, 128, 128, entry_ftex, _T("_none_"), 2, 5.0f);
	tbox_ftex->Enable(true);
	box->Add(tbox_ftex, 0, wxEXPAND|wxALL, 4);
	box->Add(entry_ftex, 0, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox2, 0, wxEXPAND);

	entry_fheight = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(48, -1));
	hbox2->Add(new wxStaticText(this, -1, _T("Height:")), 1, wxALIGN_CENTER_VERTICAL);
	hbox2->Add(entry_fheight, 0, wxEXPAND|wxALL, 4);

	// Ceiling frame
	frame = new wxStaticBox(this, -1, _T("Ceiling"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	hbox->Add(box, 0, wxEXPAND|wxALL, 4);

	entry_ctex = new wxTextCtrl(this, SE_ENTRY_CTEX, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
	tbox_ctex = new ClickableTexBox(this, 128, 128, entry_ctex, _T("_none_"), 2, 5.0f);
	tbox_ctex->Enable(true);
	box->Add(tbox_ctex, 0, wxEXPAND|wxALL, 4);
	box->Add(entry_ctex, 0, wxEXPAND|wxALL, 4);

	hbox2 = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox2, 0, wxEXPAND);

	entry_cheight = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(48, -1));
	hbox2->Add(new wxStaticText(this, -1, _T("Height:")), 1, wxALIGN_CENTER_VERTICAL);
	hbox2->Add(entry_cheight, 0, wxEXPAND|wxALL, 4);

	// Properties frame
	frame = new wxStaticBox(this, -1, _T("Properties"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	// Special
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 4);

	entry_special = new wxTextCtrl(this, SE_ENTRY_SPECIAL, _T(""), wxDefaultPosition, wxSize(48, -1));
	btn_changespecial = new wxButton(this, SE_BTN_CHANGESPECIAL, _T("Change"));
	hbox->Add(new wxStaticText(this, -1, _T("Special:")), 1, wxALIGN_CENTER_VERTICAL);
	hbox->Add(entry_special, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(btn_changespecial, 0, wxEXPAND);

	label_special = new wxStaticText(this, -1, _T(""));
	box->Add(label_special, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Tag
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	entry_tag = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(48, -1));
	btn_findtag = new wxButton(this, SE_BTN_FINDTAG, _T("Free Tag"));
	hbox->Add(new wxStaticText(this, -1, _T("Sector Tag:")), 1, wxALIGN_CENTER_VERTICAL);
	hbox->Add(entry_tag, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(btn_findtag, 0, wxEXPAND);

	// Light
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 4);

	entry_light = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(48, -1));
	hbox->Add(new wxStaticText(this, -1, _T("Light Level:")), 1, wxALIGN_CENTER_VERTICAL);
	hbox->Add(entry_light, 0);

	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	setup_widgets();
	Layout();
	SetBestFittingSize();
	Center();
}

SectorEditor::~SectorEditor()
{
}

void SectorEditor::setup_widgets()
{
	Sector* s;
	vector<int> selection = d_map.selection();

	if (selection.size() == 0 && d_map.hilight() != -1)
		s = d_map.hilight_sector();
	else if (selection.size() > 0)
		s = d_map.sector(selection[0]);
	else
		return;

	// Floor
	tbox_ftex->set_texture(s->tex_floor(), 2);
	entry_ftex->SetValue(s->tex_floor());
	entry_fheight->SetValue(wxString::Format(_T("%d"), s->floor()));

	// Ceiling
	tbox_ctex->set_texture(s->tex_ceil(), 2);
	entry_ctex->SetValue(s->tex_ceil());
	entry_cheight->SetValue(wxString::Format(_T("%d"), s->ceiling()));

	// Special
	entry_special->SetValue(wxString::Format(_T("%d"), s->action_special()));

	// Tag
	entry_tag->SetValue(wxString::Format(_T("%d"), s->sector_tag()));

	// Light
	entry_light->SetValue(wxString::Format(_T("%d"), s->light_level()));

	// Multiple items, check for inconsistency
	if (selection.size() > 0)
	{
		for (unsigned int a = 0; a < selection.size(); a++)
		{
			// Floor tex
			if (d_map.sector(selection[a])->tex_floor() != s->tex_floor())
			{
				tbox_ftex->set_texture(_T("_none_"), 2);
				entry_ftex->SetValue(_T(""));
			}

			// Ceiling tex
			if (d_map.sector(selection[a])->tex_ceil() != s->tex_ceil())
			{
				tbox_ctex->set_texture(_T("_none_"), 2);
				entry_ctex->SetValue(_T(""));
			}

			// Floor height
			if (d_map.sector(selection[a])->floor() != s->floor())
				entry_fheight->SetValue(_T(""));

			// Ceiling height
			if (d_map.sector(selection[a])->ceiling() != s->ceiling())
				entry_cheight->SetValue(_T(""));

			// Special
			if (d_map.sector(selection[a])->action_special() != s->action_special())
				entry_special->SetValue(_T(""));

			// Tag
			if (d_map.sector(selection[a])->sector_tag() != s->sector_tag())
				entry_tag->SetValue(_T(""));

			// Light
			if (d_map.sector(selection[a])->light_level() != s->light_level())
				entry_light->SetValue(_T(""));
		}
	}
}

void SectorEditor::apply_changes()
{
	vector<Sector*> items;
	vector<int> selection = d_map.selection();

	// Get items to modify
	if (selection.size() == 0 && d_map.hilight() != -1)
		items.push_back(d_map.hilight_sector());

	if (selection.size() > 0)
	{
		for (unsigned int a = 0; a < selection.size(); a++)
			items.push_back(d_map.sector(selection[a]));
	}

	for (unsigned int a = 0; a < items.size(); a++)
	{
		// Floor tex
		if (entry_ftex->GetValue().size())
			items[a]->set_ftex(entry_ftex->GetValue().Upper());

		// Floor height
		if (entry_fheight->GetValue().size())
			items[a]->set_floor(atoi(chr(entry_fheight->GetValue())));

		// Ceiling tex
		if (entry_ctex->GetValue().size())
			items[a]->set_ctex(entry_ctex->GetValue().Upper());

		// Ceiling height
		if (entry_cheight->GetValue().size())
			items[a]->set_ceil(atoi(chr(entry_cheight->GetValue())));

		// Special
		if (entry_special->GetValue().size())
			items[a]->set_special(atoi(chr(entry_special->GetValue())));

		// Tag
		if (entry_tag->GetValue().size())
			items[a]->set_tag(atoi(chr(entry_tag->GetValue())));

		// Light
		if (entry_light->GetValue().size())
			items[a]->set_light(atoi(chr(entry_light->GetValue())));
	}

	d_map.change_level(MC_SECTORS|MC_SSECTS);
}

BEGIN_EVENT_TABLE(SectorEditor, wxDialog)
	EVT_BUTTON(SE_BTN_CHANGESPECIAL, SectorEditor::btn_changespecial_clicked)
	EVT_BUTTON(SE_BTN_FINDTAG, SectorEditor::btn_findtag_clicked)
	EVT_TEXT(SE_ENTRY_FTEX, SectorEditor::entry_ftex_changed)
	EVT_TEXT(SE_ENTRY_CTEX, SectorEditor::entry_ctex_changed)
	EVT_TEXT(SE_ENTRY_SPECIAL, SectorEditor::entry_special_changed)
END_EVENT_TABLE()

void SectorEditor::btn_changespecial_clicked(wxCommandEvent &event)
{
	STypeSelectDialog sd(atoi(chr(entry_special->GetValue())));
	if (sd.ShowModal() == wxID_OK)
		entry_special->SetValue(wxString::Format(_T("%d"), sd.get_value()));
}

void SectorEditor::btn_findtag_clicked(wxCommandEvent &event)
{
	int tag = d_map.get_free_tag();
	entry_tag->SetValue(wxString::Format(_T("%d"), tag));
}

void SectorEditor::entry_ftex_changed(wxCommandEvent &event)
{
	string tex = entry_ftex->GetValue().Upper();
	tbox_ftex->set_texture(tex, 2);
}

void SectorEditor::entry_ctex_changed(wxCommandEvent &event)
{
	string tex = entry_ftex->GetValue().Upper();
	tbox_ftex->set_texture(tex, 2);
}

void SectorEditor::entry_special_changed(wxCommandEvent &event)
{
	if (entry_special->GetValue().empty())
		label_special->SetLabel(_T(""));
	else
	{
		string st = game.get_sector_type(Sector::strip_special(atoi(chr(entry_special->GetValue())), d_map.hexen()));
		label_special->SetLabel(st);
	}
}
