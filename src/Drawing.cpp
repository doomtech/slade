
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Drawing.cpp
 * Description: Various OpenGL drawing functions
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
#include "Drawing.h"


/*******************************************************************
 * FUNCTIONS
 *******************************************************************/

/* Drawing::drawLine
 * Draws a line from [start] to [end]
 *******************************************************************/
void Drawing::drawLine(fpoint2_t start, fpoint2_t end) {
	glBegin(GL_LINES);
	glVertex2d(start.x, start.y);
	glVertex2d(end.x, end.y);
	glEnd();
}

/* Drawing::drawLine
 * Draws a line from [x1,y1] to [x2,y2]
 *******************************************************************/
void Drawing::drawLine(double x1, double y1, double x2, double y2) {
	glBegin(GL_LINES);
	glVertex2d(x1, y1);
	glVertex2d(x2, y2);
	glEnd();
}

/* Drawing::drawRect
 * Draws a rectangle from [tl] to [br]
 *******************************************************************/
void Drawing::drawRect(fpoint2_t tl, fpoint2_t br) {
	glBegin(GL_LINE_LOOP);
	glVertex2d(tl.x, tl.y);
	glVertex2d(tl.x, br.y);
	glVertex2d(br.x, br.y);
	glVertex2d(br.x, tl.y);
	glEnd();
}

/* Drawing::drawRect
 * Draws a rectangle from [x1,y1] to [x2,y2]
 *******************************************************************/
void Drawing::drawRect(double x1, double y1, double x2, double y2) {
	glBegin(GL_LINE_LOOP);
	glVertex2d(x1, y1);
	glVertex2d(x1, y2);
	glVertex2d(x2, y2);
	glVertex2d(x2, y1);
	glEnd();
}

/* Drawing::drawHud
 * Draws doom hud offset guide lines, from the center
 *******************************************************************/
void Drawing::drawHud(bool statusbar, bool center, bool wide) {
	// Determine some variables
	int hw = 160;
	int hh = 100;
	if (wide) hw = 176;

	// Draw 320x200 screen outline
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glLineWidth(1.5f);
	drawRect(-hw, -hh, hw, hh);

	// Draw statusbar line if needed
	glLineWidth(1.0f);
	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	if (statusbar)
		drawLine(-hw, 68, hw, 68);

	// Draw center lines if needed
	if (center) {
		drawLine(-hw, 0, hw, 0);
		drawLine(0, -hh, 0, hh);
	}

	// Draw normal screen edge guides if widescreen
	if (wide) {
		drawLine(-160, -100, -160, 100);
		drawLine(160, -100, 160, 100);
	}
}
