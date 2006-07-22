
#include "main.h"
#include "structs.h"
#include "textures.h"

#include <GL/gl.h>

char charmap[16][16] =
{
	{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P'},
	{ 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f'},
	{ 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v'},
	{ 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '(', ')'},
	{ '!', '?', '#', '&', '*', '.', ',', ':', ';', '"', '\'', '/', '-', '+', '_', '%'},
	{ '=', '{', '}', '<', '>', '[', ']', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
};

// set_gl_colour: Sets the current OpenGL colour
// ------------------------------------------ >>
void set_gl_colour(rgba_t col)
{
	glColor4f(col.fr(), col.fg(), col.fb(), col.fa());

	if (col.blend == 0)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (col.blend == 1)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

// draw_point: Draws a point of given size at (x, y)
// ---------------------------------------------- >>
void draw_point(int x, int y, int size, rgba_t col)
{
	glDisable(GL_TEXTURE_2D);

	set_gl_colour(col);
	col.set_blend();
	glPointSize((float)size);

	glBegin(GL_POINTS);
		glVertex2d(x, y);
	glEnd();
}

// draw_line: Draws a line from a rect's top left to it's bottom right
// ---------------------------------------------------------------- >>
void draw_line(rect_t rect, rgba_t col, bool aa, bool side_indicator, float width)
{
	glDisable(GL_TEXTURE_2D);

	if (aa)
		glEnable(GL_LINE_SMOOTH);
	else
		glDisable(GL_LINE_SMOOTH);

	set_gl_colour(col);
	col.set_blend();

	glLineWidth(width);
	
	glBegin(GL_LINES);
		glVertex2d(rect.tl.x, rect.tl.y);
		glVertex2d(rect.br.x, rect.br.y);
	glEnd();

	if (side_indicator)
	{
		fpoint2_t mid((float)rect.left() + (float)rect.awidth() * 0.5f, (float)rect.top() + (float)rect.aheight() * 0.5f);
		point3_t vec(rect.x2() - rect.x1(), rect.y2() - rect.y1(), 0.0f);
		float x = -vec.y / 8.0f;
		float y = vec.x / 8.0f;
		//rect_t rect2(mid.x, mid.y, mid.x - x, mid.y - y);

		glBegin(GL_LINES);
			glVertex2f(mid.x, mid.y);
			glVertex2f(mid.x - x, mid.y - y);
		glEnd();
	}
}

// draw_rect: Draws a rectangle
// ------------------------- >>
void draw_rect(rect_t rect, rgba_t col, bool fill)
{
	glDisable(GL_TEXTURE_2D);

	set_gl_colour(col);
	col.set_blend();

	if (fill)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBegin(GL_QUADS);
	glVertex2d(rect.x1(), rect.y1());
	glVertex2d(rect.x2(), rect.y1());
	glVertex2d(rect.x2(), rect.y2());
	glVertex2d(rect.x1(), rect.y2());
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// draw_texture: Draws a texture at x, y, resized to the given width & height
// not resized if width or height = -1
// ----------------------------------------------------------------------- >>
void draw_texture(int x, int y, int width, int height, string texname, int textype, rgba_t col)
{
	Texture* tex = get_texture(texname, textype);

	if (!tex)
		return;

	tex->gen_gl_tex();

	if (width == -1)
		width = tex->width;

	if (height == -1)
		height = tex->height;

	set_gl_colour(col);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, tex->t_y);		glVertex2d(x, y);
		glTexCoord2f(tex->t_x, tex->t_y);	glVertex2d(x + width, y);
		glTexCoord2f(tex->t_x, 0.0f);		glVertex2d(x + width, y + height);
		glTexCoord2f(0.0f, 0.0f);			glVertex2d(x, y + height);
	glEnd();
}

// draw_texture_scale: Draws a texture to fit within a certain area, scaled correctly
// ------------------------------------------------------------------------------- >>
void draw_texture_scale(rect_t rect, string texname, int textype, rgba_t col, float scalef)
{
	Texture* tex = get_texture(texname, textype);

	if (!tex)
		return;

	tex->gen_gl_tex();

	int texwidth = tex->width * scalef;
	int texheight = tex->height * scalef;
	int nwidth = texwidth;
	int nheight = texheight;
	int dim = min(rect.width(), rect.height());
	int midx = rect.x1() + (rect.width() / 2);
	int midy = rect.y1() + (rect.height() / 2);

	if (texwidth > texheight)
	{
		if (texwidth > dim)
		{
			float mult = (float)dim / (float)texwidth;
			nwidth = dim;
			nheight *= mult;
		}
	}
	else if (texwidth < texheight)
	{
		if (texheight > dim)
		{
			float mult = (float)dim / (float)texheight;
			nheight = dim;
			nwidth *= mult;
		}
	}
	else
	{
		if (texwidth > dim)
		{
			nwidth = dim;
			nheight = dim;
		}
	}

	set_gl_colour(col);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);			glVertex2d(midx - (nwidth / 2), midy - (nheight / 2));
		glTexCoord2f(tex->t_x, 0.0f);		glVertex2d(midx + (nwidth / 2), midy - (nheight / 2));
		glTexCoord2f(tex->t_x, tex->t_y);	glVertex2d(midx + (nwidth / 2), midy + (nheight / 2));
		glTexCoord2f(0.0f, tex->t_y);		glVertex2d(midx - (nwidth / 2), midy + (nheight / 2));
	glEnd();
}

// draw_texture_scale: Draws a texture to fit within a certain area, scaled correctly
// ------------------------------------------------------------------------------- >>
void draw_texture_scale(rect_t rect, Texture* tex, rgba_t col, float scalef)
{
	if (!tex)
		return;

	tex->gen_gl_tex();

	int texwidth = tex->width * scalef;
	int texheight = tex->height * scalef;
	int nwidth = texwidth;
	int nheight = texheight;
	int dim = min(rect.width(), rect.height());
	int midx = rect.x1() + (rect.width() / 2);
	int midy = rect.y1() + (rect.height() / 2);

	if (texwidth > texheight)
	{
		if (texwidth > dim)
		{
			float mult = (float)dim / (float)texwidth;
			nwidth = dim;
			nheight *= mult;
		}
	}
	else if (texwidth < texheight)
	{
		if (texheight > dim)
		{
			float mult = (float)dim / (float)texheight;
			nheight = dim;
			nwidth *= mult;
		}
	}
	else
	{
		if (texwidth > dim)
		{
			nwidth = dim;
			nheight = dim;
		}
	}

	set_gl_colour(col);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);			glVertex2d(midx - (nwidth / 2), midy - (nheight / 2));
		glTexCoord2f(tex->t_x, 0.0f);		glVertex2d(midx + (nwidth / 2), midy - (nheight / 2));
		glTexCoord2f(tex->t_x, tex->t_y);	glVertex2d(midx + (nwidth / 2), midy + (nheight / 2));
		glTexCoord2f(0.0f, tex->t_y);		glVertex2d(midx - (nwidth / 2), midy + (nheight / 2));
	glEnd();
}

// draw_text: Draws a string of text at x, y (characters are 8x8)
// ----------------------------------------------------------- >>
void draw_text(int x, int y, rgba_t colour, BYTE alignment, const char* text, ...)
{
	float tx1, tx2, ty1, ty2;
	char string[256];
	va_list ap;

	va_start(ap, text);
	vsprintf(string, text, ap);
	va_end(ap);

	float size_x = 8;
	float size_y = 8;

	// Alignment
	if (alignment == 1) // Center
		x -= (((int)strlen(string) * size_x) / 2);

	if (alignment == 2) // Right
		x -= ((int)strlen(string) * size_x);

	set_gl_colour(colour);

	Texture* tex = get_texture("_font", 4);

	if (!tex)
		return;

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());

	for (DWORD char_num = 0; char_num < strlen(string); char_num++)
	{
		for (int row = 0; row < 16; row++)
		{
			for (int column = 0; column < 16; column++)
			{
				if (string[char_num] == charmap[row][column])
				{
					tx1 = (float)column * 0.0625f;
					tx2 = (float)(column + 1) * 0.0625f;
					ty1 = (float)row * 0.0625f;
					ty2 = (float)(row + 1) * 0.0625f;

					glBegin(GL_QUADS);
						glTexCoord2f(tx1, ty1); glVertex2d((char_num * size_x) + x, y);
						glTexCoord2f(tx2, ty1); glVertex2d((char_num * size_x) + x + size_x, y);
						glTexCoord2f(tx2, ty2); glVertex2d((char_num * size_x) + x + size_x, y + size_y);
						glTexCoord2f(tx1, ty2); glVertex2d((char_num * size_x) + x, y + size_y);
					glEnd();
				}
			}
		}
	}
}

// draw_text_scale: Draws a string of text at x, y, scaled (characters are 8x8 at 1x scale)
// ------------------------------------------------------------------------------------- >>
void draw_text_scale(int x, int y, rgba_t colour, BYTE alignment, int scale, const char* text, ...)
{
	float tx1, tx2, ty1, ty2;
	char string[256];
	va_list ap;

	va_start(ap, text);
	vsprintf(string, text, ap);
	va_end(ap);

	float size_x = 8 * scale;
	float size_y = 8 * scale;

	// Alignment
	if (alignment == 1) // Center
		x -= (((int)strlen(string) * size_x) / 2);

	if (alignment == 2) // Right
		x -= ((int)strlen(string) * size_x);

	set_gl_colour(colour);

	Texture* tex = get_texture("_font", 4);

	if (!tex)
		return;

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());

	for (DWORD char_num = 0; char_num < strlen(string); char_num++)
	{
		for (int row = 0; row < 16; row++)
		{
			for (int column = 0; column < 16; column++)
			{
				if (string[char_num] == charmap[row][column])
				{
					tx1 = (float)column * 0.0625f;
					tx2 = (float)(column + 1) * 0.0625f;
					ty1 = (float)row * 0.0625f;
					ty2 = (float)(row + 1) * 0.0625f;

					glBegin(GL_QUADS);
						glTexCoord2f(tx1, ty1); glVertex2d((char_num * size_x) + x, y);
						glTexCoord2f(tx2, ty1); glVertex2d((char_num * size_x) + x + size_x, y);
						glTexCoord2f(tx2, ty2); glVertex2d((char_num * size_x) + x + size_x, y + size_y);
						glTexCoord2f(tx1, ty2); glVertex2d((char_num * size_x) + x, y + size_y);
					glEnd();
				}
			}
		}
	}
}