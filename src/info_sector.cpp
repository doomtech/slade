
#include "main.h"
#include "info_bar.h"
#include "doom_map.h"
#include "dm_sector.h"
#include "game_config.h"

extern DoomMap d_map;
extern GameConfig game;

SectorInfoBar::SectorInfoBar(wxWindow *parent)
:	InfoBar(parent)
{
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(hbox);

	// Main frame
	main_frame = new wxStaticBox(this, -1, _T("No Sector Hilighted"));
	wxFont def_font = main_frame->GetFont();
	def_font.SetWeight(wxFONTWEIGHT_BOLD);
	main_frame->SetFont(def_font);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(main_frame, wxVERTICAL);
	hbox->Add(box, 1, wxEXPAND|wxRIGHT, 4);

	label_floor = new wxStaticText(this, -1, _T("Floor Height:"));
	label_ceil = new wxStaticText(this, -1, _T("Ceiling Height:"));
	label_height = new wxStaticText(this, -1, _T("Height:"));
	label_light = new wxStaticText(this, -1, _T("Light:"));
	label_special = new wxStaticText(this, -1, _T("Special:"));
	label_tag = new wxStaticText(this, -1, _T("Tag:"));

	box->Add(label_floor);
	box->Add(label_ceil);
	box->Add(label_height);
	box->Add(label_light);
	box->Add(label_special);
	box->Add(label_tag);

	// Floor frame
	floor_frame = new wxStaticBox(this, -1, _T("Floor"));
	floor_frame->SetFont(def_font);
	box = new wxStaticBoxSizer(floor_frame, wxVERTICAL);
	hbox->Add(box, 0, wxEXPAND|wxRIGHT, 4);

	tex_floor = new TextureBox(this, 96, 96, "_none_", 1, 2.0f);
	box->Add(tex_floor, 1, wxALIGN_CENTER);

	// Ceiling frame
	ceil_frame = new wxStaticBox(this, -1, _T("Ceiling"));
	ceil_frame->SetFont(def_font);
	box = new wxStaticBoxSizer(ceil_frame, wxVERTICAL);
	hbox->Add(box, 0, wxEXPAND);

	tex_ceil = new TextureBox(this, 96, 96, "_none_", 1, 2.0f);
	box->Add(tex_ceil, 1, wxALIGN_CENTER);

	SetSizeHints(-1, 128, -1, 128);
}

SectorInfoBar::~SectorInfoBar()
{
}

void SectorInfoBar::update()
{
	if (d_map.hilight() == -1)
	{
		main_frame->SetLabel(_T("No Sector Hilighted"));
		label_floor->SetLabel(_T("Floor Height:"));
		label_ceil->SetLabel(_T("Ceiling Height:"));
		label_height->SetLabel(_T("Height:"));
		label_light->SetLabel(_T("Light:"));
		label_special->SetLabel(_T("Special:"));
		label_tag->SetLabel(_T("Tag:"));

		floor_frame->SetLabel(_T("Floor"));
		tex_floor->set_texture("_none_");
		ceil_frame->SetLabel(_T("Ceiling"));
		tex_ceil->set_texture("_none_");
		return;
	}

	Sector *sector = d_map.hilight_sector();

	// Main frame
	main_frame->SetLabel(str_to_wx(s_fmt("Secor #%d", d_map.hilight())));
	label_floor->SetLabel(str_to_wx(s_fmt("Floor Height: %d", sector->floor())));
	label_ceil->SetLabel(str_to_wx(s_fmt("Ceiling Height: %d", sector->ceiling())));
	label_height->SetLabel(str_to_wx(s_fmt("Height: %d", sector->ceiling() - sector->floor())));
	label_light->SetLabel(str_to_wx(s_fmt("Light: %d", sector->light_level())));
	label_special->SetLabel(str_to_wx(s_fmt("Special: %d (%s)", sector->action_special(), game.get_sector_type(sector->stripped_special()).c_str())));
	label_tag->SetLabel(str_to_wx(s_fmt("Tag: %d", sector->sector_tag())));

	// Floor frame
	floor_frame->SetLabel(str_to_wx(s_fmt("Floor: %s", sector->tex_floor().c_str())));
	tex_floor->set_texture(sector->tex_floor(), 2);

	// Ceiling frame
	ceil_frame->SetLabel(str_to_wx(s_fmt("Ceiling: %s", sector->tex_ceil().c_str())));
	tex_ceil->set_texture(sector->tex_ceil(), 2);

	Layout();
}
