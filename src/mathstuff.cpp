
#include "main.h"
#include "mathstuff.h"

// difference: Finds the difference between 2 numbers
// ----------------------------------------------- >>
int difference(int v1, int v2)
{
	return max(v1, v2) - min(v1, v2);
}

// distance: Returns the distance between two points
// ---------------------------------------------- >>
double distance(double x1, double y1, double x2, double y2)
{
	double dist_x = x2 - x1;
	double dist_y = y2 - y1;

	return sqrt(dist_x * dist_x + dist_y * dist_y);
}

// midpoint: Returns the mid point of a line between two points
// --------------------------------------------------------- >>
point2_t midpoint(point2_t p1, point2_t p2)
{
	int mid_x = min(p1.x, p2.x) + (difference(p1.x, p2.x) / 2);
	int mid_y = min(p1.y, p2.y) + (difference(p1.y, p2.y) / 2);

	return point2_t(mid_x, mid_y);
}

// determine_line_side: Determines what side of a line a certain point is on
// (returns the actual value so we can test for being exactly on the line)
// ------------------------------------------------------------------------ >>
float determine_line_side(rect_t r, float x, float y)
{
	r.tl.y = -r.tl.y;
	r.br.y = -r.br.y;

	return (-y - (float)r.y1()) * float(r.x2() - r.x1()) - (x - (float)r.x1()) * float(r.y2() - r.y1());
}

// determine_line_side: Determines what side of a line a certain point is on
// (returns the actual value so we can test for being exactly on the line)
// ------------------------------------------------------------------------ >>
float determine_line_side(float x1, float y1, float x2, float y2, float x, float y)
{
	return (y - y1) * (x2 - x1) - (x - x1) * (y2 - y1);
}

// distance_to_line: Finds the shortest distance from a vertex to a line
// ------------------------------------------------------------------ >>
// (code borrowed & slightly modified from Doom Builder, by CodeImp)
double distance_to_line(int x1, int y1, int x2, int y2, double vx, double vy)
{
	double u;

	// Get length of linedef
	double ld = distance(x1, y1, x2, y2);

	// Check if not zero length
	if(ld)
	{
		// Get the intersection offset
		u = double((vx - x1) * (x2 - x1) + (vy - y1) * (y2 - y1)) / (ld * ld);

		// Limit intersection offset to the line
		double lbound = 1 / ld;
		double ubound = 1 - lbound;
		if(u < lbound) u = lbound;
		if(u > ubound) u = ubound;
	}
	else
	{
		u = 0;
	}

	// Calculate intersection point
	double ix = double(x1) + u * double(x2 - x1);
	double iy = double(y1) + u * double(y2 - y1);

	// Return distance between intersection and point
	// which is the shortest distance to the line
	return distance(double(vx), double(vy), ix, iy);
}

// point_in_rect: Determines if a point is within a rectangle
// ------------------------------------------------------- >>
bool point_in_rect(int x1, int y1, int x2, int y2, int x, int y)
{
	return (x >= x1) && (x <= x2) && (y >= y1) && (y <= y2);
}

bool rects_intersect(rect_t r1, rect_t r2)
{
	return !(r1.left() > r2.right() || r1.right() < r2.left() ||
		r1.top() > r2.bottom() || r1.bottom() < r2.top());
}

string test(rect_t line)
{
	return s_fmt(_T("(%d, %d) to (%d, %d)"), line.x1(), line.y1(), line.x2(), line.y2());
}


// lines_intersect: Checks if 2 lines intersect eachother
// --------------------------------------------------- >>
bool lines_intersect(rect_t line1, rect_t line2, fpoint2_t *ip)
{
	/*
	double a1 = line1.y2() - line1.y1();
	double a2 = line2.y2() - line2.y1();
	double b1 = line1.x1() - line1.x2();
	double b2 = line2.x1() - line2.x2();
	double c1 = (a1 * line1.x1()) + (b1 * line1.y1());
	double c2 = (a2 * line2.x1()) + (b2 * line2.y1());
	*/

	double a1 = line1.y2() - line1.y1();
	double a2 = line2.y2() - line2.y1();
	double b1 = line1.x1() - line1.x2();
	double b2 = line2.x1() - line2.x2();
	double c1 = (a1 * line1.x1()) + (b1 * line1.y1());
	double c2 = (a2 * line2.x1()) + (b2 * line2.y1());

	double det = a1*b2 - a2*b1;
	if (det == 0)
		return false;
	else
	{
		double x = (b2*c1 - b1*c2) / det;
        double y = (a1*c2 - a2*c1) / det;

		//log_message(_T("L1 ") + test(line1));
		//log_message(_T("L2 ") + test(line2));
		//log_message(s_fmt(_T("%1.2f %1.2f"), x, y));

		//if (point_in_rect(line1.left(), line1.top(), line1.right(), line1.bottom(), lround(x), lround(y)) &&
		//	point_in_rect(line2.left(), line2.top(), line2.right(), line2.bottom(), lround(x), lround(y)))
		if (distance_to_line(line1.x1(), line1.y1(), line1.x2(), line1.y2(), x, y) <= 1.5 &&
			distance_to_line(line2.x1(), line2.y1(), line2.x2(), line2.y2(), x, y) <= 1.5)
		{
			if (ip) ip->set(x, y);
			return true;
		}
	}

	return false;
}

