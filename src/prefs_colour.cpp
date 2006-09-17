
#include <wx/colordlg.h>
#include <wx/dir.h>

#include "main.h"
#include "prefs_dialog.h"
#include "colours.h"

extern coldef_t colours[];
extern unsigned int n_colours;

ColourBox::ColourBox(wxWindow *parent, rgba_t *colour)
:	wxPanel(parent, -1, wxDefaultPosition, wxSize(32, 24), wxSUNKEN_BORDER)
{
	rgba = colour;
}

ColourBox::~ColourBox()
{
}

BEGIN_EVENT_TABLE(ColourBox, wxPanel)
	EVT_PAINT(ColourBox::paint)
	EVT_LEFT_DOWN(ColourBox::clicked)
	EVT_RIGHT_DOWN(ColourBox::clicked2)
END_EVENT_TABLE()

void ColourBox::paint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	dc.BeginDrawing();

	dc.SetBrush(wxBrush(wxColour(rgba->r, rgba->g, rgba->b)));
	dc.DrawRectangle(0, 0, GetClientSize().x, GetClientSize().y);

	int a_point = rgba->fa() * (GetClientSize().x - 2);

	dc.SetBrush(wxBrush(wxColour(0, 0, 0)));
	dc.DrawRectangle(0, 0, GetClientSize().x, 4);

	dc.SetBrush(wxBrush(wxColour(255, 255, 255)));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(1, 1, a_point, 2);

	dc.EndDrawing();
}

void ColourBox::clicked(wxMouseEvent &event)
{
	wxColour col = wxGetColourFromUser(GetParent(), wxColour(rgba->r, rgba->g, rgba->b));

	if (col.Ok())
	{
		rgba->r = col.Red();
		rgba->g = col.Green();
		rgba->b = col.Blue();
		Refresh();
		redraw_map();
	}
}

void ColourBox::clicked2(wxMouseEvent &event)
{
	// Popup a dialog with a slider control for alpha
	wxDialog dlg(NULL, -1, _T("Set Alpha"), wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
	dlg.SetSizer(box);
	wxSlider *slider = new wxSlider(&dlg, -1, rgba->a, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	box->Add(slider, 1, wxEXPAND|wxALL, 4);
	box->Add(dlg.CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);
	dlg.SetBestFittingSize();

	if (dlg.ShowModal() == wxID_OK)
	{
		rgba->a = slider->GetValue();
		Refresh();
		redraw_map();
	}
}

ColourEdit::ColourEdit(wxWindow *parent, rgba_t *colour, string name)
:	wxPanel(parent, -1)
{
	col = colour;

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(hbox);

	hbox->Add(new wxStaticText(this, -1, name), 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);

	cbox_colour = new ColourBox(this, col);
	hbox->Add(cbox_colour, 0, wxEXPAND|wxRIGHT, 4);

	cb_additive = new wxCheckBox(this, 1, _T("Additive"));
	hbox->Add(cb_additive, 0, wxALIGN_CENTER_VERTICAL);
	if (colour->blend == 1)
		cb_additive->SetValue(true);
}

ColourEdit::~ColourEdit()
{
}

BEGIN_EVENT_TABLE(ColourEdit, wxPanel)
	EVT_CHECKBOX(1, ColourEdit::cb_additive_changed)
END_EVENT_TABLE()

void ColourEdit::cb_additive_changed(wxCommandEvent &event)
{
	if (cb_additive->GetValue())
		col->blend = 1;
	else
		col->blend = 0;

	redraw_map();
}

ColourPrefs::ColourPrefs(wxWindow *parent)
:	wxPanel(parent, -1)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Colours frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Colours"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 1, wxEXPAND|wxALL, 4);

	box->Add(new wxStaticText(this, -1, _T("Right click the colour boxes to set alpha")), 0, wxEXPAND|wxBOTTOM, 2);

	wxScrolledWindow *s_window = new wxScrolledWindow(this, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxSUNKEN_BORDER);
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	s_window->SetSizer(vbox);
	s_window->SetScrollRate(0, 4);
	box->Add(s_window, 1, wxEXPAND);

	ColourEdit *cedit = NULL;

	for (int a = 0; a < n_colours; a++)
	{
		cedit = new ColourEdit(s_window, &colours[a].col, colours[a].desc);
		vbox->Add(cedit, 0, wxEXPAND|wxALL, 4);
	}

	// Buttons
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	hbox->AddSpacer(1);

	btn_loadconfig = new wxButton(this, CP_BTN_LOAD, _T("Load Config"));
	hbox->Add(btn_loadconfig, 0, wxEXPAND|wxRIGHT, 4);

	btn_saveconfig = new wxButton(this, CP_BTN_SAVE, _T("Save Config"));
	hbox->Add(btn_saveconfig, 0, wxEXPAND);
}

ColourPrefs::~ColourPrefs()
{
}

BEGIN_EVENT_TABLE(ColourPrefs, wxPanel)
	EVT_BUTTON(CP_BTN_LOAD, ColourPrefs::load_clicked)
	EVT_BUTTON(CP_BTN_SAVE, ColourPrefs::save_clicked)
END_EVENT_TABLE()

void ColourPrefs::save_clicked(wxCommandEvent &event)
{
	string filename = wxFileSelector(_T("Save Colour Configuration"), _T("./config/colours"), _T(""),
							_T("*.cfg"), _T("Configuration Files (*.cfg)|*.cfg"), wxSAVE|wxOVERWRITE_PROMPT);

	if (filename.size())
		save_colour_config(filename);
}

void ColourPrefs::load_clicked(wxCommandEvent &event)
{
	string filename = _T("");
	wxFileDialog browse(NULL, _T("Load Colour Configuration"), _T("./config/colours"), _T(""), _T("Configuration Files (*.cfg)|*.cfg"), wxOPEN|wxFILE_MUST_EXIST);
	if (browse.ShowModal() == wxID_OK)
		filename = browse.GetPath();

	log_message(filename);

	if (filename.size())
		load_colour_config(filename);

	redraw_map();
}
