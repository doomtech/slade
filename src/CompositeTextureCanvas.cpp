
#include "Main.h"
#include "CompositeTextureCanvas.h"

CompositeTextureCanvas::CompositeTextureCanvas(wxWindow* parent, int id)
	: OGLCanvas(parent, id) {
}

CompositeTextureCanvas::~CompositeTextureCanvas() {
}

bool CompositeTextureCanvas::openTexture(CompositeTexture* tex) {
	texture = tex;

	return true;
}

void CompositeTextureCanvas::draw() {
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

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}

/* GfxCanvas::drawChequeredBackground
 * Fills the canvas with a chequered pattern (used as the
 * 'background' - to indicate transparency)
 *******************************************************************/
void CompositeTextureCanvas::drawChequeredBackground() {
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
