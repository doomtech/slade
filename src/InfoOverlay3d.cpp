
#include "Main.h"
#include "WxStuff.h"
#include "MapEditorWindow.h"
#include "MapEditor.h"
#include "InfoOverlay3d.h"
#include "SLADEMap.h"
#include "ColourConfiguration.h"
#include "Drawing.h"

InfoOverlay3D::InfoOverlay3D() {
}

InfoOverlay3D::~InfoOverlay3D() {
}

void InfoOverlay3D::update(int item_index, int item_type, SLADEMap* map) {
	// Clear current info
	info.clear();
	info_wall.clear();

	// Setup variables
	current_type = item_type;
	texture = "";

	// Wall
	if (item_type == MapEditor::SEL_SIDE_BOTTOM || item_type == MapEditor::SEL_SIDE_MIDDLE || item_type == MapEditor::SEL_SIDE_TOP) {
		// Get line and side
		MapSide* side = map->getSide(item_index);
		if (!side) return;
		MapLine* line = side->getParentLine();
		if (!line) return;

		// --- Line/side info ---
		info.push_back(S_FMT("Line #%d", line->getIndex()));
		if (side == line->s1())
			info.push_back(S_FMT("Front Side #%d", side->getIndex()));
		else
			info.push_back(S_FMT("Back Side #%d", side->getIndex()));

		// Relevant flags
		string flags = "";
		if (theGameConfiguration->lineBasicFlagSet("dontpegtop", line))
			flags += "Upper Unpegged, ";
		if (theGameConfiguration->lineBasicFlagSet("dontpegbottom", line))
			flags += "Lower Unpegged, ";
		if (theGameConfiguration->lineBasicFlagSet("blocking", line))
			flags += "Blocking, ";
		if (!flags.IsEmpty()) {
			flags.RemoveLast(2);
			info.push_back(flags);
		}

		// Other potential info: special, sector#, length


		// --- Wall part info ---

		// Part
		if (item_type == MapEditor::SEL_SIDE_BOTTOM)
			info_wall.push_back("Lower Texture");
		else if (item_type == MapEditor::SEL_SIDE_MIDDLE)
			info_wall.push_back("Middle Texture");
		else
			info_wall.push_back("Upper Texture");

		// Offsets
		if (theGameConfiguration->udmfNamespace() == "zdoom") {
			// Get x offset info
			int xoff = side->intProperty("offsetx");
			double xoff_part = 0;
			if (item_type == MapEditor::SEL_SIDE_BOTTOM)
				xoff_part = side->floatProperty("offsetx_bottom");
			else if (item_type == MapEditor::SEL_SIDE_MIDDLE)
				xoff_part = side->floatProperty("offsetx_mid");
			else
				xoff_part = side->floatProperty("offsetx_top");

			// Add x offset string
			if (xoff_part == 0)
				info_wall.push_back(S_FMT("X Offset: %d", xoff));
			else if (xoff_part > 0)
				info_wall.push_back(S_FMT("X Offset: %1.2f (%d+%1.2f)", (double)xoff+xoff_part, xoff, xoff_part));
			else
				info_wall.push_back(S_FMT("X Offset: %1.2f (%d-%1.2f)", (double)xoff+xoff_part, xoff, -xoff_part));

			// Get y offset info
			int yoff = side->intProperty("offsety");
			double yoff_part = 0;
			if (item_type == MapEditor::SEL_SIDE_BOTTOM)
				yoff_part = side->floatProperty("offsety_bottom");
			else if (item_type == MapEditor::SEL_SIDE_MIDDLE)
				yoff_part = side->floatProperty("offsety_mid");
			else
				yoff_part = side->floatProperty("offsety_top");

			// Add y offset string
			if (yoff_part == 0)
				info_wall.push_back(S_FMT("Y Offset: %d", yoff));
			else if (yoff_part > 0)
				info_wall.push_back(S_FMT("Y Offset: %1.2f (%d+%1.2f)", (double)yoff+yoff_part, yoff, yoff_part));
			else
				info_wall.push_back(S_FMT("Y Offset: %1.2f (%d-%1.2f)", (double)yoff+yoff_part, yoff, -yoff_part));
		}
		else {
			// Basic offsets
			info_wall.push_back(S_FMT("X Offset: %d", side->intProperty("offsetx")));
			info_wall.push_back(S_FMT("Y Offset: %d", side->intProperty("offsety")));
		}

		// Texture
		if (item_type == MapEditor::SEL_SIDE_BOTTOM)
			texture = side->stringProperty("texturebottom");
		else if (item_type == MapEditor::SEL_SIDE_MIDDLE)
			texture = side->stringProperty("texturemiddle");
		else
			texture = side->stringProperty("texturetop");
	}
}

void InfoOverlay3D::draw(int bottom, int right, int middle, float alpha) {
	// Don't bother if invisible
	if (alpha <= 0.0f)
		return;

	// Don't bother if no info
	if (info.size() == 0)
		return;

	// Determine overlay height
	int nlines = max(4, info.size());
	int height = nlines * 16;

	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	col_fg.a = col_fg.a*alpha;

	// Slide in/out animation
	float alpha_inv = 1.0f - alpha;
	int bottom2 = bottom;
	bottom += height*alpha_inv*alpha_inv;

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
		Drawing::drawText(info[a], middle - 44, bottom - y, col_fg, Drawing::FONT_CONDENSED, Drawing::ALIGN_RIGHT);
		y -= 16;
	}

	// Draw any extra info
	if (current_type == MapEditor::SEL_SIDE_BOTTOM || current_type == MapEditor::SEL_SIDE_MIDDLE || current_type == MapEditor::SEL_SIDE_TOP) {
		// Draw wall info strings
		y = height;
		for (unsigned a = 0; a < info_wall.size(); a++) {
			Drawing::drawText(info_wall[a], middle + 44, bottom - y, col_fg, Drawing::FONT_CONDENSED);
			y -= 16;
		}
	}

	// Draw texture if any
	drawTexture(alpha, middle - 40, bottom);
}

void InfoOverlay3D::drawTexture(float alpha, int x, int y) {
	// Get colours
	rgba_t col_bg = ColourConfiguration::getColour("map_overlay_background");
	rgba_t col_fg = ColourConfiguration::getColour("map_overlay_foreground");
	col_fg.a = col_fg.a*alpha;

	// Check texture isn't blank
	if (texture != "-" && !texture.IsEmpty()) {
		// Draw background
		glEnable(GL_TEXTURE_2D);
		rgba_t(255, 255, 255, 255*alpha, 0).set_gl();
		glPushMatrix();
		glTranslated(x, y-96, 0);
		GLTexture::bgTex().draw2dTiled(80, 80);
		glPopMatrix();

		// Get texture
		GLTexture* tex = theMapEditor->textureManager().getTexture(texture, theGameConfiguration->mixTexFlats());

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
	Drawing::drawText(texture, x + 40, y - 16, col_fg, Drawing::FONT_CONDENSED, Drawing::ALIGN_CENTER);
}
