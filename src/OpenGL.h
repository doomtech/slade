
#ifndef __OPENGL_H__
#define __OPENGL_H__

#include <wx/glcanvas.h>

namespace OpenGL {
	wxGLContext*	getContext(wxGLCanvas* canvas);
	bool			init();
	bool			np2TexSupport();
	bool			validTexDimension(unsigned dim);
}

#endif//__OPENGL_H__
