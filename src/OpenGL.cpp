
#include "Main.h"
#include "OpenGL.h"
#include "SplashWindow.h"

namespace OpenGL {
	wxGLContext*	context = NULL;
	double			version = 0;
}


wxGLContext* OpenGL::getContext(wxGLCanvas* canvas) {
	if (!context) {
		if (canvas->IsShown()) {
			context = new wxGLContext(canvas);
			context->SetCurrent(*canvas);
			init();
		}
		else
			wxLogMessage(_T("Can't create global GL context, wxGLCanvas is hidden"));
	}

	return context;
}

bool OpenGL::init() {
	string glvers = wxString::From8BitData((const char*)glGetString(GL_VERSION));
	wxLogMessage(_T("OpenGL version: %s"), glvers.c_str());

	return true;
}

bool OpenGL::np2TexSupport() {
	return version > 2;
}
