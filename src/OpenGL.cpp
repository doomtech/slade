
#include "Main.h"
#include "OpenGL.h"

namespace OpenGL {
	wxGLContext*	context = NULL;
	double			version = 0;
}


wxGLContext* OpenGL::getContext(wxGLCanvas* canvas) {
	if (!context) {
		if (canvas->IsShown()) {
			context = new wxGLContext(canvas);
			init();
		}
	}

	return context;
}

bool OpenGL::init() {
	return true;
}

bool OpenGL::np2TexSupport() {
	return version > 2;
}
