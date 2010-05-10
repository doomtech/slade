
#include "Main.h"
#include "WxStuff.h"
#include "BrowserCanvas.h"

BrowserCanvas::BrowserCanvas(wxWindow* parent) : OGLCanvas(parent, -1) {
}

BrowserCanvas::~BrowserCanvas() {
}

void BrowserCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Draw items
	int x = 0;
	int y = 0;

	for (size_t a = 0; a < items.size(); a++) {
		// Check if we need to go to the next line
		if (x + 64 > GetSize().x) {
			x = 0;
			y += 64;
		}

		// Temp
		glColor3f(1.0f, 1.0f, 1.0f);
		items[a].texture.bind();
		glBegin(GL_QUADS);
		glVertex2d(x + 4, y + 4);
		glVertex2d(x + 4, y + 60);
		glVertex2d(x + 60, y + 60);
		glVertex2d(x + 60, y + 4);
		glEnd();

		// Next item
		x += 64;
	}

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}
