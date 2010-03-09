
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
#include "SplashWindow.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
namespace OpenGL {
	wxGLContext*	context = NULL;
	double			version = 0;
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
			wxLogMessage(_T("Can't create global GL context, wxGLCanvas is hidden"));
	}

	return context;
}

/* OpenGL::init
 * Initialises general OpenGL variables and settings
 *******************************************************************/
bool OpenGL::init() {
	// Get OpenGL version
	string glvers = wxString::From8BitData((const char*)glGetString(GL_VERSION));
	glvers.Truncate(3);
	glvers.ToDouble(&version);
	wxLogMessage(_T("GL Version: %s (%1.2f)"), glvers.c_str(), version);

	return true;
}

/* OpenGL::np2TexSupport
 * Returns true if the installed OpenGL version supports non-power-
 * of-two textures, false otherwise
 *******************************************************************/
bool OpenGL::np2TexSupport() {
	return version >= 2;
}
