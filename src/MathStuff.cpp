
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MathStuff.cpp
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
#include "Console.h"


/*******************************************************************
 * MATHSTUFF NAMESPACE VARIABLES
 *******************************************************************/
namespace MathStuff {
	double rad2deg = 180 / PI;
}


/*******************************************************************
 * MATHSTUFF NAMESPACE FUNCTIONS
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

fpoint2_t MathStuff::closestPointOnLine(double x, double y, double x1, double y1, double x2, double y2) {
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

	// Return intersection point
	return fpoint2_t(x1 + u*(x2 - x1), y1 + u*(y2 - y1));
}

/* MathStuff::distanceToLine
 * Returns the shortest distance between the point at [x,y] and the
 * line from [x1,y1] to [x2,y2]
 *******************************************************************/
double MathStuff::distanceToLine(double x, double y, double x1, double y1, double x2, double y2) {
	// Calculate intersection point
	fpoint2_t i = closestPointOnLine(x, y, x1, y1, x2, y2);

	// Return distance between intersection and point
	// which is the shortest distance to the line
	return sqrt((i.x-x)*(i.x-x) + (i.y-y)*(i.y-y));
}

/* MathStuff::distanceToLineFast
 * Returns the shortest 'distance' between the point at [x,y] and the
 * line from [x1,y1] to [x2,y2]. The distance returned isn't the
 * real distance, but can be used to find the 'closest' line to the
 * point
 *******************************************************************/
double MathStuff::distanceToLineFast(double x, double y, double x1, double y1, double x2, double y2) {
	// Calculate intersection point
	fpoint2_t i = closestPointOnLine(x, y, x1, y1, x2, y2);

	// Return fast distance between intersection and point
	// which is the shortest distance to the line
	return (i.x-x)*(i.x-x) + (i.y-y)*(i.y-y);
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

	// Third, check bounding boxes
	if (max(l1x1, l1x2) < min(l2x1, l2x2) ||
		max(l2x1, l2x2) < min(l1x1, l1x2) ||
		max(l1y1, l1y2) < min(l2y1, l2y2) ||
		max(l2y1, l2y2) < min(l1y1, l1y2))
		return false;

	// Fourth, check for two perpendicular horizontal or vertical lines
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
	if (min(l1x1, l1x2) <= x && x <= max(l1x1, l1x2) &&
		min(l1y1, l1y2) <= y && y <= max(l1y1, l1y2) &&
		min(l2x1, l2x2) <= x && x <= max(l2x1, l2x2) &&
		min(l2y1, l2y2) <= y && y <= max(l2y1, l2y2))
		return true;

	// Intersection point does not lie on both lines
	return false;
}

double MathStuff::distanceRayLine(fpoint2_t ray_origin, fpoint2_t ray2, double x1, double y1, double x2, double y2) {
	//fpoint2_t ray2 = ray_origin+ray_dir;

	double u_ray = ((x2 - x1) * (ray_origin.y - y1) - (y2 - y1) * (ray_origin.x - x1)) /
				  ((y2 - y1) * (ray2.x - ray_origin.x) - (x2 - x1) * (ray2.y - ray_origin.y));
	double u_line = ((ray2.x - ray_origin.x) * (ray_origin.y - y1) - (ray2.y - ray_origin.y) * (ray_origin.x - x1)) /
				   ((y2 - y1) * (ray2.x - ray_origin.x) - (x2 - x1) * (ray2.y - ray_origin.y));

	if (u_ray >= 0 && u_line >= 0 && u_line <= 1)
		return u_ray;
	else
		return -1;
}

double MathStuff::angle2DRad(fpoint2_t p1, fpoint2_t p2, fpoint2_t p3) {
	/*
	double dot = (p1.x - p2.x) * (p3.x - p2.x) + (p1.y - p2.y) * (p3.y - p2.y);
	double cross = (p1.x - p2.x) * (p3.y - p2.y) - (p1.y - p2.y) * (p3.x - p2.x);

	if (dot == 0 && cross == 0)
		return 0;
	else {
		double angle = atan2(dot, cross);
		//if (angle < 0) angle += (2*PI);
		return angle*rad2deg;
	}
	*/

	// From: http://stackoverflow.com/questions/3486172/angle-between-3-points
	// modified not to bother converting to degrees
	fpoint2_t ab(p2.x - p1.x, p2.y - p1.y);
	fpoint2_t cb(p2.x - p3.x, p2.y - p3.y);

	// dot product
	double dot = (ab.x * cb.x + ab.y * cb.y);

	// length square of both vectors
	double abSqr = ab.x * ab.x + ab.y * ab.y;
	double cbSqr = cb.x * cb.x + cb.y * cb.y;

	// square of cosine of the needed angle
	double cosSqr = dot * dot / abSqr / cbSqr;

	// this is a known trigonometric equality:
	// cos(alpha * 2) = [ cos(alpha) ]^2 * 2 - 1
	double cos2 = 2 * cosSqr - 1;

	// Here's the only invocation of the heavy function.
	// It's a good idea to check explicitly if cos2 is within [-1 .. 1] range
	double alpha2 =
		(cos2 <= -1) ? PI :
		(cos2 >= 1) ? 0 :
		acosf(cos2);

	//double rslt = alpha2 / 2;
	//double rs = rslt * rad2deg;
	double rs = alpha2 / 2;

	// Now revolve the ambiguities.
	// 1. If dot product of two vectors is negative - the angle is definitely
	// above 90 degrees. Still we have no information regarding the sign of the angle.

	// NOTE: This ambiguity is the consequence of our method: calculating the cosine
	// of the double angle. This allows us to get rid of calling sqrt.
	if (dot < 0)
		rs = PI - rs;

	// 2. Determine the sign. For this we'll use the Determinant of two vectors.
	double det = (ab.x * cb.y - ab.y * cb.x);
	if (det < 0)
		rs = (2*PI) - rs;

	return rs;
}

fpoint2_t MathStuff::rotatePoint(fpoint2_t origin, fpoint2_t point, double angle) {
	// Translate to origin
	double x = point.x - origin.x;
	double y = point.y - origin.y;

	// Maths yay
	double rot = (PI * 2.0) * ((360.0 - angle) / 360.0);
	double srot = sin(rot);
	double crot = cos(rot);
	x = crot * x - srot * y;
	y = srot * x + crot * y;

	// Return rotated point
	return fpoint2_t(x + origin.x, y + origin.y);
}



CONSOLE_COMMAND(angle2d, 6) {
	double vals[6];
	for (unsigned a = 0; a < args.size(); a++) {
		args[a].ToDouble(&vals[a]);
	}

	double ang = MathStuff::angle2DRad(fpoint2_t(vals[0], vals[1]), fpoint2_t(vals[2], vals[3]), fpoint2_t(vals[4], vals[5]));
	wxLogMessage("Angle = %1.4f", ang);
}
