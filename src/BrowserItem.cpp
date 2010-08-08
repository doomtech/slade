
#include "Main.h"
#include "BrowserItem.h"

BrowserItem::BrowserItem(string name) {
	this->name = name;
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
		glColor3f(1, 0, 0);

		// Outline
		glBegin(GL_LINE_LOOP);
		glVertex2d(0, 0);
		glVertex2d(0, size);
		glVertex2d(size, size);
		glVertex2d(size, 0);
		glEnd();

		// X
		glBegin(GL_LINES);
		glVertex2d(0, 0);
		glVertex2d(size, size);
		glVertex2d(0, size);
		glVertex2d(size, 0);
		glEnd();

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
	glTexCoord2f(0.0f, 0.0f);	glVertex2d(top, left);
	glTexCoord2f(0.0f, 1.0f);	glVertex2d(top + height, left);
	glTexCoord2f(1.0f, 1.0f);	glVertex2d(top + height, left + width);
	glTexCoord2f(1.0f, 0.0f);	glVertex2d(top, left + width);
	glEnd();
}
