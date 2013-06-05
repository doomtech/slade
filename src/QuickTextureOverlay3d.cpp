
#include "Main.h"
#include "WxStuff.h"
#include "MapEditorWindow.h"
#include "QuickTextureOverlay3d.h"
#include "Drawing.h"
#include "ResourceManager.h"
#include "GLTexture.h"
#include "MapEditor.h"
#include "GameConfiguration.h"

QuickTextureOverlay3d::QuickTextureOverlay3d(MapEditor* editor)
{
	// Init variables
	allow_3d_mlook = true;
	current_index = 0;
	anim_offset = 0;
	this->editor = editor;

	if (editor)
	{
		vector<selection_3d_t> sel;
		editor->get3dSelectionOrHilight(sel);

		// Cancel if no selection
		if (sel.empty())
		{
			active = false;
			return;
		}

		// Cancel if only things selected
		bool ok = false;
		for (unsigned a = 0; a < sel.size(); a++)
		{
			if (sel[a].type != MapEditor::SEL_THING)
			{
				ok = true;
				break;
			}
		}
		if (!ok)
		{
			active = false;
			return;
		}

		// Determine texture type
		sel_type = 2;
		int initial = 0;
		if (!theGameConfiguration->mixTexFlats())
		{
			sel_type = 0;
			for (unsigned a = 0; a < sel.size(); a++)
			{
				if (sel[a].type != MapEditor::SEL_THING && sel[a].type != MapEditor::SEL_CEILING && sel[a].type != MapEditor::SEL_FLOOR)
				{
					sel_type = 1;
					initial = a;
					break;
				}
			}
		}

		// Get initial texture
		string tex_init;
		if (sel[initial].type == MapEditor::SEL_CEILING)
			tex_init = editor->getMap().getSector(sel[initial].index)->stringProperty("textureceiling");
		else if (sel[initial].type == MapEditor::SEL_FLOOR)
			tex_init = editor->getMap().getSector(sel[initial].index)->stringProperty("texturefloor");
		else if (sel[initial].type == MapEditor::SEL_SIDE_TOP)
			tex_init = editor->getMap().getSide(sel[initial].index)->stringProperty("texturetop");
		else if (sel[initial].type == MapEditor::SEL_SIDE_MIDDLE)
			tex_init = editor->getMap().getSide(sel[initial].index)->stringProperty("texturemiddle");
		else if (sel[initial].type == MapEditor::SEL_SIDE_BOTTOM)
			tex_init = editor->getMap().getSide(sel[initial].index)->stringProperty("texturebottom");

		// Get all available texture names (sorted alphabetically)
		vector<string> tex_names;
		if (sel_type > 0)
			theResourceManager->getAllTextureNames(tex_names);
		if (sel_type == 0 || sel_type == 2)
			theResourceManager->getAllFlatNames(tex_names);
		std::sort(tex_names.begin(), tex_names.end());

		// Init textures
		for (unsigned a = 0; a < tex_names.size(); a++)
			textures.push_back(qt_tex_t(tex_names[a]));

		// Set initial texture
		setTexture(tex_init);

		// Begin recording undo step
		editor->beginUndoRecord("Quick Texture", true, false, false);
	}
}

QuickTextureOverlay3d::~QuickTextureOverlay3d()
{
}

void QuickTextureOverlay3d::setTexture(string name)
{
	for (unsigned a = 0; a < textures.size(); a++)
	{
		if (S_CMPNOCASE(textures[a].name, name))
		{
			current_index = a;
			anim_offset = a;
			return;
		}
	}
}

void QuickTextureOverlay3d::applyTexture()
{
	// Check editor is associated
	if (!editor)
		return;

	// Get selection/hilight
	vector<selection_3d_t> selection;
	editor->get3dSelectionOrHilight(selection);

	// Go through items
	if (!selection.empty())
	{
		for (unsigned a = 0; a < selection.size(); a++)
		{
			// Thing (skip)
			if (selection[a].type == MapEditor::SEL_THING)
				continue;

			// Floor
			else if (selection[a].type == MapEditor::SEL_FLOOR && (sel_type == 0 || sel_type == 2))
			{
				MapSector* sector = editor->getMap().getSector(selection[a].index);
				if (sector) sector->setStringProperty("texturefloor", textures[current_index].name);
			}

			// Ceiling
			else if (selection[a].type == MapEditor::SEL_CEILING && (sel_type == 0 || sel_type == 2))
			{
				MapSector* sector = editor->getMap().getSector(selection[a].index);
				if (sector) sector->setStringProperty("textureceiling", textures[current_index].name);
			}

			// Wall
			else if (sel_type > 0)
			{
				MapSide* side = editor->getMap().getSide(selection[a].index);
				if (side)
				{
					// Upper
					if (selection[a].type == MapEditor::SEL_SIDE_TOP)
						side->setStringProperty("texturetop", textures[current_index].name);
					// Middle
					else if (selection[a].type == MapEditor::SEL_SIDE_MIDDLE)
						side->setStringProperty("texturemiddle", textures[current_index].name);
					// Lower
					else if (selection[a].type == MapEditor::SEL_SIDE_BOTTOM)
						side->setStringProperty("texturebottom", textures[current_index].name);
				}
			}
		}
	}
}

