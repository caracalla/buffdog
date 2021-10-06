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

		int end_x = ROUND(sin(theta) * 50) + x_mid;
		int end_y = ROUND(cos(theta) * 50) + y_mid;

		Point center{x_mid, y_mid};
		Point end{end_x, end_y};

		// drawLine(x_mid, y_mid, x, y, device::getColorValue(color.x, color.y, color.z));
		drawLine(center, end, device::getColorValue(color.x, color.y, color.z));
	}
}
