
#include "main.h"
#include "tex_browser.h"
#include "doom_map.h"
#include "draw.h"
#include "game_config.h"

#include <wx/spinctrl.h>

CVAR(Int, browser_columns, 6, CVAR_SAVE)
CVAR(Int, browser_sort, 1, CVAR_SAVE)
CVAR(Bool, browser_iwadtex, true, CVAR_SAVE)
CVAR(Bool, browser_hideunused, true, CVAR_SAVE)

vector<browse_info_t>	browse_items;
vector<browse_info_t*>	vis_items;
int						browse_type = -1;

extern wxGLContext *gl_context;

extern bool mix_tex;

extern vector<Texture*>	textures;
extern vector<Texture*>	flats;
extern vector<Texture*>	sprites;

extern DoomMap d_map;
extern GameConfig game;

TextureBrowser::TextureBrowser(string tex, bool fullscreen)
:	wxDialog(NULL, -1, _T("Browse Textures"), wxDefaultPosition, wxSize(640, 480), wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxSYSTEM_MENU|wxWANTS_CHARS)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 1, wxEXPAND|wxALL, 4);

	vscroll = new wxScrollBar(this, BROWSER_VSCROLL, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
	vscroll->SetScrollbar(0, 1, 10, 1);
	browse_area = new TexBrowseCanvas(this);

	hbox->Add(browse_area, 1, wxEXPAND);
	hbox->Add(vscroll, 0, wxEXPAND);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	hbox->Add(vbox, 0, wxEXPAND|wxALL, 4);

	search = new wxTextCtrl(this, BROWSER_SEARCH, _T(""));
	vbox->Add(new wxStaticText(this, -1, _T("Search:")), 0, wxBOTTOM, 2);
	vbox->Add(search, 0, wxEXPAND|wxBOTTOM, 8);

	wxString sort[] = {
		_T("Alphabetical"),
		_T("Most Used"),
	};

	sorting = new wxChoice(this, BROWSER_SORTING, wxDefaultPosition, wxDefaultSize, 2, sort);
	sorting->Select(browser_sort);
	vbox->Add(new wxStaticText(this, -1, _T("Sorting:")), 0, wxBOTTOM, 2);
	vbox->Add(sorting, 0, wxEXPAND|wxBOTTOM, 8);

	cb_iwad_textures = new wxCheckBox(this, BROWSER_CB_IWADTEXTURES, _T("Hide IWAD Textures"));
	vbox->Add(cb_iwad_textures, 0, wxEXPAND|wxBOTTOM, 8);
	if (browser_iwadtex)
		cb_iwad_textures->SetValue(true);

	cb_hide_unused = new wxCheckBox(this, BROWSER_CB_HIDEUNUSED, _T("Hide Unused Textures"));
	vbox->Add(cb_hide_unused, 0, wxEXPAND|wxBOTTOM, 8);
	if (browser_hideunused)
		cb_hide_unused->SetValue(true);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND);

	n_columns = new wxSpinCtrl(this, BROWSER_COLS, wxString::Format(_T("%d"), (int)browser_columns), wxDefaultPosition, wxSize(56, -1),
								wxSP_ARROW_KEYS, 4, 10, browser_columns);
	hbox->Add(new wxStaticText(this, -1, _T("Columns:")), 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->Add(n_columns, 0, wxEXPAND);

	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	update_vis_items();

	this->sort(browser_sort);

	browse_area->search = search;
	browse_area->vscroll = vscroll;
	browse_area->select_name(tex);
	browse_area->scroll_to_selected();
	browse_area->parent = this;
	browse_area->SetFocus();

	if (fullscreen)
		ShowFullScreen(true);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	SetMinSize(wxSize(320, 240));
}

TextureBrowser::~TextureBrowser()
{
}

string TextureBrowser::get_texture()
{
	if (browse_area->selected_item == -1)
		return "";

	return vis_items[browse_area->selected_item]->retval;
}

void TextureBrowser::sort(int sorting)
{
	// Don't sort things (yet)
	if (browse_type == TEXTURES_SPRITES)
		return;

	// Sort alphabetically
	if (sorting == 0 && browse_type != 3)
	{
		for (int p = 1; p < vis_items.size(); p++)
		{
			browse_info_t* tmp = vis_items[p];
			int j;

			for (j = p; j > 0 && tmp->retval < vis_items[j - 1]->retval; j--)
				vis_items[j] = vis_items[j - 1];

			vis_items[j] = tmp;
		}
	}

	// Sort by use
	else if (sorting == 1)
	{
		for (int p = 1; p < vis_items.size(); p++)
		{
			browse_info_t* tmp = vis_items[p];
			int j;

			for (j = p; j > 0 && tmp->use_count() > vis_items[j - 1]->use_count(); j--)
				vis_items[j] = vis_items[j - 1];

			vis_items[j] = tmp;
		}
	}
}

