
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GfxCanvas.cpp
 * Description: GfxCanvas class. An OpenGL canvas that displays
 *              an image and can take offsets into account etc
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
#include "WxStuff.h"
#include "GfxCanvas.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
DEFINE_EVENT_TYPE(wxEVT_GFXCANVAS_OFFSET_CHANGED)


/*******************************************************************
 * GFXCANVAS CLASS FUNCTIONS
 *******************************************************************/

/* GfxCanvas::GfxCanvas
 * GfxCanvas class constructor
 *******************************************************************/
GfxCanvas::GfxCanvas(wxWindow* parent, int id)
: OGLCanvas(parent, id) {
	// Init variables
	image = new SImage();
	view_type = GFXVIEW_DEFAULT;
	scale = 1;
	tex_image = new GLTexture();
	tex_background = new GLTexture();
	update_texture = false;
	image_hilight = false;
	drag_pos.set(0, 0);
	drag_origin.set(-1, -1);
	allow_drag = false;
	allow_scroll = false;

	// Listen to the image for changes
	listenTo(image);

	// Bind events
	Bind(wxEVT_LEFT_DOWN, &GfxCanvas::onMouseLeftDown, this);
	Bind(wxEVT_LEFT_UP, &GfxCanvas::onMouseLeftUp, this);
	Bind(wxEVT_MOTION, &GfxCanvas::onMouseMovement, this);
	Bind(wxEVT_LEAVE_WINDOW, &GfxCanvas::onMouseLeaving, this);
	Bind(wxEVT_KEY_DOWN, &GfxCanvas::onKeyDown, this);
}

/* GfxCanvas::~GfxCanvas
 * GfxCanvas class destructor
 *******************************************************************/
GfxCanvas::~GfxCanvas() {
	delete image;
	delete tex_image;
	delete tex_background;
}

/* GfxCanvas::draw
 * Draws the image/background/etc
 *******************************************************************/
void GfxCanvas::draw() {
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

	// Draw the background
	drawChequeredBackground();

	// Pan by view offset
	if (allow_scroll)
		glTranslated(offset.x, offset.y, 0);

	// Pan if offsets
	if (view_type == GFXVIEW_CENTERED || view_type == GFXVIEW_SPRITE || view_type == GFXVIEW_HUD) {
		int mid_x = GetSize().x / 2;
		int mid_y = GetSize().y / 2;
		glTranslated(mid_x, mid_y, 0);
	}

	// Draw offset lines
	if (view_type == GFXVIEW_SPRITE || view_type == GFXVIEW_HUD)
		drawOffsetLines();

	// Draw the image
	drawImage();

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}

/* GfxCanvas::drawOffsetLines
 * Draws the offset center lines
 *******************************************************************/
void GfxCanvas::drawOffsetLines() {
	if (view_type == GFXVIEW_SPRITE) {
		COL_BLACK.set_gl();

		glBegin(GL_LINES);
		glVertex2d(-9999, 0);
		glVertex2d(9999, 0);
		glVertex2d(0, -9999);
		glVertex2d(0, 9999);
		glEnd();
	}
	else if (view_type == GFXVIEW_HUD) {
		COL_BLACK.set_gl();
		double l = -160 * scale;
		double r = 160 * scale;
		double t = -100 * scale;
		double b = 100 * scale;
		double sb = 68 * scale;

		glBegin(GL_LINES);
		// Left
		glVertex2d(l, t);
		glVertex2d(l, b);
		// Bottom
		glVertex2d(l, b);
		glVertex2d(r, b);
		// Right
		glVertex2d(r, b);
		glVertex2d(r, t);
		// Top
		glVertex2d(r, t);
		glVertex2d(l, t);
		// Statusbar
		glVertex2d(l, sb);
		glVertex2d(r, sb);
		// Middle
		glVertex2d(0, t);
		glVertex2d(0, b);
		glEnd();
	}
}

/* GfxCanvas::drawChequeredBackground
 * Fills the canvas with a chequered pattern (used as the
 * 'background' - to indicate transparency)
 *******************************************************************/
