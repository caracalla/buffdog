#include <cctype>
#include <chrono>
#define _USE_MATH_DEFINES // M_PI et al
#include <cmath>
#include <cstdlib>
// #include <ctime>

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


int fps = 0;
std::chrono::steady_clock::time_point last_print_time;

void logFPS() {
	fps++;
	auto now = std::chrono::steady_clock::now();
	auto time_since_last_print = now - last_print_time;

	if (time_since_last_print >= std::chrono::seconds(1)) {
		printf("FPS: %d\n", fps);

		last_print_time = now;
		fps = 0;
	}
}



int main(int argc, char** argv) {
	if (!device::setUp()) {
		return 1;
	}

	last_print_time = std::chrono::steady_clock::now();
	auto last_frame_time = std::chrono::steady_clock::now();

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
		auto now = std::chrono::steady_clock::now();
		auto frame_duration =
				std::chrono::duration_cast<std::chrono::microseconds>(now - last_frame_time);
		last_frame_time = now;

		scene.step(frame_duration);

		logFPS();
	}

	device::tearDown();

	return 0;
}
