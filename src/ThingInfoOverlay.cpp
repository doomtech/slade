
#include "Main.h"
#include "WxStuff.h"
#include "MapThing.h"
#include "ThingInfoOverlay.h"
#include "GameConfiguration.h"
#include "ColourConfiguration.h"
#include "Drawing.h"
#include "MapEditorWindow.h"

ThingInfoOverlay::ThingInfoOverlay() {
}

ThingInfoOverlay::~ThingInfoOverlay() {
}

void ThingInfoOverlay::update(MapThing* thing) {
	if (!thing)
		return;

	info.clear();

	// Index + type
	string type = S_FMT("Type %d (Unknown)", thing->getType());
	ThingType& tt = theGameConfiguration->thingType(thing->getType());
	if (tt.getName() != "Unknown")
		type = tt.getName();
	info.push_back(S_FMT("Thing #%d: %s", thing->getIndex(), CHR(type)));

	// Flags
	info.push_back("Flags: Unimplemented");

	// Set sprite
	sprite = NULL;
	if (!tt.getSprite().IsEmpty())
		sprite = theMapEditor->textureManager().getSprite(tt.getSprite());
}

void ThingInfoOverlay::draw(int bottom, int right, float alpha) {
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

	// Draw sprite
	glEnable(GL_TEXTURE_2D);
	rgba_t(255, 255, 255, 255*alpha, 0).set_gl();
	if (sprite) {
		sprite->bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);	glVertex2d(right - 8 - sprite->getWidth(), bottom - 8 - sprite->getHeight());
		glTexCoord2f(0.0f, 1.0f);	glVertex2d(right - 8 - sprite->getWidth(), bottom - 8);
		glTexCoord2f(1.0f, 1.0f);	glVertex2d(right - 8, bottom - 8);
		glTexCoord2f(1.0f, 0.0f);	glVertex2d(right - 8, bottom - 8 - sprite->getHeight());
		glEnd();
	}
	else {
		GLTexture* tex = theMapEditor->textureManager().getThingImage("unknown");
		if (tex) {
			tex->bind();
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex2d(right - 88, bottom - 88);
			glTexCoord2f(0.0f, 1.0f);	glVertex2d(right - 88, bottom - 8);
			glTexCoord2f(1.0f, 1.0f);	glVertex2d(right - 8, bottom - 8);
			glTexCoord2f(1.0f, 0.0f);	glVertex2d(right - 8, bottom - 88);
			glEnd();
		}
	}
	glDisable(GL_TEXTURE_2D);
}