void GfxCanvas::drawChequeredBackground() {
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

/* GfxCanvas::drawImage
 * Draws the image (reloads the image as a texture each time, will
 * change this later...)
 *******************************************************************/
void GfxCanvas::drawImage() {
	// Check image is valid
	if (!image->isValid())
		return;

	// Save current matrix
	glPushMatrix();

	// Zoom
	glScaled(scale, scale, 1);

	// Pan
	if (view_type == GFXVIEW_CENTERED)
		glTranslated(-(image->getWidth() * 0.5), -(image->getHeight() * 0.5), 0);	// Pan to center image
	else if (view_type == GFXVIEW_SPRITE)
		glTranslated(-image->offset().x, -image->offset().y, 0); // Pan by offsets
	else if (view_type == GFXVIEW_HUD) {
		glTranslated(-160, -100, 0);								// Pan to hud 'top left'
		glTranslated(-image->offset().x, -image->offset().y, 0);	// Pan by offsets
	}

	// Enable textures
	glEnable(GL_TEXTURE_2D);

	// Update texture if needed
	if (update_texture) {
		tex_image->loadImage(image);
		update_texture = false;
	}

	// Determine (texture)coordinates
	double x = (double)image->getWidth();
	double y = (double)image->getHeight();

	// If tiled view
	if (view_type == GFXVIEW_TILED) {
		// Draw tiled image
		rgba_t(255, 255, 255, 255, 0).set_gl();
		tex_image->draw2dTiled(GetSize().x / scale, GetSize().y / scale);
	}
	else if (drag_origin.x < 0) {	// If not dragging
		// Draw the image
		rgba_t(255, 255, 255, 255, 0).set_gl();
		tex_image->draw2d();

		// Draw hilight
		if (image_hilight) {
			rgba_t(255, 255, 255, 80, 1).set_gl();
			tex_image->draw2d();

			// Reset colour
			rgba_t(255, 255, 255, 255, 0).set_gl();
		}
	}
	else {	// Dragging
		// Draw the original
		rgba_t(0, 0, 0, 180, 0).set_gl();
		tex_image->draw2d();

		// Draw the dragged image
		int off_x = (drag_pos.x - drag_origin.x) / scale;
		int off_y = (drag_pos.y - drag_origin.y) / scale;
		glTranslated(off_x, off_y, 0);
		rgba_t(255, 255, 255, 255, 0).set_gl();
		tex_image->draw2d();
	}

	// Disable textures
	glDisable(GL_TEXTURE_2D);

	// Draw outline
	rgba_t(0, 0, 0, 64).set_gl();
	glBegin(GL_LINES);
	glVertex2d(0, 0); glVertex2d(0, y);
	glVertex2d(0, y); glVertex2d(x, y);
	glVertex2d(x, y); glVertex2d(x, 0);
	glVertex2d(x, 0); glVertex2d(0, 0);
	glEnd();

	// Restore previous matrix
	glPopMatrix();
}

/* GfxCanvas::updateImageTexture
 * (Re)Generates the image texture from image data
 *******************************************************************/
void GfxCanvas::updateImageTexture() {
}

/* GfxCanvas::zoomToFit
 * Scales the image to fit within the gfx canvas. If mag is false,
 * the image will not be stretched to fit the canvas (only shrunk
 * if needed). Leaves a border around the image if <padding> is
 * specified (0.0f = no border, 1.0f = border 100% of canvas size)
 *******************************************************************/
void GfxCanvas::zoomToFit(bool mag, float padding) {
	// Determine padding
	double pad = (double)min(GetSize().x, GetSize().y) * padding;

	// Get image dimensions
	double x_dim = (double)image->getWidth();
	double y_dim = (double)image->getHeight();

	// Get max scale for x and y (including padding)
	double x_scale = ((double)GetSize().x - pad) / x_dim;
	double y_scale = ((double)GetSize().y - pad) / y_dim;

	// Set scale to smallest of the 2 (so that none of the image will be clipped)
	scale = min(x_scale, y_scale);

	// If we don't want to magnify the image, clamp scale to a max of 1.0
	if (!mag && scale > 1)
		scale = 1;
}

/* GfxCanvas::onImage
 * Returns true if the given coordinates are 'on' top of the image
 *******************************************************************/
bool GfxCanvas::onImage(int x, int y) {
	if (view_type == GFXVIEW_DEFAULT || view_type == GFXVIEW_TILED)
		return false;

	int left = GetSize().x * 0.5;
	int top = GetSize().y * 0.5;

	if (view_type == GFXVIEW_CENTERED) {
		left -= (double)image->getWidth() * 0.5 * scale;
		top -= (double)image->getHeight() * 0.5 * scale;
	}
	else if (view_type == GFXVIEW_SPRITE) {
		left -= image->offset().x * scale;
		top -= image->offset().y * scale;
	}
	else if (view_type == GFXVIEW_HUD) {
		left -= 160 * scale;
		top -= 100 * scale;
		left -= image->offset().x * scale;
		top -= image->offset().y * scale;
	}

	int right = left + image->getWidth() * scale;
	int bottom = top + image->getHeight() * scale;

	return (x >= left && x <= right && y >= top && y <= bottom);
}

/* GfxCanvas::endOffsetDrag
 * Finishes an offset drag
 *******************************************************************/
void GfxCanvas::endOffsetDrag() {
	// Get offset
	int x = (drag_pos.x - drag_origin.x) / scale;
	int y = (drag_pos.y - drag_origin.y) / scale;

	// If there was a change
	if (x != 0 || y != 0) {
		// Set image offsets
		image->setXOffset(image->offset().x - x);
		image->setYOffset(image->offset().y - y);
		
		// Generate event
		wxNotifyEvent e(wxEVT_GFXCANVAS_OFFSET_CHANGED, GetId());
		e.SetEventObject(this);
		GetEventHandler()->ProcessEvent(e);
	}

	// Stop drag
	drag_origin.set(-1, -1);
}

/* GfxCanvas::onAnnouncement
 * Called when an announcement is recieved from the image that this
 * GfxCanvas is displaying
 *******************************************************************/
void GfxCanvas::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	if (announcer == image && event_name.Cmp(_T("image_changed")) == 0)
		update_texture = true;
}


