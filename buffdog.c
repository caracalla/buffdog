// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "device.h"
#include "line.h"

int main () {
	int setup_status = set_up_device();
	float num = 0;
	int x, y;

	if (setup_status != 0) {
		return setup_status;
	}

	while (1) {
		usleep(5000);
		clear_screen();
		num += 0.01;
		x = ROUND(sin(num) * 50) + 320;
		y = ROUND(cos(num) * 50) + 240;
		// draw_line_DDA(320, 240, x, y, 0xFFFF);
		draw_line(320, 240, x, y, 0xFFFF);
	}
	
	return 0;
}