void QuickTextureOverlay3d::update(long frametime)
{
	double target = current_index;
	float mult = (float)frametime / 10.0f;
	if (anim_offset < target - 0.01)
		anim_offset += (target - anim_offset) * (0.2 * mult);
	else if (anim_offset > target + 0.01)
		anim_offset -= (anim_offset - target) * (0.2 * mult);
	else
		anim_offset = current_index;
}

void QuickTextureOverlay3d::draw(int width, int height, float fade)
{
	// Don't draw if invisible
	if (fade < 0.001f)
		return;

	// Draw background
	glColor4f(0.0f, 0.0f, 0.0f, fade * 0.6f);
	glDisable(GL_TEXTURE_2D);
	Drawing::drawFilledRect(0, height - 120, width, height);

	// Draw current tex name
	glEnable(GL_TEXTURE_2D);

	// Draw textures
	double x = ((double)width * 0.5) - (anim_offset * 136.0);
	glColor4f(1.0f, 1.0f, 1.0f, fade);
	for (unsigned a = 0; a < textures.size(); a++)
	{
		// Skip until first texture to show on left
		if (x < -96)
		{
			x += 136;
			continue;
		}

		// Draw texture
		drawTexture(a, x, height - 18, determineSize(x, width), fade);
		x += 136;

		// Stop when we get to the right edge of the screen
		if (x > width + 96)
			break;
	}
}

void QuickTextureOverlay3d::drawTexture(unsigned index, double x, double bottom, double size, float fade)
{
	// Get texture if needed
	if (!textures[index].texture)
	{
		if (sel_type == 1)
			textures[index].texture = theMapEditor->textureManager().getTexture(textures[index].name, false);
		else if (sel_type == 0)
			textures[index].texture = theMapEditor->textureManager().getFlat(textures[index].name, false);
		else
			textures[index].texture = theMapEditor->textureManager().getTexture(textures[index].name, true);
	}

	// Draw name
	double brightness = 0.5 + (size - 1.0);
	Drawing::drawText(textures[index].name, x, bottom + 2, rgba_t(brightness*255, brightness*255, brightness*255, brightness*255*fade),
	                  index == current_index ? Drawing::FONT_BOLD : Drawing::FONT_NORMAL, Drawing::ALIGN_CENTER);

	// Draw texture
	frect_t rect = Drawing::fitTextureWithin(textures[index].texture, x - 48*size, bottom - (96*size), x + 48*size, bottom, 0, 2);
	glColor4f(brightness, brightness, brightness, brightness*fade);
	textures[index].texture->bind();
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(rect.x1(), rect.y1());
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(rect.x1(), rect.y2());
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(rect.x2(), rect.y2());
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(rect.x2(), rect.y1());
	glEnd();
}

double QuickTextureOverlay3d::determineSize(double x, int width)
{
	double mid = (double)width * 0.5;
	if (x < mid - 384 || x > mid + 384)
		return 1.0;

	double diff = (x < mid) ? mid - x : x - mid;
	double mult = ((384 - diff) / 384);
	return 1 + (0.5 * mult * mult);
}

void QuickTextureOverlay3d::close(bool cancel)
{
	if (editor)
	{
		editor->endUndoRecord(true);
		editor->lockHilight(false);
		if (cancel)
			editor->doUndo();
	}

	active = false;
}

void QuickTextureOverlay3d::mouseMotion(int x, int y)
{
}

void QuickTextureOverlay3d::mouseLeftClick()
{
}

void QuickTextureOverlay3d::mouseRightClick()
{
}

void QuickTextureOverlay3d::doSearch()
{
	if (!search.empty())
	{
		for (unsigned a = 0; a < textures.size(); a++)
		{
			if (textures[a].name.Lower().StartsWith(search.Lower()))
			{
				current_index = a;
				applyTexture();
				return;
			}
		}
	}
}

void QuickTextureOverlay3d::keyDown(string key)
{
	// Up texture
	if ((key == "right" || key == "mwheeldown") && current_index < textures.size() - 1)
	{
		current_index++;
		search = "";
		applyTexture();
	}

	// Down texture
	else if ((key == "left" || key == "mwheelup") && current_index > 0)
	{
		current_index--;
		search = "";
		applyTexture();
	}

	// Character (search)
	else if (key.length() == 1)
	{
		search += key;
		doSearch();
	}
}
