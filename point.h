#ifndef BUFFDOG_POINT
#define BUFFDOG_POINT

#define _USE_MATH_DEFINES // M_PI et al
#include <cmath>


struct Point {
	int x;
	int y;

	double distanceFrom(Point point) {
		int xdist = x - point.x;
    int ydist = y - point.y;

    return sqrt(xdist * xdist + ydist * ydist);
	}
};

#endif
