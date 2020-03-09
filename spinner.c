// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <stdlib.h>
#include <math.h>

#include "device.h"
#include "line.h"
#include "util.h"


#define DELAY_US 10000


int main() {
	if (!set_up_device()) {
		return 1;
	}

	unsigned int xres = get_xres();
	unsigned int yres = get_yres();
	unsigned int x_mid = xres / 2;
	unsigned int y_mid = yres / 2;

	float theta = 0;
	int x;
	int y;

	while (running()) {
		usleep(DELAY_US);

		theta += 0.06;
		x = ROUND(sin(theta) * 50) + x_mid;
		y = ROUND(cos(theta) * 50) + y_mid;

		process_events();
		clear_screen();
		draw_line(x_mid, y_mid, x, y, color(0, 1, 0));
		update_screen();
	}

	close_device();

	return 0;
}
