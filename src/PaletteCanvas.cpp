

#include "Main.h"
#include "WxStuff.h"
#include "PaletteCanvas.h"

PaletteCanvas::PaletteCanvas(wxWindow* parent, int id)
: OGLCanvas(parent, id) {
}

PaletteCanvas::~PaletteCanvas() {
}

/* PaletteCanvas::setContext
 * Sets the current gl context to the canvas' context, and creates
 * it if it doesn't exist. Returns true if the context is valid,
 * false otherwise
 *******************************************************************/
bool PaletteCanvas::setContext() {
	if (!context) {
		if (IsShown())
			context = new wxGLContext(this);
	}

	if (context) {
		context->SetCurrent(*this);
		SetCurrent(*context);
		return true;
	}
	else
		return false;
}

void PaletteCanvas::draw() {
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

	// Setup some variables
	float size = float(min(GetSize().x, GetSize().y)) / 16.0f;

	// Scale to size
	glScalef(size, size, 1.0f);

	// Draw palette
	int c = 0;
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			// Set colour
			palette.colour(c++).set_gl();

			// Draw square
			glBegin(GL_QUADS);
			glVertex2d(x, y);
			glVertex2d(x, y+1);
			glVertex2d(x+1, y+1);
			glVertex2d(x+1, y);
			glEnd();
		}
	}

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}
