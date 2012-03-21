
#include "Main.h"
#include "WxStuff.h"
#include "MapSector.h"
#include "SectorTextureOverlay.h"
#include "Drawing.h"
#include "ColourConfiguration.h"
#include "MapEditorWindow.h"
#include "MapTextureBrowser.h"

SectorTextureOverlay::SectorTextureOverlay() {
	// Init variables
	hover_ceil = false;
	hover_floor = false;
	middlex = 0;
	middley = 0;
	tex_size = 0;
	border = 0;
}

SectorTextureOverlay::~SectorTextureOverlay() {
}

void SectorTextureOverlay::draw(int width, int height, float fade) {
	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	col_bg.a *= fade;
	col_fg.a *= fade;

	col_bg.set_gl();
	Drawing::drawFilledRect(0, 0, width, height);

	// Calculate layout related stuff
	middlex = width * 0.5;
	middley = height * 0.5;
	tex_size = middlex - 64;
	if (tex_size > 256)
		tex_size = 256;
	border = (middlex - tex_size) * 0.5;
	if (border > 48)
		border = 48;
	int cur_size = tex_size;
	if (active) cur_size *= fade;

	// Get textures
	string tex_floor = this->tex_floor;
	string tex_ceiling = this->tex_ceil;
	if (sectors.size() > 0) {
		if (tex_floor.IsEmpty()) tex_floor = sectors[0]->stringProperty("texturefloor");
		if (tex_ceil.IsEmpty()) tex_ceiling = sectors[0]->stringProperty("textureceiling");
	}

	// Floor texture
	drawTexture(fade, middlex - border - tex_size*0.5 - cur_size*0.5, middley - cur_size*0.5, cur_size, tex_floor, hover_floor);
	Drawing::drawText("Floor:", middlex - border - tex_size*0.5, middley - tex_size*0.5 - 16, col_fg, Drawing::FONT_BOLD, Drawing::ALIGN_CENTER);
	Drawing::drawText(tex_floor, middlex - border - tex_size*0.5, middley + tex_size*0.5, col_fg, Drawing::FONT_BOLD, Drawing::ALIGN_CENTER);

	// Ceiling texture
	drawTexture(fade, middlex + border + tex_size*0.5 - cur_size*0.5, middley - cur_size*0.5, cur_size, tex_ceiling, hover_ceil);
	Drawing::drawText("Ceiling:", middlex + border + tex_size*0.5, middley - tex_size*0.5 - 16, col_fg, Drawing::FONT_BOLD, Drawing::ALIGN_CENTER);
	Drawing::drawText(tex_ceiling, middlex + border + tex_size*0.5, middley + tex_size*0.5, col_fg, Drawing::FONT_BOLD, Drawing::ALIGN_CENTER);
}

void SectorTextureOverlay::drawTexture(float alpha, int x, int y, int size, string texture, bool hover) {
	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	rgba_t col_sel = ColourConfiguration::getColour("map_hilight");
	col_fg.a = col_fg.a*alpha;

	// Check texture isn't blank
	if (!(S_CMPNOCASE(texture, "-"))) {
		// Draw background
		glEnable(GL_TEXTURE_2D);
		rgba_t(255, 255, 255, 255*alpha, 0).set_gl();
		glPushMatrix();
		glTranslated(x, y, 0);
		GLTexture::bgTex().draw2dTiled(size, size);
		glPopMatrix();

		// Get texture
		GLTexture* tex = theMapEditor->textureManager().getFlat(texture);

		// Draw texture
		if (tex) {
			rgba_t(255, 255, 255, 255*alpha, 0).set_gl();
			Drawing::drawTextureWithin(tex, x, y, x + size, y + size, 0, true);
		}

		glDisable(GL_TEXTURE_2D);

		// Draw outline
		if (hover) {
			rgba_t(col_sel.r, col_sel.g, col_sel.b, 255*alpha, 0).set_gl();
			glLineWidth(3.0f);
		}
		else {
			rgba_t(col_fg.r, col_fg.g, col_fg.b, 255*alpha, 0).set_gl();
			glLineWidth(1.5f);
		}
		Drawing::drawRect(x, y, x+size, y+size);
	}
}

void SectorTextureOverlay::openSectors(vector<MapSector*>& list) {
	// Clear current sectors list (if any)
	sectors.clear();

	// Add list to sectors
	for (unsigned a = 0; a < list.size(); a++)
		sectors.push_back(list[a]);
}

void SectorTextureOverlay::close(bool cancel) {
	// Deactivate
	active = false;

	// Set textures if not cancelled
	if (!cancel) {
		for (unsigned a = 0; a < sectors.size(); a++) {
			if (!tex_floor.IsEmpty())
				sectors[a]->setStringProperty("texturefloor", tex_floor);
			if (!tex_ceil.IsEmpty())
				sectors[a]->setStringProperty("textureceiling", tex_ceil);
		}
	}
}

void SectorTextureOverlay::mouseMotion(int x, int y) {
	// Check if the mouse is over the floor texture
	if (x >= middlex - border - tex_size &&
		x <= middlex - border &&
		y >= middley - tex_size*0.5 &&
		y <= middley + tex_size*0.5)
		hover_floor = true;
	else
		hover_floor = false;

	// Check if the mouse is over the ceiling texture
	if (x >= middlex + border &&
		x <= middlex + border + tex_size &&
		y >= middley - tex_size*0.5 &&
		y <= middley + tex_size*0.5)
		hover_ceil = true;
	else
		hover_ceil = false;
}

void SectorTextureOverlay::mouseLeftClick() {
	// Do nothing if no sectors open
	if (sectors.size() == 0)
		return;

	// Left clicked on floor texture
	if (hover_floor) {
		// Get initial texture
		string texture = tex_floor;
		if (tex_floor.IsEmpty())
			texture = sectors[0]->stringProperty("texturefloor");

		// Open texture browser
		MapTextureBrowser browser(theMapEditor, 1, texture);
		if (browser.ShowModal() == wxID_OK) {
			// Set texture
			tex_floor = browser.getSelectedItem()->getName();
		}
	}

	// Left clicked on ceiling texture
	else if (hover_ceil) {
		// Get initial texture
		string texture = tex_ceil;
		if (tex_ceil.IsEmpty())
			texture = sectors[0]->stringProperty("textureceiling");

		// Open texture browser
		MapTextureBrowser browser(theMapEditor, 1, texture);
		if (browser.ShowModal() == wxID_OK) {
			// Set texture
			tex_ceil = browser.getSelectedItem()->getName();
		}
	}
}

void SectorTextureOverlay::mouseRightClick() {

}
