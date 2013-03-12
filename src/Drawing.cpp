
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
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
#include "Misc.h"
#include <wx/settings.h>

#ifdef USE_SFML_RENDERWINDOW
#include <SFML/Graphics.hpp>
#else
#include <FTGL/ftgl.h>
#endif

#ifdef __WXGTK20__
#define GSocket GlibGSocket
#include <gtk-2.0/gtk/gtk.h>
#undef GSocket
#endif


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, hud_statusbar, 1, CVAR_SAVE)
CVAR(Bool, hud_center, 1, CVAR_SAVE)
CVAR(Bool, hud_wide, 0, CVAR_SAVE)
CVAR(Bool, hud_bob, 0, CVAR_SAVE)

#ifdef USE_SFML_RENDERWINDOW
namespace Drawing {
	sf::RenderWindow*	render_target = NULL;
};
#endif


class FontManager {
private:
#ifdef USE_SFML_RENDERWINDOW
	sf::Font	font_normal;
	sf::Font	font_condensed;
	sf::Font	font_bold;
	sf::Font	font_boldcondensed;
	sf::Font	font_mono;
	sf::Font	font_small;
#else
	FTFont*		font_normal;
	FTFont*		font_condensed;
	FTFont*		font_bold;
	FTFont*		font_boldcondensed;
	FTFont*		font_mono;
	FTFont*		font_small;
#endif
	static FontManager*	instance;

public:
	FontManager() {}
	~FontManager() {}

	static FontManager*	getInstance() {
		if (!instance)
			instance = new FontManager();

		return instance;
	}
	int initFonts();

#ifdef USE_SFML_RENDERWINDOW
	sf::Font*	getFont(int font);
#else
	FTFont*		getFont(int font);
#endif

};
#define theFontManager FontManager::getInstance()
FontManager* FontManager::instance = NULL;

/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

#ifdef USE_SFML_RENDERWINDOW

#if SFML_VERSION_MAJOR < 2
/* FontManager::initFonts
 * Loads all needed fonts for rendering. SFML 1.x implementation
 *******************************************************************/
int FontManager::initFonts() {
	// --- Load general fonts ---
	int ret = 0;

	// Normal
	ArchiveEntry* entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans.ttf");
	if (entry) ++ret, font_normal.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	// Condensed
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_c.ttf");
	if (entry) ++ret, font_condensed.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	// Bold
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_b.ttf");
	if (entry) ++ret, font_bold.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	// Condensed Bold
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_cb.ttf");
	if (entry) ++ret, font_boldcondensed.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	// Monospace
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_mono.ttf");
	if (entry) ++ret, font_mono.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 12);

	// Small
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans.ttf");
	if (entry) ++ret, font_small.LoadFromMemory((const char*)entry->getData(), entry->getSize(), 8);

	return ret;
}
#else
/* FontManager::initFonts
 * Loads all needed fonts for rendering. SFML 2.x implementation
 *******************************************************************/
int FontManager::initFonts() {
	// --- Load general fonts ---
	int ret = 0;

	// Normal
	ArchiveEntry* entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans.ttf");
	if (entry) ++ret, font_normal.loadFromMemory((const char*)entry->getData(), entry->getSize());

	// Condensed
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_c.ttf");
	if (entry) ++ret, font_condensed.loadFromMemory((const char*)entry->getData(), entry->getSize());

	// Bold
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_b.ttf");
	if (entry) ++ret, font_bold.loadFromMemory((const char*)entry->getData(), entry->getSize());

	// Condensed Bold
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans_cb.ttf");
	if (entry) ++ret, font_boldcondensed.loadFromMemory((const char*)entry->getData(), entry->getSize());

	// Monospace
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_mono.ttf");
	if (entry) ++ret, font_small.loadFromMemory((const char*)entry->getData(), entry->getSize());

	return ret;
}
#endif//SFML_VERSION_MAJOR

#else
/* FontManager::initFonts
 * Loads all needed fonts for rendering. Non-SFML implementation
 *******************************************************************/