void TextureBrowser::update_vis_items()
{
	string cur_val = "";
	int sel = browse_area->selected_item;

	if (sel >= 0 && sel < vis_items.size())
		cur_val = vis_items[browse_area->selected_item]->retval;

	vis_items.clear();

	if (browse_type != 3)
		vis_items.push_back(&browse_items[0]);

	for (int a = 0; a < browse_items.size(); a++)
	{
		if (!browse_items[a].tex)
			continue;

		bool vis = true;

		if (browse_type == 1 || browse_type == 2)
		{
			if (browser_iwadtex && browse_items[a].tex->flags & TEX_IWAD)
				vis = false;

			if (browser_hideunused && browse_items[a].tex->use_count == 0)
				vis = false;
		}

		if (vis)
			vis_items.push_back(&browse_items[a]);
	}

	browse_area->select_name(cur_val);
}

BEGIN_EVENT_TABLE(TextureBrowser, wxDialog)
	EVT_COMMAND_SCROLL(BROWSER_VSCROLL, TextureBrowser::scroll_event)
	EVT_TEXT(BROWSER_SEARCH, TextureBrowser::search_changed)
	EVT_CHOICE(BROWSER_SORTING, TextureBrowser::sorting_changed)
	EVT_SPINCTRL(BROWSER_COLS, TextureBrowser::columns_changed)
	EVT_CHECKBOX(BROWSER_CB_IWADTEXTURES, TextureBrowser::cb_iwadtextures_toggled)
	EVT_CHECKBOX(BROWSER_CB_HIDEUNUSED, TextureBrowser::cb_hideunused_toggled)
END_EVENT_TABLE()

void TextureBrowser::cb_hideunused_toggled(wxCommandEvent &event)
{
	browser_hideunused = cb_hide_unused->GetValue();
	update_vis_items();
	sort(browser_sort);
	browse_area->scroll_to_selected();
	browse_area->redraw();
}

void TextureBrowser::cb_iwadtextures_toggled(wxCommandEvent &event)
{
	browser_iwadtex = cb_iwad_textures->GetValue();
	update_vis_items();
	sort(browser_sort);
	browse_area->scroll_to_selected();
	browse_area->redraw();
}

void TextureBrowser::key_down(wxKeyEvent &event)
{
	//wxMessageBox("Key down");
}

void TextureBrowser::scroll_event(wxScrollEvent &event)
{
	browse_area->redraw();
}

void TextureBrowser::search_changed(wxCommandEvent &event)
{
	browse_area->search_str(wx_to_str(search->GetValue().Upper()));
	browse_area->SetFocus();
}

void TextureBrowser::sorting_changed(wxCommandEvent &event)
{
	browser_sort = this->sorting->GetSelection();
	this->sort(browser_sort);
	browse_area->scroll_to_selected();
	browse_area->redraw();
}

void TextureBrowser::columns_changed(wxSpinEvent &event)
{
	browser_columns = event.GetPosition();
	browse_area->scroll_to_selected();
	browse_area->redraw();
}


TexBrowseCanvas::TexBrowseCanvas(wxWindow *parent)
:	wxGLCanvas(parent, gl_context, -1, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS)
{
}

TexBrowseCanvas::~TexBrowseCanvas()
{
}

void TexBrowseCanvas::redraw()
{
	SetCurrent();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int width = GetClientSize().x / browser_columns;
	int top = vscroll->GetThumbPosition();
	int col = 0;
	int row = 0;

	for (int a = 0; a < vis_items.size(); a++)
	{
		rect_t rect(col * width, (row * width) - top, width, width, 0);

		if (((row + 1) * width) > top && (row * width) < (top + GetClientSize().y))
		{
			if (selected_item == a)
			{
				draw_rect(rect, rgba_t(0, 180, 255, 150, 0), true);
				draw_rect(rect, rgba_t(100, 220, 255, 255, 0), false);
			}

			rect.resize(-8, -8);

			if (vis_items[a]->tex)
				draw_texture_scale(rect, vis_items[a]->tex);

			draw_text(rect.middle().x, rect.bottom() - 8, COL_WHITE, 1, false, vis_items[a]->retval.c_str());
		}

		col++;

		if (col == browser_columns)
		{
			row++;
			col = 0;
		}
	}

	SwapBuffers();
}

void TexBrowseCanvas::scroll_to_selected()
{
	if (vis_items.size() == 0)
		return;

	int rows = (vis_items.size() / browser_columns) + 1;
	int width = GetClientSize().x / browser_columns;
	int top = vscroll->GetThumbPosition();
	int bottom = top + GetClientSize().y;
	int rows_page = GetClientSize().y / width;

	vscroll->SetScrollbar(vscroll->GetThumbPosition(), GetClientSize().y, rows*width, rows_page*width);

	int a = 0;
	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < browser_columns; x++)
		{
			if (selected_item == a)
			{
				if (y * width < top)
					vscroll->SetThumbPosition(y * width);

				if ((y+1) * width > bottom)
					vscroll->SetThumbPosition(((y+1) * width) - GetClientSize().y);

				return;
			}
			a++;

			if (a >= vis_items.size())
				return;
		}
	}
}

void TexBrowseCanvas::search_str(string str)
{
	for (unsigned a = 0; a < vis_items.size(); a++)
	{
		if (vis_items[a]->retval.size() < str.size())
			continue;

		bool match = true;
		for (unsigned c = 0; c < str.size(); c++)
		{
			if (vis_items[a]->retval[c] != str[c])
				match = false;
		}

		if (match)
		{
			selected_item = a;
			scroll_to_selected();
			redraw();
			return;
		}
	}
}

