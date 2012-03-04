
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Drawing.cpp
 * Description: Various OpenGL drawing functions
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "Drawing.h"
#include "GLTexture.h"
#include "ArchiveManager.h"
#include "Console.h"
#include "MathStuff.h"

#ifdef USE_SFML_RENDERWINDOW
#include <SFML/Graphics.hpp>
#else
#include <FTGL/ftgl.h>
#endif


/*******************************************************************
 * VARIABLES
 *******************************************************************/
namespace Drawing {
#ifdef USE_SFML_RENDERWINDOW
	sf::Font			font_normal;
	sf::Font			font_condensed;
	sf::Font			font_bold;
	sf::Font			font_boldcondensed;
	sf::Font			font_mono;
	sf::RenderWindow*	render_target = NULL;
#else
	FTFont*	font_normal;
	FTFont*	font_condensed;
	FTFont*	font_bold;
	FTFont*	font_boldcondensed;
	FTFont*	font_mono;
#endif
};


/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

#ifdef USE_SFML_RENDERWINDOW
/* Drawing::initFonts
 * Loads all needed fonts for rendering. SFML implementation
 *******************************************************************/
void Drawing::initFonts() {
	// --- Load general fonts ---

	// Normal
	ArchiveEntry* entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans.ttf");
	if (entry) font_normal.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	// Condensed
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_c.ttf");
	if (entry) font_condensed.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	// Bold
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_b.ttf");
	if (entry) font_bold.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	// Condensed Bold
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_cb.ttf");
	if (entry) font_boldcondensed.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	// Monospace
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_mono.ttf");
	if (entry) font_mono.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);
}
#else
/* Drawing::initFonts
 * Loads all needed fonts for rendering. Non-SFML implementation
 *******************************************************************/
void Drawing::initFonts() {
	// --- Load general fonts ---

	// Normal
	ArchiveEntry* entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans.ttf");
	if (entry) {
		font_normal = new FTTextureFont(entry->getData(), entry->getSize());
		font_normal->FaceSize(12);

		// Check it loaded ok
		if (font_normal->Error()) {
			delete font_normal;
			font_normal = NULL;
		}
	}

	// Condensed
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_c.ttf");
	if (entry) {
		font_condensed = new FTTextureFont(entry->getData(), entry->getSize());
		font_condensed->FaceSize(12);

		// Check it loaded ok
		if (font_condensed->Error()) {
			delete font_condensed;
			font_condensed = NULL;
		}
	}

	// Bold
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_b.ttf");
	if (entry) {
		font_bold = new FTTextureFont(entry->getData(), entry->getSize());
		font_bold->FaceSize(12);

		// Check it loaded ok
		if (font_bold->Error()) {
			delete font_bold;
			font_bold = NULL;
		}
	}

	// Condensed bold
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_cb.ttf");
	if (entry) {
		font_boldcondensed = new FTTextureFont(entry->getData(), entry->getSize());
		font_boldcondensed->FaceSize(12);

		// Check it loaded ok
		if (font_boldcondensed->Error()) {
			delete font_boldcondensed;
			font_boldcondensed = NULL;
		}
	}

	// Monospace
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_mono.ttf");
	if (entry) {
		font_mono = new FTTextureFont(entry->getData(), entry->getSize());
		font_mono->FaceSize(12);

		// Check it loaded ok
		if (font_mono->Error()) {
			delete font_mono;
			font_mono = NULL;
		}
	}
}
#endif

/* Drawing::drawLine
 * Draws a line from [start] to [end]
 *******************************************************************/
void Drawing::drawLine(fpoint2_t start, fpoint2_t end) {
	glBegin(GL_LINES);
	glVertex2d(start.x, start.y);
	glVertex2d(end.x, end.y);
	glEnd();
}

/* Drawing::drawLine
 * Draws a line from [x1,y1] to [x2,y2]
 *******************************************************************/
void Drawing::drawLine(double x1, double y1, double x2, double y2) {
	glBegin(GL_LINES);
	glVertex2d(x1, y1);
	glVertex2d(x2, y2);
	glEnd();
}

/* Drawing::drawRect
 * Draws a rectangle from [tl] to [br]
 *******************************************************************/