int FontManager::initFonts() {
	// --- Load general fonts ---
	int ret = 0;

	// Normal
	ArchiveEntry* entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans.ttf");
	if (entry) {
		font_normal = new FTTextureFont(entry->getData(), entry->getSize());
		font_normal->FaceSize(12);

		// Check it loaded ok
		if (font_normal->Error()) {
			delete font_normal;
			font_normal = NULL;
		} else ++ ret;
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
		} else ++ ret;
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
		} else ++ ret;
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
		} else ++ ret;
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
		} else ++ ret;
	}

	// Small
	entry = theArchiveManager->programResourceArchive()->entryAtPath("fonts/dejavu_sans.ttf");
	if (entry) {
		font_small = new FTTextureFont(entry->getData(), entry->getSize());
		font_small->FaceSize(8);

		// Check it loaded ok
		if (font_small->Error()) {
			delete font_small;
			font_small = NULL;
		} else ++ ret;
	}

	return ret;
}
#endif

/* FontManager::getFont
 * Returns a font
 *******************************************************************/
#ifdef USE_SFML_RENDERWINDOW
sf::Font* FontManager::getFont(int font) {
	switch (font) {
	case Drawing::FONT_NORMAL:			return &font_normal;
	case Drawing::FONT_CONDENSED:		return &font_condensed;
	case Drawing::FONT_BOLD:			return &font_bold;
	case Drawing::FONT_BOLDCONDENSED:	return &font_boldcondensed;
	case Drawing::FONT_MONOSPACE:		return &font_mono;
	case Drawing::FONT_SMALL:			return &font_small;
	default:							return &font_normal;
	};
	return NULL;
}
#else // USE_SFML_RENDERWINDOW
FTFont* FontManager::getFont(int font) {
	switch (font) {
	case Drawing::FONT_NORMAL:			return font_normal;
	case Drawing::FONT_CONDENSED:		return font_condensed;
	case Drawing::FONT_BOLD:			return font_bold;
	case Drawing::FONT_BOLDCONDENSED:	return font_boldcondensed;
	case Drawing::FONT_MONOSPACE:		return font_mono;
	case Drawing::FONT_SMALL:			return font_small;
	default:							return font_normal;
	};
	return NULL;
}
#endif // USE_SFML_RENDERWINDOW


/* Drawing::initFonts
 * Creates a FontManager if needed and let it init its own fonts
 *******************************************************************/
void Drawing::initFonts() {
	theFontManager->initFonts();
}

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

/* Drawing::drawLineTabbed
 * Draws a line from [start] to [end]
 *******************************************************************/
void Drawing::drawLineTabbed(fpoint2_t start, fpoint2_t end, double tab, double tab_max) {
	// Draw line
	glBegin(GL_LINES);
	glVertex2d(start.x, start.y);
	glVertex2d(end.x, end.y);
	glEnd();

	// Calculate midpoint
	fpoint2_t mid;
	mid.x = start.x + ((end.x - start.x) * 0.5);
	mid.y = start.y + ((end.y - start.y) * 0.5);

	// Calculate tab length
	double tablen = MathStuff::distance(start.x, start.y, end.x, end.y) * tab;
	if (tablen > tab_max) tablen = tab_max;
	if (tablen < 2) tablen = 2;

	// Calculate tab endpoint
	fpoint2_t invdir(-(end.y - start.y), end.x - start.x);
	invdir.normalize();

	// Draw tab
	glBegin(GL_LINES);
	glVertex2d(mid.x, mid.y);
	glVertex2d(mid.x - invdir.x*tablen, mid.y - invdir.y*tablen);
	glEnd();
}

/* Drawing::drawLineTabbed
 * Draws a line from [x1,y1] to [x2,y2]
 *******************************************************************/
