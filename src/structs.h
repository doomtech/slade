
#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <math.h>
#include <GL/gl.h>

// rgba_t: A 32-bit colour definition
struct rgba_t
{
	BYTE r, g, b, a;
	char blend;

	// Constructors
	rgba_t() { r = 0; g = 0; b = 0; a = 0; blend = -1; }
	
	rgba_t(BYTE R, BYTE G, BYTE B, BYTE A, char BLEND = -1)
	{
		r = R;
		g = G;
		b = B;
		a = A;
		blend = BLEND;
	}

	// Functions
	void set(BYTE R, BYTE G, BYTE B, BYTE A, char BLEND = -1)
	{
		r = R;
		g = G;
		b = B;
		a = A;
		blend = BLEND;
	}

	void set(rgba_t colour)
	{
		r = colour.r;
		g = colour.g;
		b = colour.b;
		a = colour.a;
		blend = colour.blend;
	}

	float fr() { return (float)r / 255.0f; }
	float fg() { return (float)g / 255.0f; }
	float fb() { return (float)b / 255.0f; }
	float fa() { return (float)a / 255.0f; }

	void set_blend()
	{
		if (blend == 0)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else if (blend == 1)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
};

#define COL_BLEND_DONTCARE	-1
#define COL_BLEND_NORMAL	0
#define COL_BLEND_ADD		1

// Some basic colours
#define COL_WHITE	rgba_t(255, 255, 255, 255)
#define COL_BLACK	rgba_t(0, 0, 0, 255)
#define COL_RED		rgba_t(255, 0, 0, 255)
#define COL_GREEN	rgba_t(0, 255, 0, 255)
#define COL_BLUE	rgba_t(0, 0, 255, 255)
#define COL_YELLOW	rgba_t(255, 255, 0, 255)
#define COL_PURPLE	rgba_t(255, 0, 255, 255)
#define COL_CYAN	rgba_t(0, 255, 255, 255)

// point2_t: A 2d point
struct point2_t
{
	int x, y;

	// Constructors
	point2_t() { x = 0; y = 0; }

	point2_t(int X, int Y)
	{
		x = X;
		y = Y;
	}

	// Functions
	void set(int X, int Y)
	{
		x = X;
		y = Y;
	}

	void set(point2_t p)
	{
		x = p.x;
		y = p.y;
	}

	bool equals(point2_t p)
	{
		if (x == p.x && y == p.y)
			return true;
		else
			return false;
	}

	float magnitude()
	{
		return sqrt(float(x * x) + float(y * y));
	}

	point2_t perpendicular()
	{
		return point2_t(-y, x);
	}

	bool operator==(point2_t p)
	{
		return x == p.x && y == p.y;
	}
};

// fpoint2_t: An accurate 2d point ;)
struct fpoint2_t
{
	double x, y;

	// Constructors
	fpoint2_t() { x = 0; y = 0; }

	fpoint2_t(double X, double Y)
	{
		x = X;
		y = Y;
	}

	// Functions
	void set(double X, double Y)
	{
		x = X;
		y = Y;
	}

	void set(fpoint2_t p)
	{
		x = p.x;
		y = p.y;
	}

	bool equals(fpoint2_t p)
	{
		if (x == p.x && y == p.y)
			return true;
		else
			return false;
	}

	float magnitude()
	{
		return (float)sqrt((x * x) + (y * y));
	}

	fpoint2_t perpendicular()
	{
		return fpoint2_t(-y, x);
	}

	point2_t to_int()
	{
		return point2_t((int)x, (int)y);
	}

	bool operator==(fpoint2_t p)
	{
		return x == p.x && y == p.y;
	}
};

// rect_t: A rectangle
struct rect_t
{
	point2_t tl, br;

	// Constructors
	rect_t() { tl.set(0, 0); br.set(0, 0); }

	rect_t(point2_t TL, point2_t BR)
	{
		tl.set(TL);
		br.set(BR);
	}

	rect_t(int x1, int y1, int x2, int y2)
	{
		tl.set(x1, y1);
		br.set(x2, y2);
	}

	rect_t(int x, int y, int width, int height, int align)
	{
		if (align == 0) // Top-left
		{
			tl.set(x, y);
			br.set(x + width, y + height);
		}

		else if (align == 1) // Centered
		{
			tl.set(x - (width / 2), y - (height / 2));
			br.set(x + (width / 2), y + (height / 2));
		}
	}

	// Functions
	void set(point2_t TL, point2_t BR)
	{
		tl.set(TL);
		br.set(BR);
	}

	void set(int x1, int y1, int x2, int y2)
	{
		tl.set(x1, y1);
		br.set(x2, y2);
	}

