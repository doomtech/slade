
#ifndef __IMAGE_BOX_H__
#define __IMAGE_BOX_H__

#include <wx/image.h>

class ImageBox : public wxPanel
{
private:
	wxImage	*image;

public:
	ImageBox(wxWindow *parent, string filename);
	ImageBox(wxWindow *parent, wxImage img);
	~ImageBox();

	void paint(wxPaintEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif
