
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
	sprite = "";
	translation = "";
	palette = "";
	icon = "";

	// Index + type
	ThingType* tt = theGameConfiguration->thingType(thing->getType());
	string type = S_FMT("%s (Type %d)", CHR(tt->getName()), thing->getType());
	info.push_back(S_FMT("Thing #%d: %s", thing->getIndex(), CHR(type)));

	// Position
	if (theGameConfiguration->getMapFormat() != MAP_DOOM)
		info.push_back(S_FMT("Position: %d, %d, %d", (int)thing->xPos(), (int)thing->yPos(), (int)(thing->floatProperty("height"))));
	else info.push_back(S_FMT("Position: %d, %d", (int)thing->xPos(), (int)thing->yPos()));

	// Direction
	int angle = thing->intProperty("angle");
	string dir = S_FMT("%d degrees", angle);
	if (angle == 0)
		dir = "East";
	else if (angle == 45)
		dir = "Northeast";
	else if (angle == 90)
		dir = "North";
	else if (angle == 135)
		dir = "Northwest";
	else if (angle == 180)
		dir = "West";
	else if (angle == 225)
		dir = "Southwest";
	else if (angle == 270)
		dir = "South";
	else if (angle == 315)
		dir = "Southeast";
	info.push_back(S_FMT("Direction: %s", CHR(dir)));

	// Special and Args (if in hexen format or udmf with thing args)
	if (theGameConfiguration->getMapFormat() == MAP_HEXEN ||
		(theGameConfiguration->getMapFormat() == MAP_UDMF && theGameConfiguration->getUDMFProperty("arg0", MOBJ_THING))) {
		int as_id = thing->intProperty("special");
		info.push_back(S_FMT("Special: %d (%s)", as_id, CHR(theGameConfiguration->actionSpecialName(as_id))));
		int args[5];
		args[0] = thing->intProperty("arg0");
		args[1] = thing->intProperty("arg1");
		args[2] = thing->intProperty("arg2");
		args[3] = thing->intProperty("arg3");
		args[4] = thing->intProperty("arg4");
		string argstr = tt->getArgsString(args);
		if (!argstr.IsEmpty())
			info.push_back(S_FMT("%s", CHR(argstr)));
		else
			info.push_back("No Args");
	}

	// Flags
	if (theGameConfiguration->getMapFormat() != MAP_UDMF)
		info.push_back(S_FMT("Flags: %s", CHR(theGameConfiguration->thingFlagsString(thing->intProperty("flags")))));

	// TID (if in doom64/hexen/udmf format)
	if (theGameConfiguration->getMapFormat() != MAP_DOOM) {
		info.push_back(S_FMT("TID: %i", thing->intProperty("id")));
	}

	// Set sprite and translation
	sprite = tt->getSprite();
	translation = tt->getTranslation();
	palette = tt->getPalette();
	icon = tt->getIcon();
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(col_bg.fr(), col_bg.fg(), col_bg.fb(), col_bg.fa()*alpha);
	Drawing::drawFilledRect(0, bottom - height, right, bottom);
	glBegin(GL_QUADS);
	glVertex2d(0, bottom - height);
	glVertex2d(right, bottom - height);
	glColor4f(col_bg.fr(), col_bg.fg(), col_bg.fb(), 0.0f);
	glVertex2d(right, bottom - height - 16);
	glVertex2d(0, bottom - height - 16);
	glEnd();

	// Draw info text lines
	int y = height;
	for (unsigned a = 0; a < info.size(); a++) {
		Drawing::drawText(info[a], 2, bottom - y, col_fg, Drawing::FONT_CONDENSED);
		y -= 16;
	}

	// Draw sprite
	bool isicon = false;
	GLTexture* tex = theMapEditor->textureManager().getSprite(sprite, translation, palette);
	if (!tex) {
		tex = theMapEditor->textureManager().getEditorImage(S_FMT("thing/%s", CHR(icon)));
		isicon = true;
	}
	glEnable(GL_TEXTURE_2D);
	rgba_t(255, 255, 255, 255*alpha, 0).set_gl();
	if (tex) {
		double width = tex->getWidth();
		double height = tex->getHeight();
		if (isicon) {
			width = 64;
			height = 64;
		}
		tex->bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);	glVertex2d(right - 8 - width, bottom - 8 - height);
		glTexCoord2f(0.0f, 1.0f);	glVertex2d(right - 8 - width, bottom - 8);
		glTexCoord2f(1.0f, 1.0f);	glVertex2d(right - 8, bottom - 8);
		glTexCoord2f(1.0f, 0.0f);	glVertex2d(right - 8, bottom - 8 - height);
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
}
