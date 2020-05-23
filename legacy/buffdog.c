// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <stdlib.h>
#include <math.h>

#include "device.h"
#include "line.h"
#include "model.h"
#include "triangle.h"
#include "vec3.h"
#include "util.h"


#define DELAY_US 10000
#define INCREMENT 0.11


// define viewport
int Vw = 4;
int Vh = 3;
int d = 2;

vec3 viewport = {0, 0, 0};

point viewportToCanvas(double x, double y) {
	point result;
	result.x = x * get_xres() / Vw + get_xres() / 2;
	result.y = y * get_yres() / Vh + get_yres() / 2;
	return result;
}

point projectVertex(vec3 vertex) {
	double x = (vertex.x - viewport.x) * d / (vertex.z - viewport.z);
	double y = (vertex.y - viewport.y) * d / (vertex.z - viewport.z);
	return viewportToCanvas(x, y);
}

void drawCube(cube item) {
	point projectedVertices[CUBE_V_COUNT];

	for (int i = 0; i < CUBE_V_COUNT; i++) {
		projectedVertices[i] = projectVertex(item.vertices[i]);
	}

	for (int i = 0; i < CUBE_T_COUNT; i++) {
		triangle tri = {
			projectedVertices[item.triangles[i].v0],
			projectedVertices[item.triangles[i].v1],
			projectedVertices[item.triangles[i].v2],
			item.triangles[i].color
		};

		drawTriangle(tri);
	}
}

int main() {
	if (!set_up_device()) {
		return 1;
	}

	double cube1Scale = 1;
	vec3 cube1pos = {-1.5, 0, 7};
	vec3 cube1rot = {0, 0, 0};
	cube cube1 = buildCube(cube1Scale, cube1pos, cube1rot);

	double cube2Scale = 0.5;
	vec3 cube2pos = {2, -0.5, 6};
	vec3 cube2rot = {0, 0, 0};
	cube cube2 = buildCube(cube2Scale, cube2pos, cube2rot);

	while (running()) {
		usleep(DELAY_US);

		cube1.rotation.x += 0.005;
		cube1.rotation.y += 0.007;
		cube1.rotation.z += 0.009;

		clear_screen(color(1, 1, 1));

		drawCube(applyTransform(cube1));
		drawCube(applyTransform(cube2));

		update_screen();
		process_input();

		key_input next_key = get_next_key();

		if (next_key) {
			switch(next_key) {
				case up:
					viewport.z += INCREMENT;
					break;

				case down:
					viewport.z -= INCREMENT;
					break;

				case left:
					viewport.x -= INCREMENT;
					break;

				case right:
					viewport.x += INCREMENT;
					break;

				default:
					break;
			}
		}

		mouse_input mouse_motion = get_mouse_motion();


	}

	close_device();

	return 0;
}
