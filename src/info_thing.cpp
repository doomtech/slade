
#include "main.h"
#include "info_bar.h"
#include "doom_map.h"
#include "dm_thing.h"
#include "game_config.h"
#include "miscstuff.h"

extern DoomMap d_map;
extern GameConfig game;

ThingInfoBar::ThingInfoBar(wxWindow *parent)
:	InfoBar(parent)
{
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(hbox);

	// Main frame
	main_frame = new wxStaticBox(this, -1, _T("No Thing Hilighted"));
	wxFont def_font = main_frame->GetFont();
	def_font.SetWeight(wxFONTWEIGHT_BOLD);
	main_frame->SetFont(def_font);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(main_frame, wxHORIZONTAL);
	hbox->Add(box, 1, wxEXPAND|wxRIGHT, 4);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	vbox->SetMinSize(160, -1);
	box->Add(vbox, 0, wxRIGHT, 8);

	label_type = new wxStaticText(this, -1, _T("Type:"));
	label_position = new wxStaticText(this, -1, _T("Position:"));
	label_angle = new wxStaticText(this, -1, _T("Angle:"));
	label_difficulty = new wxStaticText(this, -1, _T("Difficulty:"));
	label_deaf = new wxStaticText(this, -1, _T("Deaf:"));
	label_multiplayer = new wxStaticText(this, -1, _T("Multiplayer:"));

	vbox->Add(label_type);
	vbox->Add(label_position);
	vbox->Add(label_angle);
	vbox->Add(label_difficulty);
	vbox->Add(label_deaf);
	vbox->Add(label_multiplayer);

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

	// Sprite frame
	sprite_frame = new wxStaticBox(this, -1, _T("Sprite"));
	sprite_frame->SetFont(def_font);
	box = new wxStaticBoxSizer(sprite_frame, wxVERTICAL);
	hbox->Add(box, 0, wxEXPAND);

	tex_sprite = new TextureBox(this, 96, 96, _T("_none_"), 3, 2.0f);
	box->Add(tex_sprite, 1, wxALIGN_CENTER);

	SetSizeHints(-1, 128, -1, -1);
}

ThingInfoBar::~ThingInfoBar()
{
}

void ThingInfoBar::update()
{
	if (d_map.hilight() == -1)
	{
		main_frame->SetLabel(_T("No Thing Hilighted"));
		label_type->SetLabel(_T("Type:"));
		label_position->SetLabel(_T("Position:"));
		label_angle->SetLabel(_T("Angle:"));
		label_difficulty->SetLabel(_T("Difficulty:"));
		label_deaf->SetLabel(_T("Deaf:"));

		if (d_map.hexen())
		{
			label_arg1->SetLabel(_T("Arg 1:"));
			label_arg2->SetLabel(_T("Arg 2:"));
			label_arg3->SetLabel(_T("Arg 3:"));
			label_arg4->SetLabel(_T("Arg 4:"));
			label_arg5->SetLabel(_T("Arg 5:"));
			label_multiplayer->SetLabel(_T("TID:"));
		}
		else
		{
			label_arg1->SetLabel(_T(""));
			label_arg2->SetLabel(_T(""));
			label_arg3->SetLabel(_T(""));
			label_arg4->SetLabel(_T(""));
			label_arg5->SetLabel(_T(""));
			label_multiplayer->SetLabel(_T("Multiplayer:"));
		}

		tex_sprite->set_texture(_T("_none_"));

		return;
	}

	Thing* thing = d_map.hilight_thing();

	main_frame->SetLabel(wxString::Format(_T("Thing #%d: %s"), d_map.hilight(), chr(thing->get_ttype()->name)));
	label_type->SetLabel(wxString::Format(_T("Type: %d"), thing->get_type()));
	if (d_map.hexen())
		label_position->SetLabel(wxString::Format(_T("Position: (%d, %d, %d)"), thing->pos().x, thing->pos().y, thing->z_pos()));
	else
		label_position->SetLabel(wxString::Format(_T("Position: (%d, %d)"), thing->pos().x, thing->pos().y));
	label_angle->SetLabel(wxString::Format(_T("Angle: %d (%s)"), thing->get_angle(), chr(thing->angle_string())));
	label_difficulty->SetLabel(_T("Difficulty: " + thing->difficulty_string()));
	label_deaf->SetLabel(_T("Deaf: ") + bool_to_yesno(thing->check_flag(THING_DEAF)));

	if (!d_map.hexen())
		label_multiplayer->SetLabel(_T("Multiplayer: " + bool_to_yesno(thing->check_flag(0x0010))));
	else
		label_multiplayer->SetLabel(wxString::Format(_T("TID: %d"), thing->get_tid()));

	tex_sprite->set_texture(thing->get_ttype()->spritename, 3);

	if (d_map.hexen())
	{
		ThingType* ttype = thing->get_ttype();

		argtype_t *at = game.get_arg_type(ttype->arg_types[0]);
		if (at)
			label_arg1->SetLabel(wxString::Format(_T("%s: %d (%s)"), chr(ttype->args[0]), thing->arg(0), chr(at->get_name(thing->arg(0)))));
		else
			label_arg1->SetLabel(wxString::Format(_T("%s: %d"), chr(ttype->args[0]), thing->arg(0)));

		at = game.get_arg_type(ttype->arg_types[1]);
		if (at)
			label_arg2->SetLabel(wxString::Format(_T("%s: %d (%s)"), chr(ttype->args[1]), thing->arg(1), chr(at->get_name(thing->arg(1)))));
		else
			label_arg2->SetLabel(wxString::Format(_T("%s: %d"), chr(ttype->args[1]), thing->arg(1)));

		at = game.get_arg_type(ttype->arg_types[2]);
		if (at)
			label_arg3->SetLabel(wxString::Format(_T("%s: %d (%s)"), chr(ttype->args[2]), thing->arg(2), chr(at->get_name(thing->arg(2)))));
		else
			label_arg3->SetLabel(wxString::Format(_T("%s: %d"), chr(ttype->args[2]), thing->arg(2)));

		at = game.get_arg_type(ttype->arg_types[3]);
		if (at)
			label_arg4->SetLabel(wxString::Format(_T("%s: %d (%s)"), chr(ttype->args[3]), thing->arg(3), chr(at->get_name(thing->arg(3)))));
		else
			label_arg4->SetLabel(wxString::Format(_T("%s: %d"), chr(ttype->args[3]), thing->arg(3)));

		at = game.get_arg_type(ttype->arg_types[4]);
		if (at)
			label_arg5->SetLabel(wxString::Format(_T("%s: %d (%s)"), chr(ttype->args[4]), thing->arg(4), chr(at->get_name(thing->arg(4)))));
		else
			label_arg5->SetLabel(wxString::Format(_T("%s: %d"), chr(ttype->args[4]), thing->arg(4)));
	}

	Layout();
}
