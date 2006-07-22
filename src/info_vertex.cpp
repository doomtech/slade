
#include "main.h"
#include "info_bar.h"
#include "doom_map.h"
#include "dm_vertex.h"

extern DoomMap d_map;

VertexInfoBar::VertexInfoBar(wxWindow *parent)
:	InfoBar(parent)
{
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(hbox);

	// Main frame
	main_frame = new wxStaticBox(this, -1, _T("No Vertex Hilighted"));
	wxFont def_font = main_frame->GetFont();
	def_font.SetWeight(wxFONTWEIGHT_BOLD);
	main_frame->SetFont(def_font);
	wxStaticBoxSizer *box = new wxStaticBoxSizer(main_frame, wxVERTICAL);
	hbox->Add(box, 1, wxEXPAND|wxRIGHT, 4);

	label_x = new wxStaticText(this, -1, _T("X Position:"));
	label_y = new wxStaticText(this, -1, _T("Y Position:"));

	box->Add(label_x);
	box->Add(label_y);

	SetSizeHints(-1, 128, -1, 128);
};

VertexInfoBar::~VertexInfoBar()
{
}

void VertexInfoBar::update()
{
	if (d_map.hilight() == -1)
	{
		main_frame->SetLabel(_T("No Vertex Hilighted"));
		label_x->SetLabel(_T("X Position:"));
		label_y->SetLabel(_T("Y Position:"));
	}
	else
	{
		main_frame->SetLabel(str_to_wx(s_fmt("Vertex #%d", d_map.hilight())));
		label_x->SetLabel(str_to_wx(s_fmt("X Position: %d", d_map.vertex(d_map.hilight())->x_pos())));
		label_y->SetLabel(str_to_wx(s_fmt("Y Position: %d", d_map.vertex(d_map.hilight())->y_pos())));
	}

	Layout();
}
