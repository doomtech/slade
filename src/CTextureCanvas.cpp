
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    CTextureCanvas.cpp
 * Description: An OpenGL canvas that displays a composite texture
 *              (ie from doom's TEXTUREx)
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
#include "CTextureCanvas.h"
#include "Misc.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
 wxDEFINE_EVENT(EVT_DRAG_END, wxCommandEvent);


/*******************************************************************
 * CTEXTURECANVAS CLASS FUNCTIONS
 *******************************************************************/

/* CTextureCanvas::CTextureCanvas
 * CTextureCanvas class constructor
 *******************************************************************/
CTextureCanvas::CTextureCanvas(wxWindow* parent, int id)
: OGLCanvas(parent, id) {
	// Init variables
	scale = 1.0;
	hilight_patch = -1;
	draw_outside = true;
	dragging = false;
	texture = NULL;
	show_grid = false;

	// Bind events
	Bind(wxEVT_MOTION, &CTextureCanvas::onMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &CTextureCanvas::onMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &CTextureCanvas::onMouseEvent, this);
}

/* CTextureCanvas::~CTextureCanvas
 * CTextureCanvas class destructor
 *******************************************************************/
CTextureCanvas::~CTextureCanvas() {
	clearPatchTextures();
}

/* CTextureCanvas::selectPatch
 * Selects the patch at [index]
 *******************************************************************/
void CTextureCanvas::selectPatch(int index) {
	// Check patch index is ok
	if (index < 0 || (unsigned) index >= texture->nPatches())
		return;

	// Select the patch
	selected_patches[index] = true;
}

/* CTextureCanvas::deSelectPatch
 * De-Selects the patch at [index]
 *******************************************************************/
void CTextureCanvas::deSelectPatch(int index) {
	// Check patch index is ok
	if (index < 0 || (unsigned) index >= texture->nPatches())
		return;

	// De-Select the patch
	selected_patches[index] = false;
}

/* CTextureCanvas::patchSelected
 * Returns true if the patch at [index] is selected, false otherwise
 *******************************************************************/
bool CTextureCanvas::patchSelected(int index) {
	// Check index is ok
	if (index < 0 || (unsigned)index >= texture->nPatches())
		return false;

	// Return if patch index is selected
	return selected_patches[index];
}

/* CTextureCanvas::clearTexture
 * Clears the current texture and the patch textures list
 *******************************************************************/
void CTextureCanvas::clearTexture() {
	// Stop listening to the current texture (if it exists)
	if (texture)
		stopListening(texture);

	// Clear texture;
	texture = NULL;

	// Clear patch textures
	clearPatchTextures();

	// Reset view offset
	resetOffsets();

	// Clear patch selection
	selected_patches.clear();
}

/* CTextureCanvas::clearPatchTextures
 * Clears the patch textures list
 *******************************************************************/
void CTextureCanvas::clearPatchTextures() {
	for (size_t a = 0; a < patch_textures.size(); a++)
		delete patch_textures[a];
	patch_textures.clear();
}

/* CTextureCanvas::updatePatchTextures
 * Unloads all patch textures, so they are reloaded on next draw
 *******************************************************************/
void CTextureCanvas::updatePatchTextures() {
	for (size_t a = 0; a < patch_textures.size(); a++)
		patch_textures[a]->clear();
}

/* CTextureCanvas::openTexture
 * Loads a composite texture to be displayed
 *******************************************************************/
bool CTextureCanvas::openTexture(CTexture* tex, PatchTable& ptable) {
	// Clear the current texture
	clearTexture();

	// Set texture
	texture = tex;

	// Init patches
	clearPatchTextures();
	for (uint32_t a = 0; a < tex->nPatches(); a++) {
		// Create GL texture
		patch_textures.push_back(new GLTexture());

		// Set selection
		selected_patches.push_back(false);
	}

	// Listen to it
	listenTo(tex);

	// Redraw
	Refresh();

	return true;
}

/* CTextureCanvas::draw
 * Draws the canvas contents
 *******************************************************************/
void CTextureCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Translate to middle of pixel (otherwise inaccuracies can occur on certain gl implemenataions)
	glTranslatef(0.375f, 0.375f, 0);

	// Draw background
	drawCheckeredBackground();

	// Pan by view offset
	glTranslated(offset.x, offset.y, 0);

	// Draw texture
	if (texture)
		drawTexture();

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}

/* CTextureCanvas::drawTexture
 * Draws the currently opened composite texture
 *******************************************************************/
