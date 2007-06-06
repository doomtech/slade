
#include "main.h"
#include "prefs_dialog.h"
#include "textures.h"

EXTERN_CVAR(Float, line_size)
EXTERN_CVAR(Bool, line_aa)
EXTERN_CVAR(Int, crosshair_2d)
EXTERN_CVAR(Bool, thing_sprites)
EXTERN_CVAR(Bool, thing_force_angle)
EXTERN_CVAR(Bool, grid_dashed)
EXTERN_CVAR(Bool, grid_64grid)
EXTERN_CVAR(Bool, grid_origin)
EXTERN_CVAR(Bool, edit_snap_grid)
EXTERN_CVAR(Bool, allow_np2_tex)
EXTERN_CVAR(Int, tex_filter)
EXTERN_CVAR(Bool, pan_detail)

VisualPrefs::VisualPrefs(wxWindow *parent)
:	wxPanel(parent, -1)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Editor frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Editor"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxBOTTOM|wxEXPAND, 2);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	hbox->Add(vbox, 1, wxEXPAND);

	cb_grid_dash = new wxCheckBox(this, VP_GRID_DASH, _T("Dashed grid"));
	if (grid_dashed) cb_grid_dash->SetValue(true);
	vbox->Add(cb_grid_dash, 0, wxBOTTOM, 2);

	cb_64_grid = new wxCheckBox(this, VP_64_GRID, _T("Show 64 grid"));
	if (grid_64grid) cb_64_grid->SetValue(true);
	vbox->Add(cb_64_grid, 0, wxBOTTOM, 2);

	cb_grid_origin = new wxCheckBox(this, VP_GRID_ORIGIN, _T("Hilight (0,0) on grid"));
	if (grid_origin) cb_grid_origin->SetValue(true);
	vbox->Add(cb_grid_origin, 0, wxBOTTOM, 2);

	cb_snap_to_grid = new wxCheckBox(this, VP_SNAP_GRID, _T("Snap to grid"));
	if (edit_snap_grid) cb_snap_to_grid->SetValue(true);
	vbox->Add(cb_snap_to_grid, 0, wxBOTTOM, 2);

	vbox = new wxBoxSizer(wxVERTICAL);
	hbox->Add(vbox, 1, wxEXPAND);

	cb_thing_sprites = new wxCheckBox(this, VP_THING_SPRITES, _T("Show things as sprites"));
	if (thing_sprites) cb_thing_sprites->SetValue(true);
	vbox->Add(cb_thing_sprites, 0, wxBOTTOM, 2);

	cb_thing_angles = new wxCheckBox(this, VP_THING_ANGLES, _T("Force thing angles"));
	if (thing_force_angle) cb_thing_angles->SetValue(true);
	vbox->Add(cb_thing_angles, 0, wxBOTTOM, 2);

	cb_pan_detail = new wxCheckBox(this, VP_PAN_DETAIL, _T("Full detail map panning"));
	if (pan_detail) cb_pan_detail->SetValue(true);
	vbox->Add(cb_pan_detail, 0, wxBOTTOM, 2);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxBOTTOM|wxEXPAND, 2);

	hbox->Add(new wxStaticText(this, -1, _T("Crosshair:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);

	wxString xhair[] = {
		_T("None"),
		_T("Full"),
		_T("Small"),
	};

	ch_2d_xhair = new wxChoice(this, VP_XHAIR_2D, wxDefaultPosition, wxDefaultSize, 3, xhair);
	ch_2d_xhair->SetSelection(crosshair_2d);
	hbox->Add(ch_2d_xhair, 0, wxEXPAND);

	// Lines frame
	frame = new wxStaticBox(this, -1, _T("Lines"));
	box = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	box->Add(new wxStaticText(this, -1, _T("Width:")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	sl_line_size = new wxSlider(this, VP_LINE_SIZE, line_size * 10, 10, 20, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_AUTOTICKS);
	box->Add(sl_line_size, 1, wxEXPAND);

	cb_line_aa = new wxCheckBox(this, VP_LINE_AA, _T("Antialias"));
	if (line_aa) cb_line_aa->SetValue(true);
	box->Add(cb_line_aa, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 4);

	// Textures frame
	frame = new wxStaticBox(this, -1, _T("Textures"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	cb_tex_np2 = new wxCheckBox(this, VP_TEX_NP2, _T("Allow non-power-of-two textures"));
	if (allow_np2_tex) cb_tex_np2->SetValue(true);
	box->Add(cb_tex_np2, 0, wxEXPAND|wxBOTTOM, 4);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND);

	wxString filters[] = {
		_T("None"),
		_T("Bilinear"),
		_T("Mipmapped"),
	};

	ch_tex_filter = new wxChoice(this, VP_TEX_FILTER, wxDefaultPosition, wxDefaultSize, 3, filters);
	ch_tex_filter->SetSelection(tex_filter - 1);
	hbox->Add(new wxStaticText(this, -1, _T("Filter:")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(ch_tex_filter, 1, wxEXPAND);
}

VisualPrefs::~VisualPrefs()
{
}

BEGIN_EVENT_TABLE(VisualPrefs, wxPanel)
	EVT_CHOICE(VP_TEX_FILTER, VisualPrefs::filter_changed)
	EVT_CHOICE(VP_XHAIR_2D, VisualPrefs::xhair_2d_changed)
	EVT_COMMAND_SCROLL(VP_LINE_SIZE, VisualPrefs::line_size_changed)
	EVT_CHECKBOX(VP_GRID_DASH, VisualPrefs::cbox_toggled)
	EVT_CHECKBOX(VP_64_GRID, VisualPrefs::cbox_toggled)
	EVT_CHECKBOX(VP_SNAP_GRID, VisualPrefs::cbox_toggled)
	EVT_CHECKBOX(VP_GRID_ORIGIN, VisualPrefs::cbox_toggled)
	EVT_CHECKBOX(VP_THING_SPRITES, VisualPrefs::cbox_toggled)
	EVT_CHECKBOX(VP_THING_ANGLES, VisualPrefs::cbox_toggled)
	EVT_CHECKBOX(VP_LINE_AA, VisualPrefs::cbox_toggled)
	EVT_CHECKBOX(VP_TEX_NP2, VisualPrefs::cbox_toggled)
	EVT_CHECKBOX(VP_PAN_DETAIL, VisualPrefs::cbox_toggled)
END_EVENT_TABLE()

void VisualPrefs::cbox_toggled(wxCommandEvent &event)
{
	switch(event.GetId())
	{
	case VP_GRID_DASH:
		grid_dashed = false;
		if (cb_grid_dash->GetValue()) grid_dashed = true;
		redraw_map();
		break;

	case VP_64_GRID:
		grid_64grid = false;
		if (cb_64_grid->GetValue()) grid_64grid = true;
		redraw_map();
		break;

	case VP_GRID_ORIGIN:
		grid_origin = false;
		if (cb_grid_origin->GetValue()) grid_origin = true;
		redraw_map();
		break;

	case VP_SNAP_GRID:
		edit_snap_grid = false;
		if (cb_snap_to_grid->GetValue()) edit_snap_grid = true;
		break;

	case VP_THING_SPRITES:
		thing_sprites = false;
		if (cb_thing_sprites->GetValue()) thing_sprites = true;
		redraw_map();
		break;

	case VP_THING_ANGLES:
		thing_force_angle = false;
		if (cb_thing_angles->GetValue()) thing_force_angle = true;
		redraw_map();
		break;

	case VP_LINE_AA:
		line_aa = false;
		if (cb_line_aa->GetValue()) line_aa = true;
		redraw_map();
		break;

	case VP_TEX_NP2:
		allow_np2_tex = false;
		if (cb_tex_np2->GetValue()) allow_np2_tex = true;
		break;

	case VP_PAN_DETAIL:
		pan_detail = false;
		if (cb_pan_detail->GetValue()) pan_detail = true;
		break;

	default:
		break;
	}
}

/*
void VisualPrefs::grid_dash_toggled(wxCommandEvent &event)
{
	grid_dashed = false;
	if (cb_grid_dash->GetValue()) grid_dashed = true;
	redraw_map();
}

void VisualPrefs::grid_64_toggled(wxCommandEvent &event)
{
	grid_64grid = false;
	if (cb_64_grid->GetValue()) grid_64grid = true;
	redraw_map();
}

void VisualPrefs::grid_snap_toggled(wxCommandEvent &event)
{
	edit_snap_grid = false;
	if (cb_snap_to_grid->GetValue()) edit_snap_grid = true;
}

void VisualPrefs::thing_sprites_toggled(wxCommandEvent &event)
{
	thing_sprites = false;
	if (cb_thing_sprites->GetValue()) thing_sprites = true;
	redraw_map();
}

void VisualPrefs::thing_angles_toggled(wxCommandEvent &event)
{
	thing_force_angle = false;
	if (cb_thing_angles->GetValue()) thing_force_angle = true;
	redraw_map();
}

void VisualPrefs::line_aa_toggled(wxCommandEvent &event)
{
	line_aa = false;
	if (cb_line_aa->GetValue()) line_aa = true;
	redraw_map();
}

void VisualPrefs::tex_np2_toggled(wxCommandEvent &event)
{
	allow_np2_tex = false;
	if (cb_tex_np2->GetValue()) allow_np2_tex = true;
}

void VisualPrefs::pan_detail_toggled(wxCommandEvent &event)
{
	pan_detail = false;
	if (cb_pan_detail->GetValue()) pan_detail = true;
}
*/

void VisualPrefs::filter_changed(wxCommandEvent &event)
{
	tex_filter = ch_tex_filter->GetSelection() + 1;
	purge_textures();
}

void VisualPrefs::xhair_2d_changed(wxCommandEvent &event)
{
	crosshair_2d = ch_2d_xhair->GetSelection();
}

void VisualPrefs::line_size_changed(wxScrollEvent &event)
{
	float val = sl_line_size->GetValue() / 10.0f;
	line_size = val;
	redraw_map();
}
