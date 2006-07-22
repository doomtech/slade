
#ifndef __TEX_BOX_H__
#define __TEX_BOX_H__

#include "textures.h"
#include "wx_stuff.h"
#include <wx/glcanvas.h>

class TextureBox : public wxGLCanvas
{
protected:
	Texture*	texture;
	int			textype;
	float		scale;

public:
	TextureBox(wxWindow *parent, int width, int height, string texture = "_notex", int textype = 0, float scale = 1.0f, int id = -1);
	~TextureBox();

	void set_size(int width, int height);
	void set_texture(string tex, int textype = -1);
	void redraw();

	void paint(wxPaintEvent &event);
	void size(wxSizeEvent &event);

	DECLARE_EVENT_TABLE()
};

// A texture box that opens a texture browser when clicked,
// and changes an entry box to the returned texture
class ClickableTexBox : public TextureBox
{
private:
	wxTextCtrl	*entry;

public:
	ClickableTexBox(wxWindow *parent, int width, int height, wxTextCtrl *entry,
		string texture = "_notex", int textype = 0, float scale = 1.0f, int id = -1)
		: TextureBox(parent, width, height, texture, textype, scale, id)
	{
		this->entry = entry;
	}

	~ClickableTexBox(){}

	void clicked(wxMouseEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif
