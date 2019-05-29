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

void signal_handler(int signal_number) {
	if (signal_number == SIGINT) {
		printf("received SIGINT\n");
		close_fbfd();
		exit(0);
	}
}

int main () {
	int setup_status = set_up_device();
	float num = 0;
	int x, y;

	if (setup_status != 0) {
		return setup_status;
	}

	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		printf("couldn't catch SIGINT\n");
	}

	print_fb_info();

	unsigned int xres = get_xres();
	unsigned int yres = get_yres();
	unsigned int x_mid = xres / 2;
	unsigned int y_mid = yres / 2;

	while (1) {
		usleep(5000);
		clear_screen();
		num += 0.01;
		x = ROUND(sin(num) * 50) + x_mid;
		y = ROUND(cos(num) * 50) + y_mid;
		// draw_line_DDA(x_mid, y_mid, x, y, 0xFFFF);
		draw_line(x_mid, y_mid, x, y, 0xFFFFFF);
	}
	
	return 0;
}
