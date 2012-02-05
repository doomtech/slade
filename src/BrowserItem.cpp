
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    BrowserItem.cpp
 * Description: A class representing a single browser item. Each
 *              item has a name, index and image associated with
 *              it, and handles drawing itself.
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
#include "BrowserItem.h"
#include "SFont.h"


/*******************************************************************
 * BROWSERITEM CLASS FUNCTIONS
 *******************************************************************/

/* BrowserItem::BrowserItem
 * BrowserItem class constructor
 *******************************************************************/
BrowserItem::BrowserItem(string name, unsigned index, string type) {
	this->name = name;
	this->index = index;
	this->type = type;
}

/* BrowserItem::~BrowserItem
 * BrowserItem class destructor
 *******************************************************************/
BrowserItem::~BrowserItem() {
}

/* BrowserItem::loadImage
 * Loads the item image (base class does nothing, must be overridden
 * by child classes to be useful at all)
 *******************************************************************/
bool BrowserItem::loadImage() {
	return false;
}

/* BrowserItem::draw
 * Draws the item in a [size]x[size] box, keeping the correct aspect
 * ratio of it's image
 *******************************************************************/
void BrowserItem::draw(int size) {
	// Try to load image if it isn't already
	if (!image.isLoaded())
		loadImage();

	// If it still isn't just draw a red box with an X
	if (!image.isLoaded()) {
		glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT);

		glColor3f(1, 0, 0);
		glDisable(GL_TEXTURE_2D);

		// Outline
		glBegin(GL_LINE_LOOP);
		glVertex2i(0, 0);
		glVertex2i(0, size);
		glVertex2i(size, size);
		glVertex2i(size, 0);
		glEnd();

		// X
		glBegin(GL_LINES);
		glVertex2i(0, 0);
		glVertex2i(size, size);
		glVertex2i(0, size);
		glVertex2i(size, 0);
		glEnd();

		// Item name
		glEnable(GL_TEXTURE_2D);
		SFont& fnt = SFont::vgaFont();
		glPushMatrix();
		glTranslated(size*0.5 + 1, size - fnt.lineHeight()*0.5 + 1, 0);
		fnt.drawString(name, COL_BLACK, SF_ALIGN_CENTER);
		glTranslated(-1, -1, 0);
		fnt.drawString(name, COL_WHITE, SF_ALIGN_CENTER);
		glPopMatrix();

		glPopAttrib();

		return;
	}

	// Determine texture dimensions
	double width = image.getWidth();
	double height = image.getHeight();
	if (width > height) {
		// Scale down by width
		if (width > size) {
			double scale = (double)size / width;
			width *= scale;
			height *= scale;
		}
	}
	else {
		// Scale down by height
		if (height > size) {
			double scale = (double)size / height;
			width *= scale;
			height *= scale;
		}
	}

	// Determine draw coords
	double top = ((double)size * 0.5) - (height * 0.5);
	double left = ((double)size * 0.5) - (width * 0.5);

	// Draw
	image.bind();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(left, top);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(left, top + height);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(left + width, top + height);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(left + width, top);
	glEnd();

	// Item name
	SFont& fnt = SFont::vgaFont();
	glPushMatrix();
	glTranslated(size*0.5 + 1, size - fnt.lineHeight()*0.5 + 1, 0);
	fnt.drawString(name, COL_BLACK, SF_ALIGN_CENTER);
	glTranslated(-1, -1, 0);
	fnt.drawString(name, COL_WHITE, SF_ALIGN_CENTER);
	glPopMatrix();
}

/* BrowserItem::clearImage
 * Clears the item image
 *******************************************************************/
void BrowserItem::clearImage() {
	image.clear();
}
