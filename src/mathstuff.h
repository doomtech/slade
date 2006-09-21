
#ifndef __MATHSTUFF_H__
#define __MATHSTUFF_H__

#include "structs.h"

int difference(int v1, int v2);
double distance(double x1, double y1, double x2, double y2);
point2_t midpoint(point2_t p1, point2_t p2);
double distance_to_line(int x1, int y1, int x2, int y2, double vx, double vy);
bool point_in_rect(int x1, int y1, int x2, int y2, int x, int y);
rect_t h_difference(rect_t r1, rect_t r2);
rect_t v_difference(rect_t r1, rect_t r2);
bool lines_intersect(rect_t line1, rect_t line2);
float determine_line_side(rect_t r, float x, float y);
float determine_line_side(float x1, float y1, float x2, float y2, float x, float y);
float get_2d_angle(point3_t p1, point3_t p2, point3_t p3);
float get_2d_angle_rad(point3_t p1, point3_t p2, point3_t p3);
float dot_product(point3_t v1, point3_t v2);
float line_intersect(point3_t r1, point3_t r2, float x1, float y1, float x2, float y2);
float dist_ray_plane(point3_t r_o, point3_t r_v, plane_t plane);
bool lines_intersect(rect_t line1, rect_t line2, fpoint2_t *ip = NULL);
bool rects_intersect(rect_t r1, rect_t r2);

#ifdef WIN32
int lround(double val);
#else
#include <math.h>
#endif

#endif //__MATHSTUFF_H__