// cross: Returns the cross product of 2 vectors
// ------------------------------------------ >>
point3_t cross(point3_t p1, point3_t p2)
{
	point3_t cross_product;	

	cross_product.x = ((p1.y * p2.z) - (p1.z * p2.y));
	cross_product.y = ((p1.z * p2.x) - (p1.x * p2.z));
	cross_product.z = ((p1.x * p2.y) - (p1.y * p2.x));

	return cross_product;
}

// dot_product: Returns the dot product of 2 vectors
// ---------------------------------------------- >>
float dot_product(point3_t v1, point3_t v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

// angle_between_vectors: Returns the angle between 2 vectors
// ------------------------------------------------------- >>
float angle_between_vectors(float x1, float y1, float x2, float y2)
{
	float dot = (x1 * x2) + (y1 * y2);
	float len = sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2);
	float cosin = dot / len;

	return acos(cosin) * 180.0f / 3.1415926535897932384f;
}

float angle_between_vectors_2(point3_t v1, point3_t v2)
{
	float dot = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	float len = sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z)
				* sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
	float cosin = dot / len;

	return acos(cosin) * 180.0f / 3.1415926535897932384f;
}

// get_2d_angle_rad: Returns the angle between 2 2d vectors in radians
// ---------------------------------------------------------------- >>
float get_2d_angle_rad(point3_t p1, point3_t p2, point3_t p3)
{
	float angle_rad;
	float p[2];

	p[0] = (p1.x - p2.x) * (p3.x - p2.x) + (p1.y - p2.y) * (p3.y - p2.y);
	p[1] = (p1.x - p2.x) * (p3.y - p2.y) - (p1.y - p2.y) * (p3.x - p2.x);

	if (p[0] == 0.0f && p[1] == 0.0f)
		angle_rad = 0.0f;
	else
	{
		angle_rad = atan2(p[1], p[0]);

		if (angle_rad < 0.0f)
			angle_rad = angle_rad + 2.0f * 3.1415926535897932384f;
	}

	return angle_rad;
}

// get_2d_angle: Returns the angle between 2 2d vectors (0.0f-360.0f)
// --------------------------------------------------------------- >>
float get_2d_angle(point3_t p1, point3_t p2, point3_t p3)
{
	double angle_rad;
	double p[2];
	double value;

	p[0] = (p1.x - p2.x) * (p3.x - p2.x) + (p1.y - p2.y) * (p3.y - p2.y);
	p[1] = (p1.x - p2.x) * (p3.y - p2.y) - (p1.y - p2.y) * (p3.x - p2.x);

	if (p[0] == 0.0 && p[1] == 0.0)
		value = 0.0;
	else
	{
		angle_rad = atan2(p[1], p[0]);

		if (angle_rad < 0.0)
			angle_rad = angle_rad + 2.0 * 3.1415926535897932384;

		value = (angle_rad / 3.1415926535897932384) * 180.0;
	}

	return (float)value;
}

#ifdef WIN32
int lround(double val)
{
	int ret = (int)val;
	if ((val - (double)ret) >= 0.5) ret++;
	return ret;
}
#endif

// line_intersect: Determines if the camera is looking at a line
// (borrowed from Doom Builder, thanks CodeImp :D)
// ---------------------------------------------------------- >>
float line_intersect(point3_t r1, point3_t r2, float x1, float y1, float x2, float y2)
{
	// Calculate the intersection distance from the ray
	float u_ray = ((x2 - x1) * (r1.y - y1) - (y2 - y1) * (r1.x - x1)) /
				  ((y2 - y1) * (r2.x - r1.x) - (x2 - x1) * (r2.y - r1.y));
	
	// Calculate the intersection distance from the line
	float u_line = ((r2.x - r1.x) * (r1.y - y1) - (r2.y - r1.y) * (r1.x - x1)) /
				   ((y2 - y1) * (r2.x - r1.x) - (x2 - x1) * (r2.y - r1.y));
	
	// Return the distance on the ray if intersecting, or return -1
	if((u_ray >= 0)/* && (u_ray <= 1024) */&& (u_line >= 0) && (u_line <= 1)) return u_ray; else return -1;
}

// dist_ray_plane: Returns the distance between a ray and a plane
// ----------------------------------------------------------- >>
float dist_ray_plane(point3_t r_o, point3_t r_v, plane_t plane)
{
    float cos_a;
    float delta_d;

	point3_t p_normal(plane.a, plane.b, plane.c);

    cos_a = dot_product(r_v, p_normal.normalize());

    // parallel to the plane (alpha=90)
    if (cos_a == 0)
		return -1.0f;

    delta_d = plane.d - dot_product(r_o, p_normal.normalize());
    
    return (delta_d / cos_a);
}
