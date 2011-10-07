
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    OpenGL.cpp
 * Description: OpenGL management stuff
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
#include "OpenGL.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
CVAR(Bool, gl_tex_enable_np2, true, CVAR_SAVE)

namespace OpenGL {
	wxGLContext*	context = NULL;
	bool			initialised = false;
	double			version = 0;
	unsigned		max_tex_size = 128;
	unsigned		pow_two[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };
	uint8_t			n_pow_two = 16;
	float			max_point_size = -1.0f;
}


/*******************************************************************
 * OPENGL NAMESPACE FUNCTIONS
 *******************************************************************/

/* OpenGL::getContext
 * Returns the global OpenGL context, and creates it if needed
 *******************************************************************/
wxGLContext* OpenGL::getContext(wxGLCanvas* canvas) {
	if (!context) {
		if (canvas->IsShown()) {
			context = new wxGLContext(canvas);
			context->SetCurrent(*canvas);
			init();
		}
		else
			wxLogMessage("Can't create global GL context, wxGLCanvas is hidden");
	}

	return context;
}

/* OpenGL::init
 * Initialises general OpenGL variables and settings
 *******************************************************************/
bool OpenGL::init() {
	if (initialised)
		return true;

	wxLogMessage("Initialising OpenGL...");

	// Get OpenGL version
	string temp = wxString::From8BitData((const char*)glGetString(GL_VERSION));
	temp.Truncate(3);
	temp.ToDouble(&version);
	wxLogMessage("OpenGL Version: %1.1f", version);

	// Get max texture size
	GLint val = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val);
	max_tex_size = val;
	wxLogMessage("Max Texture Size: %dx%d", max_tex_size, max_tex_size);

	// Initialise GLEW
	glewInit();

	initialised = true;
	return true;
}

/* OpenGL::np2TexSupport
 * Returns true if the installed OpenGL version supports non-power-
 * of-two textures, false otherwise
 *******************************************************************/
bool OpenGL::np2TexSupport() {
	return version >= 2 && gl_tex_enable_np2;
}

/* OpenGL::validTexDimension
 * Returns true if [dim] is a valid texture dimension on the system
 * OpenGL version
 *******************************************************************/
bool OpenGL::validTexDimension(unsigned dim) {
	if (dim > max_tex_size)
		return false;
	else if (!np2TexSupport()) {
		for (uint8_t a = 0; a < n_pow_two; a++) {
			if (dim == pow_two[a])
				return true;
		}

		return false;
	}
	else
		return true;
}

float OpenGL::maxPointSize() {
	if (max_point_size < 0) {
		GLfloat sizes[2];
		glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, sizes);
		max_point_size = sizes[1];
	}

	return max_point_size;
}
