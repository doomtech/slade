
#include "main.h"
#include "sidebar.h"
#include "linedraw.h"
#include "tex_browser.h"

#include <wx/spinctrl.h>

extern string def_midtex, def_uptex, def_lotex, def_ftex, def_ctex, def_doortex, def_tracktex;
extern short def_fheight, def_cheight, def_light;

DrawSideBar::DrawSideBar(wxWindow *parent)
:	wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Drawing Options"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	box->Add(new wxStaticText(this, -1, _T("Shape:")), 0, wxEXPAND);

	tabs_shapes = new wxChoicebook(this, DSB_TABS_SHAPES, wxDefaultPosition, wxDefaultSize, wxCHB_TOP);
	box->Add(tabs_shapes, 0, wxEXPAND);

	// Rectangle drawing
	wxPanel *rect_panel = new wxPanel(tabs_shapes, -1);
	tabs_shapes->InsertPage(0, rect_panel, _T("Rectangle"), true);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	rect_panel->SetSizer(vbox);

	cb_rect_square = new wxCheckBox(rect_panel, -1, _T("Force square"));
	vbox->Add(cb_rect_square);

	// Circle drawing
	wxPanel *circle_panel = new wxPanel(tabs_shapes, -1);
	tabs_shapes->InsertPage(1, circle_panel, _T("Ellipse"), false);

	vbox = new wxBoxSizer(wxVERTICAL);
	circle_panel->SetSizer(vbox);

	cb_ellipse_circle = new wxCheckBox(circle_panel, -1, _T("Force circle"));
	vbox->Add(cb_ellipse_circle, wxEXPAND|wxBOTTOM, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox);

	hbox->Add(new wxStaticText(circle_panel, -1, _T("Sides:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	spin_ellipse_sides = new wxSpinCtrl(circle_panel, -1, _T("8"), wxDefaultPosition, wxSize(64, -1), 0, 3, 100, 8);
	hbox->Add(spin_ellipse_sides, wxEXPAND);

	SetSizeHints(128, -1, -1, -1);
	Layout();
}

DrawSideBar::~DrawSideBar()
{
}

void DrawSideBar::draw_shape()
{
	if (tabs_shapes->GetSelection() == 0)
		line_drawbox(cb_rect_square->GetValue());
	else if (tabs_shapes->GetSelection() == 1)
		line_drawcircle(spin_ellipse_sides->GetValue(), cb_ellipse_circle->GetValue());
}

BEGIN_EVENT_TABLE(DrawSideBar, wxPanel)
	//EVT_NOTEBOOK_PAGE_CHANGED(DSB_TABS_SHAPES, DrawSideBar::tabs_shapes_changed)
END_EVENT_TABLE()

void DrawSideBar::tabs_shapes_changed(wxNotebookEvent &event)
{
	Layout();
}



ThemeSideBar::ThemeSideBar(wxWindow *parent)
:	wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER)
{
	init = false;

	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Textures frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Textures"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	// Mid tex
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("Middle:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	entry_midtex = new wxTextCtrl(this, TSB_ENTRY_MIDTEX, str_to_wx(def_midtex), wxDefaultPosition, wxSize(64, -1));
	hbox->Add(entry_midtex, 0, wxEXPAND|wxRIGHT, 4);
	btn_midtex = new wxButton(this, TSB_BTN_MIDTEX, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_midtex, 0, wxEXPAND);

	// Up tex
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("Upper:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	entry_uptex = new wxTextCtrl(this, TSB_ENTRY_UPTEX, str_to_wx(def_uptex), wxDefaultPosition, wxSize(64, -1));
	hbox->Add(entry_uptex, 0, wxEXPAND|wxRIGHT, 4);
	btn_uptex = new wxButton(this, TSB_BTN_UPTEX, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_uptex, 0, wxEXPAND);

	// Lo tex
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("Lower:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	entry_lotex = new wxTextCtrl(this, TSB_ENTRY_LOTEX, str_to_wx(def_lotex), wxDefaultPosition, wxSize(64, -1));
	hbox->Add(entry_lotex, 0, wxEXPAND|wxRIGHT, 4);
	btn_lotex = new wxButton(this, TSB_BTN_LOTEX, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_lotex, 0, wxEXPAND);

	// Door tex
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("Door:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	entry_doortex = new wxTextCtrl(this, TSB_ENTRY_DOORTEX, str_to_wx(def_doortex), wxDefaultPosition, wxSize(64, -1));
	hbox->Add(entry_doortex, 0, wxEXPAND|wxRIGHT, 4);
	btn_doortex = new wxButton(this, TSB_BTN_DOORTEX, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_doortex, 0, wxEXPAND);

	// Door track tex
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("DTrack:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	entry_tracktex = new wxTextCtrl(this, TSB_ENTRY_TRACKTEX, str_to_wx(def_tracktex), wxDefaultPosition, wxSize(64, -1));
	hbox->Add(entry_tracktex, 0, wxEXPAND|wxRIGHT, 4);
	btn_tracktex = new wxButton(this, TSB_BTN_TRACKTEX, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_tracktex, 0, wxEXPAND);

	// Floor tex
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("Floor:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	entry_floortex = new wxTextCtrl(this, TSB_ENTRY_FLOORTEX, str_to_wx(def_ftex), wxDefaultPosition, wxSize(64, -1));
	hbox->Add(entry_floortex, 0, wxEXPAND|wxRIGHT, 4);
	btn_floortex = new wxButton(this, TSB_BTN_FLOORTEX, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_floortex, 0, wxEXPAND);

	// Ceiling tex
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("Ceiling:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	entry_ceiltex = new wxTextCtrl(this, TSB_ENTRY_CEILTEX, str_to_wx(def_ctex), wxDefaultPosition, wxSize(64, -1));
	hbox->Add(entry_ceiltex, 0, wxEXPAND|wxRIGHT, 4);
	btn_ceiltex = new wxButton(this, TSB_BTN_CEILTEX, _T("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_ceiltex, 0, wxEXPAND);

	// Sector frame
	frame = new wxStaticBox(this, -1, _T("Sector Properties"));
	box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	// Floor height
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("F Height:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	spin_floorheight = new wxSpinCtrl(this, TSB_SPIN_FLOORHEIGHT, wxString::Format(_T("%d"), def_fheight),
										wxDefaultPosition, wxSize(64, -1), 0, -65535, 65535, def_fheight);
	hbox->Add(spin_floorheight, 0, wxEXPAND|wxRIGHT, 4);

	// Ceiling height
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("C Height:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	spin_ceilheight = new wxSpinCtrl(this, TSB_SPIN_CEILHEIGHT, wxString::Format(_T("%d"), def_cheight),
										wxDefaultPosition, wxSize(64, -1), 0, -65535, 65535, def_cheight);
	hbox->Add(spin_ceilheight, 0, wxEXPAND|wxRIGHT, 4);

	// Light
	hbox = new wxBoxSizer(wxHORIZONTAL);
	box->Add(hbox, 0, wxEXPAND|wxALL, 2);

	hbox->Add(new wxStaticText(this, -1, _T("Light:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->AddStretchSpacer();
	spin_light = new wxSpinCtrl(this, TSB_SPIN_LIGHT, wxString::Format(_T("%d"), def_light),
										wxDefaultPosition, wxSize(64, -1), 0, -65535, 65535, def_light);
	hbox->Add(spin_light, 0, wxEXPAND|wxRIGHT, 4);

	init = true;
}

ThemeSideBar::~ThemeSideBar()
{
}

BEGIN_EVENT_TABLE(ThemeSideBar, wxPanel)
	EVT_TEXT(TSB_ENTRY_MIDTEX, ThemeSideBar::entry_changed)
	EVT_TEXT(TSB_ENTRY_UPTEX, ThemeSideBar::entry_changed)
	EVT_TEXT(TSB_ENTRY_LOTEX, ThemeSideBar::entry_changed)
	EVT_TEXT(TSB_ENTRY_DOORTEX, ThemeSideBar::entry_changed)
	EVT_TEXT(TSB_ENTRY_TRACKTEX, ThemeSideBar::entry_changed)
	EVT_TEXT(TSB_ENTRY_FLOORTEX, ThemeSideBar::entry_changed)
	EVT_TEXT(TSB_ENTRY_CEILTEX, ThemeSideBar::entry_changed)

	EVT_BUTTON(TSB_BTN_MIDTEX, ThemeSideBar::btn_midtex_clicked)
	EVT_BUTTON(TSB_BTN_UPTEX, ThemeSideBar::btn_uptex_clicked)
	EVT_BUTTON(TSB_BTN_LOTEX, ThemeSideBar::btn_lotex_clicked)
	EVT_BUTTON(TSB_BTN_DOORTEX, ThemeSideBar::btn_doortex_clicked)
	EVT_BUTTON(TSB_BTN_TRACKTEX, ThemeSideBar::btn_tracktex_clicked)
	EVT_BUTTON(TSB_BTN_FLOORTEX, ThemeSideBar::btn_floortex_clicked)
	EVT_BUTTON(TSB_BTN_CEILTEX, ThemeSideBar::btn_ceiltex_clicked)

	EVT_TEXT(TSB_SPIN_FLOORHEIGHT, ThemeSideBar::entry_changed)
	EVT_TEXT(TSB_SPIN_CEILHEIGHT, ThemeSideBar::entry_changed)
	EVT_TEXT(TSB_SPIN_LIGHT, ThemeSideBar::entry_changed)
END_EVENT_TABLE()

void ThemeSideBar::entry_changed(wxCommandEvent &event)
{
	if (init)
	{
		def_midtex = wx_to_str(entry_midtex->GetValue());
		def_uptex = wx_to_str(entry_uptex->GetValue());
		def_lotex = wx_to_str(entry_lotex->GetValue());
		def_doortex = wx_to_str(entry_doortex->GetValue());
		def_tracktex = wx_to_str(entry_tracktex->GetValue());
		def_ftex = wx_to_str(entry_floortex->GetValue());
		def_ctex = wx_to_str(entry_ceiltex->GetValue());

		def_fheight = spin_floorheight->GetValue();
		def_cheight = spin_ceilheight->GetValue();
		def_light = spin_light->GetValue();
	}
}

void ThemeSideBar::btn_midtex_clicked(wxCommandEvent &event)
{
	TextureBrowser tb(1, def_midtex);

	if (tb.ShowModal() == wxID_OK)
		entry_midtex->SetValue(str_to_wx(tb.get_texture()));
}

void ThemeSideBar::btn_uptex_clicked(wxCommandEvent &event)
{
	TextureBrowser tb(1, def_uptex);

	if (tb.ShowModal() == wxID_OK)
		entry_uptex->SetValue(str_to_wx(tb.get_texture()));
}

void ThemeSideBar::btn_lotex_clicked(wxCommandEvent &event)
{
	TextureBrowser tb(1, def_lotex);

	if (tb.ShowModal() == wxID_OK)
		entry_lotex->SetValue(str_to_wx(tb.get_texture()));
}

void ThemeSideBar::btn_doortex_clicked(wxCommandEvent &event)
{
	TextureBrowser tb(1, def_doortex);

	if (tb.ShowModal() == wxID_OK)
		entry_doortex->SetValue(str_to_wx(tb.get_texture()));
}

void ThemeSideBar::btn_tracktex_clicked(wxCommandEvent &event)
{
	TextureBrowser tb(1, def_tracktex);

	if (tb.ShowModal() == wxID_OK)
		entry_tracktex->SetValue(str_to_wx(tb.get_texture()));
}

void ThemeSideBar::btn_floortex_clicked(wxCommandEvent &event)
{
	TextureBrowser tb(2, def_ftex);

	if (tb.ShowModal() == wxID_OK)
		entry_floortex->SetValue(str_to_wx(tb.get_texture()));
}

void ThemeSideBar::btn_ceiltex_clicked(wxCommandEvent &event)
{
	TextureBrowser tb(2, def_ctex);

	if (tb.ShowModal() == wxID_OK)
		entry_ceiltex->SetValue(str_to_wx(tb.get_texture()));
}