void Drawing::drawLineTabbed(double x1, double y1, double x2, double y2, double tab, double tab_max) {
	// Draw line
	glBegin(GL_LINES);
	glVertex2d(x1, y1);
	glVertex2d(x2, y2);
	glEnd();

	// Calculate midpoint
	fpoint2_t mid;
	mid.x = x1 + ((x2 - x1) * 0.5);
	mid.y = y1 + ((y1 - y1) * 0.5);

	// Calculate tab length
	double tablen = MathStuff::distance(x1, y1, x2, y2) * tab;
	if (tablen > tab_max) tablen = tab_max;
	if (tablen < 2) tablen = 2;

	// Calculate tab endpoint
	fpoint2_t invdir(-(y2 - y1), x2 - x1);
	invdir.normalize();

	// Draw tab
	glBegin(GL_LINES);
	glVertex2d(mid.x, mid.y);
	glVertex2d(mid.x - invdir.x*tablen, mid.y - invdir.y*tablen);
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

void Drawing::drawBorderedRect(fpoint2_t tl, fpoint2_t br, rgba_t colour, rgba_t border_colour) {
	drawBorderedRect(tl.x, tl.y, br.x, br.y, colour, border_colour);
}

void Drawing::drawBorderedRect(double x1, double y1, double x2, double y2, rgba_t colour, rgba_t border_colour) {
	// Rect
	colour.set_gl(false);
	glBegin(GL_QUADS);
	glVertex2d(x1, y1);
	glVertex2d(x1, y2);
	glVertex2d(x2, y2);
	glVertex2d(x2, y1);
	glEnd();

	// Border
	border_colour.set_gl(false);
	glBegin(GL_LINE_LOOP);
	glVertex2d(x1, y1);
	glVertex2d(x1, y2-1);
	glVertex2d(x2-1, y2-1);
	glVertex2d(x2-1, y1);
	glEnd();
}

/* Drawing::fitTextureWithin
 * Fits [tex] within the rectangle from [x1,y1] to [x2,y2], centered
 * and keeping the correct aspect ratio. If [upscale] is true the
 * texture will be zoomed to fit the rectangle. Returns the resulting
 * texture rectangle coordinates
 *******************************************************************/
frect_t Drawing::fitTextureWithin(GLTexture* tex, double x1, double y1, double x2, double y2, double padding, double max_scale) {
	// Ignore null texture
	if (!tex)
		return frect_t();

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

	// Clamp scale to maximum desired scale
	if (scale > max_scale)
		scale = max_scale;

	// Return the fitted rectangle
	return frect_t(x1 + width*0.5 - (scale*tex->getWidth()*0.5),
					y1 + height*0.5 - (scale*tex->getHeight()*0.5),
					x1 + width*0.5 + (scale*tex->getWidth()*0.5),
					y1 + height*0.5 + (scale*tex->getHeight()*0.5));
}

/* Drawing::drawTextureWithin
 * Draws [tex] within the rectangle from [x1,y1] to [x2,y2], centered
 * and keeping the correct aspect ratio. If [upscale] is true the
 * texture will be zoomed to fit the rectangle
 *******************************************************************/
void Drawing::drawTextureWithin(GLTexture* tex, double x1, double y1, double x2, double y2, double padding, double max_scale) {
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

	// Clamp scale to maximum desired scale
	if (scale > max_scale)
		scale = max_scale;

	// Now draw the texture
	glPushMatrix();
	glTranslated(x1 + width*0.5, y1 + height*0.5, 0);	// Translate to middle of area
	glScaled(scale, scale, scale);						// Scale to fit within area
	tex->draw2d(tex->getWidth()*-0.5, tex->getHeight()*-0.5);
	glPopMatrix();
}

#ifdef USE_SFML_RENDERWINDOW
#if SFML_VERSION_MAJOR < 2
/*******************************************************************
 * SFML 1.x TEXT FUNCTION IMPLEMENTATIONS
 *******************************************************************/

/* Drawing::drawText
 * Draws [text] at [x,y]. If [bounds] is not null, the bounding
 * coordinates of the rendered text string are written to it.
 *******************************************************************/
void Drawing::drawText(string text, int x, int y, rgba_t colour, int font, int alignment, frect_t* bounds) {
	// Setup SFML string
	sf::String sf_str(CHR(text));
	sf_str.SetPosition(x, y);
	sf_str.SetColor(sf::Color(colour.r, colour.g, colour.b, colour.a));

	// Set font
	sf::Font * f = theFontManager->getFont(font);
	sf_str.SetFont(*f);
	sf_str.SetSize(f->GetCharacterSize());

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

/* Drawing::textExtents
 * Returns the width and height of [text] when drawn with [font]
 *******************************************************************/
fpoint2_t Drawing::textExtents(string text, int font) {
	// Setup SFML string
	sf::String sf_str(CHR(text));

	// Set font
	sf::Font * f = theFontManager->getFont(font);
	sf_str.SetFont(*f);
	sf_str.SetSize(f->GetCharacterSize());

	// Return width and height of text
	sf::FloatRect rect = sf_str.GetRect();
	return fpoint2_t(rect.GetWidth(), rect.GetHeight());
}

#else
/*******************************************************************
 * SFML 2.x TEXT FUNCTION IMPLEMENTATIONS
 *******************************************************************/

/* Drawing::drawText
 * Draws [text] at [x,y]. If [bounds] is not null, the bounding
 * coordinates of the rendered text string are written to it.
 *******************************************************************/
void Drawing::drawText(string text, int x, int y, rgba_t colour, int font, int alignment, frect_t* bounds) {
	// Setup SFML string
	sf::Text sf_str;
	sf_str.setString(CHR(text));
	sf_str.setPosition(x, y);
	sf_str.setColor(sf::Color(colour.r, colour.g, colour.b, colour.a));

	// Set font
	sf::Font * f = theFontManager->getFont(font);
	sf_str.setFont(*f);
	if (font == FONT_SMALL)
		sf_str.setCharacterSize(8);
	else
		sf_str.setCharacterSize(12);

	// Setup alignment
	if (alignment != ALIGN_LEFT) {
		float width = sf_str.getLocalBounds().width;

		if (alignment == ALIGN_CENTER)
			sf_str.move(-MathStuff::round(width*0.5), 0.0f);
		else
			sf_str.move(-width, 0.0f);
	}

	// Set bounds rect
	if (bounds) {
		sf::FloatRect rect = sf_str.getGlobalBounds();
		bounds->set(rect.left, rect.top, rect.left+rect.width, rect.top+rect.height);
	}

	// Draw the string
	if (render_target) {
		// Push related states
		glPushMatrix();
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		render_target->resetGLStates();

		// Draw
		render_target->draw(sf_str);

		// Pop related states
		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}

/* Drawing::textExtents
 * Returns the width and height of [text] when drawn with [font]
 *******************************************************************/
fpoint2_t Drawing::textExtents(string text, int font) {
	// Setup SFML string
	sf::Text sf_str;
	sf_str.setString(CHR(text));

	// Set font
	sf::Font * f = theFontManager->getFont(font);
	sf_str.setFont(*f);
	if (font == FONT_SMALL)
		sf_str.setCharacterSize(8);
	else
		sf_str.setCharacterSize(12);

	// Return width and height of text
	sf::FloatRect rect = sf_str.getGlobalBounds();
	return fpoint2_t(rect.width, rect.height);
}
#endif//SFML_VERSION_MAJOR

#else
/*******************************************************************
 * FTGL TEXT FUNCTION IMPLEMENTATIONS
 *******************************************************************/

/* Drawing::drawText
 * Draws [text] at [x,y]. If [bounds] is not null, the bounding
 * coordinates of the rendered text string are written to it.
 *******************************************************************/
void Drawing::drawText(string text, int x, int y, rgba_t colour, int font, int alignment, frect_t* bounds) {
	// Get desired font
	FTFont* ftgl_font = theFontManager->getFont(font);

	// If FTGL font is invalid, do nothing
	if (!ftgl_font)
		return;

	// Setup alignment
	FTBBox bbox = ftgl_font->BBox(CHR(text), -1);
	int xpos = x;
	int ypos = y;
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
	glTranslatef(-0.375f, -0.375f, 0);
	glScalef(1.0f, -1.0f, 1.0f);
	ftgl_font->Render(CHR(text), -1);
	glPopMatrix();
}

/* Drawing::textExtents
 * Returns the width and height of [text] when drawn with [font]
 *******************************************************************/
fpoint2_t Drawing::textExtents(string text, int font) {
	// Get desired font
	FTFont* ftgl_font = theFontManager->getFont(font);

	// If FTGL font is invalid, return empty
	if (!ftgl_font)
		return fpoint2_t(0,0);

	// Return width and height of text
	FTBBox bbox = ftgl_font->BBox(CHR(text), -1);
	return fpoint2_t(bbox.Upper().X() - bbox.Lower().X(), bbox.Upper().Y() - bbox.Lower().Y());
}

#endif

/* Drawing::drawHud
 * Draws doom hud offset guide lines, from the center
 *******************************************************************/
void Drawing::drawHud() {
	// Determine some variables
	int hw = 160;
	int hh = 100;
	if (hud_wide) hw = 177;

	// Draw 320x200 screen outline
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glLineWidth(1.5f);
	drawRect(-hw, -hh, hw, hh);

	// Draw statusbar line if needed
	glLineWidth(1.0f);
	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	if (hud_statusbar) {
		drawLine(-hw, 68, hw, 68);	// Doom's status bar: 32 pixels tall
		drawLine(-hw, 62, hw, 62);	// Hexen: 38 pixels
		drawLine(-hw, 58, hw, 58);	// Heretic: 42 pixels
	}

	// Draw center lines if needed
	if (hud_center) {
		drawLine(-hw, 0, hw, 0);
		drawLine(0, -hh, 0, hh);
	}

	// Draw normal screen edge guides if widescreen
	if (hud_wide) {
		drawLine(-160, -100, -160, 100);
		drawLine(160, -100, 160, 100);
	}

	// Draw weapon bobbing guides
	if (hud_bob) {
		glLineWidth(0.8f);
		drawRect(-hw - 16, -hh - 16, hw + 16, hh + 16);
	}
}

#ifdef USE_SFML_RENDERWINDOW
void Drawing::setRenderTarget(sf::RenderWindow* target) {
	render_target = target;
}
#endif


// The following functions are taken from CodeLite (http://codelite.org)

wxColour Drawing::getPanelBGColour() {
#ifdef __WXGTK__
	static bool     intitialized(false);
	static wxColour bgColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));

	if( !intitialized ) {
		// try to get the background colour from a menu
		GtkWidget *menu = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		GtkStyle   *def = gtk_rc_get_style( menu );
		if(!def)
			def = gtk_widget_get_default_style();

		if(def) {
			GdkColor col = def->bg[GTK_STATE_NORMAL];
			bgColour = wxColour(col);
		}
		gtk_widget_destroy( menu );
		intitialized = true;
	}
	return bgColour;
#else
	return wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
#endif
}

wxColour Drawing::getMenuTextColour() {
/*#ifdef __WXGTK__
	static bool     intitialized(false);
	static wxColour textColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT));

	if( !intitialized ) {
		// try to get the text colour from a menu
		GtkWidget *menuBar = gtk_menu_new();
		GtkStyle   *def = gtk_rc_get_style( menuBar );
		if(!def)
			def = gtk_widget_get_default_style();

		if(def) {
			GdkColor col = def->text[GTK_STATE_NORMAL];
			textColour = wxColour(col);
		}
		gtk_widget_destroy( menuBar );
		intitialized = true;
	}
	return textColour;
#else*/
	return wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT);
//#endif
}

