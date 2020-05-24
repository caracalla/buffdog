#include <cctype>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include <sys/time.h>

#include "bmp.h"
#include "device.h"
#include "level.h"
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
	Renderer renderer = Renderer::create(scene.camera.viewport);

	Model cube = buildCube(
			1.0, Vector::direction(48, 2, 40), Vector::direction(0, 0, 0));
	BMPTexture crate_texture = BMPTexture::load("textures/crate.bmp");
	cube.addTexture(&crate_texture);
	scene.addModel(cube);

	Model city = parseOBJFile("models/city.obj");
	PPMTexture city_texture = PPMTexture::load("textures/city.ppm");
	city.addTexture(&city_texture);
	city.scale = 12.0;
	city.translation = Vector::direction(0, 0, 0);
	city.rotation = Vector::direction(0, 0, 0);
	city.setTriangleNormals();

	Level level = {
		city,
		Vector::point(38, 0, 38),
		Vector::direction(0, M_PI_2, 0)
	};

	scene.setLevel(level);

	scene.camera.translation.y = 1.7; // player height

	double velocity = 0;

	Model tetra = buildTetrahedron(0.5, Vector::direction(0, 0, 0), Vector::direction(0, 0, 0));

	while (device::running()) {
		cube.rotation.x += 0.005;
		cube.rotation.y += 0.007;
		cube.rotation.z += 0.009;

		renderer.drawScene(scene);

		device::updateScreen();
		device::processInput();

		key_input next_key = device::get_next_key();

		if (next_key) {
			Vector view_normal = Matrix::makeRotationMatrix(scene.camera.rotation).multiplyVector(Vector::direction(0, 0, -1));

			switch(next_key) {
				case x_key:
					tetra.translation = scene.level.collisionPoint(scene.camera.translation, view_normal);

					tetra.rotation = scene.camera.rotation;
					tetra.rotation.x += -M_PI - M_PI_2; // align top to point at cameras
					scene.addModel(tetra);
					break;

				default:
					break;
			}
		}

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
