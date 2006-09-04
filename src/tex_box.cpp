
#include "main.h"
#include "tex_box.h"
#include "draw.h"
#include "textures.h"
#include "tex_browser.h"
#include "game_config.h"

//extern wxGLContext *gl_context;
extern wxGLCanvas *share_canvas;
extern GameConfig game;

TextureBox::TextureBox(wxWindow *parent, int width, int height, string texture, int textype, float scale, int id)
:	wxGLCanvas(parent, share_canvas, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER)
{
	if (texture != "_none_")
		this->texture = get_texture(texture, textype, false);
	else
		this->texture = NULL;

	this->scale = scale;

	if (width == -1)
		width = this->texture->width;

	if (height == -1)
		height = this->texture->height;

	Enable(false);
	Show(true);
	Raise();

	set_size(width, height);
	this->textype = textype;
}

TextureBox::~TextureBox()
{
}

BEGIN_EVENT_TABLE(TextureBox, wxGLCanvas)
	EVT_PAINT(TextureBox::paint)
	//EVT_SIZE(TextureBox::size)
END_EVENT_TABLE()

void TextureBox::paint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	redraw();
}

void TextureBox::size(wxSizeEvent &event)
{
	SetCurrent();

	glViewport(0, 0, GetClientSize().x, GetClientSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetClientSize().x, GetClientSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void TextureBox::redraw()
{
	//if (!IsShown())
	//	return;
	Show(true);

	SetCurrent();

	glViewport(0, 0, GetClientSize().x, GetClientSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetClientSize().x, GetClientSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	rgba_t col(180, 180, 180, 255, 0);

	glClearColor(col.fr(), col.fg(), col.fb(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (texture)
		draw_texture_scale(rect_t(0, 0, GetClientSize().x, GetClientSize().y), texture, COL_WHITE, scale);

	SwapBuffers();
}

void TextureBox::set_size(int width, int height)
{
	SetSizeHints(width, height, width, height);
	Update();
}

void TextureBox::set_texture(string tex, int textype)
{
	if (tex == "_none_")
	{
		if (texture)
		{
			texture = NULL;
			redraw();
		}

		return;
	}

	if (texture)
	{
		if (texture->name == tex)
			return;
	}

	texture = get_texture(tex, textype, false);

	if (textype != -1)
		this->textype = textype;

	redraw();
}

BEGIN_EVENT_TABLE(ClickableTexBox, wxGLCanvas)
	EVT_LEFT_DOWN(ClickableTexBox::clicked)
	EVT_PAINT(TextureBox::paint)
	//EVT_SIZE(TextureBox::size)
END_EVENT_TABLE()

void ClickableTexBox::clicked(wxMouseEvent &event)
{
	string texname;
	if (textype != TEXTURES_SPRITES)
	{
		if (texture)
			texname = texture->name;
		else
			texname = "-";
	}
	else
		texname = (char*)entry->GetValue().c_str();

	setup_tex_browser(textype);
	TextureBrowser tb(texname);

	if (tb.ShowModal() == wxID_OK && tb.get_texture() != "")
	{
		if (textype != TEXTURES_SPRITES)
		{
			texture = get_texture(tb.get_texture(), textype, false);
			entry->SetValue(texture->name.Upper());
		}
		else
		{
			int type = atoi(tb.get_texture().c_str());
			ThingType* ttype = game.get_ttype(type);
			texture = get_texture(ttype->spritename, textype, false);
			entry->SetValue(wxString::Format(_T("%d"), ttype->type));
		}

		redraw();
	}
}
