
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Math.cpp
 * Description: Contains various useful math related functions
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
#include "MathStuff.h"


/*******************************************************************
 * MATH NAMESPACE FUNCTIONS
 *******************************************************************/

/* MathStuff::clamp
 * Clamps [val] to be between [min] and [max]
 *******************************************************************/
double MathStuff::clamp(double val, double min, double max) {
	if (val < min) val = min;
	if (val > max) val = max;
	return val;
}

/* MathStuff::floor
 * Returns the integral floor of [val]
 *******************************************************************/
int MathStuff::floor(double val) {
	if (val >= 0)
		return (int)val;
	else
		return (int)val - 1;
}

/* MathStuff::ceil
 * Returns the integral ceiling of [val]
 *******************************************************************/
int MathStuff::ceil(double val) {
	if (val > 0)
		return (int)val + 1;
	else
		return (int)val;
}

/* MathStuff::round
 * Returns the closest integral value of [val]
 *******************************************************************/
int MathStuff::round(double val) {
	int ret = (int)val;
	if ((val - (double)ret) >= 0.5) ret++;
	return ret;
}

/* MathStuff::distance
 * Returns the distance between [x1,y1] and [x2,y2]
 *******************************************************************/
double MathStuff::distance(double x1, double y1, double x2, double y2) {
	return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

/* MathStuff::lineSide
 * Returns the side of the line from [x1,y1] to [x2,y2] that the
 * point at [x,y] lies on. Positive is front, negative is back, zero
 * is on the line
 *******************************************************************/
double MathStuff::lineSide(double x, double y, double x1, double y1, double x2, double y2) {
	return -((y-y1)*(x2-x1) - (x-x1)*(y2-y1));
}

/* MathStuff::distanceToLine
 * Returns the shortest distance between the point at [x,y] and the
 * line from [x1,y1] to [x2,y2]
 *******************************************************************/
double MathStuff::distanceToLine(double x, double y, double x1, double y1, double x2, double y2) {
	// Get line length
	double len = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));

	// Calculate intersection distance
	double u = 0;
	if (len > 0) {
		u = ((x-x1)*(x2-x1) + (y-y1)*(y2-y1)) / (len*len);

		// Limit intersection distance to the line
		double lbound = 1 / len;
		if(u < lbound) u = lbound;
		if(u > (1.0-lbound)) u = 1.0-lbound;
	}

	// Calculate intersection point
	double ix = x1 + u*(x2 - x1);
	double iy = y1 + u*(y2 - y1);

	// Return distance between intersection and point
	// which is the shortest distance to the line
	return sqrt((ix-x)*(ix-x) + (iy-y)*(iy-y));
}

/* MathStuff::linesIntersect
 * Checks for an intersection between two lines [l1x1,l1y1]-[l1x2,l1y2]
 * and [l2x1,l2y1]-[l2x2,l2y2]. Returns true if they intersect and
 * sets [x,y] to the intersection point
 *******************************************************************/
bool MathStuff::linesIntersect(double l1x1, double l1y1, double l1x2, double l1y2,
								double l2x1, double l2y1, double l2x2, double l2y2,
								double& x, double& y) {
	// First, simple check for two parallel horizontal or vertical lines
	if ((l1x1 == l1x2 && l2x1 == l2x2) || (l1y1 == l1y2 && l2y1 == l2y2))
		return false;

	// Second, check if the lines share any endpoints
	if ((l1x1 == l2x1 && l1y1 == l2y1) ||
		(l1x2 == l2x2 && l1y2 == l2y2) ||
		(l1x1 == l2x2 && l1y1 == l2y2) ||
		(l1x2 == l2x1 && l1y2 == l2y1))
		return false;

	// Third, check for two perpendicular horizontal or vertical lines
	if (l1x1 == l1x2 && l2y1 == l2y2) {
		x = l1x1;
		y = l2y1;
		return true;
	}
	if (l1y1 == l1y2 && l2x1 == l2x2) {
		x = l2x1;
		y = l1y1;
		return true;
	}


	// Not a simple case, do full intersection calculation

	// Calculate some values
	double a1 = l1y2 - l1y1;
	double a2 = l2y2 - l2y1;
	double b1 = l1x1 - l1x2;
	double b2 = l2x1 - l2x2;
	double c1 = (a1 * l1x1) + (b1 * l1y1);
	double c2 = (a2 * l2x1) + (b2 * l2y1);
	double det = a1*b2 - a2*b1;

	// Check for no intersection
	if (det == 0)
		return false;

	// Calculate intersection point
	x = (b2*c1 - b1*c2) / det;
	y = (a1*c2 - a2*c1) / det;

	// Check that the intersection point is on both lines
	if ((min(l1x1, l1x2) < x && x < max(l1x2, l1x2)) &&
		(min(l1y1, l1y2) < y && y < max(l1y1, l1y2)) &&
		(min(l2x1, l2x2) < x && x < max(l2x1, l2x2)) &&
		(min(l2y1, l2y2) < y && y < max(l2y1, l2y2)))
		return true;

	// Intersection point does not lie on both lines
	return false;
}
