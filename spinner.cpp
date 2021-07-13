// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <cstdlib>
#include <cmath>

#include "device.h"
#include "spinner.h"
#include "vector.h"


#define DELAY_US 10000


int main() {
	if (!device::setUp()) {
		return 1;
	}

	while (device::running()) {
		usleep(DELAY_US);

		device::clearScreen(device::getColorValue(0.1, 0.1, 0.1));
		spinner::draw(Vector{0.0, 1.0, 0.0});
		device::updateScreen();
		device::processInput();
	}

	device::tearDown();

	return 0;
}