void Drawing::drawRect(fpoint2_t tl, fpoint2_t br) {
	glBegin(GL_LINE_LOOP);
	glVertex2d(tl.x, tl.y);
	glVertex2d(tl.x, br.y);
	glVertex2d(br.x, br.y);
	glVertex2d(br.x, tl.y);
	glEnd();
}

/* Drawing::drawRect
 * Draws a rectangle from [x1,y1] to [x2,y2]
 *******************************************************************/
void Drawing::drawRect(double x1, double y1, double x2, double y2) {
	glBegin(GL_LINE_LOOP);
	glVertex2d(x1, y1);
	glVertex2d(x1, y2);
	glVertex2d(x2, y2);
	glVertex2d(x2, y1);
	glEnd();
}

/* Drawing::drawFilledRect
 * Draws a filled rectangle from [tl] to [br]
 *******************************************************************/
void Drawing::drawFilledRect(fpoint2_t tl, fpoint2_t br) {
	glBegin(GL_QUADS);
	glVertex2d(tl.x, tl.y);
	glVertex2d(tl.x, br.y);
	glVertex2d(br.x, br.y);
	glVertex2d(br.x, tl.y);
	glEnd();
}

/* Drawing::drawFilledRect
 * Draws a filled rectangle from [x1,y1] to [x2,y2]
 *******************************************************************/
void Drawing::drawFilledRect(double x1, double y1, double x2, double y2) {
	glBegin(GL_QUADS);
	glVertex2d(x1, y1);
	glVertex2d(x1, y2);
	glVertex2d(x2, y2);
	glVertex2d(x2, y1);
	glEnd();
}

/* Drawing::drawTextureWithin
 * Draws [tex] within the rectangle from [x1,y1] to [x2,y2], centered
 * and keeping the correct aspect ratio. If [upscale] is true the
 * texture will be zoomed to fit the rectangle
 *******************************************************************/
void Drawing::drawTextureWithin(GLTexture* tex, double x1, double y1, double x2, double y2, double padding, bool upscale) {
	// Ignore null texture
	if (!tex)
		return;

	double width = x2 - x1;
	double height = y2 - y1;

	// Get image dimensions
	double x_dim = (double)tex->getWidth();
	double y_dim = (double)tex->getHeight();

	// Get max scale for x and y (including padding)
	double x_scale = ((double)width - padding) / x_dim;
	double y_scale = ((double)width - padding) / y_dim;

	// Set scale to smallest of the 2 (so that none of the texture will be clipped)
	double scale = MIN(x_scale, y_scale);

	// If we don't want to magnify the image, clamp scale to a max of 1.0
	if (!upscale && scale > 1)
		scale = 1;

	// Now draw the texture
	glPushMatrix();
	glTranslated(x1 + width*0.5, y1 + height*0.5, 0);	// Translate to middle of area
	glScaled(scale, scale, scale);						// Scale to fit within area
	tex->draw2d(tex->getWidth()*-0.5, tex->getHeight()*-0.5);
	glPopMatrix();
}

#ifdef USE_SFML_RENDERWINDOW
/* Drawing::drawText
 * Draws [text] at [x,y]. If [bounds] is not null, the bounding
 * coordinates of the rendered text string are written to it.
 * *SFML implementation*
 *******************************************************************/
void Drawing::drawText(string text, int x, int y, rgba_t colour, int font, int alignment, frect_t* bounds) {
	// Setup SFML string
	sf::String sf_str(CHR(text));
	sf_str.SetPosition(x, y);
	sf_str.SetColor(sf::Color(colour.r, colour.g, colour.b, colour.a));

	// Set font
	switch (font) {
	case FONT_NORMAL:			sf_str.SetFont(font_normal); break;
	case FONT_CONDENSED:		sf_str.SetFont(font_condensed); break;
	case FONT_BOLD:				sf_str.SetFont(font_bold); break;
	case FONT_BOLDCONDENSED:	sf_str.SetFont(font_boldcondensed); break;
	case FONT_MONOSPACE:		sf_str.SetFont(font_mono); break;
	default:					sf_str.SetFont(font_normal); break;
	};
	sf_str.SetSize(sf_str.GetFont().GetCharacterSize());

	// Setup alignment
	if (alignment != ALIGN_LEFT) {
		float width = sf_str.GetRect().GetWidth();

		if (alignment == ALIGN_CENTER)
			sf_str.Move(-MathStuff::round(width*0.5), 0.0f);
		else
			sf_str.Move(-width, 0.0f);
	}

	// Set bounds rect
	if (bounds) {
		sf::FloatRect rect = sf_str.GetRect();
		bounds->set(rect.Left, rect.Top, rect.Right, rect.Bottom);
	}

	// Draw the string
	if (render_target)
		render_target->Draw(sf_str);
}
#else
/* Drawing::drawText
 * Draws [text] at [x,y]. If [bounds] is not null, the bounding
 * coordinates of the rendered text string are written to it.
 * *non-SFML implementation*
 *******************************************************************/
