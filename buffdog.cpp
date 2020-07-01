#include <cctype>
#define _USE_MATH_DEFINES // M_PI et al
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "bmp.h"
#include "device.h"
#include "entity.h"
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


#ifndef _MSC_VER
#include <sys/time.h>

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
#else
void logFPS() {}
#endif



int main(int argc, char** argv) {
	if (!device::setUp()) {
		return 1;
	}

#ifndef _MSC_VER
	// for FPS determination
	clock_gettime(CLOCK_REALTIME, &lastPrintTimeSpec);
#else
	spit("not logging FPS because we're on windows");
#endif

	// add level geometry
	Model city = parseOBJFile("models/city.obj");
	PPMTexture city_texture = PPMTexture::load("textures/city.ppm");
	city.addTexture(&city_texture);
	city.setTriangleNormals();

	Level level{
			city,
			12.0, // model scale
			Vector::direction(0, 0, 0), // model position
			Vector::direction(0, 0, 0), // model rotation
			Vector::point(38, 0, 38), // player position
			Vector::direction(0, M_PI_2, 0)}; // player rotation

	spit("Level added successfully");

	Scene scene = Scene::create(level);

	spit("Scene created successfully");

	Renderer renderer = Renderer::create(scene.camera.viewport);

	spit("Renderer created successfully");

	// add spinning cube
	Model cube = buildCube();
	BMPTexture crate_texture = BMPTexture::load("textures/crate.bmp");
	cube.addTexture(&crate_texture);

	Entity cube_ent{
			&cube,
			1.0,
			Vector::direction(48, 2, 40),
			Vector::direction(0, 0, 0)};

	scene.addEntityWithAction(cube_ent, [](Entity* self) {
		self->rotation.x += 0.005;
		self->rotation.y += 0.007;
		self->rotation.z += 0.009;
	});

	spit("Spinning crate created successfully");

	while (device::running()) {
		// draw the level and models
		renderer.drawScene(scene);

		// paint the screen
		device::updateScreen();

		// grab keyboard and mouse input
		device::processInput();

		// update player, models, and level by one time step
		scene.step();

		logFPS();
	}

	device::tearDown();

	return 0;
}