wxColour Drawing::getMenuBarBGColour() {
/*
#ifdef __WXGTK__
	static bool     intitialized(false);
	static wxColour textColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR));

	if( !intitialized ) {
		// try to get the background colour from a menu
		GtkWidget *menuBar = gtk_menu_bar_new();
		GtkStyle   *def = gtk_rc_get_style( menuBar );
		if(!def)
			def = gtk_widget_get_default_style();

		if(def) {
			GdkColor col = def->bg[GTK_STATE_NORMAL];
			textColour = wxColour(col);
		}
		gtk_widget_destroy( menuBar );
		intitialized = true;
	}
	return textColour;
#else*/
	return wxSystemSettings::GetColour(wxSYS_COLOUR_MENU);
//#endif
}

wxColour Drawing::lightColour(const wxColour& colour, float percent) {
	if(percent == 0) {
		return colour;
	}

	// Convert to HSL
	hsl_t hsl = Misc::rgbToHsl(rgba_t(colour.Red(), colour.Green(), colour.Blue()));

	// Increase luminance
	hsl.l += (float)((percent * 5.0)/100.0);
	if (hsl.l > 1.0) hsl.l = 1.0;

	rgba_t rgb = Misc::hslToRgb(hsl);
	return wxColour(rgb.r, rgb.g, rgb.b);
}

wxColour Drawing::darkColour(const wxColour& colour, float percent) {
	if(percent == 0) {
		return colour;
	}

	// Convert to HSL
	hsl_t hsl = Misc::rgbToHsl(rgba_t(colour.Red(), colour.Green(), colour.Blue()));

	// Decrease luminance
	hsl.l -= (float)((percent * 5.0)/100.0);
	if (hsl.l < 0) hsl.l = 0;

	rgba_t rgb = Misc::hslToRgb(hsl);
	return wxColour(rgb.r, rgb.g, rgb.b);
}


/*
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
*/
