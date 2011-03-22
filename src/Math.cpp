
#include "Main.h"
#include "Math.h"

double Math::clamp(double val, double min, double max) {
	if (val < min) val = min;
	if (val > max) val = max;
	return val;
}
