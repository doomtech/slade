
#include "Main.h"
#include "WxStuff.h"
#include "SplashWindow.h"

SplashWindow* SplashWindow::instance = NULL;

SplashCanvas::SplashCanvas(wxWindow* parent)
: OGLCanvas(parent, -1) {
}

SplashCanvas::~SplashCanvas() {
}

void SplashCanvas::draw() {
	// Setup the viewport
	glViewport(0, 0, GetClientSize().x, GetClientSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(0.6f, 0.6f, 0.55f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// DRAWING HERE YO

	// Swap buffers (ie show what was drawn)
	SwapBuffers();
}



SplashWindow::SplashWindow()
: wxMiniFrame(NULL, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP) {
	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Add logo
	gl_splash = new SplashCanvas(this);
	//gl_splash->SetSizeHints(wxSize(300, 200));
	sizer->Add(gl_splash, 1, wxEXPAND);
	
	Layout();
}

SplashWindow::~SplashWindow() {
}

void SplashWindow::showSplash(string message) {
	// Show & init window
	Show(true);
	//SetInitialSize(wxSize(300, -1));
	Layout();
	Centre();
	Refresh();
}
