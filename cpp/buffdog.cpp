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

#define MAXCHAR 1024

// assume a str is three decimal numbers with spaces and a newline at the end
Vector readVertex(char* str, size_t index, bool is_point) {
	Vector result;
	char num[MAXCHAR];

	if (is_point) {
		result = Vector::point(0, 0, 0);
	} else {
		result = Vector::direction(0, 0, 0);
	}

	for (int i = 0; i < 3; i++) {
		while (isspace(str[index])) {
			index += 1;
		}

		size_t num_index = 0;

		while (!isspace(str[index])) {
			num[num_index] = str[index];
			index += 1;
			num_index += 1;
		}

		num[num_index] = 0;
		result.at(i) = atof(num);
	}

	return result;
}

Model parseOBJFile(const char* filename) {
	Model result;
	char str[MAXCHAR];

	FILE *file = fopen(filename, "r");

	if (!file) {
		char message[1024];
		snprintf(
				message,
				sizeof(message),
				"could't read obj file %s\n",
				filename);

		terminateFatal(message);
	}

	while (fgets(str, MAXCHAR, file) != NULL) {
		size_t index = 0;

		if (str[index] == '\n' || str[index] == '#') {
			// do nothing
		} else if (str[index] == 'v') {
			index += 1;

			if (isspace(str[index])) {
				// vertex
				result.vertices.push_back(readVertex(str, index + 1, true));
			} else if (str[index] == 'n') {
				// normal
				result.normals.push_back(readVertex(str, index + 1, false));
			} else if (str[index] == 't') {
				// texture coordinate
			} else {
				// I don't know, skip
			}
		} else if (str[index] == 'f') {
			tri3d triangle;
			// face
			char num[MAXCHAR];

			index += 1;

			while (isspace(str[index])) index += 1;

			size_t num_index = 0;
			while (isdigit(str[index])) {
				num[num_index] = str[index];
				index += 1;
				num_index += 1;
			}

			num[num_index] = 0;
			triangle.v0 = atoi(num) - 1;

			if (str[index] == '/') {
				// replace to handle vt and vn
				while (!isspace(str[index])) index += 1;
			}

			while (isspace(str[index])) index += 1;

			num_index = 0;
			while (isdigit(str[index])) {
				num[num_index] = str[index];
				index += 1;
				num_index += 1;
			}

			num[num_index] = 0;
			triangle.v1 = atoi(num) - 1;

			if (str[index] == '/') {
				// replace to handle vt and vn
				while (!isspace(str[index])) index += 1;
			}

			while (isspace(str[index])) index += 1;

			num_index = 0;
			while (isdigit(str[index])) {
				num[num_index] = str[index];
				index += 1;
				num_index += 1;
			}

			num[num_index] = 0;
			triangle.v2 = atoi(num) - 1;

			for (int i = 0; i < 3; i++) {
				triangle.color.at(i) = (rand() % 2 ? 1.0 : 0.0);
			}

			result.triangles.push_back(triangle);
		}
	}

	for (int i = 0; i < result.vertices.size(); i++) {
		result.shades.push_back(1.0);
	}

	return result;
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
		item.scale = 2.0;
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
