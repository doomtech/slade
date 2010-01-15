
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
}

/* CTextureCanvas::~CTextureCanvas
 * CTextureCanvas class destructor
 *******************************************************************/
CTextureCanvas::~CTextureCanvas() {
}

/* CTextureCanvas::openTexture
 * Loads a composite texture to be displayed
 *******************************************************************/
bool CTextureCanvas::openTexture(CTexture* tex) {
	if (!tex)
		return false;

	// Update variables
	texture = tex;

	// Init patch opengl texture id stuff
	patch_gl_id.clear();
	for (uint32_t a = 0; a < tex->nPatches(); a++)
		patch_gl_id.push_back(UINT_MAX);

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

	// Determine the number of rows and columns
	double s_size = 8.0;
	int cols = int((double)GetSize().x / s_size) + 1;
	int rows = int((double)GetSize().y / s_size) + 1;

	// Scale to square size
	glScaled(s_size, s_size, 1);

	// Draw a grid of squares
	for (int x = 0; x < cols; x++) {
		for (int y = 0; y < rows; y++) {
			// Set square colour
			rgba_t col(64, 64, 80, 255);
			if (x%2 != y%2)
				col.set(80, 80, 96, 255);
			col.set_gl();

			// Draw the square
			glBegin(GL_QUADS);
			glVertex2d(x, y);
			glVertex2d(x, y+1);
			glVertex2d(x+1, y+1);
			glVertex2d(x+1, y);
			glEnd();
		}
	}

	// Restore previous matrix
	glPopMatrix();
}

void CTextureCanvas::drawTexture() {
	// Push matrix
	glPushMatrix();

	// Translate to middle of the canvas
	glTranslated(GetSize().x * 0.5, GetSize().y * 0.5, 0);

	// Translate to top-left of texture
	glTranslated(texture->getWidth() * -0.5, texture->getHeight() * -0.5, 0);

	// Draw patches in order
	for (uint32_t a = 0; a < texture->nPatches(); a++)
		drawPatch(a);

	// Pop matrix
	glPopMatrix();
}

void CTextureCanvas::drawPatch(int num) {
	// Get patch to draw
	CTPatch* patch = texture->getPatch(num);

	// Check it exists
	if (!patch)
		return;

	// Pust gl matrix
	glPushMatrix();

	// Load the patch as an opengl texture if it isn't already
	if (patch_gl_id[num] == UINT_MAX) {
		if (patch) {
			// Get image RGBA data
			MemChunk mc;
			patch->getImage()->getRGBAData(mc);

			// Generate the texture id
			glGenTextures(1, &(patch_gl_id[num]));
			glBindTexture(GL_TEXTURE_2D, patch_gl_id[num]);

			// Generate the texture
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, 4, patch->getImage()->getWidth(), patch->getImage()->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mc.getData());
		}
	}

	// Enable textures
	glEnable(GL_TEXTURE_2D);

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, patch_gl_id[num]);

	// Draw the patch
	glTranslated(patch->xOffset(), patch->yOffset(), 0);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0);		glVertex2d(0, 0);
	glTexCoord2d(0, 1);		glVertex2d(0, patch->getImage()->getHeight());
	glTexCoord2d(1, 1);		glVertex2d(patch->getImage()->getWidth(), patch->getImage()->getHeight());
	glTexCoord2d(1, 0);		glVertex2d(patch->getImage()->getWidth(), 0);
	glEnd();

	// Disable textures
	glDisable(GL_TEXTURE_2D);

	// Pop gl matrix
	glPopMatrix();
}