void Drawing::drawText(string text, int x, int y, rgba_t colour, int font, int alignment, frect_t* bounds) {
	// Get desired font
	FTFont* ftgl_font;
	// Set font
	switch (font) {
	case FONT_NORMAL:			ftgl_font = font_normal; break;
	case FONT_CONDENSED:		ftgl_font = font_condensed; break;
	case FONT_BOLD:				ftgl_font = font_bold; break;
	case FONT_BOLDCONDENSED:	ftgl_font = font_boldcondensed; break;
	case FONT_MONOSPACE:		ftgl_font = font_mono; break;
	default:					ftgl_font = font_normal; break;
	};

	// If FTGL font is invalid, do nothing
	if (!ftgl_font)
		return;

	// Setup alignment
	FTBBox bbox = ftgl_font->BBox(CHR(text), -1);
	float xpos = x;
	float ypos = y;
	float width = bbox.Upper().X() - bbox.Lower().X();
	float height = bbox.Upper().Y() - bbox.Lower().Y();
	if (alignment != ALIGN_LEFT) {
		if (alignment == ALIGN_CENTER)
			xpos -= MathStuff::round(width*0.5);
		else
			xpos -= width;
	}

	// Set bounds rect
	if (bounds) {
		bbox = ftgl_font->BBox(CHR(text), -1, FTPoint(xpos, ypos));
		bounds->set(bbox.Lower().X(), bbox.Lower().Y(), bbox.Upper().X(), bbox.Upper().Y());
	}

	// Draw the string
	colour.set_gl();
	glPushMatrix();
	glTranslatef(xpos, ypos + ftgl_font->FaceSize(), 0.0f);
	glScalef(1.0f, -1.0f, 1.0f);
	ftgl_font->Render(CHR(text), -1);
	glPopMatrix();
}
#endif

/* Drawing::drawHud
 * Draws doom hud offset guide lines, from the center
 *******************************************************************/
void Drawing::drawHud(bool statusbar, bool center, bool wide) {
	// Determine some variables
	int hw = 160;
	int hh = 100;
	if (wide) hw = 176;

	// Draw 320x200 screen outline
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glLineWidth(1.5f);
	drawRect(-hw, -hh, hw, hh);

	// Draw statusbar line if needed
	glLineWidth(1.0f);
	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	if (statusbar)
		drawLine(-hw, 68, hw, 68);

	// Draw center lines if needed
	if (center) {
		drawLine(-hw, 0, hw, 0);
		drawLine(0, -hh, 0, hh);
	}

	// Draw normal screen edge guides if widescreen
	if (wide) {
		drawLine(-160, -100, -160, 100);
		drawLine(160, -100, 160, 100);
	}
}

#ifdef USE_SFML_RENDERWINDOW
void Drawing::setRenderTarget(sf::RenderWindow* target) {
	render_target = target;
}
#endif



CONSOLE_COMMAND(d_testfont, 1) {
	ArchiveEntry* entry = theArchiveManager->programResourceArchive()->entryAtPath(S_FMT("fonts/%s.ttf", CHR(args[0])));
	if (entry) {
		if (Drawing::font_condensed) {
			delete Drawing::font_condensed;
			Drawing::font_condensed = NULL;
		}

		long size = 12;
		if (args.size() > 1)
			args[1].ToLong(&size);

		Drawing::font_condensed = new FTTextureFont(entry->getData(), entry->getSize());
		Drawing::font_condensed->FaceSize(size);
	}
}
