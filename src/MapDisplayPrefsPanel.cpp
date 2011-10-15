
#include "Main.h"
#include "WxStuff.h"
#include "MapDisplayPrefsPanel.h"
#include <wx/statline.h>

EXTERN_CVAR(Bool, grid_dashed)
EXTERN_CVAR(Bool, vertex_round)
EXTERN_CVAR(Int, vertex_size)
EXTERN_CVAR(Bool, vertices_always)
EXTERN_CVAR(Float, line_width)
EXTERN_CVAR(Bool, line_smooth)
EXTERN_CVAR(Int, thing_drawtype)
EXTERN_CVAR(Bool, things_always)
EXTERN_CVAR(Bool, thing_force_dir)
EXTERN_CVAR(Bool, thing_overlay_square)
EXTERN_CVAR(Float, thing_shadow)
EXTERN_CVAR(Float, flat_brightness)
EXTERN_CVAR(Bool, sector_hilight_fill)
EXTERN_CVAR(Bool, flat_ignore_light)

MapDisplayPrefsPanel::MapDisplayPrefsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Create sizer
	wxBoxSizer* psizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(psizer);

	// Create frame+sizer
	wxStaticBox *frame = new wxStaticBox(this, -1, "Map Editor Display Preferences");
	wxStaticBoxSizer *fsizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	psizer->Add(fsizer, 1, wxEXPAND|wxALL, 4);

	// Create notebook
	nb_pages = new wxNotebook(this, -1);
	fsizer->Add(nb_pages, 1, wxEXPAND|wxALL, 4);


	// General tab
	wxPanel* panel = new wxPanel(nb_pages, -1);
	nb_pages->AddPage(panel, "General", true);
	wxBoxSizer* sz_border = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(sz_border);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sz_border->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Dashed grid
	cb_grid_dashed = new wxCheckBox(panel, -1, "Dashed grid");
	cb_grid_dashed->SetValue(grid_dashed);
	sizer->Add(cb_grid_dashed, 0, wxEXPAND|wxALL, 4);

	// Always show vertices
	cb_vertices_always = new wxCheckBox(panel, -1, "Always show vertices");
	sizer->Add(cb_vertices_always, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Always show things
	cb_things_always = new wxCheckBox(panel, -1, "Always show things");
	sizer->Add(cb_things_always, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	
	// Vertices tab
	panel = new wxPanel(nb_pages, -1);
	nb_pages->AddPage(panel, "Vertices");
	sz_border = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(sz_border);
	sizer = new wxBoxSizer(wxVERTICAL);
	sz_border->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Vertex size
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(panel, -1, "Vertex size: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	slider_vertex_size = new wxSlider(panel, -1, vertex_size, 2, 16, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS);
	hbox->Add(slider_vertex_size, 1, wxEXPAND);

	// Round vertices
	cb_vertex_round = new wxCheckBox(panel, -1, "Round vertices");
	sizer->Add(cb_vertex_round, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);


	// Lines tab
	panel = new wxPanel(nb_pages, -1);
	nb_pages->AddPage(panel, "Lines");
	sz_border = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(sz_border);
	sizer = new wxBoxSizer(wxVERTICAL);
	sz_border->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Line width
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(panel, -1, "Line width: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	slider_line_width = new wxSlider(panel, -1, line_width*10, 10, 30, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS);
	hbox->Add(slider_line_width, 1, wxEXPAND);

	// Smooth lines
	cb_line_smooth = new wxCheckBox(panel, -1, "Smooth lines");
	sizer->Add(cb_line_smooth, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);


	// Things tab
	panel = new wxPanel(nb_pages, -1);
	nb_pages->AddPage(panel, "Things");
	sz_border = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(sz_border);
	sizer = new wxBoxSizer(wxVERTICAL);
	sz_border->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Thing style
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(panel, -1, "Thing style: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	string t_types[] = { "Square", "Round", "Sprite", "Square + Sprite" };
	choice_thing_drawtype = new wxChoice(panel, -1, wxDefaultPosition, wxDefaultSize, 4, t_types);
	hbox->Add(choice_thing_drawtype, 1, wxEXPAND);

	// Always show angles
	cb_thing_force_dir = new wxCheckBox(panel, -1, "Always show thing angles");
	sizer->Add(cb_thing_force_dir, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Force square hilight/selection
	cb_thing_overlay_square = new wxCheckBox(panel, -1, "Force square thing hilight/selection overlay");
	sizer->Add(cb_thing_overlay_square, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Shadow opacity
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(panel, -1, "Thing shadow opacity: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	slider_thing_shadow = new wxSlider(panel, -1, thing_shadow*10, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS);
	hbox->Add(slider_thing_shadow, 1, wxEXPAND);


	// Sectors tab
	panel = new wxPanel(nb_pages, -1);
	nb_pages->AddPage(panel, "Sectors");
	sz_border = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(sz_border);
	sizer = new wxBoxSizer(wxVERTICAL);
	sz_border->Add(sizer, 1, wxEXPAND|wxALL, 4);

	// Flat brightness
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(panel, -1, "Flat brightness: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	slider_flat_brightness = new wxSlider(panel, -1, flat_brightness*10, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS);
	hbox->Add(slider_flat_brightness, 1, wxEXPAND);

	// Ignore sector light
	cb_flat_ignore_light = new wxCheckBox(panel, -1, "Flats ignore sector brightness");
	sizer->Add(cb_flat_ignore_light, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Fill sector hilight
	cb_sector_hilight_fill = new wxCheckBox(panel, -1, "Filled sector hilight");
	sizer->Add(cb_sector_hilight_fill, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	/*
	// Dashed grid
	cb_grid_dashed = new wxCheckBox(this, -1, "Dashed grid");
	cb_grid_dashed->SetValue(grid_dashed);
	sizer->Add(cb_grid_dashed, 0, wxEXPAND|wxALL, 4);

	// Separator
	sizer->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND|wxALL, 4);

	// Vertex size
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(this, -1, "Vertex size: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	slider_vertex_size = new wxSlider(this, -1, vertex_size, 2, 16);
	hbox->Add(slider_vertex_size, 1, wxEXPAND);

	// Round vertices
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	cb_vertex_round = new wxCheckBox(this, -1, "Round vertices");
	hbox->Add(cb_vertex_round, 1, wxEXPAND|wxRIGHT, 4);

	// Always show vertices
	cb_vertices_always = new wxCheckBox(this, -1, "Always show vertices");
	hbox->Add(cb_vertices_always, 1, wxEXPAND);

	// Separator
	sizer->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND|wxALL, 4);

	// Line width
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(this, -1, "Line width: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	slider_line_width = new wxSlider(this, -1, line_width*10, 10, 30);
	hbox->Add(slider_line_width, 1, wxEXPAND);

	// Smooth lines
	cb_line_smooth = new wxCheckBox(this, -1, "Smooth lines");
	sizer->Add(cb_line_smooth, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Separator
	sizer->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND|wxALL, 4);

	// Thing style
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(this, -1, "Thing style: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	string t_types[] = { "Square", "Round", "Sprite", "Square + Sprite" };
	choice_thing_drawtype = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, 4, t_types);
	hbox->Add(choice_thing_drawtype, 1, wxEXPAND);

	// Always show things
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	cb_things_always = new wxCheckBox(this, -1, "Always show things");
	hbox->Add(cb_things_always, 1, wxEXPAND|wxRIGHT, 4);

	// Always show angles
	cb_thing_force_dir = new wxCheckBox(this, -1, "Always show thing angles");
	hbox->Add(cb_thing_force_dir, 1, wxEXPAND);

	// Force square hilight/selection
	cb_thing_overlay_square = new wxCheckBox(this, -1, "Force square thing hilight/selection overlay");
	sizer->Add(cb_thing_overlay_square, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Shadow opacity
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(this, -1, "Thing shadow opacity: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	slider_thing_shadow = new wxSlider(this, -1, thing_shadow*10, 0, 10);
	hbox->Add(slider_thing_shadow, 1, wxEXPAND);

	// Separator
	sizer->Add(new wxStaticLine(this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND|wxALL, 4);

	// Flat brightness
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	hbox->Add(new wxStaticText(this, -1, "Flat brightness: "), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 2);
	slider_flat_brightness = new wxSlider(this, -1, flat_brightness*10, 0, 10);
	hbox->Add(slider_flat_brightness, 1, wxEXPAND);

	// Ignore sector light
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	cb_flat_ignore_light = new wxCheckBox(this, -1, "Flats ignore sector brightness");
	hbox->Add(cb_flat_ignore_light, 1, wxEXPAND|wxRIGHT, 4);

	// Fill sector hilight
	cb_sector_hilight_fill = new wxCheckBox(this, -1, "Filled sector hilight");
	hbox->Add(cb_sector_hilight_fill, 1, wxEXPAND);
	*/


	// Init values
	cb_vertex_round->SetValue(vertex_round);
	cb_line_smooth->SetValue(line_smooth);
	cb_vertices_always->SetValue(vertices_always);
	choice_thing_drawtype->SetSelection(thing_drawtype);
	cb_things_always->SetValue(things_always);
	cb_thing_force_dir = cb_thing_force_dir;
	cb_thing_overlay_square->SetValue(thing_overlay_square);
	cb_flat_ignore_light->SetValue(flat_ignore_light);
	cb_sector_hilight_fill->SetValue(sector_hilight_fill);


	Layout();
}

MapDisplayPrefsPanel::~MapDisplayPrefsPanel() {
}

void MapDisplayPrefsPanel::applyPreferences() {
	grid_dashed = cb_grid_dashed->GetValue();
	vertex_round = cb_vertex_round->GetValue();
	vertex_size = slider_vertex_size->GetValue();
	vertices_always = cb_vertices_always->GetValue();
	line_width = (float)slider_line_width->GetValue() * 0.1f;
	line_smooth = cb_line_smooth->GetValue();
	thing_drawtype = choice_thing_drawtype->GetSelection();
	things_always = cb_things_always->GetValue();
	thing_force_dir = cb_thing_force_dir->GetValue();
	thing_overlay_square = cb_thing_overlay_square->GetValue();
	thing_shadow = (float)slider_thing_shadow->GetValue() * 0.1f;
	flat_brightness = (float)slider_flat_brightness->GetValue() * 0.1f;
	flat_ignore_light = cb_flat_ignore_light->GetValue();
	sector_hilight_fill = cb_sector_hilight_fill->GetValue();
}
