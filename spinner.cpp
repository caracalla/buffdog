// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <cstdlib>
#include <cmath>

#include "device.h"
#include "line.h"


#define DELAY_US 10000


int main() {
	if (!device::setUp()) {
		return 1;
	}

	unsigned int xres = device::getXRes();
	unsigned int yres = device::getYRes();
	unsigned int x_mid = xres / 2;
	unsigned int y_mid = yres / 2;

	double theta = 0;
	int x;
	int y;

	while (device::running()) {
		usleep(DELAY_US);

		theta += 0.06;
		x = ROUND(sin(theta) * 50) + x_mid;
		y = ROUND(cos(theta) * 50) + y_mid;

		device::clearScreen(device::color(0.1, 0.1, 0.1));
		drawLine(x_mid, y_mid, x, y, device::color(0, 1, 0));
		device::updateScreen();
		device::processInput();
	}

	device::tearDown();

	return 0;
}
