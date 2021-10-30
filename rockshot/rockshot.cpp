#include <cctype>
#include <chrono>
#define _USE_MATH_DEFINES // M_PI et al
#include <cmath>
#include <cstdlib>

#include "../device.h"
#include "../matrix.h"
#include "../vector.h"

#include "bmp.h"
#include "entity.h"
#include "level.h"
#include "model.h"
#include "obj.h"
#include "ppm.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"
#include "triangle.h"


int main(int argc, char** argv) {
	if (!device::setUp()) {
		return 1;
	}

	auto last_frame_time = std::chrono::steady_clock::now();

	// // add level geometry
	// Model city = parseOBJFile("models/city.obj");
	// PPMTexture city_texture = PPMTexture::load("textures/city.ppm");
	// city.setTexture(&city_texture);
	// city.setTriangleNormals();

	// Level level;
	// level.model = &city;
	// level.scale = 12.0;
	// level.position = Vector::direction(-38, 0, -38);
	// level.rotation = Vector::direction(0, 0, 0);
	// level.player_start_position = Vector::point(0, 0, 0);
	// level.player_start_rotation = Vector::direction(0, M_PI_2 + M_PI_4, 0);
	// level.init();

	int floor_size = 20;
	Vector floor_start = Vector::point(-floor_size, -5, -floor_size);
	Vector floor_end = Vector::point(floor_size, 0, floor_size);
	Model floor = buildHexahedron(floor_start, floor_end);
	Level level;
	level.model = &floor;
	level.scale = 1.0;
	level.position = Vector::direction(0, 0, 0);
	level.rotation = Vector::direction(0, 0, 0);
	level.player_start_position = Vector::point(0, 0, 0);
	level.player_start_rotation = Vector::direction(0, 0, 0);
	level.init();

	spit("Level created successfully");

	Player player;
	Model player_model = player.buildModel();
	player.model = &player_model;
	player.bullet_model = buildTetrahedron();
	player.explosion_model = subdivide(subdivide(buildIcosahedron()));

	spit("Player created successfully");

	Scene scene;
	scene.init(std::move(level), std::move(player));

	spit("Scene created successfully");

	Renderer renderer = Renderer::create(scene.camera.viewport);

	spit("Renderer created successfully");

	// add spinning cube
	Model cube = buildCube();
	BMPTexture crate_texture = BMPTexture::load("rockshot/textures/crate.bmp");
	cube.setTexture(&crate_texture);

	Entity cube_ent{
			&cube,
			1.0,
			Vector::direction(15, 2, 0),
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
	}

	device::tearDown();

	return 0;
}
