
#include "main.h"
#include "stype_select.h"
#include "doom_map.h"
#include "dm_sector.h"
#include "game_config.h"

extern DoomMap d_map;
extern GameConfig game;

STypeSelectDialog::STypeSelectDialog(int init)
:	wxDialog(NULL, -1, _T("Select Sector Type"), wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Type frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Type"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	list_types = new wxListBox(this, -1);
	box->Add(list_types, 1, wxEXPAND, 4);
	//populate_list_stypes(list_types, init);

	for (int a = 1; a < game.get_stypes().size(); a++)
	{
		list_types->Append(wxString::Format(_T("%d: %s"), game.get_stypes()[a]->type, game.get_stypes()[a]->name.c_str()));
		if (Sector::strip_special(init, d_map.hexen()) == game.get_stypes()[a]->type)
			list_types->SetSelection(a-1);
	}

	if (game.boom())
	{
		wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
		m_vbox->Add(hbox, 0, wxEXPAND);

		// Damage frame
		frame = new wxStaticBox(this, -1, _T("Damage"));
		box = new wxStaticBoxSizer(frame, wxVERTICAL);
		hbox->Add(box, 1, wxEXPAND|wxALL, 4);

		wxString damage[] =
		{
			_T("None"),
			_T("5 Health/Sec"),
			_T("10 Health/Sec"),
			_T("20 Health/Sec"),
		};

		combo_damage = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, 4, damage);
		box->Add(combo_damage, 1, wxEXPAND);

		// Yeah eventually this won't be hardcoded,
		// I'll read the flags from the game configuration
		int mult = 0;
		if (d_map.hexen()) mult = 8;
		combo_damage->Select(0);

		if (init & (32 * mult) && init & (64 * mult))
			combo_damage->Select(3);
		else if (init & (32 * mult))
			combo_damage->Select(1);
		else if (init & (64 * mult))
			combo_damage->Select(2);

		// Flags frame
		frame = new wxStaticBox(this, -1, _T("Flags"));
		box = new wxStaticBoxSizer(frame, wxVERTICAL);
		hbox->Add(box, 0, wxEXPAND|wxALL, 4);

		cbox_secret = new wxCheckBox(this, -1, _T("Secret Area"));
		box->Add(cbox_secret, 0, wxBOTTOM, 2);
		if (init & (128 * mult))
			cbox_secret->SetValue(true);

		cbox_friction = new wxCheckBox(this, -1, _T("Friction Enabled"));
		box->Add(cbox_friction, 0, wxBOTTOM, 2);
		if (init & (256 * mult))
			cbox_friction->SetValue(true);

		cbox_wind = new wxCheckBox(this, -1, _T("Wind Enabled"));
		box->Add(cbox_wind, 0);
		if (init & (512 * mult))
			cbox_wind->SetValue(true);
	}


	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	Layout();
	SetBestFittingSize();
	Center();
}

STypeSelectDialog::~STypeSelectDialog()
{
}

int STypeSelectDialog::get_value()
{
	int ret = 0;
	sscanf(wx_to_str(list_types->GetStringSelection()).c_str(), "%d", &ret);

	if (game.boom())
	{
		// Damage
		int val = combo_damage->GetSelection() * 32;

		if (d_map.hexen())
			val = val * 8;

		ret += val;

		// Flags
		if (cbox_secret->GetValue())
		{
			if (d_map.hexen())
				ret += 1024;
			else
				ret += 128;
		}

		if (cbox_friction->GetValue())
		{
			if (d_map.hexen())
				ret += 2048;
			else
				ret += 256;
		}

		if (cbox_wind->GetValue())
		{
			if (d_map.hexen())
				ret += 4096;
			else
				ret += 512;
		}
	}

	return ret;
}
