
#include "main.h"
#include "prefs_dialog.h"

#ifdef WIN32
vector<DEVMODE> win32_modes;
#endif

EXTERN_CVAR(Bool, render_fog)
EXTERN_CVAR(Bool, render_fullbright)
EXTERN_CVAR(Bool, render_hilight)
EXTERN_CVAR(Bool, render_thing_hilight)
EXTERN_CVAR(Int, render_things)
EXTERN_CVAR(Int, render_text_scale)
EXTERN_CVAR(Float, move_speed_3d)
EXTERN_CVAR(Float, mouse_speed_3d)
EXTERN_CVAR(Bool, invert_mouse_3d)
EXTERN_CVAR(Int, vid_width_3d)
EXTERN_CVAR(Int, vid_height_3d)
EXTERN_CVAR(Int, vid_bpp_3d)
EXTERN_CVAR(Int, vid_refresh_3d)

RenderPrefs::RenderPrefs(wxWindow *parent)
:	wxPanel(parent, -1)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Rendering frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Rendering"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	cb_fog = new wxCheckBox(this, RP_CB_FOG, _T("Depth Fog"));
	if (render_fog) cb_fog->SetValue(true);
	box->Add(cb_fog, 0, wxBOTTOM, 2);

	cb_fullbright = new wxCheckBox(this, RP_CB_FULLBRIGHT, _T("Fullbright"));
	if (render_fullbright) cb_fullbright->SetValue(true);
	box->Add(cb_fullbright, 0, wxBOTTOM, 2);

	cb_hilight = new wxCheckBox(this, RP_CB_HILIGHT, _T("Show Hilight"));
	if (render_hilight) cb_hilight->SetValue(true);
	box->Add(cb_hilight, 0, wxBOTTOM, 2);

	cb_hilightthings = new wxCheckBox(this, RP_CB_HLTHINGS, _T("Always Show Thing Hilight"));
	if (render_thing_hilight) cb_hilightthings->SetValue(true);
	box->Add(cb_hilightthings, 0, wxBOTTOM, 2);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0);

	hbox->Add(new wxStaticText(this, -1, _T("Things:")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);

	wxString thingstyles[] = {
		_T("None"),
		_T("Sprites Only"),
		_T("Sprites + Floor Boxes"),
		_T("Sprites + Full Boxes"),
	};

	combo_thingstyle = new wxChoice(this, RP_COMBO_THINGSTYLE, wxDefaultPosition, wxDefaultSize, 4, thingstyles);
	combo_thingstyle->SetSelection(render_things);
	hbox->Add(combo_thingstyle, 0, wxEXPAND);

	// Input frame
	frame = new wxStaticBox(this, -1, _T("Input"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxBOTTOM, 2);

	sl_movespeed = new wxSlider(this, RP_SL_MOVESPEED, move_speed_3d * 100, 1, 100);
	hbox->Add(new wxStaticText(this, -1, _T("Movement Speed:")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(sl_movespeed, 1, wxEXPAND);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxBOTTOM, 2);

	sl_mousespeed = new wxSlider(this, RP_SL_MOUSESPEED, mouse_speed_3d * 100, 10, 400);
	hbox->Add(new wxStaticText(this, -1, _T("Mouse Speed:")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	hbox->Add(sl_mousespeed, 1, wxEXPAND|wxRIGHT, 4);

	cb_invertmouse = new wxCheckBox(this, RP_CB_INVERTMOUSE, _T("Invert Y"));
	if (invert_mouse_3d) cb_invertmouse->SetValue(true);
	hbox->Add(cb_invertmouse, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL);

#ifdef WIN32
	DEVMODE dm;

	// Resolution frame (Windows only for now)
	frame = new wxStaticBox(this, -1, "Resolution");
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);
	
	cb_desktopres = new wxCheckBox(this, RP_CB_DESKRES, "Use Desktop Resolution");
	box->Add(cb_desktopres, 0, wxBOTTOM, 2);

	int i = 0;
	int index = -1;
	bool done = false;
	win32_modes.clear();
	wxArrayString choices;
	while (!done)
	{
		if (EnumDisplaySettings(0, i, &dm))
		{
			// Only allow modes with a specific refresh, and >= 640x400x16
			if (dm.dmBitsPerPel >= 16 && dm.dmDisplayFrequency > 0 && dm.dmPelsWidth >= 640 && dm.dmPelsHeight >= 400)
			{
				win32_modes.push_back(dm);

				// Get a string describing the mode
				wxString mode = wxString::Format("%dx%d, %dbpp, %dHz",
											dm.dmPelsWidth, dm.dmPelsHeight,
											dm.dmBitsPerPel, dm.dmDisplayFrequency);
				
				//log_message("%s\n", mode.c_str());
				//gtk_combo_box_append_text(GTK_COMBO_BOX(combo), mode.c_str());
				choices.Add(mode);

				if (vid_width_3d == dm.dmPelsWidth &&
					vid_height_3d == dm.dmPelsHeight &&
					vid_bpp_3d == dm.dmBitsPerPel &&
					vid_refresh_3d == dm.dmDisplayFrequency)
					index = win32_modes.size() - 1;
			}

			i++;
		}
		else
			done = true;
	}

	combo_resolutions = new wxChoice(this, RP_COMBO_RES, wxDefaultPosition, wxDefaultSize, choices);
	box->Add(combo_resolutions, 1, wxEXPAND);

	if (index == -1)
	{
		cb_desktopres->SetValue(true);
		combo_resolutions->Enable(false);
	}
	else
		combo_resolutions->Select(index);

#endif
}

RenderPrefs::~RenderPrefs()
{
}

BEGIN_EVENT_TABLE(RenderPrefs, wxPanel)
	EVT_CHECKBOX(RP_CB_FOG, RenderPrefs::cb_fog_toggled)
	EVT_CHECKBOX(RP_CB_FULLBRIGHT, RenderPrefs::cb_fullbright_toggled)
	EVT_CHECKBOX(RP_CB_HILIGHT, RenderPrefs::cb_hilight_toggled)
	EVT_CHECKBOX(RP_CB_HLTHINGS, RenderPrefs::cb_hilightthings_toggled)
	EVT_CHOICE(RP_COMBO_THINGSTYLE, RenderPrefs::combo_thingstyle_changed)
	EVT_COMMAND_SCROLL(RP_SL_MOVESPEED, RenderPrefs::sl_movespeed_changed)
	EVT_COMMAND_SCROLL(RP_SL_MOUSESPEED, RenderPrefs::sl_mousespeed_changed)
	EVT_CHECKBOX(RP_CB_INVERTMOUSE, RenderPrefs::cb_invertmouse_toggled)
	EVT_CHECKBOX(RP_CB_DESKRES, RenderPrefs::cb_desktopres_toggled)
	EVT_CHOICE(RP_COMBO_RES, RenderPrefs::combo_res_changed)
END_EVENT_TABLE()

void RenderPrefs::cb_fog_toggled(wxCommandEvent &event)
{
	render_fog = cb_fog->GetValue();
}

void RenderPrefs::cb_fullbright_toggled(wxCommandEvent &event)
{
	render_fullbright = cb_fullbright->GetValue();
}

void RenderPrefs::cb_hilight_toggled(wxCommandEvent &event)
{
	render_hilight = cb_hilight->GetValue();
}

void RenderPrefs::cb_hilightthings_toggled(wxCommandEvent &event)
{
	render_thing_hilight = cb_hilightthings->GetValue();
}

void RenderPrefs::combo_thingstyle_changed(wxCommandEvent &event)
{
	render_things = combo_thingstyle->GetSelection();
}

void RenderPrefs::sl_movespeed_changed(wxScrollEvent &event)
{
	move_speed_3d = (float)sl_movespeed->GetValue() * 0.01f;
}

void RenderPrefs::sl_mousespeed_changed(wxScrollEvent &event)
{
	mouse_speed_3d = (float)sl_mousespeed->GetValue() * 0.01f;
}

void RenderPrefs::cb_invertmouse_toggled(wxCommandEvent &event)
{
	invert_mouse_3d = cb_invertmouse->GetValue();
}

void RenderPrefs::cb_desktopres_toggled(wxCommandEvent &event)
{
	if (cb_desktopres->GetValue())
	{
		combo_resolutions->Enable(false);
		vid_width_3d = -1;
		vid_height_3d = -1;
		vid_bpp_3d = -1;
		vid_refresh_3d = -1;
	}
	else
	{
		combo_resolutions->Enable(true);
		combo_resolutions->Select(0);
	}
}

void RenderPrefs::combo_res_changed(wxCommandEvent &event)
{
	int index = combo_resolutions->GetSelection();

#ifdef WIN32
	vid_width_3d = win32_modes[index].dmPelsWidth;
	vid_height_3d = win32_modes[index].dmPelsHeight;
	vid_bpp_3d = win32_modes[index].dmBitsPerPel;
	vid_refresh_3d = win32_modes[index].dmDisplayFrequency;
#endif
}