void TexBrowseCanvas::select_name(string name)
{
	selected_item = -1;

	for (int a = 0; a < vis_items.size(); a++)
	{
		if (vis_items[a]->retval == name)
		{
			selected_item = a;
			break;
		}
	}
}

BEGIN_EVENT_TABLE(TexBrowseCanvas, wxGLCanvas)
	EVT_PAINT(TexBrowseCanvas::paint)
	EVT_SIZE(TexBrowseCanvas::resize)
	EVT_KEY_DOWN(TexBrowseCanvas::key_down)
	EVT_MOUSE_EVENTS(TexBrowseCanvas::mouse_event)
END_EVENT_TABLE()

void TexBrowseCanvas::paint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	redraw();
}

void TexBrowseCanvas::resize(wxSizeEvent &event)
{
	int width = GetClientSize().x / browser_columns;
	int rows = (vis_items.size() / browser_columns) + 1;
	int rows_page = GetClientSize().y / width;
	vscroll->SetScrollbar(vscroll->GetThumbPosition(), GetClientSize().y, rows*width, rows_page*width);
	scroll_to_selected();

	SetCurrent();

	glViewport(0, 0, GetClientSize().x, GetClientSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetClientSize().x, GetClientSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	redraw();
}

void TexBrowseCanvas::key_down(wxKeyEvent &event)
{
	//wxMessageBox("Key down");
	int width = GetClientSize().x / browser_columns;
	int rows = GetClientSize().y / width;

	int index = selected_item;

	bool done = false;

	if (event.GetKeyCode() == WXK_UP)
	{
		done = true;
		if (index - browser_columns >= 0)
			index -= browser_columns;
	}

	if (event.GetKeyCode() == WXK_LEFT)
	{
		done = true;
		if (index > 0)
			index--;
	}

	if (event.GetKeyCode() == WXK_RIGHT)
	{
		done = true;
		if (index < vis_items.size() - 1)
			index++;
	}

	if (event.GetKeyCode() == WXK_DOWN)
	{
		done = true;
		if (index < vis_items.size() - browser_columns)
			index += browser_columns;
	}

	if (event.GetKeyCode() == WXK_PAGEUP ||
		event.GetKeyCode() == WXK_PRIOR)
	{
		done = true;
		if (index - (browser_columns * rows) >= 0)
			index -= (browser_columns * rows);
		else
			index = 0;
	}

	if (event.GetKeyCode() == WXK_PAGEDOWN ||
		event.GetKeyCode() == WXK_NEXT)
	{
		done = true;
		if (index < vis_items.size() - (browser_columns * rows))
			index += (browser_columns * rows);
		else
			index = vis_items.size() - 1;
	}

	if (done)
	{
		selected_item = index;
		scroll_to_selected();
		redraw();
		return;
	}

	search->EmulateKeyPress(event);
}

void TexBrowseCanvas::mouse_event(wxMouseEvent &event)
{
	SetFocus();

	int width = GetClientSize().x / browser_columns;

	// Left click
	if (event.LeftDown())
	{
		int row = (vscroll->GetThumbPosition() + event.GetPosition().y) / width;
		int col = event.GetPosition().x / width;
		int index = (row * browser_columns) + col;

		if (index < vis_items.size())
			selected_item = index;

		redraw();
	}

	// Double left click
	if (event.LeftDClick())
	{
		parent->EndModal(wxID_OK);
		return;
	}

	// Scrolling
	if (event.GetWheelRotation() > 0)
	{
		vscroll->SetThumbPosition(vscroll->GetThumbPosition() - width);
		redraw();
	}
	else if (event.GetWheelRotation() < 0)
	{
		vscroll->SetThumbPosition(vscroll->GetThumbPosition() + width);
		redraw();
	}
}

void setup_tex_browser(int type)
{
	if (browse_type == type)
		return;

	browse_items.clear();
	browse_type = type;

	// Things
	if (type == 3)
	{
		for (int a = 0; a < game.get_ttypes().size(); a++)
		{
			ThingType tt = game.get_ttypes()[a];

			browse_info_t bi;
			bi.retval = s_fmt("%d", tt.type);
			bi.tex = get_texture(tt.spritename, 3);
			browse_items.push_back(bi);
		}

		return;
	}

	browse_info_t blank;
	blank.retval = "-";
	blank.tex = NULL;
	browse_items.push_back(blank);

	// Textures
	if (type == 1 || mix_tex)
	{
		for (int a = 0; a < textures.size(); a++)
		{
			browse_info_t bi;
			bi.retval = textures[a]->name;
			bi.tex = textures[a];
			browse_items.push_back(bi);
		}
	}

	// Flats
	if (type == 2 || mix_tex)
	{
		for (int a = 0; a < flats.size(); a++)
		{
			browse_info_t bi;
			bi.retval = flats[a]->name;
			bi.tex = flats[a];
			browse_items.push_back(bi);
		}
	}
}
