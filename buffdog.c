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
#include "sphere.h"
#include "light.h"

// semi-graceful signal handling
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

// necessary constants
vec3 origin; // this is initialized to all zeroes
unsigned int xres, yres;
int BACKGROUND_COLOR = 0xFFFFFF;

sphere_list spheres;

void make_spheres() {
	size_t number_of_spheres = 3;
	spheres = make_sphere_list(number_of_spheres);

	append_sphere(
			&spheres,
			make_sphere(1.0, (vec3){2, 0, 5}, (vec3){1.0, 0, 1.0}));

	append_sphere(
			&spheres,
			make_sphere(1.2, (vec3){0, -1, 4}, (vec3){1.0, 0, 0.0}));

	append_sphere(
			&spheres,
			make_sphere(1.0, (vec3){-3, 2, 6}, (vec3){0.0, 0, 1.0}));
}

// set up lighting

light_list lights;

void make_lights() {
	size_t number_of_lights = 3;
	lights = make_light_list(number_of_lights);
	vec3 empty_vec = {0, 0, 0};

	append_light(&lights, make_light(ambient, 0.2, empty_vec, empty_vec));
	append_light(&lights, make_light(point, 0.6, (vec3){1, 2, 2}, empty_vec));
	append_light(&lights, make_light(directional, 0.2, empty_vec, (vec3){1, 4, 4}));
}

// do the work
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

double compute_lighting(vec3 destination, vec3 unit_normal) {
	double intensity = 0.0;
	int i;

	for (i = 0; i < 3; i++) {
		if (lights.data[i].type == ambient) {
			intensity += lights.data[i].intensity;
		} else {
			vec3 light_direction;
			if (lights.data[i].type == point) {
				light_direction = subtract_vec3(lights.data[i].position, destination);
			} else {
				light_direction = lights.data[i].direction;
			}

			double normal_dot_direction = dot_product(unit_normal, light_direction);

			if (normal_dot_direction > 0) {
				// isn't length(unit_normal) 1? is that still necessary? how does it
				// being a unit vector change the math in this function?
				intensity += lights.data[i].intensity * normal_dot_direction / (length(unit_normal) * length(light_direction));
			}
		}
	}

	return intensity;
}

// returns an int color, kind of stupid for now. Maybe introduce a new color
// struct? Like a vec3 but with validation that it's correct
int trace_ray(vec3 origin, vec3 direction, double t_min, double t_max) {
	double closest_t = t_max;
	int found_sphere = -1;
	int i;

	for (i = 0; i < 3; i++) {
		double t1, t2;
		vec3 oc = subtract_vec3(origin, spheres.data[i].center);

		double k1 = dot_product(direction, direction);
		double k2 = 2 * dot_product(oc, direction);
		double k3 = dot_product(oc, oc) - (spheres.data[i].radius * spheres.data[i].radius);

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
		return BACKGROUND_COLOR;
	} else {
		vec3 destination = scalar_multiply(direction, closest_t);
		vec3 normal = subtract_vec3(destination, spheres.data[found_sphere].center);
		double light_intensity = compute_lighting(destination, unit_vector(normal));
		return vec3_to_color(scalar_multiply(spheres.data[found_sphere].color, light_intensity));
	}
}

int main() {
	int setup_status = set_up_device();

	if (setup_status != 0) {
		return setup_status;
	}

	set_up_signal_handling();

	print_fb_info();

	vec3 viewport = {4, 3, 2};

	make_spheres();
	make_lights();

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
