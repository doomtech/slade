
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
};

point3_t cross(point3_t p1, point3_t p2);

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
		point3_t normal = cross(v1.normalize(), v2.normalize());
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