void CTextureCanvas::drawTexture() {
	// Push matrix
	glPushMatrix();

	// Calculate top-left position of texture (for glScissor, since it ignores the current translation/scale)
	double left = offset.x + (GetSize().x * 0.5) - (texture->getWidth() * 0.5 * scale);
	double top = -offset.y + (GetSize().y * 0.5) - (texture->getHeight() * 0.5 * scale);

	// Translate to middle of the canvas
	glTranslated(GetSize().x * 0.5, GetSize().y * 0.5, 0);

	// Zoom
	glScaled(scale, scale, 1);

	// Translate to top-left of texture
	glTranslated(texture->getWidth() * -0.5, texture->getHeight() * -0.5, 0);

	// First, draw patches semitransparently (for anything outside the texture)
	// But only if we are drawing stuff outside the texture area
	if (draw_outside) {
		for (uint32_t a = 0; a < texture->nPatches(); a++)
			drawPatch(a, rgba_t(200, 50, 50, 80, 0));
	}

	// Now, clip to texture boundaries and draw patches fully opaque
	glEnable(GL_SCISSOR_TEST);
	glScissor(left, top, texture->getWidth() * scale, texture->getHeight() * scale);
	for (uint32_t a = 0; a < texture->nPatches(); a++)
		drawPatch(a);

	glDisable(GL_SCISSOR_TEST);

	// Draw the texture border
	drawTextureBorder();

	// Now loop through selected patches and draw selection outlines
	rgba_t(70, 210, 220, 255, 0).set_gl();
	for (size_t a = 0; a < selected_patches.size(); a++) {
		// Skip if not selected
		if (!selected_patches[a])
			continue;

		// Get patch
		CTPatch* patch = texture->getPatch(a);

		// Draw outline
		glBegin(GL_LINE_LOOP);
		glVertex2i(patch->xOffset(), patch->yOffset());
		glVertex2i(patch->xOffset(), patch->yOffset() + (int)patch_textures[a]->getHeight());
		glVertex2i(patch->xOffset() + (int)patch_textures[a]->getWidth(), patch->yOffset() + (int)patch_textures[a]->getHeight());
		glVertex2i(patch->xOffset() + (int)patch_textures[a]->getWidth(), patch->yOffset());
		glEnd();
	}

	// Pop matrix
	glPopMatrix();
}

/* CTextureCanvas::drawPatch
 * Draws the patch at index [num] in the composite texture
 *******************************************************************/
void CTextureCanvas::drawPatch(int num, rgba_t col) {
	// Get patch to draw
	CTPatch* patch = texture->getPatch(num);

	// Check it exists
	if (!patch)
		return;

	// Load the patch as an opengl texture if it isn't already
	if (!patch_textures[num]->isLoaded()) {
		SImage temp;
		Misc::loadImageFromEntry(&temp, patch->getEntry());
		patch_textures[num]->loadImage(&temp, &palette);
	}

	// Enable textures
	glEnable(GL_TEXTURE_2D);

	// Draw the patch
	col.set_gl();
	patch_textures[num]->draw2d(patch->xOffset(), patch->yOffset());

	// If the patch is hilighted, draw hilight
	if (hilight_patch == num) {
		rgba_t(255, 255, 255, 80, 1).set_gl();
		patch_textures[num]->draw2d(patch->xOffset(), patch->yOffset());
	}
	// If the patch is selected, hilight it also (selection colour)
	if (selected_patches[num]) {
		rgba_t(100, 150, 255, 140, 1).set_gl();
		patch_textures[num]->draw2d(patch->xOffset(), patch->yOffset());
	}

	// Disable textures
	glDisable(GL_TEXTURE_2D);
}

/* CTextureCanvas::drawTextureBorder
 * Draws a black border around the texture
 *******************************************************************/
void CTextureCanvas::drawTextureBorder() {
	// Draw the texture border
	COL_BLACK.set_gl();
	glBegin(GL_LINE_LOOP);
	glVertex2i(0, 0);
	glVertex2i(0, texture->getHeight());
	glVertex2i(texture->getWidth(), texture->getHeight());
	glVertex2i(texture->getWidth(), 0);
	glEnd();

	// Draw vertical ticks
	int y = 0;
	glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
	while (y <= texture->getHeight()) {
		glBegin(GL_LINES);
		glVertex2i(-4, y);
		glVertex2i(0, y);
		glVertex2i(texture->getWidth(), y);
		glVertex2i(texture->getWidth() + 4, y);
		glEnd();

		y += 8;
	}

	// Draw horizontal ticks
	int x = 0;
	while (x <= texture->getWidth()) {
		glBegin(GL_LINES);
		glVertex2i(x, -4);
		glVertex2i(x, 0);
		glVertex2i(x, texture->getHeight());
		glVertex2i(x, texture->getHeight() + 4);
		glEnd();

		x += 8;
	}

	// Draw grid
	if (show_grid) {
		// Draw inverted grid lines
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		// Vertical
		y = 8;
		while (y <= texture->getHeight() - 8) {
			glBegin(GL_LINES);
			glVertex2i(0, y);
			glVertex2i(texture->getWidth(), y);
			glEnd();

			y += 8;
		}

		// Horizontal
		x = 8;
		while (x <= texture->getWidth() - 8) {
			glBegin(GL_LINES);
			glVertex2i(x, 0);
			glVertex2i(x, texture->getHeight());
			glEnd();

			x += 8;
		}


		// Darken grid lines
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

		// Vertical
		y = 8;
		while (y <= texture->getHeight() - 8) {
			glBegin(GL_LINES);
			glVertex2i(0, y);
			glVertex2i(texture->getWidth(), y);
			glEnd();

			y += 8;
		}

		// Horizontal
		x = 8;
		while (x <= texture->getWidth() - 8) {
			glBegin(GL_LINES);
			glVertex2i(x, 0);
			glVertex2i(x, texture->getHeight());
			glEnd();

			x += 8;
		}
	}
}

