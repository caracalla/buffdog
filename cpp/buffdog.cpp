// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <stdlib.h>
#include <math.h>

#include "device.h"
#include "line.h"
#include "mat4.h"
#include "model.h"
#include "triangle.h"
#include "util.h"


#define DELAY_US 10000
#define INCREMENT 0.005
#define MOUSE_SENSITIVITY_FACTOR 1000
#define MAX_VELOCITY 0.125


// define viewport
struct camera_t {
	int width;
	int height;
	int distance;
	vec4 translation;
	vec4 rotation; // z is ignored
};

camera_t camera = {4, 3, 2, {0, 0, 0}, {0, 0, 0}};

// viewport culing
vec4 near_plane = {0, 0, 1, -2};
vec4 left_plane = {0.7071, 0, 0.7071, 0};
vec4 right_plane = {-0.7071, 0, 0.7071, 0};
vec4 high_plane = {0, -0.8, 0.6, 0};
vec4 low_plane = {0, 0.8, 0.6, 0};

bool cull(vec4 vertex) {
	double nearD = vertex.dotProduct(near_plane);
	double leftD = vertex.dotProduct(left_plane);
	double rightD = vertex.dotProduct(right_plane);
	double highD = vertex.dotProduct(high_plane);
	double lowD = vertex.dotProduct(low_plane);

	return nearD > 0 && leftD > 0 && rightD > 0 && highD > 0 && lowD > 0;
}

point viewportToCanvas(double x, double y) {
	point result;
	result.x = x * get_xres() / camera.width + get_xres() / 2;
	result.y = y * get_yres() / camera.height + get_yres() / 2;
	return result;
}

point projectVertex(vec4 vertex) {
	double x = vertex.x * camera.distance / vertex.z;
	double y = vertex.y * camera.distance / vertex.z;
	return viewportToCanvas(x, y);
}

void drawCube(cube item) {
	point projectedVertices[CUBE_V_COUNT];

	for (int i = 0; i < CUBE_V_COUNT; i++) {
		if (cull(item.vertices[i])) {
			projectedVertices[i] = projectVertex(item.vertices[i]);
		} else {
			// vertex not visible, insert dummy value
			projectedVertices[i] = (point){-1, -1};
		}
	}

	for (int i = 0; i < CUBE_T_COUNT; i++) {
		// don't draw triangles with hidden vertices
		if (
				projectedVertices[item.triangles[i].v0].x == -1 ||
				projectedVertices[item.triangles[i].v1].x == -1 ||
				projectedVertices[item.triangles[i].v2].x == -1) {
			continue;
		}

		triangle tri = {
			projectedVertices[item.triangles[i].v0],
			projectedVertices[item.triangles[i].v1],
			projectedVertices[item.triangles[i].v2],
			item.triangles[i].color
		};

		drawTriangle(tri);
	}
}

mat4 cameraMatrix;

cube applyTransform(cube item) {
	mat4 worldMatrix = mat4::makeWorldMatrix(item.scale, item.rotation, item.translation);
	mat4 finalMatrix = cameraMatrix.multiplyMat4(worldMatrix);

	for (int i = 0; i < CUBE_V_COUNT; i++) {
		item.vertices[i] = finalMatrix.multiplyVec4(item.vertices[i]);
	}

	return item;
}

int main() {
	if (!set_up_device()) {
		return 1;
	}

	double cube1Scale = 1;
	vec4 cube1translation = vec4::direction(-1.5, 0, 7);
	vec4 cube1rotation = vec4::direction(0, 0, 0);
	cube cube1 = buildCube(cube1Scale, cube1translation, cube1rotation);

	double cube2Scale = 0.5;
	vec4 cube2translation = vec4::direction(2, -0.5, 6);
	vec4 cube2rotation = vec4::direction(0, 0, 0);
	cube cube2 = buildCube(cube2Scale, cube2translation, cube2rotation);

	double velocity = 0;

	while (running()) {
		usleep(DELAY_US);

		cube1.rotation.x += 0.005;
		cube1.rotation.y += 0.007;
		cube1.rotation.z += 0.009;

		clear_screen(color(1, 1, 1));

		cameraMatrix = mat4::makeCameraMatrix(camera.rotation, camera.translation);

		drawCube(applyTransform(cube1));
		drawCube(applyTransform(cube2));

		update_screen();
		process_input();

		// key_input next_key = get_next_key();
		//
		// if (next_key) {
		// 	switch(next_key) {
		// 		case up:
		// 			camera.translation.z += INCREMENT;
		// 			break;
		//
		// 		case down:
		// 			camera.translation.z -= INCREMENT;
		// 			break;
		//
		// 		case left:
		// 			camera.translation.x -= INCREMENT;
		// 			break;
		//
		// 		case right:
		// 			camera.translation.x += INCREMENT;
		// 			break;
		//
		// 		default:
		// 			break;
		// 	}
		// }

		mouse_input mouse_motion = get_mouse_motion();

		camera.rotation.x += -(double)mouse_motion.y / MOUSE_SENSITIVITY_FACTOR;
		camera.rotation.y += -(double)mouse_motion.x / MOUSE_SENSITIVITY_FACTOR;

		key_states_t key_states = get_key_states();

		vec4 translation = {0, 0, 0, 0};

		if (key_states.up || key_states.down || key_states.left || key_states.right) {
			if (velocity < MAX_VELOCITY) {
				velocity += INCREMENT;
			}
		} else {
			velocity -= INCREMENT;

			if (velocity < 0) {
				velocity = 0;
			}
		}

		if (key_states.up) {
			translation.z += 1;
		}
		if (key_states.down) {
			translation.z -= 1;
		}
		if (key_states.left) {
			translation.x -= 1;
		}
		if (key_states.right) {
			translation.x += 1;
		}

		vec4 movement = translation.unit().scalar_multiply(velocity);
		mat4 rotation = mat4::makeRotationMatrix((vec4){0, camera.rotation.y, 0, 0});

		camera.translation.add(rotation.multiplyVec4(movement));
	}

	close_device();

	return 0;
}
