// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include <sys/time.h>

#include "bmp.h"
#include "device.h"
#include "matrix.h"
#include "model.h"
#include "obj.h"
#include "ppm.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"
#include "triangle.h"
#include "vector.h"


#define DELAY_US 10000
#define INCREMENT 0.005
#define MOUSE_SENSITIVITY_FACTOR 1000
// TODO: tie velocity to FPS
#define MAX_VELOCITY 0.0625


int fps = 0;
struct timespec lastPrintTimeSpec;

void logFPS() {
	fps++;

	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	if (now.tv_sec - lastPrintTimeSpec.tv_sec >= 1) {
		printf("FPS: %d\n", fps);
		lastPrintTimeSpec = now;
		fps = 0;
	}
}

int main(int argc, char** argv) {
	if (!device::setUp()) {
		return 1;
	}

	// for FPS determination
	clock_gettime(CLOCK_REALTIME, &lastPrintTimeSpec);

	Scene scene = Scene::create();
	scene.camera.translation.y = 1.5;
	Renderer renderer = Renderer::create(scene.camera.viewport);

	Model cube = buildCube(
			1.0, Vector::direction(10, 2, 2), Vector::direction(0, 0, 0));
	BMPTexture crate_texture = BMPTexture::load("crate.bmp");
	cube.addTexture(&crate_texture);
	scene.addModel(cube);
	//
	// // scene.addModel(buildCube(
	// // 		0.5, Vector::direction(3, 0, -7), Vector::direction(0, 0, 0)));
	//
	// Model tetra = buildTetrahedron(
	// 		1.0, Vector::direction(0, 3, -7), Vector::direction(0, 0, 0));
	// scene.addModel(tetra);

	if (argc == 2) {
		Model item = parseOBJFile(argv[1]);
		item.scale = 1.0;
		item.translation = Vector::direction(2, 0, -7);
		item.rotation = Vector::direction(0, 0, 0);
		item.setTriangleNormals();
		scene.addModel(item);
	}

	// PPMTexture city_texture = PPMTexture::load("city.ppm");
	//
	// Model city = parseOBJFile("city.obj");
	// city.addTexture(&city_texture);
	// city.scale = 12.0;
	// city.translation = Vector::direction(-38, 0, -38);
	// city.rotation = Vector::direction(0, 0, 0);
	// city.setTriangleNormals();
	// scene.addModel(city);

	double velocity = 0;

	scene.camera.rotation.y = M_PI_2;

	while (device::running()) {
		usleep(DELAY_US);

		scene.models[0].rotation.x += 0.005;
		scene.models[0].rotation.y += 0.007;
		scene.models[0].rotation.z += 0.009;

		renderer.drawScene(scene);

		device::updateScreen();
		device::processInput();

		// key_input next_key = device::get_next_key();
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
			double max_velocity = MAX_VELOCITY;

			if (key_states.sprint) {
				max_velocity *= 5;
			}

			if (velocity <= max_velocity) {
				velocity += INCREMENT;
			} else {
				velocity -= INCREMENT;
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

		logFPS();
	}

	device::tearDown();

	return 0;
}
