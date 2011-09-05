
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
#include "GameConfiguration.h"

LineInfoOverlay::LineInfoOverlay() {
}

LineInfoOverlay::~LineInfoOverlay() {
}

void LineInfoOverlay::update(MapLine* line) {
	if (!line)
		return;

	info.clear();

	// General line info
	info.push_back(S_FMT("Line #%d", line->getIndex()));
	info.push_back(S_FMT("Length: %d", MathStuff::round(line->getLength())));

	// Line special
	int as_id = line->prop("special").getIntValue();
	info.push_back(S_FMT("Special: %d (%s)", as_id, CHR(theGameConfiguration->actionSpecialName(as_id))));

	// Line args (or sector tag)
	info.push_back(S_FMT("Sector Tag: %d", line->prop("arg0").getIntValue()));

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

void LineInfoOverlay::draw(int bottom, int right, float alpha) {
	// Don't bother if invisible
	if (alpha <= 0.0f)
		return;

	// Determine overlay height
	int height = info.size() * 16;

	// Slide in/out animation
	float alpha_inv = 1.0f - alpha;
	bottom += height*alpha_inv*alpha_inv;

	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	col_fg.a = col_fg.a*alpha;

	// Draw overlay background
	rgba_t(col_bg.r, col_bg.g, col_bg.b, 80*alpha, 0).set_gl();
	Drawing::drawFilledRect(0, bottom - height, right, bottom);
	Drawing::drawFilledRect(0, bottom - height+2, right, bottom);
	Drawing::drawFilledRect(0, bottom - height+4, right, bottom);

	// Draw info text lines
	int y = height;
	for (unsigned a = 0; a < info.size(); a++) {
		Drawing::drawText(info[a], 2, bottom - y, col_fg);
		y -= 16;
	}

	// Side info
	int x = right - 256;
	if (side_front.exists) {
		//drawSide(rw, font, alpha, side_front, x);
		drawSide(bottom, right, alpha, side_front, x);
		x -= 256;
	}
	if (side_back.exists)
		//drawSide(rw, font, alpha, side_back, x);
		drawSide(bottom, right, alpha, side_back, x);
}

void LineInfoOverlay::drawSide(int bottom, int right, float alpha, side_t& side, int xstart) {
	// Get colours
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	col_fg.a = col_fg.a*alpha;

	// Index and sector index
	Drawing::drawText(side.info, xstart + 2, bottom - 32, col_fg);

	// Texture offsets
	Drawing::drawText(side.offsets, xstart + 2, bottom - 16, col_fg);

	// Textures
	drawTexture(alpha, xstart + 4, bottom - 32, side.tex_upper);
	drawTexture(alpha, xstart + 88, bottom - 32, side.tex_middle, "M");
	drawTexture(alpha, xstart + 92 + 80, bottom - 32, side.tex_lower, "L");
}

void LineInfoOverlay::drawTexture(float alpha, int x, int y, string texture, string pos) {
	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	col_fg.a = col_fg.a*alpha;

	// Check texture isn't blank
	if (texture != "-") {
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
	Drawing::drawText(texture, x + 40, y - 16, col_fg, Drawing::FONT_SMALL, Drawing::ALIGN_CENTER);
}
