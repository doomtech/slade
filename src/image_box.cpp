// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-06               >>
// << ------------------------------------ >>
// << image_box.cpp - A simple 'image box' >>
// << widget                               >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "main.h"
#include "wx_stuff.h"
#include "image_box.h"

ImageBox::ImageBox(wxWindow *parent, string filename)
	: wxPanel(parent, -1)
{
	image = new wxImage(str_to_wx(filename));

	if (image->Ok())
		SetSizeHints(image->GetWidth(), image->GetHeight());
}

ImageBox::~ImageBox()
{
	delete image;
}

BEGIN_EVENT_TABLE(ImageBox, wxPanel)
	EVT_PAINT(ImageBox::paint)
END_EVENT_TABLE()

void ImageBox::paint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	if (image->Ok())
		dc.DrawBitmap(wxBitmap(*image), 0, 0, true);
}
