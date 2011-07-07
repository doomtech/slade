
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

void SectorInfoOverlay::draw(sf::RenderWindow* rw, sf::Font& font, float alpha) {
	// Don't bother if invisible
	if (alpha <= 0.0f)
		return;

	int bottom = rw->GetHeight();
	int right = rw->GetWidth();

	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");

	// Draw overlay background
	rgba_t(col_bg.r, col_bg.g, col_bg.b, 80*alpha, 0).set_gl();
	Drawing::drawFilledRect(0, bottom - 64, right, bottom);
	Drawing::drawFilledRect(0, bottom - 66, right, bottom);
	Drawing::drawFilledRect(0, bottom - 68, right, bottom);

	// Setup info string
	sf::String str_sinfo("", font, 12);
	str_sinfo.SetColor(sf::Color(col_fg.r, col_fg.g, col_fg.b, 255*alpha));

	// Sector index + type
	str_sinfo.SetPosition(2, bottom - 64);
	str_sinfo.SetText(CHR(info));
	rw->Draw(str_sinfo);

	// Height
	str_sinfo.SetPosition(2, bottom - 48);
	str_sinfo.SetText(CHR(height));
	rw->Draw(str_sinfo);

	// Brightness
	str_sinfo.SetPosition(2, bottom - 32);
	str_sinfo.SetText(CHR(light));
	rw->Draw(str_sinfo);

	// Tag
	str_sinfo.SetPosition(2, bottom - 16);
	str_sinfo.SetText(CHR(tag));
	rw->Draw(str_sinfo);

	// Ceiling texture
	drawTexture(rw, font, alpha, right - 88, bottom - 4, ctex, "C");

	// Floor texture
	drawTexture(rw, font, alpha, right - 88 - 92, bottom - 4, ftex, "F");
}

void SectorInfoOverlay::drawTexture(sf::RenderWindow* rw, sf::Font& font, float alpha, int x, int y, string texture, string pos) {
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
	str.SetText(CHR(texture));
	diff = 40 - (str.GetRect().GetWidth() * 0.5);
	str.SetPosition(x + diff, y - 16);
	rw->Draw(str);
}