	void set(rect_t rect)
	{
		tl.set(rect.tl);
		br.set(rect.br);
	}

	int x1() { return tl.x; }
	int y1() { return tl.y; }
	int x2() { return br.x; }
	int y2() { return br.y; }

	int left()		{ return min(tl.x, br.x); }
	int top()		{ return min(tl.y, br.y); }
	int right()		{ return max(br.x, tl.x); }
	int bottom()	{ return max(br.y, tl.y); }

	int width() { return br.x - tl.x; }
	int height() { return br.y - tl.y; }

	int awidth() { return max(br.x, tl.x) - min(tl.x, br.x); }
	int aheight() { return max(br.y, tl.y) - min(tl.y, br.y); }

	point2_t middle() { return point2_t(left() + (awidth() / 2), top() + (aheight() / 2)); }

	void resize(int x, int y)
	{
		if (x1() < x2())
		{
			tl.x -= x;
			br.x += x;
		}
		else
		{
			tl.x += x;
			br.x -= x;
		}

		if (y1() < y2())
		{
			tl.y -= y;
			br.y += y;
		}
		else
		{
			tl.y += y;
			br.y -= y;
		}
	}

	double length()
	{
		double dist_x = x2() - x1();
		double dist_y = y2() - y1();

		return sqrt(dist_x * dist_x + dist_y * dist_y);
	}
};

#define RECT_TOPLEFT	0
#define RECT_CENTER		1

struct point3_t
{
	float x, y, z;
	float u, v;

	point3_t()
	{
		x = y = z = 0.0f;
	}

	point3_t(float X, float Y, float Z, float U = 0.0f, float V = 0.0f)
	{
		x = X;
		y = Y;
		z = Z;
		u = U;
		v = V;
	}

	void set(float X, float Y, float Z, float U = 0.0f, float V = 0.0f)
	{
		x = X;
		y = Y;
		z = Z;
		u = U;
		v = V;
	}

	void set(point3_t p)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		u = p.u;
		v = p.v;
	}

	float magnitude()
	{
		return (float)sqrt((x * x) + (y * y) + (z * z));
	}

	point3_t normalize()
	{
		float mag = magnitude();

		return point3_t(x / mag, y / mag, z / mag);
	}

	float distance_to(point3_t point)
	{
		float dist_x = point.x - x;
		float dist_y = point.y - y;
		float dist_z = point.z - z;

		return (float)sqrt((dist_x * dist_x) + (dist_y * dist_y) + (dist_z * dist_z));
	}

	point3_t operator+(point3_t point)
	{
		return point3_t(point.x + x, point.y + y, point.z + z);
	}

	point3_t operator-(point3_t point)
	{
		return point3_t(x - point.x, y - point.y, z - point.z);
	}
	
	point3_t operator*(float num)
	{
		return point3_t(x * num, y * num, z * num);
	}

	point3_t operator/(float num)
	{
		return point3_t(x / num, y / num, z / num);
	}

	void gl_draw(bool tex)
	{
		glTexCoord2f(u, v);
		glVertex3f(x, y, z);
	}

	point3_t cross(point3_t p2)
	{
		point3_t cross_product;	

		cross_product.x = ((y * p2.z) - (z * p2.y));
		cross_product.y = ((z * p2.x) - (x * p2.z));
		cross_product.z = ((x * p2.y) - (y * p2.x));

		return cross_product;
	}
};

struct plane_t
{
	float a, b, c, d;

	point3_t normal()
	{ 
		point3_t norm(a, b, c);
		return norm.normalize();
	}

	void normalize()
	{
		point3_t vec(a, b, c);
		float mag = vec.magnitude();
		a = a / mag;
		b = b / mag;
		c = c / mag;
		d = d / mag;
	}

	void from_triangle(point3_t p1, point3_t p2, point3_t p3)
	{
		point3_t v1 = p3 - p1;
		point3_t v2 = p2 - p1;
		v1 = v1.normalize();
		point3_t normal = v1.cross(v2.normalize());
		normal.set(normal.normalize());

		a = normal.x;
		b = normal.y;
		c = normal.z;
		d = (normal.x * p1.x) + (normal.y * p1.y) + (normal.z * p1.z);
	}

	void set(float x, float y, float z, float dist)
	{
		a = x;
		b = y;
		c = z;
		d = dist;
	}

	void flip_normal()
	{
		a = -a;
		b = -b;
		c = -c;
	}
};

struct key_value_t
{
	string	name;
	int		value;

	key_value_t() : name(_T("")), value(0) { }
	key_value_t(string name, int value)
	{
		this->name = name;
		this->value = value;
	}
};

#endif
