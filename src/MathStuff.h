
#ifndef __MATHSTUFF_H__
#define __MATHSTUFF_H__

namespace MathStuff {
	double	clamp(double val, double min, double max);
	int		floor(double val);
	int		ceil(double val);
	int		round(double val);
	double	distance(double x1, double y1, double x2, double y2);
	double	lineSide(double x, double y, double x1, double y1, double x2, double y2);
	double	distanceToLine(double x, double y, double x1, double y1, double x2, double y2);
	double	distanceToLineFast(double x, double y, double x1, double y1, double x2, double y2);
	bool	linesIntersect(double l1x1, double l1y1, double l1x2, double l1y2,
							double l2x1, double l2y1, double l2x2, double l2y2,
							double& x, double& y);
}

#endif//__MATHSTUFF_H__
