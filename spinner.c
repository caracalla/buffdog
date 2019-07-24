// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>

#include "device.h"
#include "line.h"
#include "util.h"


int main() {
	int setup_status = set_up_device();
	float num = 0;
	int x, y;

	if (setup_status != 0) {
		return setup_status;
	}

	set_up_signal_handling();
	print_fb_info();

	unsigned int xres = get_xres();
	unsigned int yres = get_yres();
	unsigned int x_mid = xres / 2;
	unsigned int y_mid = yres / 2;

	while (1) {
		usleep(5000);
		clear_screen();
		num += 0.006;
		x = ROUND(sin(num) * 50) + x_mid;
		y = ROUND(cos(num) * 50) + y_mid;
		// draw_line_DDA(x_mid, y_mid, x, y, 0xFFFFFF);
		draw_line(x_mid, y_mid, x, y, color(255, 128, 128));
	}
	
	return 0;
}
