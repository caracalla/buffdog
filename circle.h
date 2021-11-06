#ifndef BUFFDOG_CIRCLE
#define BUFFDOG_CIRCLE

#include "device.h"
#include "point.h"
#include "vector.h"


struct Circle {
	Point center;
	double radius;
	Vector color;

	void draw() {
		for (int i = 0; i < 360; i++) {
			double rad = i * kPi / 180;

			int x = this->radius * cos(rad) + this->center.x;
			int y = this->radius * sin(rad) + this->center.y;

			if (device::insideViewport(x, y)) {
				device::setPixel(x, y, device::getColorValue(color.x, color.y, color.z));
			}
		}
	}
};

#endif
