// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>

#include "device.h"
#include "line.h"

int main() {
	int setup_status = set_up_device();

	if (setup_status != 0) {
		return setup_status;
	}

	set_up_signal_handling();

	print_fb_info();

	xres = get_xres();
	yres = get_yres();
	int x, y;

	while (1) {
		for (x = 0; x < xres; x++) {
			for (y = 0; y < yres; y++) {
				draw_pixel(x, y, trace_ray(origin, direction, 1, INFINITY));
			}
		}
		usleep(10000);
	}

	return 0;
}
