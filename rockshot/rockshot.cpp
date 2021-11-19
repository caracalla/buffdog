#include <cctype>
#include <chrono>
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


// ugh
#ifdef _MSC_VER
const char* city_model_file = "rockshot/models/city.obj";
const char* city_texture_file = "rockshot/textures/city.ppm";
const char* crate_texture_file = "rockshot/textures/crate.bmp";
#else
const char* city_model_file = "models/city.obj";
const char* city_texture_file = "textures/city.ppm";
const char* crate_texture_file = "textures/crate.bmp";
#endif




int main(int argc, char** argv) {
	if (!device::setUp()) {
		return 1;
	}

	auto last_frame_time = std::chrono::steady_clock::now();

	// // add level geometry
	// Model city = parseOBJFile(city_model_file);
	// PPMTexture city_texture = PPMTexture::load(city_texture_file);
	// city.setTexture(&city_texture);
	// city.setTriangleNormals();

	// Level level;
	// level.model = &city;
	// level.scale = 12.0;
	// level.position = Vector::direction(-38, 0, -38);
	// level.rotation = Vector::direction(0, 0, 0);
	// level.player_start_position = Vector::point(0, 0, 0);
	// level.player_start_rotation = Vector::direction(0, kHalfPi + kQuarterPi, 0);
	// level.init();

	Player player;
	Model player_model = player.buildModel();
	player.model = &player_model;
	player.weapon.player_local_position = Vector::direction(0.25, 0, 0);
	player.weapon.bullet = buildTetrahedron();
	player.weapon.explosion = subdivide(subdivide(buildIcosahedron()));
	Model weapon_model = player.weapon.buildModel();
	player.weapon.model = &weapon_model;
	player.weapon.translucency = 2;

	spit("Player created successfully");

	int floor_size = 20;
	Vector floor_start = Vector::point(-floor_size, -5, -floor_size);
	Vector floor_end = Vector::point(floor_size, 0, floor_size);
	Model floor = buildHexahedron(floor_start, floor_end);
	Level level;
	level.model = &floor;
	level.scale = 1.0;
	level.position = Vector::point(0, 0, 0);
	level.rotation = Vector::direction(0, 0, 0);
	// player's position is where their eyes are
	level.player_start_position = Vector::point(0, player.eye_height, 0);
	level.player_start_rotation = Vector::direction(0, 0, 0);
	level.init();

	spit("Level created successfully");

	Scene scene;
	scene.init(std::move(level), std::move(player));

	spit("Scene created successfully");

	Renderer renderer = Renderer::create(scene.camera.viewport);

	spit("Renderer created successfully");

	// add spinning cube
	Model cube_model = buildCube();
	BMPTexture crate_texture = BMPTexture::load(crate_texture_file);
	cube_model.setTexture(&crate_texture);

	Entity cube_entity;
	cube_entity.model = &cube_model;
	cube_entity.position = Vector::point(15, 2, 0);

	scene.addEntityWithAction(std::move(cube_entity), [](Entity* self) {
		self->rotation.x += 0.005;
		self->rotation.y += 0.007;
		self->rotation.z += 0.009;

		key_input key = device::getNextKey();

		if (key == z_key) {
			self->translucency += 1;
			printf("translucency is now %d\n", self->translucency);
		}

		if (key == x_key) {
			self->translucency -= 1;
			printf("translucency is now %d\n", self->translucency);
		}
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
