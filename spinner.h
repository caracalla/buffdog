#include "device.h"
#include "line.h"
#include "vector.h"


namespace spinner {
	int x_mid = device::getXRes() / 2;
	int y_mid = device::getYRes() / 2;

	double theta = 0;

	// automatically moves the line forward (with global state lol)
	void draw(Vector color) {
		theta += 0.06;

		int x = ROUND(sin(theta) * 50) + x_mid;
		int y = ROUND(cos(theta) * 50) + y_mid;

		drawLine(x_mid, y_mid, x, y, device::color(color.x, color.y, color.z));
	}
}
