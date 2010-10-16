
#include "Main.h"
#include "BrowserItem.h"
#include "SFont.h"

BrowserItem::BrowserItem(string name, unsigned index) {
	this->name = name;
	this->index = index;
}

BrowserItem::~BrowserItem() {
}

bool BrowserItem::loadImage() {
	return false;
}

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

void BrowserItem::clearImage() {
	image.clear();
}
