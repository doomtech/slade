
#include "Main.h"
#include "WxStuff.h"
#include "SectorInfoOverlay.h"
#include "MapSector.h"
#include "Drawing.h"
#include "MapEditorWindow.h"
#include "ColourConfiguration.h"

SectorInfoOverlay::SectorInfoOverlay() {
}

SectorInfoOverlay::~SectorInfoOverlay() {
}

void SectorInfoOverlay::update(MapSector* sector) {
	if (!sector)
		return;

	// Info (index + type)
	info = S_FMT("Sector #%d (Type %d)", sector->getIndex(), sector->prop("special").getIntValue());

	// Height
	int fh = sector->prop("heightfloor");
	int ch = sector->prop("heightceiling");
	height = S_FMT("Height: %d to %d (%d total)", fh, ch, ch - fh);

	// Brightness
	light = S_FMT("Brightness: %d", sector->prop("lightlevel").getIntValue());

	// Tag
	tag = S_FMT("Tag: %d", sector->prop("id").getIntValue());

	// Textures
	ftex = sector->floorTexture();
	ctex = sector->ceilingTexture();
}

void SectorInfoOverlay::draw(int bottom, int right, float alpha) {
	// Don't bother if invisible
	if (alpha <= 0.0f)
		return;

	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	col_fg.a = col_fg.a*alpha;

	// Draw overlay background
	rgba_t(col_bg.r, col_bg.g, col_bg.b, 80*alpha, 0).set_gl();
	Drawing::drawFilledRect(0, bottom - 64, right, bottom);
	Drawing::drawFilledRect(0, bottom - 66, right, bottom);
	Drawing::drawFilledRect(0, bottom - 68, right, bottom);

	// Sector index + type
	Drawing::drawText(info, 2, bottom - 64, col_fg);

	// Height
	Drawing::drawText(height, 2, bottom - 48, col_fg);

	// Brightness
	Drawing::drawText(light, 2, bottom - 32, col_fg);

	// Tag
	Drawing::drawText(tag, 2, bottom - 16, col_fg);

	// Ceiling texture
	drawTexture(alpha, right - 88, bottom - 4, ctex, "C");

	// Floor texture
	drawTexture(alpha, right - 88 - 92, bottom - 4, ftex, "F");
}

void SectorInfoOverlay::drawTexture(float alpha, int x, int y, string texture, string pos) {
	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	col_fg.a = col_fg.a*alpha;

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
		GLTexture* tex = theMapEditor->textureManager().getFlat(texture);

		// Draw texture
		if (tex) {
			rgba_t(255, 255, 255, 255*alpha, 0).set_gl();
			Drawing::drawTextureWithin(tex, x, y - 96, x + 80, y - 16, 0, true);
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
