
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


/*******************************************************************
 * CTEXTURECANVAS CLASS FUNCTIONS
 *******************************************************************/

/* CTextureCanvas::CTextureCanvas
 * CTextureCanvas class constructor
 *******************************************************************/
CTextureCanvas::CTextureCanvas(wxWindow* parent, int id)
: OGLCanvas(parent, id) {
	texture = NULL;
	tex_background = new GLTexture();
}

/* CTextureCanvas::~CTextureCanvas
 * CTextureCanvas class destructor
 *******************************************************************/
CTextureCanvas::~CTextureCanvas() {
	clearPatchTextures();
	delete tex_background;
}

/* CTextureCanvas::clearTexture
 * Clears the current texture and the patch textures list
 *******************************************************************/
void CTextureCanvas::clearTexture() {
	// Clear texture
	texture = NULL;

	// Clear patch textures
	clearPatchTextures();
}

/* CTextureCanvas::clearPatchTextures
 * Clears the patch textures list
 *******************************************************************/
void CTextureCanvas::clearPatchTextures() {
	for (size_t a = 0; a < patch_textures.size(); a++)
		delete patch_textures[a];
	patch_textures.clear();
}

/* CTextureCanvas::openTexture
 * Loads a composite texture to be displayed
 *******************************************************************/
bool CTextureCanvas::openTexture(CTexture* tex) {
	// Clear the current texture
	clearTexture();

	// Check a texture was given
	if (!tex)
		return false;

	// Update variables
	texture = tex;

	// Init patch opengl texture id stuff
	clearPatchTextures();
	for (uint32_t a = 0; a < tex->nPatches(); a++) {
		// Create GL texture
		patch_textures.push_back(new GLTexture());
	}

	// Redraw
	Refresh();

	return true;
}

/* CTextureCanvas::draw
 * Draws the canvas contents
 *******************************************************************/
void CTextureCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetClientSize().x, GetClientSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw background
	drawChequeredBackground();

	// Draw texture
	if (texture) {
		drawTexture();
		drawTextureBorder();
	}

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}

/* CTextureCanvas::drawChequeredBackground
 * Fills the canvas with a chequered pattern (used as the
 * 'background' - to indicate transparency)
 *******************************************************************/
void CTextureCanvas::drawChequeredBackground() {
	// Save current matrix
	glPushMatrix();

	// Generate background texture if needed
	if (!tex_background->isLoaded())
		tex_background->genChequeredTexture(8, rgba_t(64, 64, 80, 255), rgba_t(80, 80, 96, 255));

	// Enable textures
	glEnable(GL_TEXTURE_2D);

	// Bind background texture
	tex_background->bind();

	// Draw background
	frect_t rect(0, 0, GetSize().x, GetSize().y);
	COL_WHITE.set_gl();
	glBegin(GL_QUADS);
	glTexCoord2d(rect.x1()*0.0625, rect.y1()*0.0625);	glVertex2d(rect.x1(), rect.y1());
	glTexCoord2d(rect.x1()*0.0625, rect.y2()*0.0625);	glVertex2d(rect.x1(), rect.y2());
	glTexCoord2d(rect.x2()*0.0625, rect.y2()*0.0625);	glVertex2d(rect.x2(), rect.y2());
	glTexCoord2d(rect.x2()*0.0625, rect.y1()*0.0625);	glVertex2d(rect.x2(), rect.y1());
	glEnd();

	// Disable textures
	glDisable(GL_TEXTURE_2D);

	// Restore previous matrix
	glPopMatrix();
}

/* CTextureCanvas::drawTexture
 * Draws the currently opened composite texture
 *******************************************************************/
void CTextureCanvas::drawTexture() {
	// Push matrix
	glPushMatrix();

	// Translate to middle of the canvas
	glTranslated(GetSize().x * 0.5, GetSize().y * 0.5, 0);

	// Translate to top-left of texture
	glTranslated(texture->getWidth() * -0.5, texture->getHeight() * -0.5, 0);

	// Draw patches in order
	COL_WHITE.set_gl();
	for (uint32_t a = 0; a < texture->nPatches(); a++)
		drawPatch(a);

	// Pop matrix
	glPopMatrix();
}

/* CTextureCanvas::drawPatch
 * Draws the patch at index [num] in the composite texture
 *******************************************************************/
void CTextureCanvas::drawPatch(int num) {
	// Get patch to draw
	CTPatch* patch = texture->getPatch(num);

	// Check it exists
	if (!patch)
		return;

	// Load the patch as an opengl texture if it isn't already
	if (!patch_textures[num]->isLoaded()) {
		if (patch)
			patch_textures[num]->loadImage(patch->getImage(), &palette);
	}

	// Enable textures
	glEnable(GL_TEXTURE_2D);

	// Draw the patch
	patch_textures[num]->draw2d(patch->xOffset(), patch->yOffset());

	// Disable textures
	glDisable(GL_TEXTURE_2D);
}

/* CTextureCanvas::drawTextureBorder
 * Draws a black border around the texture
 *******************************************************************/
void CTextureCanvas::drawTextureBorder() {
	// Push matrix
	glPushMatrix();

	// Translate to middle of the canvas
	glTranslated(GetSize().x * 0.5, GetSize().y * 0.5, 0);

	// Translate to top-left of texture
	glTranslated(texture->getWidth() * -0.5, texture->getHeight() * -0.5, 0);

	// Draw border
	COL_BLACK.set_gl();
	glBegin(GL_LINES);
	glVertex2d(0, 0);
	glVertex2d(texture->getWidth(), 0);
	glVertex2d(texture->getWidth(), 0);
	glVertex2d(texture->getWidth(), texture->getHeight());
	glVertex2d(texture->getWidth(), texture->getHeight());
	glVertex2d(0, texture->getHeight());
	glVertex2d(0, texture->getHeight());
	glVertex2d(0, 0);
	glEnd();

	// Pop matrix
	glPopMatrix();
}
