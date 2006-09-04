
#include "main.h"
#include "info_bar.h"
#include "doom_map.h"
#include "dm_line.h"
#include "dm_side.h"
#include "mathstuff.h"
#include "game_config.h"

extern DoomMap d_map;
extern GameConfig game;

LineInfoBar::LineInfoBar(wxWindow *parent)
:	InfoBar(parent)
{
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(hbox);

	// Main frame
	main_frame = new wxStaticBox(this, -1, _T("No Line Hilighted"));
	wxFont def_font = main_frame->GetFont();
	def_font.SetWeight(wxFONTWEIGHT_BOLD);
	main_frame->SetFont(def_font);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(main_frame, wxHORIZONTAL);
	hbox->Add(box, 1, wxEXPAND|wxRIGHT, 4);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	vbox->SetMinSize(160, -1);
	box->Add(vbox, 0, wxRIGHT, 8);

	label_length = new wxStaticText(this, -1, _T("Length:"));
	label_front = new wxStaticText(this, -1, _T("Front Sector:"));
	label_back = new wxStaticText(this, -1, _T("Back Sector:"));
	label_special = new wxStaticText(this, -1, _T("Special:"));
	label_tag = new wxStaticText(this, -1, _T("Sector Tag:"));
	label_offsets1 = new wxStaticText(this, -1, _T("Front Offsets:"));
	label_offsets2 = new wxStaticText(this, -1, _T("Back Offsets:"));

	vbox->Add(label_length);
	vbox->Add(label_front);
	vbox->Add(label_back);
	vbox->Add(label_special);
	vbox->Add(label_tag);
	vbox->Add(label_offsets1);
	vbox->Add(label_offsets2);

	vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox);

	label_arg1 = new wxStaticText(this, -1, _T(""));
	label_arg2 = new wxStaticText(this, -1, _T(""));
	label_arg3 = new wxStaticText(this, -1, _T(""));
	label_arg4 = new wxStaticText(this, -1, _T(""));
	label_arg5 = new wxStaticText(this, -1, _T(""));

	vbox->Add(label_arg1);
	vbox->Add(label_arg2);
	vbox->Add(label_arg3);
	vbox->Add(label_arg4);
	vbox->Add(label_arg5);

	// Front side
	front_frame = new wxStaticBox(this, -1, _T("Front Side"));
	front_frame->SetFont(def_font);
	box = new wxStaticBoxSizer(front_frame, wxHORIZONTAL);
	box->SetMinSize(192, -1);
	hbox->Add(box, 0, wxEXPAND|wxRIGHT, 4);

	tex_upper1 = new TextureBox(this, 64, 64, _T("_none_"), 1, 2.0f);
	tex_middle1 = new TextureBox(this, 64, 64, _T("_none_"), 1, 2.0f);
	tex_lower1 = new TextureBox(this, 64, 64, _T("_none_"), 1, 2.0f);
	label_upper1 = new wxStaticText(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	label_middle1 = new wxStaticText(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	label_lower1 = new wxStaticText(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);

	vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox);
	vbox->Add(new wxStaticText(this, -1, _T("Upper"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE), 0, wxEXPAND);
	vbox->Add(tex_upper1, 0, wxALIGN_CENTRE|wxEXPAND|wxTOP, 4);
	vbox->Add(label_upper1, 0, wxEXPAND|wxTOP, 4);

	vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox);
	vbox->Add(new wxStaticText(this, -1, _T("Middle"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE), 0, wxEXPAND);
	vbox->Add(tex_middle1, 0, wxALIGN_CENTRE|wxEXPAND|wxTOP, 4);
	vbox->Add(label_middle1, 0, wxEXPAND|wxTOP, 4);

	vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox);
	vbox->Add(new wxStaticText(this, -1, _T("Lower"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE), 0, wxEXPAND);
	vbox->Add(tex_lower1, 0, wxALIGN_CENTRE|wxEXPAND|wxTOP, 4);
	vbox->Add(label_lower1, 0, wxEXPAND|wxTOP, 4);

	// Back side
	back_frame = new wxStaticBox(this, -1, _T("Back Side"));
	back_frame->SetFont(def_font);
	box = new wxStaticBoxSizer(back_frame, wxHORIZONTAL);
	box->SetMinSize(192, -1);
	hbox->Add(box, 0, wxEXPAND);

	tex_upper2 = new TextureBox(this, 64, 64, _T("_none_"), 1, 2.0f);
	tex_middle2 = new TextureBox(this, 64, 64, _T("_none_"), 1, 2.0f);
	tex_lower2 = new TextureBox(this, 64, 64, _T("_none_"), 1, 2.0f);
	label_upper2 = new wxStaticText(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	label_middle2 = new wxStaticText(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	label_lower2 = new wxStaticText(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);

	vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox);
	vbox->Add(new wxStaticText(this, -1, _T("Upper"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE), 0, wxEXPAND);
	vbox->Add(tex_upper2, 0, wxALIGN_CENTRE|wxEXPAND|wxTOP, 4);
	vbox->Add(label_upper2, 0, wxEXPAND|wxTOP, 4);

	vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox);
	vbox->Add(new wxStaticText(this, -1, _T("Middle"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE), 0, wxEXPAND);
	vbox->Add(tex_middle2, 0, wxALIGN_CENTRE|wxEXPAND|wxTOP, 4);
	vbox->Add(label_middle2, 0, wxEXPAND|wxTOP, 4);

	vbox = new wxBoxSizer(wxVERTICAL);
	box->Add(vbox);
	vbox->Add(new wxStaticText(this, -1, _T("Lower"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE), 0, wxEXPAND);
	vbox->Add(tex_lower2, 0, wxALIGN_CENTRE|wxEXPAND|wxTOP, 4);
	vbox->Add(label_lower2, 0, wxEXPAND|wxTOP, 4);

	SetSizeHints(-1, 128, -1, 128);
}

LineInfoBar::~LineInfoBar()
{
}

void LineInfoBar::update()
{
	if (d_map.hilight() == -1)
	{
		main_frame->SetLabel(_T("No Line Hilighted"));
		label_length->SetLabel(_T("Length:"));
		label_front->SetLabel(_T("Front Sector:"));
		label_back->SetLabel(_T("Back Sector:"));
		label_special->SetLabel(_T("Special:"));
		label_offsets1->SetLabel(_T("Front Offsets:"));
		label_offsets2->SetLabel(_T("Back Offsets:"));		

		front_frame->SetLabel(_T("Front Side"));
		label_upper1->SetLabel(_T(""));
		label_middle1->SetLabel(_T(""));
		label_lower1->SetLabel(_T(""));
		tex_upper1->set_texture(_T("_none_"));
		tex_middle1->set_texture(_T("_none_"));
		tex_lower1->set_texture(_T("_none_"));

		back_frame->SetLabel(_T("Back Side"));

		if (d_map.hexen())
		{
			label_arg1->SetLabel(_T("Arg 1:"));
			label_arg2->SetLabel(_T("Arg 2:"));
			label_arg3->SetLabel(_T("Arg 3:"));
			label_arg4->SetLabel(_T("Arg 4:"));
			label_arg5->SetLabel(_T("Arg 5:"));
			label_tag->SetLabel(_T("Trigger:"));
		}
		else
		{
			label_arg1->SetLabel(_T(""));
			label_arg2->SetLabel(_T(""));
			label_arg3->SetLabel(_T(""));
			label_arg4->SetLabel(_T(""));
			label_arg5->SetLabel(_T(""));
			label_tag->SetLabel(_T("Sector Tag:"));
		}

		return;
	}

	// Main frame
	Line* line = d_map.hilight_line();
	main_frame->SetLabel(wxString::Format(_T("Line #%d"), d_map.hilight()));
	label_length->SetLabel(wxString::Format(_T("Length: %d (%d %d)"), lround(line->get_rect().length()), d_map.index(line->vertex1()), d_map.index(line->vertex2())));
	label_front->SetLabel(wxString::Format(_T("Front Sector: %d"), line->sector_index(true)));
	label_back->SetLabel(wxString::Format(_T("Back Sector: %d"), line->sector_index(false)));
	label_special->SetLabel(wxString::Format(_T("Special: %d (%s)"),
											line->get_special(),
											game.get_special(line->get_special())->name.c_str()));

	if (!d_map.hexen())
		label_tag->SetLabel(wxString::Format(_T("Sector Tag: %d"), line->get_sector_tag()));
	else
		label_tag->SetLabel(wxString::Format(_T("Trigger: %s"), line->get_trigger_string().c_str()));

	if (d_map.hexen())
	{
		ActionSpecial* as = game.get_special(line->get_special());

		argtype_t *at = game.get_arg_type(as->arg_types[0]);
		if (at)
			label_arg1->SetLabel(wxString::Format(_T("%s: %d (%s)"), as->args[0].c_str(), line->arg(0), at->get_name(line->arg(0)).c_str()));
		else
			label_arg1->SetLabel(wxString::Format(_T("%s: %d"), as->args[0].c_str(), line->arg(0)));

		at = game.get_arg_type(as->arg_types[1]);
		if (at)
			label_arg2->SetLabel(wxString::Format(_T("%s: %d (%s)"), as->args[1].c_str(), line->arg(1), at->get_name(line->arg(1)).c_str()));
		else
			label_arg2->SetLabel(wxString::Format(_T("%s: %d"), as->args[1].c_str(), line->arg(1)));

		at = game.get_arg_type(as->arg_types[2]);
		if (at)
			label_arg3->SetLabel(wxString::Format(_T("%s: %d (%s)"), as->args[2].c_str(), line->arg(2), at->get_name(line->arg(2)).c_str()));
		else
			label_arg3->SetLabel(wxString::Format(_T("%s: %d"), as->args[2].c_str(), line->arg(2)));

		at = game.get_arg_type(as->arg_types[3]);
		if (at)
			label_arg4->SetLabel(wxString::Format(_T("%s: %d (%s)"), as->args[3].c_str(), line->arg(3), at->get_name(line->arg(3)).c_str()));
		else
			label_arg4->SetLabel(wxString::Format(_T("%s: %d"), as->args[3].c_str(), line->arg(3)));

		at = game.get_arg_type(as->arg_types[4]);
		if (at)
			label_arg5->SetLabel(wxString::Format(_T("%s: %d (%s)"), as->args[4].c_str(), line->arg(4), at->get_name(line->arg(4)).c_str()));
		else
			label_arg5->SetLabel(wxString::Format(_T("%s: %d"), as->args[4].c_str(), line->arg(4)));
	}

	// Front side frame
	Side *side = line->side1();
	if (d_map.valid(side))
	{
		front_frame->SetLabel(str_to_wx(s_fmt(_T("Front Side: %d"), d_map.index(side))));

		// Upper texture
		label_upper1->SetLabel(str_to_wx(side->get_texname(TEX_UPPER)));
		if (line->needs_upper(true))
			tex_upper1->set_texture(side->get_texname(TEX_UPPER), 1);
		else
			tex_upper1->set_texture(_T("_none_"));

		// Middle texture
		label_middle1->SetLabel(str_to_wx(side->get_texname(TEX_MIDDLE)));
		if (line->needs_middle(true))
			tex_middle1->set_texture(side->get_texname(TEX_MIDDLE), 1);
		else
			tex_middle1->set_texture(_T("_none_"));

		// Lower texture
		label_lower1->SetLabel(str_to_wx(side->get_texname(TEX_LOWER)));
		if (line->needs_lower(true))
			tex_lower1->set_texture(side->get_texname(TEX_LOWER), 1);
		else
			tex_lower1->set_texture(_T("_none_"));

		label_offsets1->SetLabel(str_to_wx(s_fmt(_T("Front Offsets: (%d, %d)"), side->get_xoff(), side->get_yoff())));
	}
	else
	{
		front_frame->SetLabel(_T("Front Side"));
		tex_upper1->set_texture(_T("_none_"));
		tex_middle1->set_texture(_T("_none_"));
		tex_lower1->set_texture(_T("_none_"));
		label_upper1->SetLabel(_T(""));
		label_middle1->SetLabel(_T(""));
		label_lower1->SetLabel(_T(""));
		label_offsets1->SetLabel(_T("Front Offsets: (None)"));
	}

	// Back side frame
	side = line->side2();
	if (d_map.valid(side))
	{
		back_frame->SetLabel(str_to_wx(s_fmt(_T("Back Side: %d"), d_map.index(side))));

		// Upper texture
		label_upper2->SetLabel(str_to_wx(side->get_texname(TEX_UPPER)));
		if (line->needs_upper(false))
			tex_upper2->set_texture(side->get_texname(TEX_UPPER), 1);
		else
			tex_upper2->set_texture(_T("_none_"));

		// Middle texture
		label_middle2->SetLabel(str_to_wx(side->get_texname(TEX_MIDDLE)));
		if (line->needs_middle(false))
			tex_middle2->set_texture(side->get_texname(TEX_MIDDLE), 1);
		else
			tex_middle2->set_texture(_T("_none_"));

		// Lower texture
		label_lower2->SetLabel(str_to_wx(side->get_texname(TEX_LOWER)));
		if (line->needs_lower(false))
			tex_lower2->set_texture(side->get_texname(TEX_LOWER), 1);
		else
			tex_lower2->set_texture(_T("_none_"));

		label_offsets2->SetLabel(str_to_wx(s_fmt(_T("Back Offsets: (%d, %d)"), side->get_xoff(), side->get_yoff())));
	}
	else
	{
		back_frame->SetLabel(_T("Back Side"));
		tex_upper2->set_texture(_T("_none_"));
		tex_middle2->set_texture(_T("_none_"));
		tex_lower2->set_texture(_T("_none_"));
		label_upper2->SetLabel(_T(""));
		label_middle2->SetLabel(_T(""));
		label_lower2->SetLabel(_T(""));
		label_offsets2->SetLabel(_T("Back Offsets: (None)"));
	}

	Layout();
}
