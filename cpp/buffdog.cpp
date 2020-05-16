// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "bmp.h"
#include "device.h"
#include "line.h"
#include "matrix.h"
#include "model.h"
#include "obj.h"
#include "renderer.h"
#include "scene.h"
#include "triangle.h"
#include "vector.h"


#define DELAY_US 10000
#define INCREMENT 0.005
#define MOUSE_SENSITIVITY_FACTOR 1000
// TODO: tie velocity to FPS
#define MAX_VELOCITY 0.0625


int fps = 0;
clock_t nextPrintTime;

void printFPS() {
	fps++;

	clock_t nowTime = clock();

	double timeDiff = ((double) (nowTime - nextPrintTime)) / CLOCKS_PER_SEC;

	if (timeDiff >= 1.0) {
		nextPrintTime = nowTime;
		printf("FPS: %d\n", fps);
		fps = 0;
	}
}

int main(int argc, char** argv) {
	// for generating random colors in parseOBJFile
	srand(time(NULL));

	if (!device::setUp()) {
		return 1;
	}

	// for FPS determination
	nextPrintTime = clock();

	Scene scene = Scene::create();
	Renderer renderer = Renderer::create(scene.camera.viewport);

	Model cube = buildCube(
			1.0, Vector::direction(-3, 0, -7), Vector::direction(0, 0, 0));
	cube.addTexture(BMPTexture::load("crate.bmp"));
	scene.addModel(cube);

	// scene.addModel(buildCube(
	// 		0.5, Vector::direction(3, 0, -7), Vector::direction(0, 0, 0)));

	scene.addModel(buildTetrahedron(
			1.0, Vector::direction(0, 3, -7), Vector::direction(0, 0, 0)));

	if (argc == 2) {
		Model item = parseOBJFile(argv[1]);
		item.scale = 1.0;
		item.translation = Vector::direction(2, 0, -7);
		item.rotation = Vector::direction(0, 0, 0);
		item.setTriangleNormals();
		scene.addModel(item);
	}

	double velocity = 0;

	while (device::running()) {
		usleep(DELAY_US);

		scene.models[0].rotation.x += 0.005;
		scene.models[0].rotation.y += 0.007;
		scene.models[0].rotation.z += 0.009;

		renderer.drawScene(scene);

		device::updateScreen();
		device::processInput();

		// key_input next_key = get_next_key();
		//
		// if (next_key) {
		// 	switch(next_key) {
		// 		case x_key:
		// 			cameraMatrix.log();
		// 			break;
		//
		// 		default:
		// 			break;
		// 	}
		// }

		mouse_input mouse_motion = device::getMouseMotion();

		scene.camera.rotation.x += (double)mouse_motion.y / MOUSE_SENSITIVITY_FACTOR;
		scene.camera.rotation.y += (double)mouse_motion.x / MOUSE_SENSITIVITY_FACTOR;

		key_states_t key_states = device::get_key_states();

		Vector translation = {0, 0, 0, 0};

		if (key_states.up || key_states.down || key_states.left || key_states.right || key_states.yup || key_states.ydown) {
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
			translation.z -= 1;
		}
		if (key_states.down) {
			translation.z += 1;
		}
		if (key_states.left) {
			translation.x -= 1;
		}
		if (key_states.right) {
			translation.x += 1;
		}
		if (key_states.yup) {
			translation.y += 1;
		}
		if (key_states.ydown) {
			translation.y -= 1;
		}

		Vector movement = translation.unit().scalarMultiply(velocity);

		// the direction of motion is determined only by the rotation about the y axis
		Matrix rotationAboutY = Matrix::makeRotationMatrix((Vector){0, scene.camera.rotation.y, 0, 0});
		scene.camera.translation = scene.camera.translation.add(rotationAboutY.multiplyVector(movement));

		printFPS();
	}

	device::tearDown();

	return 0;
}
