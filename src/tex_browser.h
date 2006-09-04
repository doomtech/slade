
#include "wx_stuff.h"
#include <wx/glcanvas.h>
#include <wx/spinctrl.h>
#include "textures.h"

struct browse_info_t
{
	string	retval;
	Texture	*tex;

	browse_info_t() : retval(_T("")), tex(NULL) { }

	int use_count()
	{
		if (tex)
			return tex->use_count;
		else
			return 0;
	}
};

class TexBrowseCanvas : public wxGLCanvas
{
private:

public:
	wxDialog		*parent;
	wxScrollBar		*vscroll;
	wxTextCtrl		*search;
	int				selected_item;

	TexBrowseCanvas(wxWindow *parent);
	~TexBrowseCanvas();

	void search_str(string str);
	void scroll_to_selected();
	void redraw();
	void select_name(string name);

	void mouse_event(wxMouseEvent& event);
	void key_down(wxKeyEvent &event);
	void paint(wxPaintEvent &event);
	void resize(wxSizeEvent &event);

	DECLARE_EVENT_TABLE()
};


class TextureBrowser : public wxDialog
{
private:
	TexBrowseCanvas	*browse_area;
	wxScrollBar		*vscroll;
	wxTextCtrl		*search;
	wxChoice		*sorting;
	wxSpinCtrl		*n_columns;
	wxCheckBox		*cb_iwad_textures;
	wxCheckBox		*cb_hide_unused;

public:
	enum
	{
		BROWSER_VSCROLL,
		BROWSER_SEARCH,
		BROWSER_SORTING,
		BROWSER_COLS,
		BROWSER_CB_IWADTEXTURES,
		BROWSER_CB_HIDEUNUSED,
	};

	TextureBrowser(string tex = _T("-"), bool fullscreen = false);
	~TextureBrowser();

	string get_texture();
	void sort(int sorting);
	void update_vis_items();

	void key_down(wxKeyEvent &event);
	void scroll_event(wxScrollEvent &event);
	void search_changed(wxCommandEvent &event);
	void sorting_changed(wxCommandEvent &event);
	void columns_changed(wxSpinEvent &event);
	void cb_iwadtextures_toggled(wxCommandEvent &event);
	void cb_hideunused_toggled(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

void setup_tex_browser(int type);
