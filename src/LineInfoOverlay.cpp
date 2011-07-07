
#include "Main.h"
#include "WxStuff.h"
#include "LineInfoOverlay.h"
#include "MapLine.h"
#include "MapSide.h"
#include "MapSector.h"
#include "Drawing.h"
#include "MathStuff.h"
#include "MapEditorWindow.h"
#include "ColourConfiguration.h"

LineInfoOverlay::LineInfoOverlay() {
}

LineInfoOverlay::~LineInfoOverlay() {
}

void LineInfoOverlay::update(MapLine* line) {
	if (!line)
		return;

	// General line info
	index = S_FMT("Line #%d", line->getIndex());
	length = S_FMT("Length: %d", MathStuff::round(line->getLength()));

	// Line special
	special = S_FMT("Special: %d", line->prop("special").getIntValue());

	// Line args (or sector tag)
	args = S_FMT("Sector Tag: %d", line->prop("arg0").getIntValue());

	// Front side
	MapSide* s = line->s1();
	if (s) {
		int xoff = s->prop("offsetx");
		int yoff = s->prop("offsety");
		side_front.exists = true;
		side_front.info = S_FMT("Front Side #%d (Sector %d)", s->getIndex(), s->getSector()->getIndex());
		side_front.offsets = S_FMT("Offsets: (%d, %d)", xoff, yoff);
		side_front.tex_upper = s->prop("texturetop").getStringValue();
		side_front.tex_middle = s->prop("texturemiddle").getStringValue();
		side_front.tex_lower = s->prop("texturebottom").getStringValue();
	}
	else side_front.exists = false;

	// Back side
	s = line->s2();
	if (s) {
		int xoff = s->prop("offsetx");
		int yoff = s->prop("offsety");
		side_back.exists = true;
		side_back.info = S_FMT("Back Side #%d (Sector %d)", s->getIndex(), s->getSector()->getIndex());
		side_back.offsets = S_FMT("Offsets: (%d, %d)", xoff, yoff);
		side_back.tex_upper = s->prop("texturetop").getStringValue();
		side_back.tex_middle = s->prop("texturemiddle").getStringValue();
		side_back.tex_lower = s->prop("texturebottom").getStringValue();
	}
	else side_back.exists = false;
}

void LineInfoOverlay::draw(sf::RenderWindow* rw, sf::Font& font, float alpha) {
	// Don't bother if invisible
	if (alpha <= 0.0f)
		return;

	int bottom = rw->GetHeight();
	int right = rw->GetWidth();

	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");

	// Setup info string
	sf::String str_linfo("", font, 12);
	str_linfo.SetPosition(2, bottom - 64);
	str_linfo.SetColor(sf::Color(col_fg.r, col_fg.g, col_fg.b, 255*alpha));

	// Draw overlay background
	rgba_t(col_bg.r, col_bg.g, col_bg.b, 80*alpha, 0).set_gl();
	Drawing::drawFilledRect(0, bottom - 64, right, bottom);
	Drawing::drawFilledRect(0, bottom - 66, right, bottom);
	Drawing::drawFilledRect(0, bottom - 68, right, bottom);

	// Line index
	str_linfo.SetText(CHR(index));
	rw->Draw(str_linfo);

	// Line length
	str_linfo.SetText(CHR(length));
	str_linfo.SetPosition(2, bottom - 48);
	rw->Draw(str_linfo);

	// Line special
	str_linfo.SetText(CHR(special));
	str_linfo.SetPosition(2, bottom - 32);
	rw->Draw(str_linfo);

	// Line args / sector tag
	str_linfo.SetText(CHR(args));
	str_linfo.SetPosition(2, bottom - 16);
	rw->Draw(str_linfo);

	// Side info
	int x = right - 256;
	if (side_front.exists) {
		drawSide(rw, font, alpha, side_front, x);
		x -= 256;
	}
	if (side_back.exists)
		drawSide(rw, font, alpha, side_back, x);
}

void LineInfoOverlay::drawSide(sf::RenderWindow* rw, sf::Font& font, float alpha, side_t& side, int xstart) {
	int bottom = rw->GetHeight();

	// Get colours
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");

	// Setup string
	sf::String str_sinfo("", font, 12);
	str_sinfo.SetColor(sf::Color(col_fg.r, col_fg.g, col_fg.b, 255*alpha));
	str_sinfo.SetPosition(xstart + 2, bottom - 32);

	// Index and sector index
	str_sinfo.SetText(CHR(side.info));
	rw->Draw(str_sinfo);

	// Texture offsets
	str_sinfo.SetText(CHR(side.offsets));
	str_sinfo.SetPosition(xstart + 2, bottom - 16);
	rw->Draw(str_sinfo);

	// Textures
	drawTexture(rw, font, alpha, xstart + 4, bottom - 32, side.tex_upper);
	drawTexture(rw, font, alpha, xstart + 88, bottom - 32, side.tex_middle, "M");
	drawTexture(rw, font, alpha, xstart + 92 + 80, bottom - 32, side.tex_lower, "L");
}

void LineInfoOverlay::drawTexture(sf::RenderWindow* rw, sf::Font& font, float alpha, int x, int y, string texture, string pos) {
	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");

	// Init string object
	sf::String str(CHR(pos), font, 12);
	str.SetColor(sf::Color(col_fg.r, col_fg.g, col_fg.b, 255*alpha));
	float diff = 40 - (str.GetRect().GetWidth() * 0.5);

	// Check texture isn't blank
	if (!(S_CMPNOCASE(texture, "-"))) {
		// Draw background
		glEnable(GL_TEXTURE_2D);
		rgba_t(255, 255, 255, 255*alpha, 0).set_gl();
		glPushMatrix();
		glTranslated(x, y-96, 0);
		GLTexture::bgTex().draw2dTiled(80, 80);
		glPopMatrix();

		// Get texture
		GLTexture* tex = theMapEditor->textureManager().getTexture(texture);

		// Draw texture
		if (tex) {
			rgba_t(255, 255, 255, 255*alpha, 0).set_gl();
			Drawing::drawTextureWithin(tex, x, y - 96, x + 80, y - 16, 0);
		}

		glDisable(GL_TEXTURE_2D);

		// Draw outline
		rgba_t(col_fg.r, col_fg.g, col_fg.b, 255*alpha, 0).set_gl();
		glLineWidth(1.0f);
		glDisable(GL_LINE_SMOOTH);
		Drawing::drawRect(x, y-96, x+80, y-16);
	}

	// Draw texture name (even if texture is blank)
	texture.Prepend(":");
	texture.Prepend(pos);
	str.SetText(CHR(texture));
	diff = 40 - (str.GetRect().GetWidth() * 0.5);
	str.SetPosition(x + diff, y - 16);
	rw->Draw(str);
}