/* CTextureCanvas::screenToTextPosition
 * Convert from [x,y] from the top left of the canvas to coordinates
 * relative to the top left of the texture
 *******************************************************************/
point2_t CTextureCanvas::screenToTexPosition(int x, int y) {
	// Check a texture is open
	if (!texture)
		return point2_t(0, 0);

	// Get top-left of texture in screen coordinates (ie relative to the top-left of the canvas)
	int left = GetSize().x * 0.5 + offset.x;
	int top = GetSize().y * 0.5 + offset.y;
	left -= (double)texture->getWidth() * 0.5 * scale;
	top -= (double)texture->getHeight() * 0.5 * scale;

	return point2_t(double(x - left) / scale, double(y - top) / scale);
}

/* CTextureCanvas::patchAt
 * Returns the index of the patch at [x,y] on the texture, or -1
 * if no patch is at that position
 *******************************************************************/
int CTextureCanvas::patchAt(int x, int y) {
	// Check a texture is open
	if (!texture)
		return -1;

	// Go through texture patches backwards (ie from frontmost to back)
	for (int a = texture->nPatches() - 1; a >= 0; a--) {
		// Check if x,y is within patch bounds
		CTPatch* patch = texture->getPatch(a);
		if (x >= patch->xOffset() && x < patch->xOffset() + (int)patch_textures[a]->getWidth() &&
			y >= patch->yOffset() && y < patch->yOffset() + (int)patch_textures[a]->getHeight()) {
			return a;
		}
	}

	// No patch at x,y
	return -1;
}

/* CTextureCanvas::swapPatches
 * Swaps patches at [p1] and [p2] in the texture. Returns false if
 * either index is invalid, true otherwise
 *******************************************************************/
bool CTextureCanvas::swapPatches(size_t p1, size_t p2) {
	// Check a texture is open
	if (!texture)
		return false;

	// Check indices
	if (p1 >= texture->nPatches() || p2 >= texture->nPatches())
		return false;

	// Swap patch gl textures
	GLTexture* temp = patch_textures[p1];
	patch_textures[p1] = patch_textures[p2];
	patch_textures[p2] = temp;

	// Swap patches in the texture itself
	return texture->swapPatches(p1, p2);
}

/* CTextureCanvas::onAnnouncement
 * Called when the texture canvas recieves an announcement from the
 * texture being displayed
 *******************************************************************/
void CTextureCanvas::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	// If the announcer isn't this canvas' texture, ignore it
	if (announcer != texture)
		return;

	// Texture modified
	//if (event_name == "modified")
	//	Refresh();

	// Patches modified
	if (event_name == "patches_modified") {
		// Reload patches
		selected_patches.clear();
		clearPatchTextures();
		for (uint32_t a = 0; a < texture->nPatches(); a++) {
			// Create GL texture
			patch_textures.push_back(new GLTexture());

			// Set selection
			selected_patches.push_back(false);
		}

		Refresh();
	}
}

/* CTextureCanvas::onMouseEvent
 * Called when and mouse event is generated (movement/clicking/etc)
 *******************************************************************/
void CTextureCanvas::onMouseEvent(wxMouseEvent& e) {
	bool refresh = false;

	// MOUSE MOVEMENT
	if (e.Moving() || e.Dragging()) {
		dragging = false;

		// Pan if right button is down
		if (e.RightIsDown()) {
			offset = offset + point2_t(e.GetPosition().x - mouse_prev.x, e.GetPosition().y - mouse_prev.y);
			refresh = true;
			dragging = true;
		}
		else if (e.LeftIsDown())
			dragging = true;

		// Check if patch hilight changes
		point2_t pos = screenToTexPosition(e.GetX(), e.GetY());
		int patch = patchAt(pos.x, pos.y);
		if (hilight_patch != patch) {
			hilight_patch = patch;
			refresh = true;
		}
	}

	// LEFT BUTTON UP
	else if (e.LeftUp()) {
		// If we were dragging, generate end drag event
		if (dragging) {
			dragging = false;
			wxCommandEvent evt(EVT_DRAG_END, GetId());
			evt.SetInt(wxMOUSE_BTN_LEFT);
			ProcessWindowEvent(evt);
		}
	}

	// LEAVING
	if (e.Leaving()) {
		// Set no hilighted patch
		hilight_patch = -1;
		refresh = true;
	}

	// Refresh is needed
	if (refresh)
		Refresh();

	// Update 'previous' mouse coordinates
	mouse_prev.set(e.GetPosition().x, e.GetPosition().y);
}