/*******************************************************************
 * GFXCANVAS EVENTS
 *******************************************************************/

/* GfxCanvas::onMouseLeftDown
 * Called when the left button is pressed within the canvas
 *******************************************************************/
void GfxCanvas::onMouseLeftDown(wxMouseEvent& e) {
	int x = e.GetPosition().x;
	int y = e.GetPosition().y;
	bool on_image = onImage(x, y);

	// Left mouse down
	if (e.LeftDown()) {
		// Begin drag if mouse is over image and dragging allowed
		if (on_image && allow_drag) {
			drag_origin.set(x, y);
			drag_pos.set(x, y);
			Refresh();
		}
	}

	e.Skip();
}

/* GfxCanvas::onMouseLeftUp
 * Called when the left button is released within the canvas
 *******************************************************************/
void GfxCanvas::onMouseLeftUp(wxMouseEvent& e) {
	// Stop dragging
	if (drag_origin.x >= 0) {
		endOffsetDrag();
		Refresh();
	}
}

/* GfxCanvas::onMouseMovement
 * Called when the mouse pointer is moved within the canvas
 *******************************************************************/
void GfxCanvas::onMouseMovement(wxMouseEvent& e) {
	bool refresh = false;

	// Check if the mouse is over the image
	bool on_image = onImage(e.GetPosition().x, e.GetPosition().y);
	if (on_image != image_hilight) {
		image_hilight = on_image;
		refresh = true;
	}

	// Drag
	if (e.LeftIsDown()) {
		drag_pos.set(e.GetPosition().x, e.GetPosition().y);
		refresh = true;
	}
	else if (e.RightIsDown()) {
		offset = offset + point2_t(e.GetPosition().x - mouse_prev.x, e.GetPosition().y - mouse_prev.y);
		refresh = true;
	}

	if (refresh)
		Refresh();

	mouse_prev.set(e.GetPosition().x, e.GetPosition().y);
}

/* GfxCanvas::onMouseLeaving
 * Called when the mouse pointer leaves the gfx canvas
 *******************************************************************/
void GfxCanvas::onMouseLeaving(wxMouseEvent& e) {
	image_hilight = false;
	Refresh();
}

/* GfxCanvas::onKeyDown
 * Called when a key is pressed while the canvas has focus
 *******************************************************************/
void GfxCanvas::onKeyDown(wxKeyEvent& e) {
	if (e.GetKeyCode() == WXK_UP) {
		offset.y += 8;
		Refresh();
	}

	else if (e.GetKeyCode() == WXK_DOWN) {
		offset.y -= 8;
		Refresh();
	}

	else if (e.GetKeyCode() == WXK_LEFT) {
		offset.x += 8;
		Refresh();
	}

	else if (e.GetKeyCode() == WXK_RIGHT) {
		offset.x -= 8;
		Refresh();
	}

	else
		e.Skip();
}
