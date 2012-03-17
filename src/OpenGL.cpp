
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
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
CVAR(Bool, gl_point_sprite, true, CVAR_SAVE)
CVAR(Bool, gl_tweak_accuracy, false, CVAR_SAVE)

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

	// Test extensions
	wxLogMessage("Checking extensions...");
	if (GLEW_ARB_vertex_buffer_object)
		wxLogMessage("Vertex Buffer Objects supported");
	else
		wxLogMessage("Vertex Buffer Objects not supported");
	if (GLEW_ARB_point_sprite)
		wxLogMessage("Point Sprites supported");
	else
		wxLogMessage("Point Sprites not supported");
	if (GLEW_ARB_framebuffer_object)
		wxLogMessage("Framebuffer Objects supported");
	else
		wxLogMessage("Framebuffer Objects not supported");

	initialised = true;
	return true;
}

/* OpenGL::np2TexSupport
 * Returns true if the installed OpenGL version supports non-power-
 * of-two textures, false otherwise
 *******************************************************************/
bool OpenGL::np2TexSupport() {
	return GLEW_ARB_texture_non_power_of_two && gl_tex_enable_np2;
}

/* OpenGL::np2TexSupport
 * Returns true if the installed OpenGL version supports point
 * sprites, false otherwise
 *******************************************************************/
bool OpenGL::pointSpriteSupport() {
	return GLEW_ARB_point_sprite && gl_point_sprite;
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

/* OpenGL::maxPointSize
 * Returns the implementation-dependant maximum size for GL_POINTS
 *******************************************************************/
float OpenGL::maxPointSize() {
	if (max_point_size < 0) {
		GLfloat sizes[2];
		glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, sizes);
		max_point_size = sizes[1];
		//wxLogMessage("Max GL point size %1.2f", max_point_size);
	}

	return max_point_size;
}

/* OpenGL::maxTextureSize
 * Returns the maximum texture size
 *******************************************************************/
unsigned OpenGL::maxTextureSize() {
	return max_tex_size;
}

/* OpenGL::isInitialised
 * Returns true if OpenGL has been initialised
 *******************************************************************/
bool OpenGL::isInitialised() {
	return initialised;
}

/* OpenGL::accuracyTweak
 * Returns true if the 'accuracy tweak' is enabled. This can fix
 * inaccuracies when rendering 2d textures, but tends to cause fonts
 * to blur when using FTGL
 *******************************************************************/
bool OpenGL::accuracyTweak() {
	return gl_tweak_accuracy;
}
