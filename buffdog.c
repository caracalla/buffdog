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
#include "vec3.h"

void signal_handler(int signal_number) {
	if (signal_number == SIGINT) {
		printf("\nreceived SIGINT\n");
		close_fbfd();
		exit(0);
	}
}

void set_up_signal_handling() {
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		printf("\ncouldn't catch SIGINT\n");
	}
}

// this is initialized to all zeroes
vec3 origin;
unsigned int xres, yres;

struct sphere {
	vec3 center;
	double radius;
	vec3 color;
};

typedef struct sphere sphere;

sphere spheres[3];

void make_spheres() {
	spheres[0].center.x = 1;
	spheres[0].center.y = -1;
	spheres[0].center.z = 6;
	spheres[0].radius = 1.0;
	spheres[0].color.x = 1.0;

	spheres[1].radius = 0.1;
	spheres[2].radius = 0.1;
}

vec3 pixel_to_viewport(int x, int y, vec3 viewport) {
	if (xres <= 0 || yres <= 0) {
		printf("what the hell are you doing\n");
		exit(1);
	}

	vec3 result;
	result.x = (x * viewport.x / xres) - (viewport.x / 2);
	result.y = (y * viewport.y / yres) - (viewport.y / 2);
	result.z = viewport.z;

	return result;
}

int vec3_to_color(vec3 vec) {
	return color(vec.x, vec.y, vec.z);
}

// returns an int color, kind of stupid for now.  Maybe introduce a new color
// struct?  Like a vec3 but with validation that it's correct
int trace_ray(vec3 origin, vec3 direction, double t_min, double t_max) {
	double closest_t = t_max;
	int found_sphere = -1;
	int i;

	for (i = 0; i < 3; i++) {
		double t1, t2;
		vec3 oc = subtract_vec3(origin, spheres[i].center);

		double k1 = dot_product(direction, direction);
		double k2 = 2 * dot_product(oc, direction);
		double k3 = dot_product(oc, oc) - (spheres[i].radius * spheres[i].radius);

		double discriminant = (k2 * k2) - (4 * k1 * k3);

		if (discriminant >= 0) {
			t1 = (-k2 + sqrt(discriminant)) / (2 * k1);
			t2 = (-k2 - sqrt(discriminant)) / (2 * k1);

			if (t1 > t_min && t1 < closest_t) {
				closest_t = t1;
				found_sphere = i;
			}

			if (t2 > t_min && t2 < closest_t) {
				closest_t = t2;
				found_sphere = i;
			}
		}
	}

	if (found_sphere == -1) {
		// found nothing, return background color
		return 0xFFFFFF;
	} else {
		return vec3_to_color(spheres[found_sphere].color);
	}
}

int main() {
	int setup_status = set_up_device();

	if (setup_status != 0) {
		return setup_status;
	}

	set_up_signal_handling();

	print_fb_info();

	vec3 viewport;
	viewport.x = 4;
	viewport.y = 3;
	viewport.z = 2;

	make_spheres();

	xres = get_xres();
	yres = get_yres();
	int x, y;

	for (x = 0; x < xres; x++) {
		for (y = 0; y < yres; y++) {
			// draw_pixel(x, y, color((double)x / xres, (double)y / yres, 0));

			vec3 direction = pixel_to_viewport(x, y, viewport);
			draw_pixel(x, y, trace_ray(origin, direction, 1, INFINITY));
		}
	}


	while (1) {
		usleep(10000);
	}

	return 0;
}
